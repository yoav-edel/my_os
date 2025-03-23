//
// Created by Yoav on 3/16/2025.
//

#include "process.h"
#include "../memory/kmalloc.h"
#include "../memory/vmm.h"
#include "pid.h"
#include "../std/string.h"
#include "../errors.h"


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


void switch_process(process_t *process) {
	if(process == NULL)
		panic("Trying to switch to a NULL process, what the hell are you doing?");
	current_process = process;

}


void processes_init() {

}
