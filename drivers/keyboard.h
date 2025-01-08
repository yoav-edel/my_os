//
// Created by Yoav on 11/27/2024.
//

#ifndef MYKERNELPROJECT_KEYBOARD_H
#define MYKERNELPROJECT_KEYBOARD_H


#include "../std/stdint.h"
#include "../std/stdbool.h"
void keyboard_handler();

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYRELEASE_MASK 0x80

#define SHIFT_KEY 0x2A


#define BUFFER_SIZE (256)



static bool is_key_released(uint8_t scancode)
{
    return scancode & KEYRELEASE_MASK;
}


char scancode_to_ascii(uint8_t scancode);
char shift_ascii(char c);
void keyboard_buffer_put(char c);
void init_keyboard();
void handle_scancode(uint8_t scancode);
char keyboard_buffer_get();
#endif //MYKERNELPROJECT_KEYBOARD_H
