section .text
    global start
    extern kernel_main

start:
    cli                         ; Clear interrupts
    mov esp, [_kernel_stack_top]  ; Set stack pointer
    call kernel_main            ; Jump to kernel main function
    hlt                         ; Halt CPU


section .data
    global _kernel_stack_top
    _kernel_stack_top: dd 0x1000000 - 1

    global _kernel_stack_pages_amount
    _kernel_stack_pages_amount: dd 30
