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

#define NUM_CACHES 8
#define MAX_CACHE_SIZE 2048
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





void init_slab(struct slab *slab, size_t object_size) {
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
    struct slab *slab = (void *)pmm_alloc_frame();
    if (slab == NULL){
        return NULL;
    }
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


void init_kmalloc(){
    for (size_t i = 0; i < NUM_CACHES; i++){
        caches[i].slabs = create_slab(caches[i].object_size);
        if(caches[i].slabs == NULL){
            panic("Failed to allocate memory for kmalloc, what piece of shit computer do you have?\n"
                  "Its probably my fault :)");
        }
    }
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
    if(curr_slab == NULL)
        goto failed_alloc_from_cache_create_new_slab;
    while(curr_slab != NULL)
    {
        void *res = alloc_from_slab(curr_slab);
        if(res != NULL)
            return res;
        curr_slab = curr_slab->next;
    }

    //Failed to allocate, allocate new slab
    failed_alloc_from_cache_create_new_slab:
    struct slab* new_slab = create_slab(cache->object_size);
    new_slab->next = cache->slabs;
    cache->slabs = new_slab;
    return alloc_from_slab(new_slab);
}

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
