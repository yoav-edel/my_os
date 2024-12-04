//
// Created by Yoav on 11/29/2024.
//

#include <assert.h>
#include "vmm.h"
#include "pmm.h"
#include "utills.h"
#include "../errors.h"
#include "../drivers/disk.h"


typedef struct page_t page_t;
static page_directory_t *current_directory = NULL;


uint32_t disk_alloc_slot();

page_directory_t *vmm_create_page_directory() {
    //TODO free frames if allocation fails and then aloocate again
    uint32_t phys_adrr_page_dir = pmm_alloc_frame();
    if (phys_adrr_page_dir == PMM_NO_FRAME_AVAILABLE)
        return NULL;

    page_directory_t *page_dir = (page_directory_t *) phys_adrr_page_dir;
    memset(page_dir, 0, sizeof(page_directory_t)); // Clear the page directory
    //todo copy the kernel mapping to the new page directory
    if (current_directory != NULL)
        for (size_t i = KERNEL_START_DIRECTORY_INDEX; i < PAGE_DIR_SIZE; i++) {
            page_dir->entries[i] = current_directory->entries[i];
        }
    return page_dir;
}


void enable_paging() {
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set the paging bit in cr0
    asm volatile ("mov %0, %%cr0"::"r"(cr0));
}

inline void load_curr_page_dir() {
    asm volatile ("mov %0, %%cr3"::"r"(current_directory));
}

void vmm_switch_page_directory(page_directory_t *dir) {
    assert(dir != NULL); // Make sure the directory is not NULL
    current_directory = dir;
    load_curr_page_dir(); // Load the current page directory into the cr3 register
}

inline void page_entry_set_frame(page_entry_t *e, uint32_t frame_addr) {
    *e = (*e & ~0xFFFFF000) | frame_addr;
}

inline void page_entry_add_attrib(page_entry_t *e, uint32_t attrib) {
    *e |= attrib;
}

inline void page_entry_remove_attrib(page_entry_t *e, uint32_t attrib) {
    *e &= ~attrib;
}

inline bool is_page_swapped(page_entry_t *e) {
    return *e & SWAPPED;
}

// return the page to swap out if there is no page to swap out return NULL
page_entry_t *vmm_get_page_to_swap_out() {
    return NULL;
}


bool vmm_swap_out_some_page() {
    page_entry_t *e = vmm_get_page_to_swap_out();
    if (e == NULL)
        return false;

    //swap out the page
    uint32_t disk_slot = disk_alloc_slot();
    if (disk_slot == NO_SLOT_AVAILABLE)
        return false;

    //write the page to the disk
    disk_write(disk_slot, 0, (uint8_t *) get_frame_addr(e), PAGE_SIZE);

    // update the page entry
    page_entry_add_attrib(e, SWAPPED);
    page_entry_remove_attrib(e, PRESENT);
    page_entry_set_frame(e, disk_slot);

    return true;
}

/*
 * Swaps in a page from the disk
 * return true if the swap was successful, false otherwise
 */
bool vmm_swap_in_page(page_entry_t *e) {
    uint32_t frame_addr = pmm_alloc_frame();
    if(frame_addr == PMM_NO_FRAME_AVAILABLE)
    {
        if (!vmm_swap_out_some_page())
            return false;
        frame_addr = pmm_alloc_frame();
        if (frame_addr == PMM_NO_FRAME_AVAILABLE)
            return false;
    }

    uint32_t disk_slot = get_frame_addr(e);

    page_entry_set_frame(e, frame_addr);

    disk_read(disk_slot, 0, (uint8_t *) frame_addr, PAGE_SIZE);

    // add the present attribute and remove the swapped attribute
    page_entry_add_attrib(e, PRESENT);
    page_entry_remove_attrib(e, SWAPPED);
    return true;
}


bool vmm_alloc_page(page_entry_t *e) {
    //allocate physical frame
    uint32_t frame_addr = pmm_alloc_frame();
    if (frame_addr == PMM_NO_FRAME_AVAILABLE) {
        if (!vmm_swap_out_some_page())
            return false;
        frame_addr = pmm_alloc_frame();
        if (frame_addr == PMM_NO_FRAME_AVAILABLE)
            return false;
    }

    //map the frame to the page entry
    page_entry_set_frame(e, frame_addr);
    page_entry_add_attrib(e, PRESENT);
}



void vmm_free_page(page_entry_t *e) {
    pmm_free_frame(*e & 0xFFFFF000);
    *e = 0;
}


/*
 * Maps a page to a frame
 */
void vmm_map_page(void *vir_addr, void *phys_addr, uint32_t flags) {
    assert(current_directory != NULL);
    uint32_t pd_index = get_directory_index(vir_addr);
    uint32_t pt_index = get_table_index(vir_addr);
    // check if the table is
    page_table_t *page_table;
    if (is_page_present(&current_directory->entries[pd_index])) // the table is exist and present
         page_table = (page_table_t *) get_page_table_addr(current_directory, pd_index);

    else if(is_page_swapped(&current_directory->entries[pd_index])) // the table is exist but swapped
        {
            if (!vmm_swap_in_page(&current_directory->entries[pd_index]))
                panic("Failed to swap in page. how tf did we mange to get here?");
            page_table = (page_table_t *) get_page_table_addr(current_directory, pd_index);
        }
    else // the table is not exist. create a new one
        {
        uint32_t phys_addr_page_table = pmm_alloc_frame();
        if (phys_addr_page_table == PMM_NO_FRAME_AVAILABLE)
        {
            if(!vmm_swap_out_some_page())
               panic("Failed to allocate a frame for the page table. how tf did we mange to get here?");
            phys_addr_page_table = pmm_alloc_frame();
        }
        page_entry_set_frame(&current_directory->entries[pd_index], phys_addr_page_table);
        page_entry_add_attrib(&current_directory->entries[pd_index], PRESENT);
        page_table = (page_table_t *) phys_addr_page_table;
    }

    // map the page to the frame
    page_entry_t *page_entry = &page_table->entries[pt_index];
    page_entry_set_frame(page_entry, (uint32_t) phys_addr);
    page_entry_add_attrib(page_entry, flags | PRESENT);

    // todo add the flags to the table entry
}

/*
 * Unmaps a page from a frame
 */
void vmm_unmap_page() {
    assert(current_directory != NULL);
    assert(page != NULL); // Make sure the page is not NULL


}


void init_paging() {
    current_directory = (page_directory_t *) pmm_alloc_frame();
    if (current_directory == NULL)
        panic("Failed to initialize paging : could not allocate a frame for the page directory. you really have fucked up computer");

    for (size_t i = 0; i < PAGE_DIR_SIZE; i++) {
        page_directory[i] = 0x00000002; // Supervisor, Read/Write, Not Present
    }

    enable_paging();

}