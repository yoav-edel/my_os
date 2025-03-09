//
// Created by Yoav on 2/21/2025.
//

/*
 *
 */

#ifndef MYKERNELPROJECT_KMALLOC_H
#define MYKERNELPROJECT_KMALLOC_H
#include "../std/stdint.h"
#include "pmm.h"

void init_kmalloc(); // When loading the kernel the function allocated the necessary stuff for the kmalloc
void *kmalloc(size_t size);
void kfree(void *ptr);

extern uint32_t KERNEL_BASE_HEAP_ADDR;
#define NUM_CACHES 8
#define MAX_CACHE_SIZE 2048
#define KERNEL_HEAP_SIZE (PMM_BLOCK_SIZE * NUM_CACHES)




#endif //MYKERNELPROJECT_KMALLOC_H
