//
// Created by Tests on 2025.
//

/*
 * Test Kernel - Main entry point for memory management tests
 * This kernel initializes the necessary subsystems and runs all memory tests
 */

#include "../drivers/screen.h"
#include "../interupts/idt.h"
#include "../interupts/pic.h"
#include "../gdt.h"
#include "../memory/pmm.h"
#include "../memory/vmm.h"
#include "../memory/kmalloc.h"
#include "../std/string.h"
#include "../std/stdio.h"

// Include test frameworks
#include "test_framework.h"
#include "memory/test_pmm.h"
#include "memory/test_vmm.h"
#include "memory/test_kmalloc.h"

void test_kernel_main() {
    // Initialize basic kernel subsystems needed for memory management
    init_gdt();
    init_idt();
    remap_pic();
    
    // Enable interrupts
    asm volatile("sti");
    
    // Clear screen and show header
    clear_screen();
    printf(COLOR_BLUE "=== Memory Management Test Suite ===" COLOR_RESET "\n");
    printf("Testing memory subsystems: PMM, VMM, KMALLOC\n");
    printf("Running on QEMU with same configuration as main OS\n\n");
    
    // Initialize test framework
    test_framework_init();
    
    // Initialize memory subsystems in correct order
    printf(COLOR_YELLOW "Initializing memory subsystems..." COLOR_RESET "\n");
    
    // Initialize PMM first
    printf("Initializing PMM...\n");
    pmm_init();
    printf(COLOR_GREEN "PMM initialized successfully" COLOR_RESET "\n");
    
    // Initialize VMM second
    printf("Initializing VMM...\n");
    vmm_init();
    printf(COLOR_GREEN "VMM initialized successfully" COLOR_RESET "\n");
    
    // Initialize KMALLOC last
    printf("Initializing KMALLOC...\n");
    init_kmalloc();
    printf(COLOR_GREEN "KMALLOC initialized successfully" COLOR_RESET "\n\n");
    
    // Run all memory tests
    printf(COLOR_BLUE "=== Starting Memory Tests ===" COLOR_RESET "\n");
    
    // Run PMM tests (building blocks)
    run_pmm_tests();
    
    // Run VMM tests (virtual memory management)
    run_vmm_tests();
    
    // Run KMALLOC tests (high-level allocation)
    run_kmalloc_tests();
    
    // Print final test summary
    printf("\n");
    test_framework_print_summary();
    
    // Final result
    if (test_framework_all_passed()) {
        printf("\n" COLOR_GREEN "🎉 ALL MEMORY TESTS PASSED! 🎉" COLOR_RESET "\n");
        printf(COLOR_GREEN "Memory management system is working correctly." COLOR_RESET "\n");
    } else {
        printf("\n" COLOR_RED "❌ SOME TESTS FAILED! ❌" COLOR_RESET "\n");
        printf(COLOR_RED "Memory management system has issues that need to be addressed." COLOR_RESET "\n");
    }
    
    printf("\nTest execution completed. System will halt.\n");
    
    // Halt the system
    while (1) {
        asm volatile("hlt");
    }
}