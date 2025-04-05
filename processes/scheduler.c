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
#include "../std/panic.h"

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
static process_t *current_process = NULL;

static process_queue_node_t *scheduler_create_queue_node(process_t *process) {
    process_queue_node_t *node = (process_queue_node_t *) kmalloc(sizeof(process_queue_node_t));
    if (node == NULL)
        panic("Failed to allocate memory for process queue node");

    node->process = process;
    node->next = NULL;

    return node;
}

static inline void scheduler_destroy_queue_node(process_queue_node_t *node) {
    kfree(node);
}

void scheduler_add_process(process_t *process) {
    if (process == NULL)
        panic("Tried to add a NULL process to the scheduler, you sneaky bastard (probably a bug in the kernel)");
    process_queue_node_t *node = scheduler_create_queue_node(process);


    node->process = process;
    node->next = NULL;

    if (ready_queue.head == NULL) {
        ready_queue.head = ready_queue.tail = node;
    } else {
        ready_queue.tail->next = node;
        ready_queue.tail = node;
    }
    ready_queue.count++;
}

process_t *scheduler_get_next_process() {
    if (ready_queue.head == NULL)
        return NULL;

    if (current_process_node == NULL)
        current_process_node = ready_queue.head;
    else
        current_process_node = current_process_node->next;
    if(current_process_node == NULL)
        current_process_node = ready_queue.head;

    return current_process_node->process;
}


void scheduler_remove_process(process_t *process) {
    process_queue_node_t *prev = NULL;
    process_queue_node_t *current = ready_queue.head;

    while (current != NULL) {
        if (current->process == process) {
            if (prev == NULL) {
                ready_queue.head = current->next;
                if (ready_queue.head == NULL)
                    ready_queue.tail = NULL;
            } else {
                prev->next = current->next;
                if (current == ready_queue.tail)
                    ready_queue.tail = prev;
            }

            scheduler_destroy_queue_node(current);
            ready_queue.count--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

static uint32_t tick_count = 0;
#define TICKS_UINTIL_SWITCH 10
void scheduler_handle_tick(){
    tick_count++;
    if(tick_count >= TICKS_UINTIL_SWITCH){
        tick_count = 0;
        process_t *next_process = scheduler_get_next_process();
        if(next_process != NULL){
            process_switch(next_process);
        }
    }

}
