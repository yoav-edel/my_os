//
// Created by Yoav on 3/15/2025.
//

#include "pcb.h"
#include "kmalloc.h"

context_t *context_create(uint32_t eip, uint32_t esp) {
    context_t *context = kmalloc(sizeof(context_t));
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

pcb_t *pcb_create(uint32_t eip, uint32_t esp, page_directory_t *page_dir) {
    pcb_t *pcb = kmalloc(sizeof(pcb_t));
    pcb->context = context_create(eip, esp);
    pcb->page_dir = page_dir;
    pcb->state = READY;
    return pcb;
}

void pcb_destroy(pcb_t *pcb) {
    if(pcb == NULL) return;
    context_destroy(pcb->context);
    kfree(pcb);
}
