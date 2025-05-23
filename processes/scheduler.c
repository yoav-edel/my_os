//
// Created by Yoav on 3/23/2025.
//

/*
 * This file is a basic implementation of a process scheduler.
* I choose to implement a simple round-robin scheduler as a proof of concept.
* In the future I will implement more advanced and cool schedulers and see how fast I will burn my computer.
 */

#include "scheduler.h"
#include "../memory/kmalloc.h"
#include "../errors.h"
#include "../std/stdio.h"

typedef struct process_queue_node {
    process_t *process;
    struct process_queue_node *next;
} process_queue_node_t;

typedef struct {
    process_queue_node_t *head;
    process_queue_node_t *tail;
    size_t count;
} process_queue_t;

static process_queue_t ready_queue = {NULL, NULL, 0};
static process_queue_node_t *current_process_node = NULL;
process_t *current_process = NULL;

/**
 * @brief Allocates and initializes a new process queue node.
 *
 * Creates a new node for the process queue, assigning the given process and initializing the next pointer to NULL.
 * Panics if memory allocation fails.
 *
 * @param process Pointer to the process to associate with the new node.
 * @return Pointer to the newly created process queue node.
 */
static process_queue_node_t *scheduler_create_queue_node(process_t *process) {
    process_queue_node_t *node = (process_queue_node_t *) kmalloc(sizeof(process_queue_node_t));
    if (node == NULL)
        panic("Failed to allocate memory for process queue node");

    node->process = process;
    node->next = NULL;

    return node;
}

/**
 * @brief Frees the memory allocated for a process queue node.
 *
 * @param node Pointer to the process queue node to be destroyed.
 */
static inline void scheduler_destroy_queue_node(process_queue_node_t *node) {
    kfree(node);
}

/**
 * @brief Adds a process to the scheduler's ready queue.
 *
 * Inserts the given process into the circular ready queue for round-robin scheduling.
 * Panics if the process pointer is NULL.
 */
void scheduler_add_process(process_t *process) {
    if (process == NULL)
        panic("Tried to add a NULL process to the scheduler, you sneaky bastard (probably a bug in the kernel)");
    process_queue_node_t *node = scheduler_create_queue_node(process);

    if (ready_queue.tail == NULL) {
        ready_queue.head = ready_queue.tail = node;
        node->next = node;
    }
    else {
        ready_queue.tail->next = node;
        ready_queue.tail = node;
        node->next = ready_queue.head;
    }
    ready_queue.count++;
}

/**
 * @brief Advances to and returns the next process in the ready queue.
 *
 * Moves the current process node pointer to the next node in the circular ready queue and returns the associated process. Panics if the scheduler is in an invalid state.
 *
 * @return Pointer to the next process in the ready queue, or NULL if the queue is empty.
 */
process_t *scheduler_get_next_process() {
    if (ready_queue.head == NULL)
        return NULL;

    if (current_process_node == NULL)
        panic("Current process node is NULL, this should never happen");
    if (current_process_node->next == NULL)
        panic("Current process node next is NULL, this should never happen");
    else
        current_process_node = current_process_node->next;

    return current_process_node->process;
}

/**
 * @brief Removes a specified process from the ready queue.
 *
 * If the process is found in the ready queue, it is removed and the queue's structure is updated accordingly. Handles empty, single-element, and multi-element queue cases. If the removed process is the current process node, advances or clears the current process node pointer as appropriate.
 *
 * @param process Pointer to the process to remove from the ready queue.
 */
void scheduler_remove_process(process_t *process) {
    // Handle empty queue case
    if (ready_queue.head == NULL || process == NULL) {
        return;
    }

    // Handle single element case
    if (ready_queue.head == ready_queue.tail) {
        if (ready_queue.head->process == process) {
            scheduler_destroy_queue_node(ready_queue.head);
            ready_queue.head = NULL;
            ready_queue.tail = NULL;
            ready_queue.count = 0;
        }
        return;
    }

    // Handle multi-element case
    process_queue_node_t *prev = NULL;
    process_queue_node_t *current = ready_queue.head;

    // Loop through queue to find the process
    do {
        if (current->process == process) {
            if (prev == NULL) {
                ready_queue.head = current->next;
            } else {
                prev->next = current->next;
            }

            if (current == ready_queue.tail) {
                ready_queue.tail = prev ? prev : ready_queue.head;
            }
			if(current == current_process_node && ready_queue.count > 1)
               current_process_node = current_process_node->next;
             else if(current == current_process_node && ready_queue.count == 1)
                current_process_node = NULL;

            scheduler_destroy_queue_node(current);
            ready_queue.count--;
            return;
        }

        prev = current;
        current = current->next;
    } while (current != ready_queue.head);
}

static uint32_t tick_count = 0;
#define TICKS_UNTIL_SWITCH 10 // Changed from 30 to 10
/**
 * @brief Handles a scheduler tick and performs a context switch if needed.
 *
 * Increments the scheduler's tick counter and, after a defined number of ticks,
 * switches to the next process in the ready queue if it differs from the current process.
 */
void scheduler_handle_tick(){
    tick_count++;
    if(tick_count >= TICKS_UNTIL_SWITCH){
        tick_count = 0;
        process_t *next_process = scheduler_get_next_process();
        if(next_process != NULL && next_process != current_process) {
          	printf("Switching process\n");
            process_switch(next_process);
        }
    }

}

/**
 * @brief Returns the currently running process.
 *
 * @return Pointer to the process currently scheduled to run, or NULL if no process is active.
 */
process_t *scheduler_get_current_process() {
    return current_process;
}

/**
 * @brief Initializes the process scheduler with the initial process.
 *
 * Adds the initial process to the ready queue, sets it as the current process, and prepares the scheduler for operation. Panics if the initial process is NULL.
 */
void scheduler_init(process_t *init_process) {
    if (init_process == NULL)
        panic("Tried to initialize the scheduler with a NULL process, you sneaky bastard (probably a bug in the kernel)");

    tick_count = 0;
    scheduler_add_process(init_process);
    current_process = init_process;
    current_process_node = ready_queue.head;
}
