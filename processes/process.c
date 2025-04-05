//
// Created by Yoav on 3/16/2025.
//

#include "process.h"
#include "../memory/kmalloc.h"
#include "../memory/vmm.h"
#include "pid.h"
#include "../std/string.h"
#include "../errors.h"
#include "../std/stdio.h"

void process_print(process_t *process) {
    if(process == NULL)
        panic("Trying to print a NULL process, what the hell are you doing?");
    printf("Process name: %s\n", process->name);
    printf("Process PID: %d\n", process->pid);
    printf("Process priority: %d\n", process->priority);
    pcb_print(process->pcb);
    printf("Process VM Context at %p\n", process->pcb->vm_context);
}

process_t *current_process = NULL;
void process_destroy(process_t *process) {
  	if(process == NULL)
          return;
  	pcb_destroy(process->pcb);
  	pid_free(process->pid);
  	kfree(process);
}




process_t *process_create(void (*entry_point)(), char *name, priority_t priority, process_t *parent) {
	process_t *process = kmalloc(sizeof(process_t));
	if(process == NULL)
        return NULL;
  	process->pcb = pcb_create((uint32_t)entry_point, 0, parent->pcb->vm_context);
	if(process->pcb == NULL){
    	kfree(process);
        return NULL;
        }
	process->pid = pid_alloc();
	strcpy(process->name, name);
	process->priority = priority;
  	return process;
}


void processes_init() {

}
