BITS 32
extern isr_handler

; ----------------------------------------------
; Macros
; ----------------------------------------------
%macro ISR_NO_ERR_CODE 1
global isr%1
isr%1:
    cli                         ; disable interrupts

    ; CPU did NOT push an error code for this exception,
    ; so push a dummy 0 so that stack layout remains consistent:
    push dword 0                ; err_code = 0

    ; Push the interrupt number:
    push dword %1               ; int_no

    ; Push all general-purpose registers:
    pushad                       ; pushes EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI

    ; At this point, stack layout from top (ESP) down is:
    ;   [EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX, int_no, err_code, retEIP, retCS, retEFLAGS, (retUserESP?), (retSS?) ]

    mov eax, esp
    push eax                     ; pass (registers_t*) to isr_handler
    call isr_handler
    add esp, 4                   ; pop the argument to isr_handler

    popad                        ; restore general-purpose registers
    add esp, 8                   ; pop int_no and err_code
    sti                          ; re-enable interrupts
    iret                         ; return from interrupt
%endmacro

%macro ISR_WITH_ERR_CODE 1
global isr%1
isr%1:
    cli

    ; For exceptions that *do* push an error code automatically:
    ; CPU has already pushed [error_code, EIP, CS, EFLAGS, ...].
    ; But we want the same layout as the "no err code" case; so we *only* push the int_no.
    ;
    ; The hardware-pushed error code is *below* EIP on the stack. We'll treat that
    ; as the "err_code" in our struct. So just push the int number here:

    push dword %1               ; int_no

    pushad

    mov eax, esp
    push eax
    call isr_handler
    add esp, 4

    popad
    add esp, 8                  ; pop int_no + the error_code the CPU pushed
    sti
    iret
%endmacro


; ----------------------------------------------
; Exceptions WITHOUT error codes
; ----------------------------------------------
ISR_NO_ERR_CODE 0    ; Divide Error
ISR_NO_ERR_CODE 1    ; Debug
ISR_NO_ERR_CODE 2    ; NMI
ISR_NO_ERR_CODE 3    ; Breakpoint
ISR_NO_ERR_CODE 4    ; Overflow
ISR_NO_ERR_CODE 5    ; Bound Range
ISR_NO_ERR_CODE 6    ; Invalid Opcode
ISR_NO_ERR_CODE 7    ; Device Not Available
ISR_NO_ERR_CODE 9    ; Coprocessor Segment Overrun
ISR_NO_ERR_CODE 15   ; Reserved
ISR_NO_ERR_CODE 16   ; x87 Floating-Point
ISR_NO_ERR_CODE 18   ; Machine Check
ISR_NO_ERR_CODE 19   ; SIMD Floating-Point
ISR_NO_ERR_CODE 20   ; Virtualization
ISR_NO_ERR_CODE 21   ; Reserved
ISR_NO_ERR_CODE 22   ; Reserved
ISR_NO_ERR_CODE 23   ; Reserved
ISR_NO_ERR_CODE 24   ; Reserved
ISR_NO_ERR_CODE 25   ; Reserved
ISR_NO_ERR_CODE 26   ; Reserved
ISR_NO_ERR_CODE 27   ; Reserved
ISR_NO_ERR_CODE 28   ; Reserved
ISR_NO_ERR_CODE 29   ; Reserved
ISR_NO_ERR_CODE 30   ; Security Exception
ISR_NO_ERR_CODE 31   ; Reserved
ISR_NO_ERR_CODE 32   ; Reserved
ISR_NO_ERR_CODE 33   ; Keyboard Interrupt

; ----------------------------------------------
; Exceptions WITH error codes
; ----------------------------------------------
ISR_WITH_ERR_CODE 8    ; Double Fault
ISR_WITH_ERR_CODE 10   ; Invalid TSS
ISR_WITH_ERR_CODE 11   ; Segment Not Present
ISR_WITH_ERR_CODE 12   ; Stack-Segment Fault
ISR_WITH_ERR_CODE 13   ; General Protection Fault
ISR_WITH_ERR_CODE 14   ; Page Fault
ISR_WITH_ERR_CODE 17   ; Alignment Check
