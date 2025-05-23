//
// Created by Yoav on 2/21/2025.
//

/*
 * This file is a basic implementation of a kernel memory allocator.
 * Its base on slab allocator.
 */


#include "kmalloc.h"
#include "../std/stdbool.h"
#include "../std/assert.h"
#include "pmm.h"
#include "../errors.h"
#include "vmm.h"


#define MIN_NUM_HEAPS 4
uint32_t KERNEL_BASE_HEAP_ADDR;
static uint32_t current_heap_addr = 0;
static uint32_t current_large_heap_addr = 0;
// this values represents the start of the kernel heap address
static const int MIN_SIZE = 16;

struct slab {
    size_t object_size;      // Size of objects in this slab
    void *free_list;         // Pointer to the first free object
    size_t offset;           // Offset to the next free object
    size_t free_count;       // Number of free objects
    struct slab *next;       // Pointer to the next slab
    uint8_t data[PMM_BLOCK_SIZE -  sizeof(size_t) - sizeof(void *) - 2 * sizeof(size_t) - sizeof(struct slab *)];
};

struct cache {
    size_t object_size;      // Size of objects in this cache
    struct slab *slabs;      // Linked list of slabs
};

typedef struct large_alloc {
    size_t size;             // Size of the allocation
    void *ptr;              // Pointer to the allocated memory
    struct large_alloc *next; // Pointer to the next large allocation
    struct large_alloc *prev; // Pointer to the previous large allocation

} large_alloc_t;

static large_alloc_t *large_allocations = NULL; /**
 * @brief Allocates and initializes a metadata record for a large memory allocation.
 *
 * Creates a new `large_alloc_t` structure to track a large allocation of the specified size and pointer.
 * Panics if memory for the metadata cannot be allocated.
 *
 * @param size The size of the large allocation in bytes.
 * @param ptr Pointer to the allocated memory block.
 * @return Pointer to the initialized `large_alloc_t` structure.
 */

static large_alloc_t *create_large_alloc(size_t size, void *ptr) {
	large_alloc_t *alloc = (large_alloc_t *) kmalloc(sizeof(large_alloc_t));
    if (alloc == NULL) {
       	panic("Failed to allocate memory for large allocation");
        return NULL;
    }
    alloc->size = size;
    alloc->ptr = ptr;
    alloc->next = NULL;
    alloc->prev = NULL;
    return alloc;
}
/**
 * @brief Frees the metadata structure for a large memory allocation.
 *
 * Assumes the associated memory has already been freed and the allocation is not in the tracking list.
 *
 * @param alloc Pointer to the large allocation metadata to destroy.
 */
static void large_alloc_destroy(large_alloc_t *alloc) {
    if (alloc == NULL) {
        return;
    }
    //Does not free beacuse its already freeed
    kfree(alloc);
}



/**
 * @brief Inserts a large allocation record at the head of the large allocations list.
 *
 * Updates the doubly linked list of large allocations to include the given allocation as the new head.
 */
static void insert_large_alloc(large_alloc_t *alloc) {
    if (large_allocations == NULL)
        large_allocations = alloc;
    else {
        large_alloc_t *current = large_allocations;
        current->prev = alloc;
        alloc->next = current;
        large_allocations = alloc;
    }

}

/**
 * @brief Removes a large allocation from the global linked list and destroys its metadata.
 *
 * If the provided allocation is NULL, the function does nothing. The memory pointed to by the allocation is not freed; only the allocation's metadata is destroyed.
 */
static void large_alloc_remove(large_alloc_t *alloc) {
    if (alloc == NULL)
        return;

    if (alloc->prev != NULL)
        alloc->prev->next = alloc->next;
    else
        large_allocations = alloc->next;
    if (alloc->next != NULL)
        alloc->next->prev = alloc->prev;
	alloc->next = NULL;
    alloc->prev = NULL;
    alloc->ptr = NULL;
    large_alloc_destroy(alloc);
}

/**
 * @brief Searches for a large allocation record matching the given pointer.
 *
 * Iterates through the list of large allocations to find the allocation whose memory pointer matches the specified address.
 *
 * @param ptr Pointer to the memory block to search for.
 * @return Pointer to the corresponding large_alloc_t record, or NULL if not found.
 */
