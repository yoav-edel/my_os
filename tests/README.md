# Memory Management Test Suite

## Overview

This test suite provides comprehensive testing for the memory management components of the OS kernel. The tests are designed to run in the same QEMU environment as the main OS and follow a progressive testing strategy from simple building blocks to complex scenarios.

## Test Structure

### Test Directory Layout

```
tests/
├── test_framework.h          # Core test framework with assertions and colored output
├── memory/
│   ├── test_pmm.h            # Physical Memory Manager tests
│   ├── test_vmm.h            # Virtual Memory Manager tests
│   └── test_kmalloc.h        # Kernel Memory Allocator tests
├── simple_test_kernel.c      # Simple test kernel (PMM tests only)
├── test_kernel.c             # Full test kernel (all memory tests)
├── simple_test_multiboot.asm # Multiboot header for simple tests
└── test_multiboot.asm        # Multiboot header for full tests
```

## Test Components

### 1. Physical Memory Manager (PMM) Tests

Tests the basic building blocks of physical memory allocation:

- `test_pmm_init_basic()` - Basic initialization
- `test_pmm_alloc_frame_basic()` - Frame allocation
- `test_pmm_free_frame_basic()` - Frame freeing
- `test_pmm_is_frame_free_basic()` - Frame status checking
- `test_pmm_alloc_free_cycle()` - Allocation/free cycles
- `test_pmm_double_free()` - Edge case: double free
- `test_pmm_frame_alignment()` - Frame alignment verification

### 2. Virtual Memory Manager (VMM) Tests

Tests virtual memory management functionality:

- `test_vmm_init_basic()` - VMM initialization
- `test_vmm_get_kernel_page_directory()` - Kernel page directory access
- `test_vmm_map_page_basic()` - Basic page mapping
- `test_vmm_map_page_to_curr_dir()` - Current directory mapping
- `test_vmm_calc_phys_addr()` - Physical address calculation
- `test_vmm_create_vm_context()` - VM context creation
- `test_vmm_destroy_vm_context()` - VM context destruction
- `test_vmm_multiple_mappings()` - Multiple page mappings
- `test_vmm_unmap_page()` - Page unmapping

### 3. Kernel Memory Allocator (KMALLOC) Tests

Tests kernel memory allocation and deallocation:

- `test_kmalloc_init_basic()` - KMALLOC initialization
- `test_kmalloc_basic_allocation()` - Basic memory allocation
- `test_kmalloc_free_basic()` - Memory freeing
- `test_kmalloc_alloc_free_cycle()` - Allocation/free cycles
- `test_kmalloc_small_allocations()` - Small allocations (slab allocator)
- `test_kmalloc_large_allocations()` - Large allocations
- `test_kmalloc_zero_allocation()` - Edge case: zero allocation
- `test_kmalloc_double_free()` - Edge case: double free
- `test_kmalloc_memory_access()` - Memory read/write verification
- `test_kmalloc_all_cache_sizes()` - All cache size testing
- `test_kmalloc_stress_test()` - Stress testing with many allocations

## Test Framework Features

### Colored Output
- Green for passed tests
- Red for failed tests
- Blue for test sections
- Yellow for informational messages

### Assertion Macros
- `ASSERT(condition, message)` - Basic assertion
- `ASSERT_EQ(expected, actual, message)` - Equality assertion
- `ASSERT_NEQ(not_expected, actual, message)` - Inequality assertion
- `ASSERT_NULL(ptr, message)` - NULL pointer assertion
- `ASSERT_NOT_NULL(ptr, message)` - Non-NULL pointer assertion
- `ASSERT_TRUE(condition, message)` - Boolean true assertion
- `ASSERT_FALSE(condition, message)` - Boolean false assertion

### Test Management
- `TEST_START(name)` - Mark test beginning
- `TEST_END(name)` - Mark test completion
- `RUN_TEST(test_func)` - Execute a test function
- Test result tracking and summary reporting

## Building and Running Tests

### Prerequisites
- i686-elf-gcc cross-compiler (recommended) or gcc with 32-bit support
- NASM assembler
- QEMU i386 emulator
- GRUB tools (grub-mkrescue)
- mtools

### Build Commands

#### Simple Tests (PMM only)
```bash
make simple-test        # Build and run simple PMM tests
make simple-test-run    # Run simple tests (if already built)
```

#### Full Tests (PMM + VMM + KMALLOC)
```bash
make test              # Build and run full memory tests
make test-run          # Run full tests (if already built)
```

#### Clean Test Artifacts
```bash
make test-clean        # Clean test build artifacts
```

## Test Execution

Tests run in QEMU with the same configuration as the main OS:
- i386 architecture
- Serial output for test results
- Same memory layout and hardware emulation
- Boots from ISO image created by GRUB

### Expected Output

Successful test run should show:
```
=== Memory Management Test Suite ===
Testing memory subsystems: PMM, VMM, KMALLOC

=== Test Framework Initialized ===

Initializing memory subsystems...
Initializing PMM...
PMM initialized successfully

=== Running PMM Tests ===
[PASS] PMM Init Basic
[PASS] PMM Alloc Frame Basic
[PASS] PMM Free Frame Basic
...

=== TEST SUMMARY ===
Tests run:    25
Tests passed: 25
Tests failed: 0

🎉 ALL MEMORY TESTS PASSED! 🎉
Memory management system is working correctly.
```

## Progressive Testing Strategy

The test suite follows a building-block approach:

1. **Level 1: PMM Tests** - Test physical memory frame allocation/deallocation
2. **Level 2: VMM Tests** - Test virtual memory mapping built on PMM
3. **Level 3: KMALLOC Tests** - Test high-level allocation built on VMM/PMM

This ensures that basic components work before testing more complex ones.

## Edge Cases and Error Conditions

The tests include comprehensive edge case coverage:

- Double free operations
- NULL pointer handling
- Zero-size allocations
- Memory alignment verification
- Stress testing with many allocations
- Boundary condition testing
- Error recovery scenarios

## Integration with Main OS

The test system is designed to:

- Use the same memory management code as the main OS
- Run in the same QEMU environment
- Use the same build system (Makefile)
- Follow the same coding style and conventions
- Test actual kernel functions, not mocks

This ensures that passing tests indicate that the memory management system will work correctly in the main OS.

## Future Enhancements

Potential improvements to the test suite:

- Performance benchmarking
- Memory leak detection
- Multi-threading tests (when implemented)
- Page fault testing
- Swap system testing
- Advanced VMM features testing

## Usage Example

To run the complete test suite:

```bash
# Build the OS first to ensure dependencies are available
make clean
make all

# Run the simple tests (PMM only)
make simple-test

# Run the full test suite
make test
```

The tests will output detailed results and provide a summary at the end indicating whether all memory management functions are working correctly.