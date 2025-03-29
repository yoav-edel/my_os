

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
    // test the interrupts
    asm volatile("sti"); // enable interrupts
    clear_screen();
    printf("Kernel loaded successfully. its yoav kernel\n");
    init_disk_driver();
    printf("Disk initialized.\n");
    pmm_init();
    vmm_init();

    printf("VMM initialized.\n");
    test_pmm();
    test_kmalloc();
    while (1) {
        asm volatile("hlt");
    }
}
