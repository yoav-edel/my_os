//
// Created by Yoav on 11/27/2024.
//

#include <stdbool.h>
#include "keyboard.h"
#include "io.h"
#include "screen.h"
#include "../interupts/pic.h"

static char keyboard_buffer[BUFFER_SIZE] = {0};
static volatile uint8_t buffer_head = 0;
static volatile uint8_t buffer_tail = 0;



inline static bool is_keyboard_buffer_Empty()
{
    return buffer_head == buffer_tail;
}

void keyboard_handler()
{
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    handle_scancode(scancode);
    send_ack_keyboard();
}

static bool shift_pressed = false;
static bool put_on_screen = true;




void handle_scancode(uint8_t scancode)
{
    if(is_key_released(scancode))
    {
        scancode -= KEYRELEASE_MASK;
        // Handle the key release
        if(scancode == SHIFT_KEY)
            shift_pressed = false;
        //todo add more key releases like caps lock
    }
    else // key was pressed
    {
        //todo add more special keys like caps lock
        // Handle the key press
        if(scancode == SHIFT_KEY)
            shift_pressed = true;

        else
        {
            char c = scancode_to_ascii(scancode);
            if(shift_pressed)
            {
                c = shift_ascii(c);
            }

            // Add the character to the buffer
            keyboard_buffer_put(c);

            if(put_on_screen) {
                put_char(c);
            }
        }
    }

}

void keyboard_buffer_put(char c) {
    keyboard_buffer[buffer_head] = c;
    buffer_head = (buffer_head + 1) % BUFFER_SIZE;
}

void set_put_on_screen(bool value)
{
    put_on_screen = value;
}

char keyboard_buffer_get() {
    while(is_keyboard_buffer_Empty());
    char c = keyboard_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;
    return c;
}


char scancode_to_ascii(uint8_t scancode)
{
    static char kbd_us[128] = {
            0,  27, '1', '2', '3', '4', '5', '6', '7', '8', // 0x00 - 0x09
            '9', '0', '-', '=', '\b', // Backspace
            '\t',                     // Tab
            'q', 'w', 'e', 'r',       // 0x10 - 0x13
            't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   // Enter key
            0,                        // Control
            'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 0x1E - 0x27
            '\'', '`', 0,             // Left shift
            '\\', 'z', 'x', 'c', 'v', 'b', 'n',             // 0x2B - 0x31
            'm', ',', '.', '/', 0,    // Right shift
            '*', 0,                   // Alt and Space are handled elsewhere
            ' ', 0,                   // CapsLock
            // ... fill in the rest if necessary
    };

    if (scancode > 128) {
        return 0;
    } else {
        return kbd_us[scancode];
    }
}

char shift_ascii(char c)
{
    if(c >= 'a' && c <= 'z')
    {
        return c - 32;
    }

    switch (c) {
        case '1':
            return '!';
        case '2':
            return '@';
        case '3':
            return '#';
        case '4':
            return '$';
        case '5':
            return '%';
        case '6':
            return '^';
        case '7':
            return '&';
        case '8':
            return '*';
        case '9':
            return '(';
        case '0':
            return ')';
        case '-':
            return '_';
        case '=':
            return '+';
        case '[':
            return '{';
        case ']':
            return '}';
        case '\\':
            return '|';
        case ';':
            return ':';
        case '\'':
            return '"';
        case ',':
            return '<';
        case '.':
            return '>';
        case '/':
            return '?';
        case '`':
            return '~';
        default:
            return c;
    }
}