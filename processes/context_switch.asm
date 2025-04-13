; This file implement in assembly the context switch function
; c declaration: void process_switch(process_t *next);
; Before calling this function the caller needs to disable interrupts and enable afterwards

global process_switch
extern current_process
extern vmm_switch_vm_context
; Define some helpful constants for clarity:
%define CONTEXT_EDI    0
%define CONTEXT_ESI    4
%define CONTEXT_EBP    8
%define CONTEXT_ESP   12
%define CONTEXT_EBX   16
%define CONTEXT_EDX   20
%define CONTEXT_ECX   24
%define CONTEXT_EAX   28
%define CONTEXT_EIP   32
%define CONTEXT_EFLAGS 36

%define VM_CONTEXT 4

; process->pcb is at offset 0, so:
%define OFFSET_PCB  0
; pcb->context is at offset 0, so:
%define OFFSET_CONTEXT  0

;-----------------------------------------
process_switch:
    ;-------------------------------------------------------------------------
    ; function prologue
    push ebp
    mov  ebp, esp
    ;we also save the register in the context for debuging information(its enogth to save on the stack)

    ; pushfd + pushad to get EFLAGS & general-purpose registers on stack
    pushfd                 ; top of stack = EFLAGS
    pushad                 ; top of stack = EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI (in that order)

    mov  eax, [current_process] ; current_process
    mov  ecx, [eax + OFFSET_PCB]     ; pcb_t * old_pcb
    mov  ecx, [ecx + OFFSET_CONTEXT] ; context_t * old_context

    ; We just did pushad, so the layout on top of the stack is:
    ;   +0  : EAX
    ;   +4  : ECX
    ;   +8  : EDX
    ;   +12 : EBX
    ;   +16 : old ESP (before pushad)
    ;   +20 : EBP
    ;   +24 : ESI
    ;   +28 : EDI
    ; then at +32 is the EFLAGS from pushfd

    ; Store EAX
    mov  edx, [esp +  0]            ; top of pushad
    mov  [ecx + CONTEXT_EAX], edx
    ; Store ECX
    mov  edx, [esp +  4]
    mov  [ecx + CONTEXT_ECX], edx
    ; Store EDX
    mov  edx, [esp +  8]
    mov  [ecx + CONTEXT_EDX], edx
    ; Store EBX
    mov  edx, [esp + 12]
    mov  [ecx + CONTEXT_EBX], edx
    ; Store ESP (the original pre-pushad esp)
    mov  edx, [esp + 16]
    mov  [ecx + CONTEXT_ESP], edx
    ; Store EBP
    mov  edx, [esp + 20]
    mov  [ecx + CONTEXT_EBP], edx
    ; Store ESI
    mov  edx, [esp + 24]
    mov  [ecx + CONTEXT_ESI], edx
    ; Store EDI
    mov  edx, [esp + 28]
    mov  [ecx + CONTEXT_EDI], edx
    ; Store EFLAGS
    mov  edx, [esp + 32]            ; right after pushad is pushfd
    mov  [ecx + CONTEXT_EFLAGS], edx

    ; Finally, we also want to store the "would-be" return EIP
    ; i.e. the address to which process_switch() would return in the old process.
    ; That's at [ebp + 4] (the saved return address),
    mov  edx, [ebp + 4]             ; the 'return address'
    mov  [ecx + CONTEXT_EIP], edx


    ;-------------------------------------------------------------------------
.set_new:
 ; 2) current_process = next
    mov  edx, [ebp + 8]             ; 'next Process'
    mov  [current_process], edx

    ; Reload the new process pointer from the global variable:
    mov  eax, [current_process]     ; now eax points to the new process
    mov  ecx, [eax + OFFSET_PCB]     ; pcb_t * new_pcb
    mov  ecx, [ecx + VM_CONTEXT]     ; vm_context_t * new_vm_context
    push ecx
    call vmm_switch_vm_context
    add esp, 4
    ;load the new stack pointer
    mov  eax, [current_process]     ; now eax points to the new process
    mov ecx, [eax + OFFSET_PCB]     ; pcb_t * new_pcb
    mov ecx, [ecx + OFFSET_CONTEXT] ; context_t * new_context
    mov esp, [ecx + CONTEXT_ESP] ; load the new stack pointer

    ;We can load the context from the context structure, but its faster to load it from the stack
    popad                 ; pop EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    popfd                 ; pop EFLAGS

    pop ebp
    ret


