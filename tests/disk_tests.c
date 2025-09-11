// tests/disk_tests.c
#include "../drivers/disk.h"
#include "../std/stdio.h"
#include "../std/string.h"
#include "../std/stdint.h"
#include "../memory/kmalloc.h"
#include "../memory/utills.h"

// ---------- Minimal COM1 serial (mirrors logs to CLI via -serial stdio) ----------
static inline void outb(uint16_t port, uint8_t val) { __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port)); }

static inline uint8_t inb(uint16_t port) {
    uint8_t r;
    __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

#define COM1 0x3F8

static void serial_init(void) {
    outb(COM1 + 1, 0x00); // disable interrupts
    outb(COM1 + 3, 0x80); // DLAB on
    outb(COM1 + 0, 0x03); // 38400 baud divisor (lo)
    outb(COM1 + 1, 0x00); // (hi)
    outb(COM1 + 3, 0x03); // 8N1
    outb(COM1 + 2, 0xC7); // FIFO enable/clear, 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

static int serial_tx_empty(void) { return (inb(COM1 + 5) & 0x20) != 0; }

static void serial_putc(char c) {
    while (!serial_tx_empty()) {
    }
    outb(COM1, (uint8_t) c);
}

static void serial_puts(const char *s) {
    while (*s) {
        if (*s == '\n') serial_putc('\r');
        serial_putc(*s++);
    }
}

// QEMU exits with (code<<1)|1. We'll use 0x10 for PASS => exit 33; 0x11 for FAIL => exit 35.
static inline void qemu_exit_code(uint8_t code) {
    __asm__ volatile ("outb %0, %1" : : "a"(code), "Nd"(0xF4));
    for (;;) { __asm__ volatile("hlt"); }
}

static inline void qemu_exit_pass(void) { qemu_exit_code(0x10); }
static inline void qemu_exit_fail(void) { qemu_exit_code(0x11); }

// ---------- Tiny test framework ----------
static int g_failures = 0;
static int g_tests_run = 0;

#define TEST(name) static void name(void)
#define RUN(testfn) do { \
    const char* tn = #testfn; \
    printf("[ RUN ] %s\n", tn); serial_puts("[ RUN ] "); serial_puts(tn); serial_puts("\n"); \
    g_tests_run++; testfn(); \
} while(0)

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("[FAIL] %s\n", msg); serial_puts("[FAIL] "); \
        serial_puts(msg); serial_puts("\n"); g_failures++; \
    } \
    else { \
        printf("[PASS] %s\n", msg); serial_puts("[PASS] "); \
        serial_puts(msg); serial_puts("\n"); \
    } \
} while (0)

#define CHECK_EQ(a,b,msg)   CHECK((a)==(b), msg)
#define CHECK_NE(a,b,msg)   CHECK((a)!=(b), msg)
#define CHECK_MEMEQ(a,b,n,msg) do { \
    if (memcmp((a),(b),(n))!=0) { \
        printf("[FAIL] %s\n", msg); serial_puts("[FAIL] "); serial_puts(msg); serial_puts("\n"); g_failures++; } \
    else { \
        printf("[PASS] %s\n", msg); serial_puts("[PASS] "); serial_puts(msg); serial_puts("\n"); } \
    } while(0)

static inline void fill_pattern(uint8_t *buf, size_t n, uint32_t seed) {
    uint32_t x = seed;
    for (size_t i = 0; i < n; i++) {
        x = x * 1664525u + 1013904223u;
        buf[i] = (uint8_t) (x >> 24);
    }
}

#define LBA_BASE  4096U   // keep clear of boot sectors

// ---------- Tests (kept small – safe for stack bounce buffers) ----------

TEST(test_disk_basic_info) {
    switch_disk(0);
    size_t sec = disk_get_current_disk_logical_sector_size();
    CHECK(sec == 512 || sec == 4096, "logical sector size is 512 or 4096");
}

TEST(test_ata_read_write_small_counts) {
    const size_t sec = disk_get_current_disk_logical_sector_size();
    const uint32_t lbs[] = {LBA_BASE + 10, LBA_BASE + 100, LBA_BASE + 200};
    const uint16_t counts[] = {1, 2, 4};
    for (int i = 0; i < 3; i++) {
        const uint32_t lba = lbs[i];
        const uint16_t cnt = counts[i];
        const size_t bytes = (size_t) cnt * sec;
        uint8_t *w = (uint8_t *) kmalloc(bytes), *r = (uint8_t *) kmalloc(bytes);
        fill_pattern(w, bytes, 0xABCD0000u + i);
        CHECK(ata_write_sectors(0, lba, cnt, w), "ata_write_sectors (small)");
        CHECK(ata_read_sectors(0, lba, cnt, r), "ata_read_sectors  (small)");
        CHECK_MEMEQ(w, r, bytes, "ata small round-trip");
        kfree(w);
        kfree(r);
    }
}

TEST(test_disk_zero_length_rw) {
    uint8_t tmp = 0x5A;
    CHECK_EQ(disk_write(LBA_BASE+700, &tmp, 0), 0, "disk_write len=0 returns 0");
    CHECK_EQ(disk_read (LBA_BASE+700, &tmp, 0), 0, "disk_read  len=0 returns 0");
}

TEST(test_disk_wrapper_exact_small) {
    const size_t sec = disk_get_current_disk_logical_sector_size();
    const uint32_t lba = LBA_BASE + 800;
    const size_t len = 4 * sec;
    uint8_t *w = (uint8_t *) kmalloc(len), *r = (uint8_t *) kmalloc(len);
    fill_pattern(w, len, 0x1111);
    CHECK_EQ(disk_write(lba, w, len), len, "disk_write exact (4 sectors)");
    CHECK_EQ(disk_read (lba, r, len), len, "disk_read  exact (4 sectors)");
    CHECK_MEMEQ(w, r, len, "exact multiple round-trip (small)");
    kfree(w);
    kfree(r);
}

