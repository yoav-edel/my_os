//
// Created by Yoav on 2/21/2025.
//

/*
 *
 */

#ifndef MYKERNELPROJECT_KMALLOC_H
#define MYKERNELPROJECT_KMALLOC_H
#include "../std/stdint.h"

#define MIN_NUM_HEAPS 4
const int MIN_SIZE = 16;

void init_kmalloc(); // When loading the kernel the function allocated the necessary stuff for the kmalloc
void *kmalloc(size_t size);
void kfree(void *ptr);



#define FREE 1








#endif //MYKERNELPROJECT_KMALLOC_H
