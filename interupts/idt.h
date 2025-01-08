//
// Created by Yoav on 11/24/2024.
//

#ifndef MYKERNELPROJECT_IDT_H
#define MYKERNELPROJECT_IDT_H

#include "../std/stdint.h"




void init_idt();
void init_idt_entries();
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t type_attr);
void init_idt_ptr();
void load_idt();

// IDT Attributes
#define IDT_PRESENT 0x80          // Present bit (bit 7)
#define IDT_DPL0 0x00             // Descriptor Privilege Level 0 (bits 6-5)
#define IDT_DPL3 0x60             // Descriptor Privilege Level 3 (bits 6-5)
#define IDT_INTERRUPT_GATE 0x0E   // Interrupt gate type (bits 3-0)

// Combined Attributes
#define IDT_ATTR_KERNEL (IDT_PRESENT | IDT_DPL0 | IDT_INTERRUPT_GATE) // 0x8E
#define IDT_ATTR_USER (IDT_PRESENT | IDT_DPL3 | IDT_INTERRUPT_GATE)   // 0xEE

#define IDT_OFFSET_LOW(offset) ((offset) & 0xFFFF)
#define IDT_OFFSET_HIGH(offset) ((offset) >> 16)



#endif //MYKERNELPROJECT_IDT_H
