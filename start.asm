section .text
    global start
extern kernel_main
start:
    cli                         ; Clear interrupts
    mov esp, 0x90000            ; Set stack pointer
    call kernel_main            ; Jump to kernel main function
    hlt                         ; Halt CPU
