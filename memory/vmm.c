//
// Created by Yoav on 11/29/2024.
//

#include "../std/assert.h"
#include "vmm.h"
#include "pmm.h"
#include "utills.h"
#include "../errors.h"
#include "../drivers/disk.h"
#include "kmalloc.h"
#include "../gdt.h"

#include "../drivers/screen.h"


typedef struct page_t page_t;
static page_directory_t *current_directory = NULL;
static page_directory_t *kernel_directory = NULL;

typedef struct page_fifo_node {
    page_entry_t *entry;
    struct page_fifo_node *next;
} page_fifo_node_t;

typedef struct {
    page_fifo_node_t *head;
    page_fifo_node_t *tail;
    size_t count;
} page_fifo_queue_t;

static page_fifo_queue_t current_page_fifo_queue = {NULL, NULL, 0};

// ---------------------------- Helper functions ----------------------------

static inline uint32_t get_directory_index(void *vir_addr) {
    return (uint32_t) (vir_addr) >> 22;
}

static inline uint32_t get_table_index(void *vir_addr) {
    return ((uint32_t) (vir_addr) >> 12) & 0x03FF;
}

static inline uint32_t get_page_table_addr(const page_directory_t *dir, uint32_t pd_index) {
    // the address is the 20 most significant bits of the entry
    return dir->entries[pd_index] & 0xFFFFF000;
}

static inline uint32_t get_frame_addr(const page_entry_t *entry) {
    return *entry & 0xFFFFF000;
}


static inline bool is_page_present(const page_entry_t *entry) {
    return *entry & PRESENT;
}

static inline void load_curr_page_dir() {
    asm volatile ("mov %0, %%cr3"::"r"(current_directory));
}

static inline void page_entry_set_frame(page_entry_t *e, uint32_t frame_addr) {
    *e = (*e & ~0xFFFFF000) | frame_addr;
}

static inline void page_entry_add_attrib(page_entry_t *e, uint32_t attrib) {
    *e |= attrib;
}

static inline void table_entry_add_attrib(page_entry_t *e, uint32_t attrib) {
    *e |= attrib;
}

static inline void page_entry_remove_attrib(page_entry_t *e, uint32_t attrib) {
    *e &= ~attrib;
}

static inline bool is_page_swapped(const page_entry_t *e) {
    return *e & SWAPPED;
}

static inline bool is_page_present_error(uint32_t error_code) {
    return error_code & 0x1;
}

static inline bool is_page_write_error(uint32_t error_code) {
    return error_code & 0x2;
}

static inline bool is_page_user_error(uint32_t error_code) {
    return error_code & 0x4;
}

static inline bool is_cow(page_entry_t *e) {
    //todo after the cow mechanism is implemented return *e & COW
    return false;
}


static inline void flush_tlb() {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    asm volatile ("mov %0, %%cr3"::"r"(cr3));
}

// ---------------------------- Page FIFO Algorithm ----------------------------


static void page_fifo_enqueue(page_fifo_node_t *node) {
    if (current_page_fifo_queue.head == NULL) {
        current_page_fifo_queue.head = current_page_fifo_queue.tail = node;
    } else {
        current_page_fifo_queue.tail->next = node;
        current_page_fifo_queue.tail = node;
    }
    current_page_fifo_queue.count++;
}

static page_entry_t *page_fifo_dequeue() {
    if (current_page_fifo_queue.head == NULL)
        return NULL;
    page_fifo_node_t *node = current_page_fifo_queue.head;
    current_page_fifo_queue.head = current_page_fifo_queue.head->next;
    current_page_fifo_queue.count--;
    return node->entry;
}



// ---------------------------- VMM functions ----------------------------

//page_directory_t *vmm_create_page_directory() {
//    //TODO free frames if allocation fails and then aloocate again
//    physical_addr phys_adrr_page_dir = pmm_alloc_frame();
//    if (phys_adrr_page_dir == PMM_NO_FRAME_AVAILABLE)
//        return NULL;
//
//    page_directory_t *page_dir = (page_directory_t *) phys_adrr_page_dir;
//    memset(page_dir, 0, sizeof(page_directory_t)); // Clear the page directory
//    //todo copy the kernel mapping to the new page directory
//    if (current_directory != NULL)
//        for (size_t i = KERNEL_START_DIRECTORY_INDEX; i < TABLES_PER_DIR; i++) {
//            page_dir->entries[i] = current_directory->entries[i];
//        }
//    return page_dir;
//}


void enable_paging() {
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set the paging bit in cr0
    asm volatile ("mov %0, %%cr0"::"r"(cr0));
}



void vmm_switch_page_directory(page_directory_t *dir) {
    assert(dir != NULL); // Make sure the directory is not NULL
    current_directory = dir;
    load_curr_page_dir(); // Load the current page directory into the cr3 register
}



// return the page to swap out if there is no page to swap out return NULL
page_entry_t *vmm_get_page_to_swap_out() {
    return page_fifo_dequeue();
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
    while (disk_write(disk_slot, (void *) get_frame_addr(e), PAGE_SIZE) != PAGE_SIZE);
    //todo handle if the write failed allot of times

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
    physical_addr frame_addr = pmm_alloc_frame();
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

    disk_read(disk_slot, (void *) frame_addr, PAGE_SIZE);

    // add the present attribute and remove the swapped attribute
    page_entry_add_attrib(e, PRESENT);
    page_entry_remove_attrib(e, SWAPPED);
    return true;
}


