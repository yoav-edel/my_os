//
// Created by Yoav on 3/15/2025.
//
#include "../memory/vmm.h"
#include "process_state.h"

#ifndef MYKERNEL_PCB_H
#define MYKERNEL_PCB_H


//todo add support of more regiristers
struct {
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

typedef uint32_t pid_t;

typedef struct pcb {
    context_t context;
    page_directory_t *page_dir;
    process_state_t state;
} pcb_t;

pcb_t *create_pcb(context_t
context,
page_directory_t *page_dir
);

void destroy_pcb(pcb_t *pcb);


#endif //MYKERNEL_PCB_H
