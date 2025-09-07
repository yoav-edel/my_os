//
// Created by Tests on 2025.
//

/*
 * Simple test framework for kernel-mode testing
 * Provides basic test infrastructure for memory management tests
 */

#ifndef MYKERNELPROJECT_TEST_FRAMEWORK_H
#define MYKERNELPROJECT_TEST_FRAMEWORK_H

#include "../std/stdio.h"
#include "../std/string.h"

// Test result tracking
typedef struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
} test_results_t;

static test_results_t global_test_results = {0, 0, 0};

// ANSI color codes for output
#define COLOR_GREEN "\033[32m"
#define COLOR_RED   "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE  "\033[34m"
#define COLOR_RESET "\033[0m"

// Test macros
#define TEST_START(name) \
    printf("\n" COLOR_BLUE "=== Starting test: %s ===" COLOR_RESET "\n", name); \
    global_test_results.tests_run++;

#define TEST_END(name) \
    printf(COLOR_BLUE "=== Finished test: %s ===" COLOR_RESET "\n", name);

#define ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf(COLOR_GREEN "[PASS]" COLOR_RESET " %s\n", message); \
        } else { \
            printf(COLOR_RED "[FAIL]" COLOR_RESET " %s\n", message); \
            global_test_results.tests_failed++; \
            return false; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual, message) \
    do { \
        if ((expected) == (actual)) { \
            printf(COLOR_GREEN "[PASS]" COLOR_RESET " %s (expected: %p, actual: %p)\n", message, (void*)(expected), (void*)(actual)); \
        } else { \
            printf(COLOR_RED "[FAIL]" COLOR_RESET " %s (expected: %p, actual: %p)\n", message, (void*)(expected), (void*)(actual)); \
            global_test_results.tests_failed++; \
            return false; \
        } \
    } while(0)

#define ASSERT_NEQ(not_expected, actual, message) \
    do { \
        if ((not_expected) != (actual)) { \
            printf(COLOR_GREEN "[PASS]" COLOR_RESET " %s (not expected: %p, actual: %p)\n", message, (void*)(not_expected), (void*)(actual)); \
        } else { \
            printf(COLOR_RED "[FAIL]" COLOR_RESET " %s (not expected: %p, actual: %p)\n", message, (void*)(not_expected), (void*)(actual)); \
            global_test_results.tests_failed++; \
            return false; \
        } \
    } while(0)

#define ASSERT_NULL(ptr, message) \
    ASSERT_EQ(NULL, ptr, message)

#define ASSERT_NOT_NULL(ptr, message) \
    ASSERT_NEQ(NULL, ptr, message)

#define ASSERT_TRUE(condition, message) \
    ASSERT(condition, message)

#define ASSERT_FALSE(condition, message) \
    ASSERT(!(condition), message)

// Test function type
typedef bool (*test_func_t)(void);

// Test registration
#define RUN_TEST(test_func) \
    do { \
        printf("\n" COLOR_YELLOW "Running test: " #test_func COLOR_RESET "\n"); \
        if (test_func()) { \
            global_test_results.tests_passed++; \
            printf(COLOR_GREEN "[SUCCESS]" COLOR_RESET " " #test_func " passed\n"); \
        } else { \
            printf(COLOR_RED "[FAILURE]" COLOR_RESET " " #test_func " failed\n"); \
        } \
    } while(0)

// Helper functions
static inline void test_framework_init(void) {
    global_test_results.tests_run = 0;
    global_test_results.tests_passed = 0;
    global_test_results.tests_failed = 0;
    printf(COLOR_BLUE "=== Test Framework Initialized ===" COLOR_RESET "\n");
}

static inline void test_framework_print_summary(void) {
    printf("\n" COLOR_BLUE "=== TEST SUMMARY ===" COLOR_RESET "\n");
    printf("Tests run:    %d\n", global_test_results.tests_run);
    printf("Tests passed: " COLOR_GREEN "%d" COLOR_RESET "\n", global_test_results.tests_passed);
    printf("Tests failed: " COLOR_RED "%d" COLOR_RESET "\n", global_test_results.tests_failed);
    
    if (global_test_results.tests_failed == 0) {
        printf("\n" COLOR_GREEN "ALL TESTS PASSED!" COLOR_RESET "\n");
    } else {
        printf("\n" COLOR_RED "SOME TESTS FAILED!" COLOR_RESET "\n");
    }
}

static inline bool test_framework_all_passed(void) {
    return global_test_results.tests_failed == 0;
}

#endif // MYKERNELPROJECT_TEST_FRAMEWORK_H