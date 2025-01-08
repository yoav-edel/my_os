//
// Created by Yoav on 11/26/2024.
//

#include "../std/assert.h"
#include "interupts_handler.h"
#include "../drivers/screen.h"
#include "../std/stdlib.h"

const char *exception_messages[] = {
        "Division By Zero",
        "Debug",
        "Non Maskable Interrupt",
        "Breakpoint",
        "Into Detected Overflow",
        "Out of Bounds",
        "Invalid Opcode",
        "No Coprocessor",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Bad TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection Fault",
        "Page Fault",
        "Unknown Interrupt",
        "Coprocessor Fault",
        "Alignment Check",
        "Machine Check",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"
};



//currently supports only CPU exceptions
void isr_handler(registers_t *regs) {
    assert(regs->int_no < CPU_EXCEPTIONS);

    put_char('\n'); // New line for better visibility
    put_string("Exception: ");
    put_string(exception_messages[regs->int_no]);
    put_char('\n');

    // Print the interrupt number for debugging
    put_string("Interrupt Number: ");
    char buffer[16];
    int_to_string(regs->int_no, buffer);
    put_string(buffer);
    put_char('\n');

    // Halt the system (optional)
    put_string("System Halted.\n");
    while (1) {
        asm volatile("hlt");
    }
}