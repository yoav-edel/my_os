[BITS 16]
[ORG 0x7C00]

start:
    ; Clear the screen
    mov ax, 0x0003       ; Set video mode 3 (80x25 text mode, clears the screen)
    int 0x10

    ; Print boot message
    mov si, boot_msg
    call print_string

    ; Load the kernel into memory at 0x1000
    mov ah, 0x02         ; BIOS read sectors function
    mov al, 1            ; Number of sectors to read (just the first sector for simplicity)
    mov ch, 0            ; Cylinder 0
    mov cl, 2            ; Start at the second sector (1-based)
    mov dh, 0            ; Head 0
    mov dl, 0            ; Drive 0 (floppy)
    mov bx, 0x1000       ; Segment to load the kernel
    int 0x13             ; BIOS interrupt to read disk
    jc disk_error        ; Jump if carry flag is set (error)

    ; Jump to kernel entry point
    jmp 0x1000:0x0000    ; Far jump to kernel at 0x1000:0x0000

disk_error:
    ; Print error message and halt
    mov si, error_msg
    call print_string
    cli
    hlt

print_string:
    mov ah, 0x0E         ; BIOS teletype output function
.print_char:
    lodsb                ; Load a character from [SI] into AL and increment SI
    cmp al, 0            ; Check for null terminator
    je .done             ; Exit loop if null terminator found
    int 0x10             ; Print the character in AL
    jmp .print_char      ; Repeat for next character
.done:
    ret                  ; Return to caller

boot_msg db "Booting kernel...", 0
error_msg db "Disk read error!", 0

times 510 - ($ - $$) db 0  ; Pad to 512 bytes
dw 0xAA55                 ; Boot sector signature
