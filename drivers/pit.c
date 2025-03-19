//
// Created by Yoav on 3/18/2025.
//

#include "pit.h"
#include "../interupts/idt.h"
#include "screen.h"
#include "io.h"
#include "../interupts/pic.h"

static size_t frequency = 1;

void pit_handler() {
    put_string("Tick   ");
    pic_send_ack();
}

void pit_init() {

    uint16_t divisor = PIT_CLOCK_FREQUENCY / frequency;
    // Set the PIT MODE, channel, access mode and binary mode
    outb(PIT_COMMAND, PIT_COMMAND_CHANNEL_0 | PIT_ACCESS_MODE_16_BIT | PIT_CLOCK_MODE | PIT_BINARY_MODE);
    // Set the frequency
    outb(PIT_CHANNEL_0, divisor & 0xFF);
    outb(PIT_CHANNEL_0, (divisor >> 8) & 0xFF);
    unmask_irq(PIT_IRQ);
}