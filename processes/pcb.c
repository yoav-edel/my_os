//
// Created by Yoav on 3/15/2025.
//

#include "pcb.h"

pcb_t *create_pcb(context_t context, page_directory_t *page_dir) {
    pcb_t *pcb = kmalloc(sizeof(pcb_t));
    pcb->context = context;
    pcb->page_dir = page_dir;
    pcb->state = READY;
    return pcb;
}

void destroy_pcb(pcb_t *pcb) {
    kfree(pcb);
}
