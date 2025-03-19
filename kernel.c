

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


void test_kmalloc() {
    init_kmalloc();
    char *str = kmalloc(16);
    kfree(str);
}


void test_pmm() {
    put_string("Testing PMM\n");
    physical_addr addr = pmm_alloc_frame();
    put_string("Allocated frame at: ");
    put_int(addr);
    put_string("\n");
    pmm_free_frame(addr);
    put_string("Freed frame at: ");
    put_int(addr);
    put_string("\n");
}

void kernel_main() {
    init_gdt();
    init_idt();
    remap_pic();
    // test the interrupts
    asm volatile("sti"); // enable interrupts
    clear_screen();
    put_string("Kernel loaded successfully. its yoav kernel\n");
    init_disk_driver();
    put_string("Disk initialized.\n");
    pmm_init();
    vmm_init();
    put_string("VMM initialized.\n");
    test_kmalloc();
    shell();
    while (1) {
        asm volatile("hlt");
    }
}

