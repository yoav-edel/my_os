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
#include "../drivers/screen.h"


typedef struct page_t page_t;
static page_directory_t *current_directory = NULL;
static page_directory_t kernel_directory = {0};
static bool paging_enabled = false;

page_directory_t *vmm_get_kernel_page_directory() {
    return &kernel_directory;
}

typedef struct page_fifo_node {
    void *vir_addr;
    struct page_fifo_node *next;
} page_fifo_node_t;

typedef struct {
    page_fifo_node_t *head;
    page_fifo_node_t *tail;
    size_t count;
} page_fifo_queue_t;

static page_fifo_queue_t current_page_fifo_queue = {NULL, NULL, 0};

static page_entry_t *vmm_get_page_entry(void *vir_addr);

// ---------------------------- Helper functions ----------------------------

static inline uint32_t get_directory_index(void *vir_addr) {
    return (uint32_t) (vir_addr) >> 22;
}

static inline uint32_t get_table_index(void *vir_addr) {
    return ((uint32_t) (vir_addr) >> 12) & 0x03FF;
}

// This function can only be used if paging is enabled because it returns the virtual address of the page table
static inline uint32_t get_page_table_vir_addr(const page_directory_t *dir, uint16_t pd_index) {
    return (RECURSIVE_PAGE_TABLE_INDEX << 22) | (pd_index << 12);
}

static inline uint32_t get_frame_addr(const page_entry_t entry) {
    return entry & 0xFFFFF000;
}

static inline uint32_t get_page_table_addr(const page_directory_t *dir, uint16_t pd_index) {
    if (paging_enabled) //Todo add that this is the likely case
        return get_page_table_vir_addr(dir, pd_index);
    else
        return get_frame_addr(dir->tables[pd_index]);
}



static inline bool is_page_present(const page_entry_t entry) {
    return entry & PRESENT;
}

static inline void load_page_dir(physical_addr page_dir_addr) {
    asm volatile ("mov %0, %%cr3"::"r"(page_dir_addr));
}

static inline void page_entry_set_frame(page_entry_t *e, uint32_t frame_addr) {
    *e = (*e & ~0xFFFFF000) | frame_addr;
}

static inline void page_entry_add_attrib(page_entry_t *e, uint32_t attrib) {
    *e |= attrib;
}


static inline void page_entry_remove_attrib(page_entry_t *e, uint32_t attrib) {
    *e &= ~attrib;
}

