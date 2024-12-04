//
// Created by Yoav on 11/26/2024.
//

#ifndef MYKERNELPROJECT_INTERUPTS_HANDLER_H
#define MYKERNELPROJECT_INTERUPTS_HANDLER_H

#include <stdint.h>

typedef struct {
    uint32_t ds;                                      // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  // Pushed by pusha
    uint32_t int_no, err_code;                        // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss;            // Pushed by the processor automatically
} registers_t;

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


#define CPU_EXCEPTIONS 32
void isr_handler(registers_t *regs);

#endif //MYKERNELPROJECT_INTERUPTS_HANDLER_H
