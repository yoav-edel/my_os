//
// Created by Yoav on 3/18/2025.
//

/**
 * This file implements driver for the Programmable Interval Timer (PIT 8253/8254).
 */
#ifndef MYKERNEL_PIT_H
#define MYKERNEL_PIT_H

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42
#define PIT_COMMAND 0x43 // write only
/*
 * Command format:
 * Bits         Usage
   7 and 6      Select channel(which timer to use)
   5 and 4      Access mode(what part of the counter to use)
   3 to 1      Operating mode
   0            BCD/Binary mode
 */

#define PIT_COMMAND_CHANNEL_0 0x00
#define PIT_COMMAND_CHANNEL_1 0x40
#define PIT_COMMAND_CHANNEL_2 0x80
#define PIT_COMMAND_READ_BACK 0xC0

#define PIT_ACCESS_MODE_16_BIT 0x30

#define PIT_OPERATING_MODE_SQUARE_WAVE 0x06
#define PIT_CLOCK_MODE PIT_OPERATING_MODE_SQUARE_WAVE

#define PIT_BINARY_MODE 0x00

#define PIT_CLOCK_FREQUENCY 1193182


#define PIT_IRQ 0

/**
 * Initialize the PIT:
 * 1. Set the PIT to the desired frequency.
 * 2. Set the desired operating mode.
 * 3. Set the desired access mode.
 * 4. Set the desired channel.
 * 5. Set the desired binary mode.
 * 6. Enable the PIT.
 * 7. Enable the IRQ0.
 *
 */
void pit_init();


#endif //MYKERNEL_PIT_H
