//
// Created by Yoav on 3/18/2025.
//

#include "pit.h"
#include "../interupts/idt.h"
#include "screen.h"
#include "io.h"
#include "../interupts/pic.h"
#include "../processes/process.h"
#include "../processes/scheduler.h"
#include "../std/stdio.h"

static size_t frequency = 100;

/**
 * @brief Handles the Programmable Interval Timer (PIT) interrupt.
 *
 * Acknowledges the interrupt to the Programmable Interrupt Controller (PIC) and notifies the scheduler to perform a tick update for process management.
 */
void pit_handler() {
    pic_send_ack();
    scheduler_handle_tick();
}

/**
 * @brief Initializes the Programmable Interval Timer (PIT) with the configured frequency.
 *
 * Configures the PIT hardware to generate periodic interrupts at the specified frequency,
 * clamping the value to a safe range. Programs the PIT command and data registers and
 * enables the PIT interrupt line on the Programmable Interrupt Controller (PIC).
 */
void pit_init() {
    // Validate frequency to prevent divisor overflow and ensure a reasonable rate
    if (frequency < 20) {
        printf("PIT frequency too low, clamping to 20Hz.\n");
        frequency = 20; // Minimum frequency
    }
    if (frequency > PIT_CLOCK_FREQUENCY) {
        printf("PIT frequency too high, clamping to PIT_CLOCK_FREQUENCY.\n");
        frequency = PIT_CLOCK_FREQUENCY; // Maximum frequency
    }

    uint16_t divisor = PIT_CLOCK_FREQUENCY / frequency;
    // Set the PIT MODE, channel, access mode and binary mode
    outb(PIT_COMMAND, PIT_COMMAND_CHANNEL_0 | PIT_ACCESS_MODE_16_BIT | PIT_CLOCK_MODE | PIT_BINARY_MODE);
    // Set the frequency
    outb(PIT_CHANNEL_0, divisor & 0xFF);
    outb(PIT_CHANNEL_0, (divisor >> 8) & 0xFF);
    unmask_irq(PIT_IRQ);
}

