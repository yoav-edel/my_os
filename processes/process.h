//
// Created by Yoav on 3/16/2025.
//

#ifndef MYKERNEL_PROCESS_H
#define MYKERNEL_PROCESS_H

#include "../std/stdint.h"
#include "pid.h"
#include "pcb.h"
#include "priority.h"



typedef struct {
    pcb_t *pcb;
    pid_t pid;
    char name[32];
    priority_t priority;
} process_t;

extern void process_switch(process_t *new_process);
void process_print(process_t *process);
void processes_init();
void process_destroy(process_t *process);
process_t *process_create(void (*entry_point)(), char *name, priority_t priority, process_t *parent);
#endif //MYKERNEL_PROCESS_H
