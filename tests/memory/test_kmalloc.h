//
// Created by Tests on 2025.
//

/*
 * Kernel Memory Allocator (KMALLOC) Tests
 * Tests kernel memory allocation and deallocation
 */

#include "../test_framework.h"
#include "../../memory/kmalloc.h"
#include "../../std/assert.h"
#include "../../std/string.h"
#include "../../std/stdint.h"

// Test KMALLOC initialization
bool test_kmalloc_init_basic(void) {
    TEST_START("KMALLOC Init Basic");
    
    // KMALLOC init should be callable without crashing
    init_kmalloc();
    
    // After init, we should be able to allocate memory
    void *ptr = kmalloc(16);
    ASSERT_NOT_NULL(ptr, "Should be able to allocate memory after init");
    
    // Clean up
    kfree(ptr);
    
    TEST_END("KMALLOC Init Basic");
    return true;
}

// Test basic memory allocation
bool test_kmalloc_basic_allocation(void) {
    TEST_START("KMALLOC Basic Allocation");
    
    // Test various allocation sizes
    size_t sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048};
    void *ptrs[8];
    
    for (int i = 0; i < 8; i++) {
        ptrs[i] = kmalloc(sizes[i]);
        ASSERT_NOT_NULL(ptrs[i], "Allocation should succeed for all cache sizes");
        
        // Check alignment (should be at least aligned to pointer size)
        ASSERT_TRUE(((uint32_t)ptrs[i] % sizeof(void*)) == 0, "Allocated memory should be properly aligned");
    }
    
    // Clean up
    for (int i = 0; i < 8; i++) {
        kfree(ptrs[i]);
    }
    
    TEST_END("KMALLOC Basic Allocation");
    return true;
}

// Test memory freeing
bool test_kmalloc_free_basic(void) {
    TEST_START("KMALLOC Free Basic");
    
    // Allocate memory
    void *ptr = kmalloc(64);
    ASSERT_NOT_NULL(ptr, "Allocation should succeed");
    
    // Free the memory (should not crash)
    kfree(ptr);
    ASSERT_TRUE(true, "Free completed without crash");
    
    // Test freeing NULL (should not crash)
    kfree(NULL);
    ASSERT_TRUE(true, "Freeing NULL completed without crash");
    
    TEST_END("KMALLOC Free Basic");
    return true;
}

// Test allocation and free cycle
bool test_kmalloc_alloc_free_cycle(void) {
    TEST_START("KMALLOC Alloc-Free Cycle");
    
    // Allocate and free multiple times
    for (int cycle = 0; cycle < 5; cycle++) {
        void *ptrs[10];
        
        // Allocate multiple blocks
        for (int i = 0; i < 10; i++) {
            ptrs[i] = kmalloc(64);
            ASSERT_NOT_NULL(ptrs[i], "Allocation should succeed in cycle");
            
            // Verify uniqueness (no overlapping allocations)
            for (int j = 0; j < i; j++) {
                ASSERT_NEQ(ptrs[j], ptrs[i], "Allocations should be unique");
            }
        }
        
        // Free all blocks
        for (int i = 0; i < 10; i++) {
            kfree(ptrs[i]);
        }
    }
    
    TEST_END("KMALLOC Alloc-Free Cycle");
    return true;
}

// Test small allocations (slab allocator)
bool test_kmalloc_small_allocations(void) {
    TEST_START("KMALLOC Small Allocations");
    
    // Test allocations that should use the slab allocator
    size_t small_sizes[] = {1, 8, 15, 16, 17, 31, 32, 33, 63, 64};
    void *ptrs[10];
    
    for (int i = 0; i < 10; i++) {
        ptrs[i] = kmalloc(small_sizes[i]);
        ASSERT_NOT_NULL(ptrs[i], "Small allocation should succeed");
    }
    
    // Clean up
    for (int i = 0; i < 10; i++) {
        kfree(ptrs[i]);
    }
    
    TEST_END("KMALLOC Small Allocations");
    return true;
}

// Test large allocations (beyond cache sizes)
bool test_kmalloc_large_allocations(void) {
    TEST_START("KMALLOC Large Allocations");
    
    // Test allocations larger than MAX_CACHE_SIZE (2048)
    size_t large_sizes[] = {4096, 8192, 16384};
    void *ptrs[3];
    
    for (int i = 0; i < 3; i++) {
        ptrs[i] = kmalloc(large_sizes[i]);
        ASSERT_NOT_NULL(ptrs[i], "Large allocation should succeed");
    }
    
    // Clean up
    for (int i = 0; i < 3; i++) {
        kfree(ptrs[i]);
    }
    
    TEST_END("KMALLOC Large Allocations");
    return true;
}

