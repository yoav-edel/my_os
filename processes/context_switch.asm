    ; ---------------------------------------------------------------------------
    ; void process_switch(process_t *next);
    ;   — caller must have interrupts disabled
    ; ---------------------------------------------------------------------------
   global  process_switch
   extern  current_process
   extern  vmm_switch_vm_context

   %define CONTEXT_EDI     0
   %define CONTEXT_ESI     4
   %define CONTEXT_EBP     8
   %define CONTEXT_ESP    12
   %define CONTEXT_EBX    16
   %define CONTEXT_EDX    20
   %define CONTEXT_ECX    24
   %define CONTEXT_EAX    28
   %define CONTEXT_EIP    32
   %define CONTEXT_EFLAGS 36

   %define OFFSET_PCB      0
   %define OFFSET_CONTEXT  0
   %define VM_CONTEXT      4

   process_switch:
       push    ebp
       mov     ebp, esp
       pushfd
       pushad

       mov     eax, [current_process]
       mov     ecx, [eax + OFFSET_PCB]
       mov     ecx, [ecx + OFFSET_CONTEXT]
       mov     [ecx + CONTEXT_ESP], esp

       mov     edx, [esp]
       mov     [ecx + CONTEXT_EDI], edx
       mov     edx, [esp + 4]
       mov     [ecx + CONTEXT_ESI], edx
       mov     edx, [esp + 8]
       mov     [ecx + CONTEXT_EBP], edx
       mov     edx, [esp + 16]
       mov     [ecx + CONTEXT_EBX], edx
       mov     edx, [esp + 20]
       mov     [ecx + CONTEXT_EDX], edx
       mov     edx, [esp + 24]
       mov     [ecx + CONTEXT_ECX], edx
       mov     edx, [esp + 28]
       mov     [ecx + CONTEXT_EAX], edx
       mov     edx, [esp + 36]
       mov     [ecx + CONTEXT_EFLAGS], edx
       mov     edx, [ebp + 4]
       mov     [ecx + CONTEXT_EIP], edx

       mov     edx, [ebp + 8]
       mov     [current_process], edx

       mov     eax, [current_process]
       mov     ecx, [eax + OFFSET_PCB]
       mov     ecx, [ecx + VM_CONTEXT]
       push    ecx
       call    vmm_switch_vm_context
       add     esp, 4

       mov     eax, [current_process]
       mov     ecx, [eax + OFFSET_PCB]
       mov     ecx, [ecx + OFFSET_CONTEXT]
       mov     esp, [ecx + CONTEXT_ESP]

       popad
       popfd
       pop     ebp
       ret
