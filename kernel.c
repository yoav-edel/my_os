////#include "gdt.h"
//#include "drivers/screen.h"
////#include "interupts/idt.h"
////#include "interupts/pic.h"
//
//void kernel_main() {
//    clear_screen();
//    put_string("Kernel loaded successfully.\n");
//
////    // Initialize subsystems
////    init_gdt();
////    put_string("GDT initialized.\n");
////
////    init_idt();
////    put_string("IDT initialized.\n");
////
////    remap_pic();
////    put_string("PIC remapped.\n");
////    // Additional kernel logic...
//    while (1) {
//        // Halt to prevent CPU from running invalid instructions
//        asm volatile("hlt");
//    }
//}
//

#include "drivers/screen.h"
#include "shell.h"
#include "interupts/idt.h"
#include "interupts/pic.h"
#include "gdt.h"
void kernel_main() {
    init_idt();
    remap_pic();
    // test the interrupts
    asm volatile("sti"); // enable interrupts
    put_string("Kernel loaded successfully. its yoav kernel\n");
    shell();
    while (1) {
        asm volatile("hlt");
    }
}

