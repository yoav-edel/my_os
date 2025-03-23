//
// Created by Yoav on 3/15/2025.
//

#include "pcb.h"
#include "kmalloc.h"

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

void context_destroy(context_t *context) {
    kfree(context);
}

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

void pcb_destroy(pcb_t *pcb) {
    if(pcb == NULL) return;
    context_destroy(pcb->context);
    vmm_destroy_vm_context(pcb->vm_context);
    kfree(pcb);
}
