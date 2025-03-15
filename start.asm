section .text
global start
global _kernel_stack_top
global _kernel_stack_pages_amount

extern kernel_main

_kernel_stack_top dd 0x4000000 - 1

_kernel_stack_pages_amount dd 100

start:
    cli                          ; Clear interrupts
    mov esp, [_kernel_stack_top] ; Now loads 0x3FFFFFF directly into ESP
    call kernel_main             ; Jump to kernel main function
    hlt                          ; Halt CPU
