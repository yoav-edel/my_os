//
// Created by Yoav on 11/24/2024.
//
#include "screen.h"
#include "../std/stdlib.h"

static unsigned int pos_on_screen = 0; // Start at the top-left corner of the screen
static uint8_t color = WHITE_ON_BLACK; // Default color
/*
 * Clears the entire screen by setting all rows to EMPTY_LETTER.
 */
void clear_screen() {
    struct letter *video_memory = (struct letter *)VGA_ADDRESS;
    for (int i = 0; i < VGA_MAX_CHARS; i++) {
        video_memory[i] = EMPTY_LETTER;
    }
    pos_on_screen = 0; // Reset cursor to the start
}

/*
 * Clears one row of the screen by setting it to EMPTY_LETTER.
 */
void clear_row(int row) {
    if (row < 0 || row >= VGA_HEIGHT) {
        return; // Ignore invalid rows
    }
    struct letter *video_memory = (struct letter *)VGA_ADDRESS;
    unsigned int pos = row * VGA_WIDTH; // Calculate row start position
    for (int i = 0; i < VGA_WIDTH; i++) {
        video_memory[pos + i] = EMPTY_LETTER;
    }
}


/*
 * Scrolls the screen by moving all rows up by one and clearing the last row.
 */
void scroll_screen() {
    struct letter *video_memory = (struct letter *)VGA_ADDRESS;

    // Move rows up by copying each row to the one above it
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            video_memory[(row - 1) * VGA_WIDTH + col] =
                    video_memory[row * VGA_WIDTH + col];
        }
    }

    // Clear the last row
    clear_row(VGA_HEIGHT - 1);

    // Adjust cursor position to the start of the last row
    pos_on_screen = (VGA_HEIGHT - 1) * VGA_WIDTH;
}

/*
 * Puts a character on the screen at the current cursor position.
 * If the screen is full, it scrolls before writing.
 */
void put_char(uint8_t c) {
    struct letter *video_memory = (struct letter *)VGA_ADDRESS;

    // Scroll if the cursor reaches the end of the screen
    if (pos_on_screen >= VGA_MAX_CHARS) {
        scroll_screen();
    }

    // Handle newline character
    if (c == '\n') {
        pos_on_screen = (pos_on_screen / VGA_WIDTH + 1) * VGA_WIDTH;
        return;
    }

    // Write the character to the current cursor position
    video_memory[pos_on_screen].c = c;
    video_memory[pos_on_screen].color = color;

    // Move the cursor forward
    pos_on_screen++;
}

/*
 * Puts an integer on the screen at the current cursor position.
 */
void put_int(int num) {
    // Convert the integer to a string
    char num_str[12]; // Longest int is -2147483648 (11 characters) + null terminator
    int_to_string(num, num_str);

    // Display the string
    put_string(num_str);
}

/*
 * Puts a null-terminated string on the screen starting at the current cursor position.
 */
void put_string(const char *str) {
    while (*str) {
        put_char(*str++);
    }
}


void set_color(uint8_t new_color) {
    color = new_color;
}

void set_color_to_default() {
    color = WHITE_ON_BLACK;
}

void move_cursor_back() {
    struct letter *video_memory = (struct letter *)VGA_ADDRESS;

    while (pos_on_screen > 0) {
        pos_on_screen--;

        if (video_memory[pos_on_screen].c != EMPTY_LETTER.c) {
            video_memory[pos_on_screen] = EMPTY_LETTER;
            break;
        }
    }
}

