//
// Created by Yoav on 3/15/2025.
//

#include "pcb.h"
#include "../memory/kmalloc.h"
#include "../std/stdio.h"
#include "../memory/vmm.h"
#include "../errors.h"

/**
 * @brief Prints the details of a process control block (PCB).
 *
 * Displays the pointers to the PCB's CPU context and virtual memory context, as well as its current state.
 * Triggers a kernel panic if a NULL PCB pointer is provided.
 */
void pcb_print(pcb_t *pcb) {
    if(pcb == NULL)
        panic("Trying to print a NULL pcb, what the hell are you doing?\n Closing the computer as punishment");
    printf("PCB context: %p\n", pcb->context);
    printf("PCB vm_context: %p\n", pcb->vm_context);
    printf("PCB state: %d\n", pcb->state);
}
/**
 * @brief Allocates and initializes a new CPU context structure.
 *
 * Sets all general-purpose registers to zero, assigns the provided instruction pointer (eip) and stack pointer (esp), and initializes the flags register to a default value.
 *
 * @param eip Initial value for the instruction pointer.
 * @param esp Initial value for the stack pointer.
 * @return Pointer to the newly created context structure, or NULL if allocation fails.
 */
context_t *context_create(uint32_t eip, uint32_t esp) {
    context_t *context = kmalloc(sizeof(context_t));
    if(context == NULL)
        return NULL;
    context->edi = 0;
    context->esi = 0;
    context->ebp = 0;
    context->esp = esp;
    context->ebx = 0;
    context->edx = 0;
    context->ecx = 0;
    context->eax = 0;
    context->eip = eip;
    context->eflags = DEFAULT_EFLAGS;
    return context;
}

/**
 * @brief Frees the memory allocated for a CPU context structure.
 *
 * Safely deallocates the specified context. If the context pointer is NULL, no action is taken.
 */
void context_destroy(context_t *context) {
    kfree(context);
}

/**
 * @brief Creates a new process control block (PCB) with a CPU context and virtual memory context.
 *
 * Allocates and initializes a PCB structure with a new CPU context using the specified instruction and stack pointers,
 * and a new virtual memory context based on the parent context's page directory. The PCB state is set to READY.
 *
 * @param eip Initial instruction pointer for the new process.
 * @param esp Initial stack pointer for the new process.
 * @param vm_context_parent Parent virtual memory context to base the new context on.
 * @return pcb_t* Pointer to the newly created PCB, or NULL if allocation fails or the parent context is NULL.
 */
pcb_t *pcb_create(uint32_t eip, uint32_t esp, vm_context_t *vm_context_parent) {
  	if(vm_context_parent == NULL)
        return NULL;

    pcb_t *pcb = kmalloc(sizeof(pcb_t));
    if(pcb == NULL)
        return NULL;
    pcb->context = context_create(eip, esp);
    pcb->vm_context = vmm_create_vm_context(vm_context_parent->page_dir);
    pcb->state = READY;
    return pcb;
}

/**
 * @brief Releases all resources associated with a process control block (PCB).
 *
 * Frees the CPU context, virtual memory context, and the PCB structure itself. If the provided PCB pointer is NULL, the function returns immediately.
 */
void pcb_destroy(pcb_t *pcb) {
    if(pcb == NULL) return;
    context_destroy(pcb->context);
    vmm_destroy_vm_context(pcb->vm_context);
    kfree(pcb);
}

/**
 * @brief Saves the state of the currently running process.
 *
 * Intended to capture and store the CPU state and relevant context for the specified process control block.
 *
 * @param pcb Pointer to the process control block whose state should be saved.
 */
void pcb_save_state(pcb_t *pcb){

}