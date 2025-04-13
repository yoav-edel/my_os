//
// Created by Yoav on 3/16/2025.
//

#ifndef MYKERNEL_PID_H
#define MYKERNEL_PID_H
#include "../std/stdint.h"
typedef uint32_t pid_t;

/**
 * Allocates and returns a new PID.
 * Returns 0 (or some special value) if no PID is available.
 */
pid_t pid_alloc(void);

/**
 * Frees a PID so it can be reused.
 */
void pid_free(pid_t pid);

#endif //MYKERNEL_PID_H