TEST(test_disk_wrapper_unaligned_sizes) {
    const size_t sec = disk_get_current_disk_logical_sector_size();
    const uint32_t bases[] = {LBA_BASE + 1200, LBA_BASE + 1300, LBA_BASE + 1400, LBA_BASE + 1500};
    const size_t lens[] = {1, sec - 1, sec + 1, 3 * sec + 123};
    for (int i = 0; i < 4; i++) {
        const uint32_t lba = bases[i];
        const size_t len = lens[i];

        // Seed a small area to validate RMW correctness
        const size_t pre = 4 * sec;
        uint8_t *garb = (uint8_t *) kmalloc(pre);
        memset(garb, 0xA5, pre);
        (void) disk_write(lba, garb, pre);
        kfree(garb);

        uint8_t *w = (uint8_t *) kmalloc(len), *r = (uint8_t *) kmalloc(len);
        fill_pattern(w, len, 0x3333 + i);
        memset(r, 0, len);
        CHECK_EQ(disk_write(lba, w, len), len, "disk_write unaligned (small)");
        CHECK_EQ(disk_read (lba, r, len), len, "disk_read  unaligned (small)");
        CHECK_MEMEQ(w, r, len, "unaligned round-trip");
        kfree(w);
        kfree(r);
    }
}

TEST(test_ata_oob_guard) {
    uint8_t secbuf[512] = {0};
    CHECK(!ata_read_sectors(0, 0x0FFFFFF0u, 0x10, secbuf), "ata_read_sectors rejects OOB");
    CHECK(!ata_write_sectors(0, 0x0FFFFFF0u, 0x10, secbuf), "ata_write_sectors rejects OOB");
}

TEST(test_switch_disk_invalid) {
    size_t before = disk_get_current_disk_logical_sector_size();
    switch_disk(3); // likely invalid with single drive
    size_t after = disk_get_current_disk_logical_sector_size();
    CHECK_EQ(before, after, "switch_disk(invalid) keeps current disk");
}

TEST(test_slot_allocator_small) {
    uint32_t slots[8];
    int ok = 1;
    for (int i = 0; i < 8; i++) {
        slots[i] = disk_alloc_slot();
        if (slots[i] == DISK_NO_SLOT_AVAILABLE) {
            ok = 0;
            break;
        }
    }
    CHECK(ok, "disk_alloc_slot x8 ok");
    for (int i = 0; i < 8; i++) disk_free_slot(slots[i]);

    ok = 1;
    for (int i = 0; i < 8; i++) {
        uint32_t s = disk_alloc_slot();
        if (s == DISK_NO_SLOT_AVAILABLE) {
            ok = 0;
            break;
        }
        disk_free_slot(s);
    }
    CHECK(ok, "disk_free_slot -> slots reusable");
}

TEST(test_interleaved_writes_reads_small) {
    const size_t sec = disk_get_current_disk_logical_sector_size();
    const uint32_t lba = LBA_BASE + 3000;
    const size_t len1 = 2*sec + 17;
    uint8_t *w1 = (uint8_t *) kmalloc(len1);
    fill_pattern(w1, len1, 0x7777);
    CHECK_EQ(disk_write(lba, w1, len1), len1, "first partial write (small)");
    kfree(w1);

    const size_t len2 = 3*sec + 9;
    uint8_t *w2 = (uint8_t *) kmalloc(len2);
    fill_pattern(w2, len2, 0x8888);
    CHECK_EQ(disk_write(lba + 1, w2, len2), len2, "second partial write overlapped (small)");

    const size_t total_len = (1 * sec) + len2;
    uint8_t *all = (uint8_t *) kmalloc(total_len);
    CHECK_EQ(disk_read(lba, all, total_len), total_len, "readback overlapped region (small)");

    uint8_t *expect = (uint8_t *) kmalloc(total_len);
    memset(expect, 0, total_len); {
        uint8_t *tmp = (uint8_t *) kmalloc(2 * sec + 17);
        fill_pattern(tmp, 2 * sec + 17, 0x7777);
        memcpy(expect, tmp, (total_len < (2 * sec + 17)) ? total_len : (2 * sec + 17));
        kfree(tmp);
    }
    memcpy(expect + 1 * sec, w2, len2);
    CHECK_MEMEQ(expect, all, total_len, "interleaved write expected image (small)");
    kfree(expect);
    kfree(w2);
    kfree(all);
}

// ---------- Main ----------
void run_disk_tests(void) {
    serial_init();
    serial_puts("\n=== DISK DRIVER TESTS: START ===\n");
    printf      ("\n=== DISK DRIVER TESTS: START ===\n");

    switch_disk(0);

    RUN(test_disk_basic_info);
    RUN(test_ata_read_write_small_counts);
    RUN(test_disk_zero_length_rw);
    RUN(test_disk_wrapper_exact_small);
    RUN(test_disk_wrapper_unaligned_sizes);
    RUN(test_ata_oob_guard);
    RUN(test_switch_disk_invalid);
    RUN(test_slot_allocator_small);
    RUN(test_interleaved_writes_reads_small);

    const int failed = g_failures;
    printf("\n=== DISK DRIVER TESTS: %s (%d failed of %d) ===\n",
           failed ? "FAILED" : "PASSED", g_failures, g_tests_run);
    serial_puts("\n=== DISK DRIVER TESTS: ");
    serial_puts(failed ? "FAILED" : "PASSED");
    serial_puts(" ===\n");

    if (failed) qemu_exit_fail();
    else        qemu_exit_pass();
}
