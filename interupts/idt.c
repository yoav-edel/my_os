//
// Created by Yoav on 11/24/2024.
//
#include "idt.h"
#include "../gdt.h"


struct idt_entry { // size = 8 bytes
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#define IDT_ENTRIES 256
static struct idt_entry idt[IDT_ENTRIES];

static struct idt_ptr idtPtr;

void init_idt() {
    init_idt_entries();
    init_idt_ptr();
    load_idt();
}


// Declare the ISR functions for exceptions
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr33();

void init_idt_entries() {
    idt_set_gate(0, (uint32_t)isr0, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(1, (uint32_t)isr1, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(2, (uint32_t)isr2, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(3, (uint32_t)isr3, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(4, (uint32_t)isr4, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(5, (uint32_t)isr5, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(6, (uint32_t)isr6, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(7, (uint32_t)isr7, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(8, (uint32_t)isr8, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(9, (uint32_t)isr9, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(10, (uint32_t)isr10, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(11, (uint32_t)isr11, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(12, (uint32_t)isr12, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(13, (uint32_t)isr13, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(14, (uint32_t)isr14, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(15, (uint32_t)isr15, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(16, (uint32_t)isr16, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(17, (uint32_t)isr17, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(18, (uint32_t)isr18, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(19, (uint32_t)isr19, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(20, (uint32_t)isr20, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(21, (uint32_t)isr21, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(22, (uint32_t)isr22, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(23, (uint32_t)isr23, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(24, (uint32_t)isr24, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(25, (uint32_t)isr25, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(26, (uint32_t)isr26, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(27, (uint32_t)isr27, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(28, (uint32_t)isr28, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(29, (uint32_t)isr29, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(30, (uint32_t)isr30, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    idt_set_gate(31, (uint32_t)isr31, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);
    // add the keyboard isr
    idt_set_gate(33, (uint32_t) isr33, KERNEL_CODE_SELECTOR, IDT_ATTR_KERNEL);



}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t type_attr) {
    idt[num].offset_low = IDT_OFFSET_LOW(base);
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].type_attr = type_attr;
    idt[num].offset_high = IDT_OFFSET_HIGH(base);
}


void init_idt_ptr()
{
    idtPtr.limit = sizeof(idt) - 1;
    idtPtr.base = (uint32_t) &idt;
}





void load_idt() {
    asm volatile ("lidt %0" : : "m"(idtPtr));
}
