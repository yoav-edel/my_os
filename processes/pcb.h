//
// Created by Yoav on 3/15/2025.
//
#include "../memory/vmm.h"
#include "process_state.h"

#ifndef MYKERNEL_PCB_H
#define MYKERNEL_PCB_H

#define DEFAULT_EFLAGS 0x202 // the 1-bit is always 1, and the 9th bit enables interrupts

//todo add support of more regiristers
typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t eflags;
} context_t;

context_t *context_create(uint32_t eip, uint32_t esp);
void context_destroy(context_t *context);

typedef struct pcb {
    context_t *context;
    vm_context_t *vm_context;
    process_state_t state;
} pcb_t;

pcb_t *pcb_create(uint32_t eip, uint32_t esp, vm_context_t *vm_context);
void pcb_destroy(pcb_t *pcb);


#endif //MYKERNEL_PCB_H
