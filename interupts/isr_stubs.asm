; Set assembly to 32-bit mode
BITS 32

; ----------------------------------------------
; Macro for exceptions without an error code
; ----------------------------------------------
%macro ISR_NO_ERR_CODE 1
    global isr%1             ; Declare the ISR label as global for linking
isr%1:
    cli                      ; Clear Interrupt Flag - disable interrupts
    push dword 0             ; Push a dummy error code onto the stack
    push dword %1            ; Push the interrupt number onto the stack
    pusha                    ; Push all general-purpose registers onto the stack
    mov eax, esp             ; Move the stack pointer to EAX (pointer to 'registers_t')
    push eax                 ; Push the pointer to the 'registers_t' structure
    call isr_handler         ; Call the common ISR handler function
    add esp, 4               ; Clean up the stack (remove the argument to isr_handler)
    popa                     ; Restore all general-purpose registers from the stack
    add esp, 8               ; Clean up the interrupt number and error code from the stack
    sti                      ; Set Interrupt Flag - re-enable interrupts
    iret                     ; Return from interrupt
%endmacro

; ----------------------------------------------
; Macro for exceptions with an error code
; ----------------------------------------------
%macro ISR_WITH_ERR_CODE 1
    global isr%1             ; Declare the ISR label as global for linking
isr%1:
    cli                      ; Clear Interrupt Flag - disable interrupts
    push dword %1            ; Push the interrupt number onto the stack
    pusha                    ; Push all general-purpose registers onto the stack
    mov eax, esp             ; Move the stack pointer to EAX (pointer to 'registers_t')
    push eax                 ; Push the pointer to the 'registers_t' structure
    call isr_handler         ; Call the common ISR handler function
    add esp, 4               ; Clean up the stack (remove the argument to isr_handler)
    popa                     ; Restore all general-purpose registers from the stack
    add esp, 4               ; Clean up the interrupt number from the stack
    sti                      ; Set Interrupt Flag - re-enable interrupts
    iret                     ; Return from interrupt
%endmacro

; ----------------------------------------------
; Generate ISR stubs for exceptions without error codes
; ----------------------------------------------
ISR_NO_ERR_CODE 0    ; Divide Error Exception
ISR_NO_ERR_CODE 1    ; Debug Exception
ISR_NO_ERR_CODE 2    ; Non-Maskable Interrupt Exception
ISR_NO_ERR_CODE 3    ; Breakpoint Exception
ISR_NO_ERR_CODE 4    ; Overflow Exception
ISR_NO_ERR_CODE 5    ; Bound Range Exceeded Exception
ISR_NO_ERR_CODE 6    ; Invalid Opcode Exception
ISR_NO_ERR_CODE 7    ; Device Not Available Exception
ISR_NO_ERR_CODE 9    ; Coprocessor Segment Overrun Exception
ISR_NO_ERR_CODE 16   ; x87 Floating-Point Exception
ISR_NO_ERR_CODE 17   ; Alignment Check Exception
ISR_NO_ERR_CODE 18   ; Machine Check Exception
ISR_NO_ERR_CODE 19   ; SIMD Floating-Point Exception
ISR_NO_ERR_CODE 20   ; Virtualization Exception
ISR_NO_ERR_CODE 30   ; Security Exception
ISR_NO_ERR_CODE 31   ; Reserved Exception

; ----------------------------------------------
; Generate ISR stubs for exceptions with error codes
; ----------------------------------------------
ISR_WITH_ERR_CODE 8    ; Double Fault Exception
ISR_WITH_ERR_CODE 10   ; Invalid TSS Exception
ISR_WITH_ERR_CODE 11   ; Segment Not Present Exception
ISR_WITH_ERR_CODE 12   ; Stack-Segment Fault Exception
ISR_WITH_ERR_CODE 13   ; General Protection Fault Exception
ISR_WITH_ERR_CODE 14   ; Page Fault Exception
ISR_WITH_ERR_CODE 17   ; Alignment Check Exception (also listed without error code)
