//
// Created by Yoav on 11/29/2024.
//

#ifndef MYKERNELPROJECT_PMM_H
#define MYKERNELPROJECT_PMM_H

#include "../std/stdint.h"
#include <stdbool.h>

// Memory Configuration
#define MEMORY_SIZE 0x100000000 // 4GB
#define PMM_BLOCK_SIZE 4096     // 4KB
#define PMM_BITMAP_SIZE (MEMORY_SIZE / PMM_BLOCK_SIZE / 8) // 4GB / 4KB / 8 = 512KB
#define PMM_NO_FRAME_AVAILABLE 0

// Kernel reserved memory (e.g., first 1 MB)
#define KERNEL_RESERVED_MEMORY (1 * 1024 * 1024)

void pmm_init();
uint32_t pmm_alloc_frame();
void pmm_free_frame(uint32_t frame_addr);
bool pmm_is_frame_free(uint32_t frame_addr);

#endif // MYKERNELPROJECT_PMM_H
