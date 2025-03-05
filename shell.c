//
// Created by Yoav on 1/8/2025.
//

#include "shell.h"
#include "drivers/screen.h"
#include "drivers/keyboard.h"
#include "std/string.h"
#include "std/stdlib.h"
// Main shell function
void shell() {
    char input[MAX_INPUT_LENGTH]; // Buffer for user input

    // Display a welcome message
    shell_welcome_message();

    // Shell loop
    while (1) {
        // Display the prompt
        shell_prompt();

        // Read user input
        int index = 0;
        char c;
        do {
            c = keyboard_buffer_get(); // Read a character from the keyboard buffer

            if (c == '\n') { // Enter key
                break; // End input on Enter
            } else if (c == '\b') { // Backspace key
                if (index > 0) {
                    index--;
                    put_string("\b \b"); // Remove character visually
                }
            } else {
                if (index < MAX_INPUT_LENGTH - 1) {
                    input[index++] = c;
                }
            }
        } while (1);

        input[index] = '\0'; // Null-terminate the input string

        // Execute the entered command
        execute_command(input);
    }
}

// Display the welcome message
void shell_welcome_message() {
    clear_screen(); // Clear the screen initially
    put_string("Welcome to Enhanced Shell!\n");
    put_string("Type 'help' to see available commands.\n\n");
}

// Display the shell prompt
void shell_prompt() {
    put_string("EnhancedShell> ");
}

// Execute commands entered in the shell
void execute_command(const char *input) {
    if (!strcmp(input, "help")) {
        put_string("\nAvailable commands:\n");
        put_string("  clear         - Clears the screen\n");
        put_string("  echo [text]   - Echoes the input text\n");
        put_string("  color [num]   - Changes the text color\n");
        put_string("  scroll        - Scrolls the screen\n");
        put_string("  clearrow [n]  - Clears a specific row (0-24)\n");
        put_string("  exit          - Exits the shell\n");
    } else if (!strcmp(input, "clear")) {
        clear_screen();
    } else if (!strncmp(input, "echo ", 5)) {
        put_string("\n");
        put_string(input + 5); // Print everything after "echo "
        put_string("\n");
    } else if (!strncmp(input, "color ", 6)) {
        int color_code = atoi(input + 6); // Convert argument to an integer
        if (color_code >= 0 && color_code <= 255) {
            set_color(color_code); // Set the color
            put_string("\nText color changed.\n");
        } else {
            put_string("\nInvalid color code! Use a value between 0 and 255.\n");
        }
    } else if (!strcmp(input, "scroll")) {
        put_string("\nScrolling the screen...\n");
        scroll_screen();
    } else if (!strncmp(input, "clearrow ", 9)) {
        int row = atoi(input + 9); // Parse the row number
        if (row >= 0 && row < VGA_HEIGHT) {
            clear_row(row); // Clear the specified row
            put_string("\nRow cleared.\n");
        } else {
            put_string("\nInvalid row number! Use a value between 0 and ");
            put_int(VGA_HEIGHT - 1);
            put_string(".\n");
        }
    } else if (!strcmp(input, "exit")) {
        put_string("\nExiting Enhanced Shell. Goodbye!\n");
        while (1) {
            // Infinite loop to halt
        }
    } else {
        put_string("\nUnknown command: ");
        put_string(input);
        put_string("\nType 'help' for a list of commands.\n");
    }
}