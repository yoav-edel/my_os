//
// Created by Yoav on 3/16/2025.
//
#include "pcb.h"
#include "priority.h"

#ifndef MYKERNEL_PROCESS_H
#define MYKERNEL_PROCESS_H

typedef uint32_t pid_t;


typedef struct {
    pcb_t *pcb;
    pid_t pid;
    char name[32];
    priority_t priority;
} process_t;

void processes_init();
void process_destroy(process_t *process);
process_t *process_create(void (*entry_point)(), char *name, priority_t priority, process_t *parent);
#endif //MYKERNEL_PROCESS_H
