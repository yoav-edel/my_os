; Test kernel multiboot header
; Simple multiboot header for the test kernel

section .multiboot
align 4
	dd 0x1BADB002    ; magic number
	dd 0x00          ; flags
	dd -(0x1BADB002 + 0x00) ; checksum

section .text
global _start
extern test_kernel_main

_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Call the test kernel main function
    call test_kernel_main
    
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