static large_alloc_t *get_large_alloc_by_ptr(void *ptr) {
    large_alloc_t *current = large_allocations;
    while (current != NULL) {
        if (current->ptr == ptr) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}


static struct cache caches[NUM_CACHES] = {
        {16, NULL},
        {32, NULL},
        {64, NULL},
        {128, NULL},
        {256, NULL},
        {512, NULL},
        {1024, NULL},
        {2048, NULL}
};


static void *convert_to_vir_addr(physical_addr addr) {
    return (void *) (addr + KERNEL_BASE_HEAP_ADDR);
}


static void init_slab(struct slab *slab, size_t object_size) {
    size_t available_size = sizeof(slab->data);
    size_t num_objects = available_size / object_size;
    slab->free_count = num_objects;
    slab->free_list = slab->data;
    void **current = (void **)slab->free_list;
    for (size_t i = 1; i < num_objects; i++) {
        void *next_obj = (void *)((uint8_t *)slab->data + i * object_size);
        *current = next_obj;
        current = (void **)next_obj;
    }
    *current = NULL;  // End of free list
}


void *alloc_from_slab(struct slab *slab) {
    if (slab->free_count == 0) {
        return NULL;
    }
    void *object = slab->free_list;
    slab->free_list = *(void **)object;
    slab->free_count--;
    return object;
}

/**
 * @brief Frees an object back to the specified slab's free list.
 *
 * Returns the given object to the slab, making it available for future allocations.
 *
 * @param slab Pointer to the slab from which the object was originally allocated.
 * @param object Pointer to the object being freed.
 */
void slab_free(struct slab *slab, void *object){
    *(void **)object = slab->free_list;
    slab->free_list = object;
    slab->free_count++;
}

/**
 * @brief Allocates and initializes a new slab for objects of the specified size.
 *
 * Reserves a physical memory block, maps it to virtual memory, and sets up the slab structure for use in the slab allocator.
 *
 * @param object_size Size of each object to be stored in the slab.
 * @return Pointer to the initialized slab structure.
 */
struct slab *create_slab(size_t object_size){
    assert(current_heap_addr <= KERNEL_BASE_HEAP_ADDR + KERNEL_HEAP_SIZE);
    //todo handle alocation of not continous memory
    physical_addr slab_phys_addr = current_heap_addr;
    current_heap_addr += PMM_BLOCK_SIZE;
    void *slab_vir_addr = convert_to_vir_addr(slab_phys_addr);
    vmm_map_page_to_curr_dir(slab_vir_addr, slab_phys_addr, PAGE_WRITEABLE);

    struct slab *slab = (struct slab *) slab_vir_addr;
    slab->object_size = object_size;
    slab->offset = 0;
    slab->next = NULL;
    init_slab(slab, object_size);
    return slab;
}

/**
 * @brief Returns the smallest cache capable of holding objects of the given size.
 *
 * Searches the available caches and returns a pointer to the first cache whose object size is greater than or equal to the requested size. Returns NULL if no suitable cache exists.
 *
 * @param size The minimum object size required.
 * @return Pointer to a suitable cache, or NULL if none found.
 */
static struct cache *get_cache(size_t size) {
    for (size_t i = 0; i < NUM_CACHES; i++) {
        if (caches[i].object_size >= size) {
            return &caches[i];
        }
    }
    return NULL;
}

/**
 * @brief Returns the index of the smallest slab size that can accommodate the given size.
 *
 * @param size The requested allocation size in bytes.
 * @return int8_t Index of the appropriate slab size, or -1 if no suitable slab exists.
 */
static int8_t get_slab_index(size_t size)
{
    static const size_t block_sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048};
    for (size_t i = 0; i < sizeof(block_sizes) / sizeof(block_sizes[0]); i++) {
        if (size <= block_sizes[i]) {
            return i;
        }
    }
    return -1; // Not possible
}


/**
 * @brief Allocates an object from the specified cache.
 *
 * Attempts to allocate an object from the existing slabs in the cache. If all slabs are full or none exist, creates a new slab and allocates an object from it.
 *
 * @param cache Pointer to the cache from which to allocate an object.
 * @return Pointer to the allocated object, or NULL if allocation fails.
 */
void* alloc_from_cache(struct cache* cache)
{
    struct slab* curr_slab = cache->slabs;
    struct slab *new_slab;  // Declaration moved to the top.

    if (curr_slab == NULL)
        goto failed_alloc_from_cache_create_new_slab;
    while (curr_slab != NULL)
    {
        void *res = alloc_from_slab(curr_slab);
        if (res != NULL)
            return res;
        curr_slab = curr_slab->next;
    }

    failed_alloc_from_cache_create_new_slab:
    // Failed to allocate, so allocate a new slab.
    new_slab = create_slab(cache->object_size);
    if (new_slab == NULL)  // Check if slab creation failed.
        return NULL;
    new_slab->next = cache->slabs;
    cache->slabs = new_slab;
    return alloc_from_slab(new_slab);
}