bool vmm_alloc_page(page_entry_t *e) {
    //allocate physical frame
    physical_addr frame_addr = pmm_alloc_frame();
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
    page_fifo_node_t *node = (page_fifo_node_t *) kmalloc(sizeof(page_fifo_node_t));
    if (node == NULL)
        return false; // todo handle the error

    node->entry = e;
    page_fifo_enqueue(node);

    return true;
}



void vmm_free_page(page_entry_t *e) {
    pmm_free_frame(*e & 0xFFFFF000);
    *e = 0;
}



/*
 * Maps a page to a frame
 */
void vmm_map_page(void *vir_addr, physical_addr phys_addr, uint32_t flags) {
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
        } else // the table does not exist. create a new one
        {
            physical_addr phys_addr_page_table = pmm_alloc_frame();
        if (phys_addr_page_table == PMM_NO_FRAME_AVAILABLE)
        {
            if(!vmm_swap_out_some_page())
               panic("Failed to allocate a frame for the page table. how tf did we mange to get here?");
            phys_addr_page_table = pmm_alloc_frame();
        }

        page_entry_set_frame(&current_directory->entries[pd_index], phys_addr_page_table);
            page_entry_add_attrib(&current_directory->entries[pd_index], PRESENT | PAGE_WRITEABLE);
        page_table = (page_table_t *) phys_addr_page_table;
            //clear the page table (using the virtual address)

    }

    // map the page to the frame
    page_entry_t *page_entry = &page_table->entries[pt_index];
    page_entry_set_frame(page_entry, (uint32_t) phys_addr);
    page_entry_add_attrib(page_entry, flags | PRESENT);

    // todo add the flags to the table entry
}

/*
 * Unmaps a page from a frame
 * Uses FIFO to swap out the page if needed
 */
void vmm_unmap_page() {
    assert(current_directory != NULL);

}




// Error code flags for page faults
#define PRESENT_ERROR_CODE 0x1       // Page present
#define WRITE_ERROR_CODE 0x2         // Write operation caused the fault
#define USER_ERROR_CODE 0x4          // Fault occurred in user mode
#define RESERVED_ERROR_CODE 0x8      // Reserved bits set in page table entry
#define INSTRUCTION_ERROR_CODE 0x10  // Instruction fetch caused the fault
page_entry_t *vmm_get_page_entry(void *vir_addr) {
    assert(current_directory != NULL);
    uint32_t pd_index = get_directory_index(vir_addr);
    uint32_t pt_index = get_table_index(vir_addr);

    if (!is_page_present(&current_directory->entries[pd_index])) { // need to swap in the page table
        if (!vmm_swap_in_page(&current_directory->entries[pd_index]))
            panic("Failed to swap in page. Dont know what to do noq");
    }

    page_table_t *page_table = (page_table_t *) get_page_table_addr(current_directory, pd_index);
    return &page_table->entries[pt_index];
}

void page_fault_handler(uint32_t error_code) {
    put_string("Page fault");
    uint32_t fault_addr;
    asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
    page_entry_t *e = vmm_get_page_entry((void *) fault_addr);
    if (!is_page_present_error(error_code)) {
        // The page fault was caused by a page not present
        // fetch the page from disk if exits, else allocate a new frame
        // and map the page to the frame
        if (is_page_swapped(e)) {
            if (!vmm_swap_in_page(e))
                //todo Handle differently if its the user page(Probably make his life miserable)
                panic("Failed to swap in page. dont know what to do so lets shut down the computer :)");
            return; // the iret in the page fault handler will refetch the instruction
        } else {
            if (!vmm_alloc_page(e))
                //todo Handle differently if its the user page(Probably throw an error that there is now memory)
                panic("Failed to allocate a frame for the page. how tf did we mange to get here?");
            return;
        }
    } else if (!is_cow(e)) {
        //page present, if its copy on write, copy the page and map it to the new frame

        //permission error, punish the user and panic if its the kernel beacsue I dont know how we got here
        if (!is_page_user_error(error_code))
            panic("Permission error in kernel mode. how tf did we mange to get here?");
        else
            //todo punish the user
            return;
    } else
        //todo implement the copy on write
        return;

}


void vmm_init() {
    current_directory = kernel_directory = (page_directory_t *) pmm_alloc_frame();
    if (current_directory == NULL)
        panic("Failed to initialize paging : could not allocate a frame for the page directory. you really have fucked up computer");

    // Initialize the page directory entries.
    // Each entry is set to 0x00000002: Supervisor, Read/Write, Not Present.
    for (size_t i = 0; i < TABLES_PER_DIR; i++)
        page_entry_add_attrib(current_directory->entries + i, KERNEL_PAGE_FLAGS); //


    extern char _kernel_start, _kernel_end;
    size_t kernel_size = (size_t) (&_kernel_end - &_kernel_start);
    //Create frame for the kernel and map them to the lower half memory(identity mapping)
    // This is done so it would be easy to copy for new page directories the kernel mapping because we dont want to swap in the kernel mapping
//    size_t kernel_frames = (kernel_size + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t kernel_frames = 2048;
    // the allocation of the frames in pmm is done in the pmm_init function
    _kernel_start = 0;
    for (size_t i = 0; i < kernel_frames; i++)
        vmm_map_page((void *) (&_kernel_start + i * PAGE_SIZE), (physical_addr) (&_kernel_start + i * PAGE_SIZE),
                     KERNEL_PAGE_FLAGS | PRESENT);

    enable_paging();

}