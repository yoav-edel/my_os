//
// Created by Yoav on 11/25/2024.
//

#include "std/assert.h"
#include "gdt.h"

// Define a GDT entry structure size - 8 bytes


// Define the number of GDT entries
#define GDT_ENTRIES 5

struct gdt_entry gdt[GDT_ENTRIES];

// Define the GDT pointer
struct gdt_ptr gdtPtr;



/**
 * @brief Loads the Global Descriptor Table (GDT).
 *
 * This function loads the GDT using the `lgdt` instruction and sets up the segment registers.
 * It performs a far jump to reload the code segment (CS) register.
 */
void load_gdt()
{
    //todo fix this
//    __asm__ volatile (
//            "lgdt (%0)\n"
//            "mov $0x10, %%ax\n"
//            "mov %%ax, %%ds\n"
//            "mov %%ax, %%es\n"
//            "mov %%ax, %%fs\n"
//            "mov %%ax, %%gs\n"
//            "mov %%ax, %%ss\n"
//            "ljmp $0x08, $gdt_flush\n" // Provide both segment and offset
//            "gdt_flush:\n"
//            :
//            : "r"(&gdtPtr)
//            : "memory", "eax"
//            );
}


/**
 * @brief Initializes the Global Descriptor Table (GDT).
 *
 * This function initializes the GDT by setting up the GDT pointer and then loading the GDT.
 */
void init_gdt()
{
    setup_gdt_entries();
    init_gdt_ptr();
    load_gdt();
}

/**
 * @brief Sets up the GDT pointer.
 *
 * This function sets the limit and base address of the GDT pointer.
 */
void init_gdt_ptr() {
    gdtPtr.limit = sizeof(gdt) - 1;
    gdtPtr.base = (uint32_t) &gdt;
}

void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    assert(num < GDT_ENTRIES);

    gdt[num].limit_low = GDT_LIMIT_LOW(limit);
    gdt[num].base_low = GDT_BASE_LOW(base);
    gdt[num].base_mid = GDT_BASE_MID(base);
    gdt[num].access = GDT_ACCESS(access);
    gdt[num].granularity = GDT_COMBINE_HIGH_LIMIT_AND_FLAGS(GDT_LIMIT_HIGH(limit), flags);
    gdt[num].base_high = GDT_BASE_HIGH(base);
}
/*
 * @brief Sets up the GDT entries.
 *
 * This function sets up the GDT entries for the null segment and the code and data segments.
 */
void setup_gdt_entries() {
    // Null segment
    set_gdt_entry(NULL_ENTRY, 0, 0, 0, 0);

    // Code segment
    set_gdt_entry(KERNEL_CODE_ENTRY, 0, MAX_LIMIT, GDT_CODE_ACCESS, GDT_CODE_FLAGS);

    // Data segment
    set_gdt_entry(KERNEL_DATA_ENTRY, 0, MAX_LIMIT, GDT_DATA_ACCESS, GDT_DATA_FLAGS);

    // User code segment
    set_gdt_entry(USER_CODE_ENTRY, 0, MAX_LIMIT, GDT_USER_CODE_ACCESS, GDT_CODE_FLAGS);

    // User data segment
    set_gdt_entry(USER_DATA_ENTRY, 0, MAX_LIMIT, GDT_USER_DATA_ACCESS, GDT_DATA_FLAGS);

}