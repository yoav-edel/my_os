//
// Created by Yoav on 3/16/2025.
//
#include "pcb.h"
#include "priority.h"

#ifndef MYKERNEL_PROCESS_H
#define MYKERNEL_PROCESS_H

typedef struct {
    pcb_t *pcb;
    pid_t pid;
    char[32] name;
    priority_t priority;
} process_t;

#endif //MYKERNEL_PROCESS_H
