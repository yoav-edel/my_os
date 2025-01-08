//
// Created by Yoav on 1/8/2025.
//

#include "shell.h"
#include "drivers/screen.h" // For screen driver functionality
#include "drivers/keyboard.h" // For keyboard driver functionality#include "std/string.h"      // For string operations
#include "std/string.h" // For string operations

// Main shell function
void shell() {
    char input[MAX_INPUT_LENGTH]; // Buffer for user input

    // Display a welcome mesage
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
                put_char(c); // Echo newline to the screen
                break;
            } else if (c == '\b') { // Backspace key
                if (index > 0) {
                    index--;
                    put_string("\b \b"); // Handle backspace on screen
                }
            } else {
                if (index < MAX_INPUT_LENGTH - 1) {
                    input[index++] = c;
                    put_char(c); // Echo the character to the screen
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
    clear_screen();
    set_color(WHITE_ON_BLACK);
    put_string("Welcome to Simple Shell!\n");
    put_string("Type 'help' to see available commands.\n\n");
    set_color_to_default();
}

// Display the shell prompt
void shell_prompt() {
    set_color(WHITE_ON_BLACK);
    put_string("SimpleShell> ");
    set_color_to_default();
}

// Execute commands entered in the shell
void execute_command(const char *input) {
    if (strcmp(input, "help") == 0) {
        put_string("\nAvailable commands:\n");
        put_string("  clear  - Clears the screen\n");
        put_string("  echo   - Echoes the input text\n");
        put_string("  exit   - Exits the shell\n");
    } else if (strcmp(input, "clear") == 0) {
        clear_screen();
    } else if (strncmp(input, "echo ", 5) == 0) {
        put_string("\n");
        put_string(input + 5); // Print everything after "echo "
        put_string("\n");
    } else if (strcmp(input, "exit") == 0) {
        put_string("\nExiting Simple Shell. Goodbye!\n");
        while (1) {
            // Infinite loop to halt
        }
    } else {
        put_string("\nUnknown command: ");
        put_string(input);
        put_string("\nType 'help' for a list of commands.\n");
    }
}

