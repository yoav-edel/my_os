[org 0x7c00]           ; BIOS loads bootloader at 0x7C00
bits 16                ; 16-bit real mode

start:
    mov ah, 0x0E       ; BIOS teletype function
    mov al, 'B'        ; Print 'B'
    int 0x10
    mov al, 'L'        ; Print 'L'
    int 0x10

    ; Hang the system
    cli                ; Disable interrupts
    hlt                ; Halt CPU

times 510-($-$$) db 0  ; Pad bootloader to 510 bytes
dw 0xAA55              ; Boot signature
