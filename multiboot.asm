[BITS 32]
section .multiboot_header
align 4
    dd 0x1BADB002          ; Multiboot magic number
    dd 0x0                 ; Multiboot flags
    dd -(0x1BADB002 + 0x0) ; Multiboot checksum


