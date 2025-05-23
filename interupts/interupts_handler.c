//
// Created by Yoav on 11/26/2024.
//

#include "../std/assert.h"
#include "interupts_handler.h"
#include "../drivers/screen.h"
#include "../std/stdlib.h"
#include "../drivers/keyboard.h"
#include "../memory/vmm.h"
#include "../drivers/pit.h"

#define PAGE_FAULT_ISR 14
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


void cpu_handler(registers_t *regs)
{
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

/**
 * @brief Dispatches hardware and CPU interrupts to their appropriate handlers.
 *
 * Determines the type of interrupt based on the interrupt number in the provided register state and calls the corresponding handler for keyboard, page fault, PIT timer, or CPU exceptions. Prints a message for unknown interrupts.
 *
 * @param regs Pointer to the register state at the time of the interrupt.
 */
void isr_handler(registers_t *regs) {
    // todo maybe add sainty checks
    if (regs->int_no == KEYBOARD_ISR)
    {
        keyboard_handler();
    } else if (regs->int_no == PAGE_FAULT_ISR) {
        page_fault_handler(regs->err_code);
    }
    else if(regs->int_no == PIT_ISR)
    {
        pit_handler();
    }
    else if (regs->int_no < CPU_EXCEPTIONS)
        cpu_handler(regs);
    else
    {
        put_string("Unknown interrupt\n");
    }
}