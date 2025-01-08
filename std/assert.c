//
// Created by Yoav on 1/6/2025.
//

#include "../std/assert.h"
#include "../drivers/screen.h"
#include "../std/stdlib.h"


void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function) {
    // Clear the screen and print the assertion failure message
    clear_screen();
    put_string("Assertion failed: ");
    put_string(assertion);
    put_string("\nFile: ");
    put_string(file);
    put_string("\nLine: ");

    // Convert line number to string
    char line_str[16];
    int_to_string(line, line_str);
    put_string(line_str);

    if (function) {
        put_string("\nFunction: ");
        put_string(function);
    }

    put_string("\nSystem halted.\n");

    // Halt the system
    while (1) {
        asm volatile("hlt");
    }
}