//
// Created by Tests on 2025.
//

/*
 * Physical Memory Manager (PMM) Tests
 * Tests the basic building blocks of physical memory allocation
 */

#include "../test_framework.h"
#include "../../memory/pmm.h"
#include "../../std/assert.h"

// Test PMM initialization
bool test_pmm_init_basic(void) {
    TEST_START("PMM Init Basic");
    
    // PMM init should be callable without crashing
    pmm_init();
    
    // After init, we should be able to allocate at least one frame
    physical_addr frame = pmm_alloc_frame();
    ASSERT_NEQ(PMM_NO_FRAME_AVAILABLE, frame, "Should be able to allocate at least one frame after init");
    ASSERT_TRUE((frame % PMM_BLOCK_SIZE) == 0, "Allocated frame should be aligned to block size");
    
    // Clean up
    pmm_free_frame(frame);
    
    TEST_END("PMM Init Basic");
    return true;
}

// Test basic frame allocation
bool test_pmm_alloc_frame_basic(void) {
    TEST_START("PMM Alloc Frame Basic");
    
    // Allocate a frame
    physical_addr frame1 = pmm_alloc_frame();
    ASSERT_NEQ(PMM_NO_FRAME_AVAILABLE, frame1, "First frame allocation should succeed");
    ASSERT_TRUE((frame1 % PMM_BLOCK_SIZE) == 0, "Frame should be aligned to PMM_BLOCK_SIZE");
    
    // Allocate another frame
    physical_addr frame2 = pmm_alloc_frame();
    ASSERT_NEQ(PMM_NO_FRAME_AVAILABLE, frame2, "Second frame allocation should succeed");
    ASSERT_NEQ(frame1, frame2, "Different allocations should return different frames");
    
    // Clean up
    pmm_free_frame(frame1);
    pmm_free_frame(frame2);
    
    TEST_END("PMM Alloc Frame Basic");
    return true;
}

// Test frame freeing
bool test_pmm_free_frame_basic(void) {
    TEST_START("PMM Free Frame Basic");
    
    // Allocate a frame
    physical_addr frame = pmm_alloc_frame();
    ASSERT_NEQ(PMM_NO_FRAME_AVAILABLE, frame, "Frame allocation should succeed");
    
    // Frame should not be free when allocated
    ASSERT_FALSE(pmm_is_frame_free(frame), "Allocated frame should not be marked as free");
    
    // Free the frame
    pmm_free_frame(frame);
    
    // Frame should be free after freeing
    ASSERT_TRUE(pmm_is_frame_free(frame), "Frame should be marked as free after freeing");
    
    TEST_END("PMM Free Frame Basic");
    return true;
}

// Test frame status checking
bool test_pmm_is_frame_free_basic(void) {
    TEST_START("PMM Is Frame Free Basic");
    
    // Allocate a frame
    physical_addr frame = pmm_alloc_frame();
    ASSERT_NEQ(PMM_NO_FRAME_AVAILABLE, frame, "Frame allocation should succeed");
    
    // Check that allocated frame is not free
    ASSERT_FALSE(pmm_is_frame_free(frame), "Allocated frame should not be free");
    
    // Free the frame
    pmm_free_frame(frame);
    
    // Check that freed frame is free
    ASSERT_TRUE(pmm_is_frame_free(frame), "Freed frame should be free");
    
    TEST_END("PMM Is Frame Free Basic");
    return true;
}

// Test allocation and freeing cycle
bool test_pmm_alloc_free_cycle(void) {
    TEST_START("PMM Alloc-Free Cycle");
    
    physical_addr frames[10];
    
    // Allocate multiple frames
    for (int i = 0; i < 10; i++) {
        frames[i] = pmm_alloc_frame();
        ASSERT_NEQ(PMM_NO_FRAME_AVAILABLE, frames[i], "Frame allocation should succeed");
        
        // Check uniqueness
        for (int j = 0; j < i; j++) {
            ASSERT_NEQ(frames[j], frames[i], "All allocated frames should be unique");
        }
    }
    
    // Free all frames
    for (int i = 0; i < 10; i++) {
        pmm_free_frame(frames[i]);
        ASSERT_TRUE(pmm_is_frame_free(frames[i]), "Frame should be free after freeing");
    }
    
    // Re-allocate frames (should work again)
    for (int i = 0; i < 10; i++) {
        physical_addr new_frame = pmm_alloc_frame();
        ASSERT_NEQ(PMM_NO_FRAME_AVAILABLE, new_frame, "Re-allocation should succeed");
        pmm_free_frame(new_frame);
    }
    
    TEST_END("PMM Alloc-Free Cycle");
    return true;
}

// Test edge case: double free (should not crash)
bool test_pmm_double_free(void) {
    TEST_START("PMM Double Free");
    
    // Allocate a frame
    physical_addr frame = pmm_alloc_frame();
    ASSERT_NEQ(PMM_NO_FRAME_AVAILABLE, frame, "Frame allocation should succeed");
    
    // Free once
    pmm_free_frame(frame);
    ASSERT_TRUE(pmm_is_frame_free(frame), "Frame should be free after first free");
    
    // Free again (should not crash, though behavior may vary)
    pmm_free_frame(frame);  // This should not crash
    ASSERT_TRUE(pmm_is_frame_free(frame), "Frame should still be free after double free");
    
    TEST_END("PMM Double Free");
    return true;
}

// Test alignment of allocated frames
bool test_pmm_frame_alignment(void) {
    TEST_START("PMM Frame Alignment");
    
    // Allocate several frames and check alignment
    for (int i = 0; i < 5; i++) {
        physical_addr frame = pmm_alloc_frame();
        ASSERT_NEQ(PMM_NO_FRAME_AVAILABLE, frame, "Frame allocation should succeed");
        ASSERT_TRUE((frame % PMM_BLOCK_SIZE) == 0, "Frame should be aligned to PMM_BLOCK_SIZE");
        pmm_free_frame(frame);
    }
    
    TEST_END("PMM Frame Alignment");
    return true;
}

// Run all PMM tests
void run_pmm_tests(void) {
    printf("\n" COLOR_BLUE "=== Running PMM Tests ===" COLOR_RESET "\n");
    
    RUN_TEST(test_pmm_init_basic);
    RUN_TEST(test_pmm_alloc_frame_basic);
    RUN_TEST(test_pmm_free_frame_basic);
    RUN_TEST(test_pmm_is_frame_free_basic);
    RUN_TEST(test_pmm_alloc_free_cycle);
    RUN_TEST(test_pmm_double_free);
    RUN_TEST(test_pmm_frame_alignment);
}