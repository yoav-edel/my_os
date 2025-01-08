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
void kernel_main() {
    clear_screen();
    put_string("Kernel loaded successfully. its yoav kernel\n");
    shell();
    while (1) {
        asm volatile("hlt");
    }
}

