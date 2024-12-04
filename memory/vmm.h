// vmm.h
#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stdbool.h>

// Page size
#define PAGE_SIZE           4096    // 4 KB

// Entries per page table/directory
#define ENTRIES_PER_TABLE   1024

/*
 * Page table structure
 * 1024 entries of 4 bytes each - 4 KB in total
 * structures are aligned to 4 KB
 * layout:
 * BIT 0: Present - 1 if page is present in memory 0 if not
 * BIT 1: Read/Write - 1 if page is writeable(and readble) 0 if read-only
 * BIT 2: User/Supervisor - 1 if page is accessible by user-mode(and kernel-mode) 0 if kernel-mode only
 * BIT 3: Write-Through - 1 if write-through caching enabled. if the WT bit is 0, the processor uses write-back caching
 * BIT 4: Cache Disable - 1 if page-level cache disable 0 if cache enabled
 * BIT 5: Accessed - 1 if page has been accessed 0 if not
 * BIT 6: Dirty - 1 if page has been written to 0 if not
 * BIT 7: Page Size - 1 if page size bit. Page is 4MB 0 if 4KB(defualt is 4KB)
 * BIT 8: Global - 1 if global page. TLB entries are not invalidated on CR3 writes
 * BIT 9: Swapped - 1 if page is swapped 0 if not. if the page
 * BIT 10-11: Available for use.
 * BIT 12-31: Page Table Base Address - 20 bits. if swapped the address of the slot address
 */
typedef uint32_t page_entry_t;

typedef struct {
    page_entry_t entries[ENTRIES_PER_TABLE];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;


/*
 * Page directory structure
 * 1024 entries of 4 bytes each - 4 KB in total
 * structures are aligned to 4 KB
 * layout of page directory entry:
 * BIT 0: Present - 1 if page is present in memory 0 if not
 * BIT 1: Read/Write - 1 if page is writeable(and readble) 0 if read-only
 * BIT 2: User/Supervisor - 1 if page is accessible by user-mode(and kernel-mode) 0 if kernel-mode only
 * BIT 3: Write-Through - 1 if write-through caching enabled. if the WT bit is 0, the processor uses write-back caching
 * BIT 4: Cache Disable - 1 if page-level cache disable 0 if cache enabled
 * BIT 5: Accessed - 1 if page has been accessed 0 if not
 * BIT 6: Dirty - 1 if page has been written to 0 if not
 * BIT 7: Page Size - 1 if page size bit. Page is 4MB 0 if 4KB(defualt is 4KB)
 * BIT 8: Global - 1 if global page. TLB entries are not invalidated on CR3 writes
 * BIT 9: Swapped - 1 if page is swapped 0 if not. if the page
 * BIT 10-11: Available for use.
 * BIT 12-31: Page Table Base Address - 20 bits. if swapped the address of the slot address
 */
typedef struct {
    page_entry_t entries[ENTRIES_PER_TABLE];
} __attribute__((aligned(PAGE_SIZE))) page_directory_t;

#define PAGE_DIR_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define PRESENT 0x1 // page present in memory
#define PAGE_WRITEABLE 0x2// page is writeable
#define PAGE_USER 0x4 // page is accessible by user-mode(and kernel-mode)
#define WRITE_THROUGH 0x8 // write-through caching enabled. if the WT bit is 0, the processor uses write-back caching
#define CACHE_DISABLE 0x10 // page-level cache disable
#define ACCESSED 0x20 // page has been accessed
#define DIRTY 0x40 // page has been written to
#define PAGE_SIZE 0x80 // page size bit. Page is 4MB
#define GLOBAL 0x100 // global page. TLB entries are not invalidated on CR3 writes
#define SWAPPED 0x200 // page is swapped
#define EMPTY_USER_PAGE_DIR_FLAGS (PAGE_WRITEABLE | PAGE_USER)
#define EMPTY_KERNEL_PAGE_DIR_FLAGS (PAGE_WRITEABLE)

#define KERNEL_START_DIRECTORY_INDEX 768

inline uint32_t get_directory_index(void *vir_addr)
{
    return (uint32_t)(vir_addr) >> 22;
}

inline uint32_t get_table_index(void *vir_addr)
{
    return ((uint32_t)(vir_addr) >> 12 )& 0x03FF;
}

inline uint32_t get_page_table_addr(page_directory_t *dir, uint32_t pd_index)
{
    // the address is the 20 most significant bits of the entry
    return dir->entries[pd_index] & 0xFFFFF000;
}

inline uint32_t get_frame_addr(page_entry_t *entry)
{
    return *entry & 0xFFFFF000;
}



inline bool is_page_present(page_entry_t *entry)
{
    return *entry & PRESENT;
}

inline bool is_table_present(page_table_t *entry)
{
    return *entry & PRESENT;
}



void vmm_init();
void vmm_switch_page_directory(page_directory_t *dir);

#endif // VMM_H