//
// Created by Yoav on 11/25/2024.
//

#ifndef MYKERNELPROJECT_GDT_H
#define MYKERNELPROJECT_GDT_H

#include "std/stdint.h"

// Define a GDT pointer structure
struct gdt_ptr{
    uint16_t limit; // Size of the GDT table minus 1
    uint32_t base;   // Address of the first GDT entry
} __attribute__((packed));

struct gdt_entry {
    uint16_t limit_low;  // Lower 16 bits of the segment limit
    uint16_t base_low;   // Lower 16 bits of the base address
    uint8_t base_mid;    // Middle 8 bits of the base address
    uint8_t access;      // Access flags (type, privilege level, present)
    uint8_t granularity; // Granularity and upper 4 bits of the segment limit
    uint8_t base_high;   // Upper 8 bits of the base address
} __attribute__((packed));




// Function to initialize the GDT
void init_gdt();

// Function to setup GDT entries
void setup_gdt_entries();

// Function to set a GDT entry
void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

// Function to initialize the GDT pointer
void init_gdt_ptr();
// Function to load the GDT pointer into the register
void load_gdt();



// Macros to access the GDT entry
#define GDT_ENTRY(num) ((uint32_t) &gdt[num]) // Get the GDT entry address


// Macros to process the GDT entry parameters
#define GDT_LIMIT_LOW(limit) ((limit) & 0xFFFF) // Get the lower 16 bits of the limit
#define GDT_LIMIT_HIGH(limit) (((limit) >> 16) & 0x0F) // Get the upper 4 bits of the limit
#define GDT_BASE_LOW(base) ((base) & 0xFFFF)     // Get the lower 16 bits of the base
#define GDT_BASE_MID(base) (((base) >> 16) & 0xFF) // Get the middle 8 bits of the base
#define GDT_ACCESS(access) (access)               // Get the access flags
#define GDT_BASE_HIGH(base) (((base) >> 24) & 0xFF) // Get the upper 8 bits of the base

#define GDT_COMBINE_HIGH_LIMIT_AND_FLAGS(high_limit, flags) ((high_limit) | (flags << 4)) // Combine the high limit and flags


#define NULL_ENTRY 0
#define KERNEL_CODE_ENTRY 2
#define KERNEL_DATA_ENTRY 3
#define USER_CODE_ENTRY 4
#define USER_DATA_ENTRY 5

#define KERNEL_CODE_SELECTOR 0x10
#define KERNEL_DATA_SELECTOR 0x18
#define USER_CODE_SELECTOR 0x20
#define USER_DATA_SELECTOR 0x28

#define GDT_CODE_ACCESS 0x9A
#define GDT_CODE_FLAGS 0xC


#define GDT_DATA_ACCESS 0x92
#define GDT_DATA_FLAGS 0xC


#define GDT_USER_CODE_ACCESS 0xFA
#define GDT_USER_DATA_ACCESS 0xF2




#define MAX_LIMIT 0xFFFFFFFF


#endif //MYKERNELPROJECT_GDT_H
