//
// Created by Yoav on 11/29/2024.
//

#ifndef MYKERNELPROJECT_PMM_H
#define MYKERNELPROJECT_PMM_H

#include "../std/stdint.h"
#include <stdbool.h>

// Memory Configuration
#define MEMORY_SIZE 0x100000000u // 4GB
#define PMM_BLOCK_SIZE 4096u     // 4KB
#define PMM_BITMAP_SIZE (MEMORY_SIZE / PMM_BLOCK_SIZE / 8u) // 4GB / 4KB / 8 = 512KB
#define PMM_NO_FRAME_AVAILABLE 0
#define ALIGNED_TO_PHYSICAL_PAGE(addr) ((addr + PMM_BLOCK_SIZE - 1) & ~(PMM_BLOCK_SIZE - 1))

// Kernel reserved memory (e.g., first 1 MB)
#define KERNEL_RESERVED_MEMORY (1 * 1024 * 1024)
typedef uint32_t physical_addr;
void pmm_init();
physical_addr pmm_alloc_frame();
void pmm_free_frame(physical_addr frame_addr);
bool pmm_is_frame_free(physical_addr frame_addr);

#endif // MYKERNELPROJECT_PMM_H