/**
 * @brief Allocates a large memory block using page-aligned physical frames.
 *
 * Rounds the requested size up to the nearest multiple of the physical memory block size,
 * allocates and maps the required number of pages in the large allocation heap region,
 * and tracks the allocation in the large allocations list. Returns NULL if allocation fails.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory block, or NULL on failure.
 */
static void* kmalloc_large(size_t size)
{
    size_t aligned_size = (size + PMM_BLOCK_SIZE - 1) & ~(PMM_BLOCK_SIZE - 1);

    for(size_t i = 0; i < aligned_size / PMM_BLOCK_SIZE; i++)
    {
      uint32_t vir_addr = current_large_heap_addr + i * PMM_BLOCK_SIZE;
      physical_addr addr = pmm_alloc_frame();
        if (addr == PMM_NO_FRAME_AVAILABLE) {
           	// unmap the pages that were already allocated
            for (size_t j = 0; j < i; j++)
                vmm_unmap_page((void *)(current_large_heap_addr + j * PMM_BLOCK_SIZE));
            return NULL;
        }
      vmm_map_page_to_curr_dir((void *)vir_addr, addr, PAGE_WRITEABLE);//todo add the right flags
    }

    void *ptr = (void *)current_large_heap_addr;
    current_large_heap_addr += aligned_size;
    insert_large_alloc(create_large_alloc(size, ptr));
    return ptr;
}

/**
 * @brief Frees a large memory allocation previously allocated by kmalloc_large.
 *
 * Unmaps all pages associated with the given pointer and removes the allocation from the large allocations list.
 *
 * @param ptr Pointer to the start of the large allocation to free.
 * @return true if the allocation was found and freed; false if the pointer is NULL or not tracked as a large allocation.
 */
static bool _kmalloc_large_free(void *ptr)
{
    if (ptr == NULL)
        return false;

    large_alloc_t *alloc = get_large_alloc_by_ptr(ptr);
    if (alloc == NULL)
        return false;
    size_t size = alloc->size;
    size_t aligned_size = (size + PMM_BLOCK_SIZE - 1) & ~(PMM_BLOCK_SIZE - 1);
    for (size_t i = 0; i < aligned_size / PMM_BLOCK_SIZE; i++) {
        uint32_t addr = (uint32_t)ptr + i * PMM_BLOCK_SIZE;
        vmm_unmap_page((void *)addr);
    }
    large_alloc_remove(alloc);
    return true;
}


/**
 * @brief Allocates a block of memory of the specified size from the kernel heap.
 *
 * Selects the appropriate allocation strategy based on the requested size: uses a slab allocator for small allocations and a separate large allocation mechanism for sizes exceeding the maximum cache size. Panics if no suitable cache is found for the requested size.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory block, or NULL if allocation fails for large requests.
 */
void* kmalloc(size_t size)
{
    if(size > MAX_CACHE_SIZE) // less likely
        return kmalloc_large(size);
    struct cache* cache = get_cache(size);
    if(cache == NULL)
    {
        panic("Cache for slab was not found, how tf did we get here?");
    }
    return alloc_from_cache(cache);
}

/**
 * @brief Frees memory previously allocated by kmalloc.
 *
 * Determines whether the pointer refers to a large allocation or a slab allocation and frees it accordingly. Does nothing if the pointer is NULL.
 *
 * @param ptr Pointer to the memory block to free.
 */
void kfree(void *ptr) {
    if (ptr == NULL)
        return;
    if (ptr < (void *)KERNEL_BASE_HEAP_ADDR || ptr > (void *)(KERNEL_BASE_HEAP_ADDR + KERNEL_HEAP_SIZE))
         _kmalloc_large_free(ptr);
    else{
      struct slab *slab = (struct slab *) ((size_t) ptr & ~(PMM_BLOCK_SIZE - 1));
      slab_free(slab, ptr);
    }
}


/**
 * @brief Initializes the kernel memory allocator and its caches.
 *
 * Sets up the initial heap addresses and creates the first slab for each cache size category. Panics if slab allocation fails.
 */
void init_kmalloc() {
    // The heap is already mapped in the vmm_init function
    current_heap_addr = KERNEL_BASE_HEAP_ADDR;
    current_large_heap_addr = KERNEL_BASE_HEAP_ADDR + KERNEL_HEAP_SIZE;
    for (size_t i = 0; i < NUM_CACHES; i++) {
        caches[i].slabs = create_slab(caches[i].object_size);
        if (caches[i].slabs == NULL) {
            panic("Failed to allocate memory for kmalloc, what piece of shit computer do you have?\n"
                  "Its probably my fault :)");
        }
    }
}
