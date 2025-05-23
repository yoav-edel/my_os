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


/**
 * @brief Tests the kernel memory allocator by allocating and freeing memory.
 *
 * Allocates 16 bytes of memory using the kernel memory allocator and then frees it to verify basic allocation and deallocation functionality.
 */
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

/**
 * @brief Entry point for the kernel, performing system initialization.
 *
 * Initializes core kernel subsystems including descriptor tables, interrupt controllers, disk driver, memory managers, and enables CPU interrupts. Clears the screen, prints a startup message, runs memory management tests, and enters an idle loop.
 */
void kernel_main() {
    init_gdt();
    init_idt();
    remap_pic();
    init_disk_driver();
    pmm_init();
    vmm_init();
//    processes_init();
    asm volatile("sti"); // enable interrupts

    clear_screen();
    printf("Kernel loaded successfully. its yoav kernel\n");
    test_pmm();
    test_kmalloc();
    while (1) {
        asm volatile("hlt");
    }
}
