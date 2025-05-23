//
// Created by Yoav on 3/16/2025.
//

#include "pid.h"

static pid_t next_pid = 1;

/**
 * @brief Allocates and returns a new unique process identifier (PID).
 *
 * Each call returns a monotonically increasing PID value.
 *
 * @return pid_t The next available process identifier.
 */
pid_t pid_alloc() {
    return next_pid++;
}

/**
 * @brief Placeholder for freeing a previously allocated process ID.
 *
 * Currently, this function does not perform any operations. Intended for future implementation of PID release logic.
 *
 * @param pid The process ID to be freed.
 */
void pid_free(pid_t pid) {
    // TODO: Implement PID freeing logic
    // For now, do nothing
}