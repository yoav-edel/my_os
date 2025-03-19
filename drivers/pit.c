//
// Created by Yoav on 3/18/2025.
//

#include "pit.h"
#include "../interrupts/idt.h"
#include "vga.h"

static size_t frequency = 1000;

static void pit_handler() {
    put_string("Tick\n");
}

void pit_init() {

    uint16_t divisor = PIT_CLOCK_FREQUENCY / frequency;
    // Set the PIT MODE, channel, access mode and binary mode
    outb(PIT_COMMAND, PIT_COMMAND_CHANNEL_0 | PIT_ACCESS_MODE_16_BIT | PIT_CLOCK_MODE | PIT_BINARY_MODE);
    // Set the frequency
    outb(PIT_CHANNEL_0, divisor & 0xFF);
    outb(PIT_CHANNEL_0, (divisor >> 8) & 0xFF);
    // Enable the PIT
    idt_irq_install_handler(PIT_IRQ, pit_handler);
}