// Test zero allocation
bool test_kmalloc_zero_allocation(void) {
    TEST_START("KMALLOC Zero Allocation");
    
    // Allocate zero bytes
    void *ptr = kmalloc(0);
    // Behavior may vary - some allocators return NULL, others return a valid pointer
    // Just ensure it doesn't crash
    ASSERT_TRUE(true, "Zero allocation completed without crash");
    
    if (ptr != NULL) {
        kfree(ptr);
    }
    
    TEST_END("KMALLOC Zero Allocation");
    return true;
}

// Test double free (edge case)
bool test_kmalloc_double_free(void) {
    TEST_START("KMALLOC Double Free");
    
    // Allocate memory
    void *ptr = kmalloc(64);
    ASSERT_NOT_NULL(ptr, "Allocation should succeed");
    
    // Free once
    kfree(ptr);
    
    // Free again (should not crash, though behavior may vary)
    kfree(ptr);
    ASSERT_TRUE(true, "Double free completed without crash");
    
    TEST_END("KMALLOC Double Free");
    return true;
}

// Test memory write and read
bool test_kmalloc_memory_access(void) {
    TEST_START("KMALLOC Memory Access");
    
    // Allocate memory for a test pattern
    size_t size = 64;
    char *ptr = (char*)kmalloc(size);
    ASSERT_NOT_NULL(ptr, "Allocation should succeed");
    
    // Write a pattern to memory
    for (size_t i = 0; i < size; i++) {
        ptr[i] = (char)(i % 256);
    }
    
    // Read back and verify
    for (size_t i = 0; i < size; i++) {
        char expected = (char)(i % 256);
        char actual = ptr[i];
        ASSERT_TRUE(expected == actual, "Memory content should match written pattern");
    }
    
    // Clean up
    kfree(ptr);
    
    TEST_END("KMALLOC Memory Access");
    return true;
}

// Test allocation of all cache sizes
bool test_kmalloc_all_cache_sizes(void) {
    TEST_START("KMALLOC All Cache Sizes");
    
    // Test exact cache sizes from kmalloc.h
    size_t cache_sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048};
    void *ptrs[NUM_CACHES];
    
    // Allocate one of each cache size
    for (int i = 0; i < NUM_CACHES; i++) {
        ptrs[i] = kmalloc(cache_sizes[i]);
        ASSERT_NOT_NULL(ptrs[i], "Cache size allocation should succeed");
    }
    
    // Free all
    for (int i = 0; i < NUM_CACHES; i++) {
        kfree(ptrs[i]);
    }
    
    TEST_END("KMALLOC All Cache Sizes");
    return true;
}

// Test memory boundaries (stress test)
bool test_kmalloc_stress_test(void) {
    TEST_START("KMALLOC Stress Test");
    
    const int num_allocs = 50;
    void *ptrs[num_allocs];
    
    // Allocate many blocks of varying sizes
    for (int i = 0; i < num_allocs; i++) {
        size_t size = 16 + (i % 8) * 32;  // Sizes from 16 to 240
        ptrs[i] = kmalloc(size);
        ASSERT_NOT_NULL(ptrs[i], "Stress test allocation should succeed");
    }
    
    // Free every other block
    for (int i = 0; i < num_allocs; i += 2) {
        kfree(ptrs[i]);
        ptrs[i] = NULL;
    }
    
    // Allocate new blocks in freed spaces
    for (int i = 0; i < num_allocs; i += 2) {
        ptrs[i] = kmalloc(32);
        ASSERT_NOT_NULL(ptrs[i], "Re-allocation should succeed");
    }
    
    // Free all remaining blocks
    for (int i = 0; i < num_allocs; i++) {
        if (ptrs[i] != NULL) {
            kfree(ptrs[i]);
        }
    }
    
    TEST_END("KMALLOC Stress Test");
    return true;
}

// Run all KMALLOC tests
void run_kmalloc_tests(void) {
    printf("\n" COLOR_BLUE "=== Running KMALLOC Tests ===" COLOR_RESET "\n");
    
    RUN_TEST(test_kmalloc_init_basic);
    RUN_TEST(test_kmalloc_basic_allocation);
    RUN_TEST(test_kmalloc_free_basic);
    RUN_TEST(test_kmalloc_alloc_free_cycle);
    RUN_TEST(test_kmalloc_small_allocations);
    RUN_TEST(test_kmalloc_large_allocations);
    RUN_TEST(test_kmalloc_zero_allocation);
    RUN_TEST(test_kmalloc_double_free);
    RUN_TEST(test_kmalloc_memory_access);
    RUN_TEST(test_kmalloc_all_cache_sizes);
    RUN_TEST(test_kmalloc_stress_test);
}