//
// Created by Yoav on 3/16/2025.
//

#include "process.h"
#include "../memory/kmalloc.h"
#include "../memory/vmm.h"
#include "../memory/utills.h"
#include "pid.h"
#include "../std/string.h"
#include "../errors.h"
#include "../std/stdio.h"
#include "scheduler.h"
#include "../std/stdint.h"

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

void process_create_stack(process_t *process, uint32_t stack_size)
{
    if (!process)
        return;

    vm_context_t *prev_vm_context = scheduler_get_current_process()->pcb->vm_context;

    // Round stack_size up to a multiple of PAGE_SIZE
    stack_size = (stack_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uint32_t pages   = stack_size / PAGE_SIZE;
    uint32_t esp_top = process->pcb->context->esp;

    // Switch into the child's page directory
    vmm_switch_vm_context(process->pcb->vm_context);

    for (uint32_t i = 0; i < pages; ++i) {
        physical_addr pa = pmm_alloc_frame();
        if (pa == PMM_NO_FRAME_AVAILABLE)
            panic("pmm_alloc_frame() failed while building stack");

        uint32_t vir_addr = esp_top - (i + 1) * PAGE_SIZE;

        vmm_map_page_to_curr_dir((void *)vir_addr,
                                 pa,
                                  EMPTY_USER_PAGE_DIR_FLAGS);
        // Invalidate TLB for this page
        asm volatile ("invlpg (%0)" :: "r"(vir_addr) : "memory");

        // Zero the newly mapped page in-place
        memset((void *)vir_addr, 0, PAGE_SIZE);
    }

    // Build the initial trap frame at the top of the stack
    const uint32_t FRAME_DWORDS = 11;
    const uint32_t frame_bytes  = FRAME_DWORDS * sizeof(uint32_t);
    uint32_t esp_frame = esp_top - frame_bytes;

    // Pointer within the child's address space (active CR3)
    uint32_t *f = (uint32_t *)esp_frame;

    // Zero the frame and initialize saved registers
    memset(f, 0, frame_bytes);
    f[3]  = 0;                              // saved EBP
    f[8]  = DEFAULT_EFLAGS;                // initial EFLAGS
    f[10] = process->pcb->context->eip;    // entry EIP

    vmm_switch_vm_context(prev_vm_context);
    process->pcb->context->esp = esp_frame;
}

process_t *process_create(void (*entry_point)(), char *name, priority_t priority, process_t *parent) {
    if(parent == NULL)
		return NULL;
	//todo maybe add more sainty checks
	process_t *process = kmalloc(sizeof(process_t));
	if(process == NULL)
        return NULL;
  	process->pcb = pcb_create((uint32_t)entry_point, 100000000, parent->pcb->vm_context);
    if(process->pcb == NULL){
    	kfree(process);
        return NULL;
    }
    process_create_stack(process, 0x1000 * 5);
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
	scheduler_init(proc);
}
