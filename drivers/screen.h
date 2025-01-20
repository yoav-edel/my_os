//
// Created by Yoav on 11/24/2024.
//

#ifndef MYKERNELPROJECT_SCREEN_H
#define MYKERNELPROJECT_SCREEN_H

#include "../std/stdint.h"

// VGA buffer constants
#define VGA_ADDRESS 0xB8000 // VGA text buffer start address
#define VGA_WIDTH 80        // Number of characters per row
#define VGA_HEIGHT 25       // Number of rows
#define VGA_BUFFER_SIZE (VGA_WIDTH * VGA_HEIGHT) // Total number of characters
#define VGA_MAX_CHARS VGA_BUFFER_SIZE            // Total usable characters

// Default text color
#define WHITE_ON_BLACK 0x07

// Structure representing a character in the VGA buffer
struct letter {
    uint8_t c;     // ASCII character
    uint8_t color; // Foreground and background color
};

// Define an empty letter macro
#define EMPTY_LETTER (struct letter){'\0', WHITE_ON_BLACK}

// Function declarations

/**
 * Clears the entire screen by setting all rows to EMPTY_LETTER.
 */
void clear_screen();

/**
 * Clears one row of the screen by setting it to EMPTY_LETTER.
 *
 * @param row The row index to clear (0-based).
 */
void clear_row(int row);

/**
 * Scrolls the screen by moving all rows up by one and clearing the last row.
 */
void scroll_screen();

/**
 * Displays a character at the current cursor position.
 * Scrolls the screen if the cursor reaches the end of the screen.
 *
 * @param c The character to display.
 */
void put_char(uint8_t c);

/**
 * Display a integer at the current cursor position.
 */
void put_int(int num);

/**
 * Displays a null-terminated string starting at the current cursor position.
 *
 * @param str The string to display.
 */
void put_string(const char *str);

/**
 * Sets the text color for future characters.
 *
 * @param new_color The new color value (foreground and background).
 */
void set_color(uint8_t new_color);

/**
 * Resets the text color to the default (WHITE_ON_BLACK).
 */
void set_color_to_default();


/**
 * moves the cursor back by one character(none empty character - not null)
 */
void move_cursor_back();

#endif // MYKERNELPROJECT_SCREEN_H
