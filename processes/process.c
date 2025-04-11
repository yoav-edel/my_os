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
#include "scheduler.h"

void process_print(process_t *process) {
    if(process == NULL)
        panic("Trying to print a NULL process, what the hell are you doing?");
    printf("Process name: %s\n", process->name);
    printf("Process PID: %d\n", process->pid);
    printf("Process priority: %d\n", process->priority);
    pcb_print(process->pcb);
    printf("Process VM Context at %p\n", process->pcb->vm_context);
}

void process_destroy(process_t *process) {
  	if(process == NULL)
          return;
  	pcb_destroy(process->pcb);
  	pid_free(process->pid);
  	kfree(process);
}




process_t *process_create(void (*entry_point)(), char *name, priority_t priority, process_t *parent) {
    if(parent == NULL)
		return NULL;
	//todo maybe add more sainty checks
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
	// Map the running kernel process to the current process
    process_t *proc = kmalloc(sizeof(process_t));
    if(proc == NULL)
		panic("Failed to allocate memory for the current process");
    vm_context_t *vm_context = kmalloc(sizeof(vm_context_t));
    if(vm_context == NULL)
      	panic("Failed to allocate memory for the current process vm_context");
    //The kernel init page directory virtual address and physical address are the same
    vm_context->page_dir = vmm_get_kernel_page_directory();
    vm_context->page_dir_phys_addr = (physical_addr) vm_context->page_dir;
    proc->pcb = pcb_create((uint32_t)0, 0, vm_context);
    proc->pid = pid_alloc();
    strcpy(proc->name, "kernel_init");
    proc->priority = HIGH;
    proc->pcb->context->edi = 0;
	scheduler_init(proc);=
}
