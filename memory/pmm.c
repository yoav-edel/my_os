//
// Created by Yoav on 11/29/2024.
//

#include <stdbool.h>
#include <assert.h>
#include "pmm.h"

// Bitmap for tracking used and free frames
static uint8_t pmm_bitmap[PMM_BITMAP_SIZE] = {0};

// Track the last freed frame
static struct {
    int32_t index;
    int8_t offset;
} last_free_frame = {-1, -1};

// Track the last allocated index for next-fit strategy
static size_t last_alloc_index = 0;

// Macros for bitmap operations
#define BIT_MASK(offset) (1 << (offset))

// Helper Functions
static inline size_t calc_frame_addr(size_t index, size_t offset) {
    return index * 8 * PMM_BLOCK_SIZE + offset * PMM_BLOCK_SIZE;
}

static inline size_t get_bitmap_index(uint32_t frame_addr) {
    return frame_addr / PMM_BLOCK_SIZE / 8;
}

static inline size_t get_bitmap_offset(uint32_t frame_addr) {
    return frame_addr / PMM_BLOCK_SIZE % 8;
}

static inline bool is_valid_frame_addr(uint32_t frame_addr) {
    return frame_addr < MEMORY_SIZE && (frame_addr % PMM_BLOCK_SIZE == 0); // Frame address is aligned
}

static inline void pmm_mark_used(uint32_t frame_addr) {
    pmm_bitmap[get_bitmap_index(frame_addr)] |= BIT_MASK(get_bitmap_offset(frame_addr));
}

static inline void pmm_mark_free(uint32_t frame_addr) {
    pmm_bitmap[get_bitmap_index(frame_addr)] &= ~BIT_MASK(get_bitmap_offset(frame_addr));
}

static inline bool is_full(size_t index) {
    return pmm_bitmap[index] == 0xFF;
}

// Initialize the Physical Memory Manager
void pmm_init() {
    // Initialize the bitmap (mark all frames as free)
    for (size_t i = 0; i < PMM_BITMAP_SIZE; i++) {
        pmm_bitmap[i] = 0;
    }

    // Reserve memory for kernel and hardware
    for (uint32_t addr = 0; addr < KERNEL_RESERVED_MEMORY; addr += PMM_BLOCK_SIZE) {
        pmm_mark_used(addr);
    }
}

// Allocate a single frame using next-fit strategy
uint32_t pmm_alloc_frame() {
    // Check if we can reuse the last freed frame
    if (last_free_frame.index != -1 && last_free_frame.offset != -1) {
        uint32_t frame_addr = calc_frame_addr(last_free_frame.index, last_free_frame.offset);
        pmm_mark_used(frame_addr);
        last_free_frame.index = -1;
        last_free_frame.offset = -1;
        return frame_addr;
    }

    // Use next-fit strategy to allocate a frame
    for (size_t i = last_alloc_index; i < PMM_BITMAP_SIZE; i++) {
        if (!is_full(i)) {
            for (uint8_t j = 0; j < 8; j++) {
                if (!(pmm_bitmap[i] & BIT_MASK(j))) {
                    uint32_t frame_addr = calc_frame_addr(i, j);
                    pmm_mark_used(frame_addr);
                    last_alloc_index = i; // Update the next-fit index
                    return frame_addr;
                }
            }
        }
    }

    // Wrap around and continue searching from the beginning
    for (size_t i = 0; i < last_alloc_index; i++) {
        if (!is_full(i)) {
            for (uint8_t j = 0; j < 8; j++) {
                if (!(pmm_bitmap[i] & BIT_MASK(j))) {
                    uint32_t frame_addr = calc_frame_addr(i, j);
                    pmm_mark_used(frame_addr);
                    last_alloc_index = i; // Update the next-fit index
                    return frame_addr;
                }
            }
        }
    }

    // No free frames available
    return PMM_NO_FRAME_AVAILABLE;
}

// Free a previously allocated frame
void pmm_free_frame(uint32_t frame_addr) {
    assert(is_valid_frame_addr(frame_addr));
    pmm_mark_free(frame_addr);
    last_free_frame.index = get_bitmap_index(frame_addr);
    last_free_frame.offset = get_bitmap_offset(frame_addr);
}

bool pmm_is_frame_free(uint32_t frame_addr) {
    assert(is_valid_frame_addr(frame_addr));
    return !(pmm_bitmap[get_bitmap_index(frame_addr)] & BIT_MASK(get_bitmap_offset(frame_addr)));
}

