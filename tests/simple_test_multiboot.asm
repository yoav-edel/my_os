; Simple test kernel multiboot header
; Minimal multiboot header for the simple test kernel

section .multiboot_header
align 4
	dd 0x1BADB002    ; magic number
	dd 0x00          ; flags
	dd -(0x1BADB002 + 0x00) ; checksum

section .text
global start
extern simple_test_kernel_main

start:
_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Call the simple test kernel main function
    call simple_test_kernel_main
    
    ; If we return, halt
    cli
.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
resb 16384 ; 16 KB stack
stack_top: