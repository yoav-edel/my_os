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

void process_create_stack(process_t *process, uint32_t stack_size) {
    if (process == NULL)
        return;

    uint32_t esp = process->pcb->context->esp;

    // Align the stack size to the page size.
    stack_size = (stack_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    // Map memory for the stack pages.
    for (uint32_t i = 0; i < stack_size; i += PAGE_SIZE) {
        physical_addr addr = pmm_alloc_frame();
        if (addr == 0) {
            printf("Failed to allocate memory for the stack\n");
            return;
        }
        // Map each page from (esp - i)
        vmm_map_page(process->pcb->vm_context->page_dir,
                     (void *)(esp - i),
                     addr,
                     PAGE_WRITEABLE);
    }

    /*
     * Create an initial stack frame with 11 words, laid out as follows:
     *
     * Index 0: will be popped into EDI
     * Index 1: will be popped into ESI
     * Index 2: will be popped into EBP
     * Index 3: will be popped into ESP
     * Index 4: will be popped into EBX
     * Index 5: will be popped into EDX
     * Index 6: will be popped into ECX
     * Index 7: will be popped into EAX
     * Index 8: will be popped into EFLAGS
     * Index 9: will be popped into EBP (dummy)
     * Index 10: will be popped by ret (the return address: the entry point)
     *
     * After process_switch sets ESP from context->esp, the popad/popfd/pop ebp/ret
     * will load these values accordingly.
     */
    uint32_t *stack_frame = (uint32_t *)(esp);
    for (int i = 0; i < 11; i++) {
        stack_frame[i] = 0;
    }
    stack_frame[3] = esp;

    stack_frame[8] = DEFAULT_EFLAGS;

    stack_frame[10] = process->pcb->context->eip;
}

process_t *process_create(void (*entry_point)(), char *name, priority_t priority, process_t *parent) {
    if(parent == NULL)
		return NULL;
	//todo maybe add more sainty checks
	process_t *process = kmalloc(sizeof(process_t));
	if(process == NULL)
        return NULL;
  	process->pcb = pcb_create((uint32_t)entry_point, 10000000, parent->pcb->vm_context);
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
