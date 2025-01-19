//
// Created by Yoav on 11/26/2024.
//

#ifndef MYKERNELPROJECT_INTERUPTS_HANDLER_H
#define MYKERNELPROJECT_INTERUPTS_HANDLER_H

#include "../std/stdint.h"
#define KEYBOARD_ISR 33
typedef struct {
    // pushad order (lowest address first)
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;      // value when pushad was executed
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    // manually-pushed in ASM
    uint32_t int_no;
    uint32_t err_code; // 0 if "no err code" exception, real if "with err code"

    // automatically pushed by the CPU on interrupt
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;

    // If interrupt happened from ring 3 to ring 0, CPU also pushes:
    uint32_t useresp;
    uint32_t ss;
} registers_t;


#define CPU_EXCEPTIONS 32
void isr_handler(registers_t *regs);

#endif //MYKERNELPROJECT_INTERUPTS_HANDLER_H
