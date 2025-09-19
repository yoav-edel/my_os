

#include "drivers/screen.h"
#include "shell.h"
#include "interupts/idt.h"
#include "interupts/pic.h"
#include "gdt.h"
#include "drivers/disk.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "memory/kmalloc.h"
#include "std/string.h"
#include "std/stdio.h"
#include "processes/process.h"


void test_kmalloc() {
    init_kmalloc();
    char *str = kmalloc(16);
    kfree(str);
}



void test_pmm() {
    printf("Testing PMM\n");
    physical_addr addr = pmm_alloc_frame();
    printf("Allocated frame at: %p\n", addr);
    pmm_free_frame(addr);
    printf("Freed frame at: %p\n", addr);
}

void kernel_main() {
    init_gdt();
    init_idt();
    remap_pic();
    init_disk_driver();
    pmm_init();
    vmm_init();
    init_kmalloc();
    //    processes_init();
    asm volatile("sti"); // enable interrupts

    clear_screen();
    printf("Kernel loaded successfully. its yoav kernel\n");
#ifdef RUN_TESTS
#include "tests/disk_tests.h"
    printf("testing\n");
    // test_pmm();
    // test_kmalloc();
    run_disk_tests();
#else
    shell();
#endif
    while (1) {
        asm volatile("hlt");
    }
}