static inline bool is_swapped(page_entry_t e) {
    return e & SWAPPED;
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

static inline uint16_t get_frame_offset(void *vir_addr) {
    return (uint16_t) vir_addr & 0xFFF;
}


static inline void flush_tlb() {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    asm volatile ("mov %0, %%cr3"::"r"(cr3));
}

static inline void flush_page(uint32_t vir_addr) {
    asm volatile ("invlpg (%0)"::"r"(vir_addr) : "memory");
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

static bool page_enqueue(void *vir_addr) {
    page_fifo_node_t *node = (page_fifo_node_t *) kmalloc(sizeof(page_fifo_node_t));
    if (node == NULL)
        return false;

    node->vir_addr = vir_addr;
    node->next = NULL;
    page_fifo_enqueue(node);
    return true;
}

static void *page_fifo_dequeue() {
    if (current_page_fifo_queue.head == NULL)
        return NULL;
    page_fifo_node_t *node = current_page_fifo_queue.head;
    current_page_fifo_queue.head = current_page_fifo_queue.head->next;
    current_page_fifo_queue.count--;
    void *vir_addr = node->vir_addr;
    kfree(node);
    return vir_addr;
}



// ---------------------------- VMM functions ----------------------------



void enable_paging() {
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set the paging bit in cr0
    asm volatile ("mov %0, %%cr0"::"r"(cr0));
    paging_enabled = true;
}


void vmm_switch_vm_context(vm_context_t *vm_context) {
    if (vm_context == NULL)
        panic("Trying to switch to a NULL vm_context, what the hell are you doing?");
    current_directory = vm_context->page_dir;
    load_page_dir(vm_context->page_dir_phys_addr);

}


// return the page to swap out if there is no page to swap out return NULL
static void *vmm_get_page_to_swap_out() {
    return page_fifo_dequeue();
}

bool vmm_swap_out_page(void *vir_addr) {
    page_entry_t *e = vmm_get_page_entry(vir_addr);
    if (!is_page_present(*e))
        return false;

    uint32_t disk_slot = disk_alloc_slot();
    if (disk_slot == NO_SLOT_AVAILABLE)
        return false;

    //write the page to the disk
    while (disk_write(disk_slot, (void *) get_frame_addr(*e), PAGE_SIZE) != PAGE_SIZE);
    //todo handle if the write failed allot of times

    // update the page entry
    page_entry_add_attrib(e, SWAPPED);
    page_entry_remove_attrib(e, PRESENT);
    page_entry_set_frame(e, disk_slot);
    flush_page((uint32_t) vir_addr);

    return true;
}


bool vmm_swap_out_some_page() {
    void *vir_addr = vmm_get_page_to_swap_out();
    if (vir_addr == NULL)
        return false;
    return vmm_swap_out_page(vir_addr);
}



/*
 * Swaps in a page from the disk
 * return true if the swap was successful, false otherwise
 */
bool vmm_swap_in_page(page_entry_t *e, uint32_t vir_addr) {
    physical_addr frame_addr = pmm_alloc_frame();
    if (frame_addr == PMM_NO_FRAME_AVAILABLE) {
        if (!vmm_swap_out_some_page())
            return false;
        frame_addr = pmm_alloc_frame();
        if (frame_addr == PMM_NO_FRAME_AVAILABLE)
            return false;
    }

    uint32_t disk_slot = get_frame_addr(*e);

    page_entry_set_frame(e, frame_addr);

    while (disk_read(disk_slot, (void *) frame_addr, PAGE_SIZE) != PAGE_SIZE);
    disk_free_slot(disk_slot);

    // add the present attribute and remove the swapped attribute
    page_entry_add_attrib(e, PRESENT);
    page_entry_remove_attrib(e, SWAPPED);
    flush_page(vir_addr);
    return true;
}
/*
 * Allocates a new page and maps it to a frame, and doeesnt add it to the pages that can't be swapped.
 * return true if the allocation was successful, false otherwise
 */
bool vmm_alloc_permanent_page(page_entry_t *e) {
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
    return true;
}

/*
 * Allocates a new page and maps it to a frame and adds it to the page fifo queue
 * return true if the allocation was successful, false otherwise
 */
bool vmm_alloc_page(page_entry_t *e, void *vir_addr) {

    if (!vmm_alloc_permanent_page(e))
        return false;

    if (!page_enqueue(vir_addr)) {
        pmm_free_frame(get_frame_addr(*e));
        page_entry_remove_attrib(e, PRESENT);
        return false; // todo handle the error
    }

    return true;
}


void vmm_free_page(page_entry_t *e) {
    pmm_free_frame(*e & 0xFFFFF000);
    *e = 0;
}


/*
 * Maps a page to a frame
 */
void vmm_map_page(page_directory_t *page_dir, void *vir_addr, physical_addr phys_addr, uint32_t flags) {
    assert(page_dir != NULL);
    uint32_t pd_index = get_directory_index(vir_addr);
    uint32_t pt_index = get_table_index(vir_addr);
    page_table_t *page_table;
    if (is_page_present(page_dir->tables[pd_index])) // the table is exist and present
        page_table = (page_table_t *) get_page_table_addr(page_dir, pd_index);

    else if (is_swapped(page_dir->tables[pd_index])) // the table is exist but swapped
    {
        if (!vmm_swap_in_page(&page_dir->tables[pd_index],
                              get_page_table_vir_addr(page_dir, pd_index)))
            panic("Failed to swap in page. how tf did we mange to get here?");
        page_table = (page_table_t *) get_page_table_addr(page_dir, pd_index);
    } else // the table does not exist. create a new one
    {
        if (!paging_enabled) {
          	// We are in the kernel space and the kernel is not swapped
            if (!vmm_alloc_permanent_page(&page_dir->tables[pd_index]))
                panic("Failed to allocate a frame for the page table. We fucked up?");
        }
        else {
            if (!vmm_alloc_page(&page_dir->tables[pd_index],
                                (void *) get_page_table_vir_addr(page_dir, pd_index)))
                panic("Failed to allocate a frame for the page table. We fucked up?");
        }
        //needs to clear the page table
        page_entry_add_attrib(&page_dir->tables[pd_index], PAGE_WRITEABLE);
        page_table = (page_table_t *) get_page_table_addr(page_dir, pd_index);
        flush_page((uint32_t) page_table);
        memset(page_table, 0, sizeof(page_table_t));

    }

    // map the page to the frame
    page_entry_t *page_entry = &page_table->entries[pt_index];
    page_entry_set_frame(page_entry, (uint32_t) phys_addr);
    page_entry_add_attrib(page_entry, flags | PRESENT);

    // todo add the flags to the table entry
}

void vmm_map_page_to_curr_dir(void *vir_addr, physical_addr frame_addr, uint32_t flags) {
    assert(current_directory != NULL);
    vmm_map_page(current_directory, vir_addr, frame_addr, flags);
}

void vmm_unmap_page(void *vir_addr) {
    assert(current_directory != NULL);
    page_entry_t *e = vmm_get_page_entry(vir_addr);
   	if(is_page_present(*e))
    {
    	pmm_free_frame(get_frame_addr(*e));
    	*e = 0;
    }
    else if(is_swapped(*e))
    {
    	uint32_t disk_slot = get_frame_addr(*e);
    	disk_free_slot(disk_slot);
    	*e = 0;
    }
    flush_page((uint32_t) vir_addr);

}




// Error code flags for page faults
#define PRESENT_ERROR_CODE 0x1       // Page present
#define WRITE_ERROR_CODE 0x2         // Write operation caused the fault
#define USER_ERROR_CODE 0x4          // Fault occurred in user mode
#define RESERVED_ERROR_CODE 0x8      // Reserved bits set in page table entry
#define INSTRUCTION_ERROR_CODE 0x10  // Instruction fetch caused the fault

/*
 * Returns the page entry of the page that contains the virtual address
 * if the page table is swapped out, it swaps it in
 * If its not present and
 *
 */
static page_entry_t *vmm_get_page_entry(void *vir_addr) {
    assert(current_directory != NULL);
    uint32_t pd_index = get_directory_index(vir_addr);
    uint32_t pt_index = get_table_index(vir_addr);

    if (is_swapped(current_directory->tables[pd_index])) { // need to swap in the page table
        if (!vmm_swap_in_page(&current_directory->tables[pd_index],
                              get_page_table_vir_addr(current_directory, pd_index)))
            panic("Failed to swap in page. Dont know what to do noq");
    }

    page_table_t *page_table = (page_table_t *) get_page_table_addr(current_directory, pd_index);
    return &page_table->entries[pt_index];
}

//todo fix this implementation - we fetch the page entry when we dont know if it exists
void page_fault_handler(uint32_t error_code) {
    uint32_t fault_addr;
    asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
    page_entry_t *e = vmm_get_page_entry((void *) fault_addr);
    if (!is_page_present_error(error_code)) {
        // The page fault was caused by a page not present
        // fetch the page from disk if exits, else allocate a new frame
        // and map the page to the frame
        if (is_swapped(*e)) { // the page is swapped so we need to swap it in
            if (!vmm_swap_in_page(e, fault_addr))
                //todo Handle differently if its the user page(Probably make his life miserable)
                panic("Failed to swap in page. dont know what to do so lets shut down the computer :)");
            return; // the iret in the page fault handler will refetch the instruction
        } else { // the page is not present and not swapped so we need to allocate a new frame
            if (!vmm_alloc_page(e, (void *) fault_addr))
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

physical_addr vmm_calc_phys_addr(void *vir_addr) {
    page_entry_t *e = vmm_get_page_entry(vir_addr);
    if (!is_page_present(*e))
        panic("Trying to calculate the physical address of a page that is not present");
    return get_frame_addr(*e) + get_frame_offset(vir_addr);
}


page_directory_t *vmm_create_empty_page_directory() {
    page_directory_t *page_dir = (page_directory_t *) kmalloc(sizeof(page_directory_t));
    if (page_dir == NULL)
        return NULL;

    memset(page_dir, 0, sizeof(page_directory_t));
    return page_dir;
}

void vmm_destroy_page_directory(page_directory_t *page_dir) {
  	//todo handle cow pages
    for (size_t i = 0; i < TABLES_PER_DIR; i++) {
        if (is_page_present(page_dir->tables[i])) {
            page_table_t *page_table = (page_table_t *) get_page_table_addr(page_dir, i);
            for (size_t j = 0; j < PAGE_TABLE_SIZE; j++) {
                if (is_page_present(page_table->entries[j]))
                    pmm_free_frame(get_frame_addr(page_table->entries[j]));
            }
            pmm_free_frame(get_frame_addr(page_dir->tables[i]));
        }
        //todo handle cow pages
        if (is_swapped(page_dir->tables[i])) {
            uint32_t disk_slot = get_frame_addr(page_dir->tables[i]);
            disk_free_slot(disk_slot);
        }
    }
    kfree(page_dir);
}

/*
    * Creates a new vm context - a new page directory
*   This function should only be used after the paging is enabled
*    copy the mapping of the page_directory to the new page directory (if null copy the kernel mapping)
*
 */
vm_context_t *vmm_create_vm_context(page_directory_t *page_dir) {
  //todo handle cow pages
    vm_context_t *vm_context = (vm_context_t *) kmalloc(sizeof(vm_context_t));
    if (vm_context == NULL)
        return NULL;

    vm_context->page_dir = vmm_create_empty_page_directory();
    // Copy the kernel mapping to the new page directory
    //todo the not kernel mapping should be copied using copy on write
    for (size_t i = 0; i < TABLES_PER_DIR - 1; i++)
        vm_context->page_dir->tables[i] = page_dir->tables[i];
    // calc the physical address of the page directory using the kernel mapping beacuse the page direcotry is saved in the kerenl space
    vm_context->page_dir_phys_addr = vmm_calc_phys_addr(vm_context->page_dir);
    // Map the recursive page table to point to the new page directory
    vm_context->page_dir->tables[RECURSIVE_PAGE_TABLE_INDEX] = vm_context->page_dir_phys_addr | KERNEL_PAGE_FLAGS;
    return vm_context;
}

void vmm_destroy_vm_context(vm_context_t *vm_context) {
    vmm_destroy_page_directory(vm_context->page_dir);
    kfree(vm_context);
}


void vmm_init() {
    current_directory = &kernel_directory;

    // Initialize the page directory entries.
    // Each entry is set to 0x00000002: Supervisor, Read/Write, Not Present.
    for (size_t i = 0; i < TABLES_PER_DIR; i++)
        page_entry_add_attrib(current_directory->tables + i, KERNEL_PAGE_FLAGS); //


    extern char _kernel_start, _kernel_end;
    extern const unsigned int _kernel_stack_top, _kernel_stack_pages_amount;
    size_t kernel_size = (size_t) (&_kernel_end - &_kernel_start);
    //Create frame for the kernel and map them to the lower half memory(identity mapping)
    // This is done so it would be easy to copy for new page directories the kernel mapping because we dont want to swap in the kernel mapping
    size_t kernel_frames = (kernel_size + PAGE_SIZE - 1) / PAGE_SIZE;

    // the allocation of the frames in pmm is done in the pmm_init function
    // todo give the wrtie permission only to the data section
    for (size_t i = 0; i < kernel_frames; i++) {
        void *addr = (void *) (&_kernel_start + i * PAGE_SIZE);
        vmm_map_page_to_curr_dir(addr, (physical_addr) addr, KERNEL_PAGE_FLAGS);
    }

    // Map the Recursive page_table to point to the page directory
    current_directory->tables[RECURSIVE_PAGE_TABLE_INDEX] = (physical_addr) current_directory | KERNEL_PAGE_FLAGS;

    // map Kernel stack
    for (size_t i = 0; i < _kernel_stack_pages_amount; i++) {
        void *addr = (void *) (_kernel_stack_top - i * PAGE_SIZE);
        vmm_map_page_to_curr_dir(addr, (physical_addr) addr, KERNEL_PAGE_FLAGS);
    }

    //Map heap address
    KERNEL_BASE_HEAP_ADDR = ALIGN_TO_PAGE((uint32_t) _kernel_stack_top);
    size_t num_pages = KERNEL_HEAP_SIZE / PAGE_SIZE;
    for (size_t i = 0; i < num_pages; i++) {
        void *addr = (void *) (KERNEL_BASE_HEAP_ADDR + i * PAGE_SIZE);
        vmm_map_page_to_curr_dir(addr, (physical_addr) addr, KERNEL_PAGE_FLAGS);
    }

    //Map the VGA buffer
    vmm_map_page_to_curr_dir((void *) VGA_ADDRESS, (physical_addr) VGA_ADDRESS, PRESENT | PAGE_WRITEABLE);

    // load the physical address of the kernel page directory
    load_page_dir((physical_addr) &kernel_directory);
    enable_paging();
}