//
// Created by Yoav on 11/26/2024.
//

#ifndef MYKERNELPROJECT_PIC_H
#define MYKERNELPROJECT_PIC_H
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define IRQ_NUM 16
#define PIC_IRQ_AMOUNT 8

#define ICW1_INIT    0x10 // Initialization
#define ICW1_ICW4    0x01
#define PIC1_VECTOR_OFFSET 0x20
#define PIC2_VECTOR_OFFSET 0x28

#define PIC1_IRQ2_MASK 0x04
#define PIC2_CASCADE_ID 0x02

#define ICW4_8086    0x01 // 8086/88 Mode

void remap_pic();
void unmask_irq(uint8_t irq);
#endif //MYKERNELPROJECT_PIC_H
