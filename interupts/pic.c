#include "../std/assert.h"
#include "../std/stdint.h"
#include "pic.h"
#include "../drivers/io.h"

void remap_pic() {
    // Disable interrupts
    asm volatile("cli");

    // Save masks
    unsigned char master_mask = inb(PIC1_DATA);
    unsigned char slave_mask = inb(PIC2_DATA);

    // Start the initialization sequence (in cascade mode)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // Set vector offset
    outb(PIC1_DATA, PIC1_VECTOR_OFFSET); // Master PIC vector offset
    outb(PIC2_DATA, PIC2_VECTOR_OFFSET); // Slave PIC vector offset
    // Tell Master PIC there is a slave PIC at IRQ2
    outb(PIC1_DATA, PIC1_IRQ2_MASK); // ICW3: Bit mask for IRQ2
    outb(PIC2_DATA, PIC2_CASCADE_ID); // ICW3: Slave PIC's cascade identity

    // Set PICs to operate in 8086/88 mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Restore saved masks
    outb(PIC1_DATA, master_mask);
    outb(PIC2_DATA, slave_mask);

    // Re-enable interrupts
    asm volatile("sti");
}



void unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    assert(irq < IRQ_NUM); // The system only supports IRQ_NUM IRQs

    if (irq < PIC_IRQ_AMOUNT) {
        port = PIC1_DATA; // Master PIC
    } else {
        port = PIC2_DATA; // Slave PIC
        irq -= PIC_IRQ_AMOUNT; // Adjust for slave PIC
    }

    value = inb(port) & ~(1 << irq); // Clear the mask for this IRQ
    outb(port, value);
}
