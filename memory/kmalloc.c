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



static struct cache caches[NUM_CACHES] = {
        {16, NULL},
        {32, NULL},
        {64, NULL},
        {128, NULL},
        {256, NULL},
        {512, NULL},
        {1024, NULL},
        {2048, NULL},
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

void slab_free(struct slab *slab, void *object){
    *(void **)object = slab->free_list;
    slab->free_list = object;
    slab->free_count++;
}

struct slab *create_slab(size_t object_size){
    assert(current_heap_addr < KERNEL_BASE_HEAP_ADDR + KERNEL_HEAP_SIZE);
    //todo handle alocation of not continous memory
    physical_addr slab_phys_addr = current_heap_addr;
    current_heap_addr += PMM_BLOCK_SIZE;
    void *slab_vir_addr = convert_to_vir_addr(slab_phys_addr);
    vmm_map_page(slab_vir_addr, slab_phys_addr, PAGE_WRITEABLE);

    struct slab *slab = (struct slab *) slab_vir_addr;
    slab->object_size = object_size;
    slab->offset = 0;
    slab->next = NULL;
    init_slab(slab, object_size);
    return slab;
}

static struct cache *get_cache(size_t size) {
    for (size_t i = 0; i < NUM_CACHES; i++) {
        if (caches[i].object_size >= size) {
            return &caches[i];
        }
    }
    return NULL;
}

static size_t get_slab_index(size_t size)
{
    static const size_t block_sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048};
    for (size_t i = 0; i < sizeof(block_sizes) / sizeof(block_sizes[0]); i++) {
        if (size <= block_sizes[i]) {
            return i;
        }
    }
    return -1; // Not possible
}


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


// todo handle large allocations requests
static void* _kamlloc_large(size_t size)
{
    return NULL;
}


void* kmalloc(size_t size)
{
    if(size > MAX_CACHE_SIZE) // less likely
        return _kamlloc_large(size);
    struct cache* cache = get_cache(size);
    if(cache == NULL)
    {
        panic("Cache for slab was not found, how tf did we get here?");
    }
    return alloc_from_cache(cache);
}

void kfree(void *ptr) {
    //Lets hope that ptr is actually a valid address :)
    struct slab *slab = (struct slab *) ((size_t) ptr & ~(PMM_BLOCK_SIZE - 1));
    slab_free(slab, ptr);
}


void init_kmalloc() {
    // The heap is already mapped in the vmm_init function
    current_heap_addr = KERNEL_BASE_HEAP_ADDR;
    for (size_t i = 0; i < NUM_CACHES; i++) {
        caches[i].slabs = create_slab(caches[i].object_size);
        if (caches[i].slabs == NULL) {
            panic("Failed to allocate memory for kmalloc, what piece of shit computer do you have?\n"
                  "Its probably my fault :)");
        }
    }
}
