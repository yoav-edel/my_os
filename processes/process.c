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

#define PROCESS_NAME_MAX_LENGTH 32

/**
 * @brief Prints detailed information about a process.
 *
 * Outputs the process's name, PID, priority, PCB details, and virtual memory context address.
 * Panics if the provided process pointer is NULL.
 */
void process_print(process_t *process) {
    if(process == NULL)
        panic("Trying to print a NULL process, what the hell are you doing?");
    printf("Process name: %s\n", process->name);
    printf("Process PID: %d\n", process->pid);
    printf("Process priority: %d\n", process->priority);
    pcb_print(process->pcb);
    printf("Process VM Context at %p\n", process->pcb->vm_context);
}

/**
 * @brief Releases all resources associated with a process.
 *
 * Frees the process control block, releases the process ID, and deallocates the process structure.
 * If the process pointer is NULL, the function returns immediately.
 */
void process_destroy(process_t *process) {
  	if(process == NULL)
          return;
  	pcb_destroy(process->pcb);
  	pid_free(process->pid);
  	kfree(process);
}

/**
 * @brief Allocates and sets up the user stack for a process.
 *
 * Allocates physical memory pages for the process's stack, maps them into the process's virtual memory context, zeroes the stack memory, and builds an initial trap frame with default register values and the process entry point. Updates the process's stack pointer to point to the new trap frame. Returns immediately if the process pointer is NULL.
 *
 * @param process The process for which to create the stack.
 * @param stack_size The desired stack size in bytes; rounded up to a multiple of the page size.
 */
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

/**
 * @brief Creates a new process with the specified entry point, name, priority, and parent.
 *
 * Allocates and initializes a process structure, sets up its process control block (PCB) using the parent's virtual memory context, creates a user stack, assigns a PID, and copies the process name. Returns NULL if the parent is NULL or if any allocation fails.
 *
 * @param entry_point Function pointer to the process's entry point.
 * @param name Name of the process (truncated to maximum allowed length).
 * @param priority Priority level for scheduling.
 * @param parent Parent process whose VM context is inherited.
 * @return Pointer to the newly created process, or NULL on failure.
 */
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
    strncpy(process->name, name, PROCESS_NAME_MAX_LENGTH - 1);
    process->name[PROCESS_NAME_MAX_LENGTH - 1] = '\0'; // Ensure null termination
	process->priority = priority;
  	return process;
}


/**
 * @brief Initializes the initial kernel process and scheduler.
 *
 * Allocates and sets up the kernel's initial process ("kernel_init"), including its virtual memory context, PCB, PID, name, and priority. Initializes the scheduler with this process as the starting point. Panics if memory allocation fails.
 */
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
    strncpy(proc->name, "kernel_init", PROCESS_NAME_MAX_LENGTH - 1);
    proc->name[PROCESS_NAME_MAX_LENGTH - 1] = '\0'; // Ensure null termination
    proc->priority = HIGH;
    proc->pcb->context->edi = 0;
	scheduler_init(proc);
}
