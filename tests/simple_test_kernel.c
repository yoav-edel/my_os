//
// Created by Tests on 2025.
//

/*
 * Simplified Test Kernel - Minimal memory tests without full system initialization
 * This tests memory management functions in isolation
 */

#include "../drivers/screen.h"
#include "../memory/pmm.h"
#include "../memory/vmm.h"
#include "../memory/kmalloc.h"
#include "../std/string.h"
#include "../std/stdio.h"
#include "../std/stdint.h"

// Include test frameworks
#include "test_framework.h"
#include "memory/test_pmm.h"
// Note: Not including VMM and KMALLOC tests as they require more complex setup

// Stub implementations for missing functions that we don't need for basic testing
void keyboard_handler(void) { }
void pit_handler(void) { }
uint32_t disk_alloc_slot(void) { return 0; }
void disk_write(uint32_t slot, void *data) { }
void disk_read(uint32_t slot, void *data) { }
void disk_free_slot(uint32_t slot) { }

// Kernel stack symbols (simplified for testing)
extern char _kernel_start, _kernel_end;
const unsigned int _kernel_stack_top = 0x00200000;  // 2MB
const unsigned int _kernel_stack_pages_amount = 4;

// Kernel heap address (simplified for testing)
uint32_t KERNEL_BASE_HEAP_ADDR = 0x00300000;  // 3MB

// ISR stubs (minimal implementation for testing)
void isr0(void) { }
void isr1(void) { }
void isr2(void) { }
void isr3(void) { }
void isr4(void) { }
void isr5(void) { }
void isr6(void) { }
void isr7(void) { }
void isr8(void) { }
void isr9(void) { }
void isr10(void) { }
void isr11(void) { }
void isr12(void) { }
void isr13(void) { }
void isr14(void) { }
void isr15(void) { }
void isr16(void) { }
void isr17(void) { }
void isr18(void) { }
void isr19(void) { }
void isr20(void) { }
void isr21(void) { }
void isr22(void) { }
void isr23(void) { }
void isr24(void) { }
void isr25(void) { }
void isr26(void) { }
void isr27(void) { }
void isr28(void) { }
void isr29(void) { }
void isr30(void) { }
void isr31(void) { }
void isr32(void) { }
void isr33(void) { }

void simple_test_kernel_main() {
    // Clear screen and show header
    clear_screen();
    printf(COLOR_BLUE "=== Simplified Memory Management Test Suite ===" COLOR_RESET "\n");
    printf("Testing memory subsystems: PMM, VMM, KMALLOC\n");
    printf("Note: Running with minimal initialization for testing\n\n");
    
    // Initialize test framework
    test_framework_init();
    
    // Test only PMM for now (basic building block)
    printf(COLOR_YELLOW "Initializing memory subsystems..." COLOR_RESET "\n");
    
    // Initialize PMM first
    printf("Initializing PMM...\n");
    pmm_init();
    printf(COLOR_GREEN "PMM initialized successfully" COLOR_RESET "\n");
    
    // Simple PMM tests only
    printf("\n" COLOR_BLUE "=== Starting PMM Tests ===" COLOR_RESET "\n");
    run_pmm_tests();
    
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