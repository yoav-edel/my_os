//
// Created by Yoav on 3/23/2025.
//

#ifndef scheduler_H
#define scheduler_H
#include "process.h"

/**
 * @brief Adds a process to the scheduler's ready queue
 *
 * @param process Pointer to the process to be added
 */
void scheduler_add_process(process_t *process);

/**
 * @brief Retrieves the next process to be executed based on scheduling algorithm
 *
 * @return process_t* Pointer to the next process to run, or NULL if no processes available
 */
process_t *scheduler_get_next_process();

/**
 * @brief Removes a process from the scheduler's queue
 *
 * @param process Pointer to the process to be removed
 */
void scheduler_remove_process(process_t *process);

/**
 * @brief Gets the currently running process
 *
 * @return process_t* Pointer to the currently running process, or NULL if no process is running
 */
process_t *scheduler_get_current_process();

/**
 * @brief Initializes the scheduler subsystem
 *
 * Must be called before any other scheduler functions
 */
void scheduler_init();

#endif //scheduler_H
