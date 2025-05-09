//
// Created by Yoav on 1/7/2025.
//
#include "stdlib.h"
#include "stdbool.h"
/*
 * This file is my implementation of my version of the gnu stdlib file
 */

/**
 * Converts an integer to a 10base string.
 * @param num The integer to convert.
 * @param str The buffer to store the string.
 */
void int_to_string(int num, char *str) {
    int i = 0;
    int is_negative = 0;

    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // Convert the number to a string in reverse order
    do {
        str[i++] = num % 10 + '0';
        num /= 10;
    } while (num > 0);

    // Add the negative sign if needed
    if (is_negative) {
        str[i++] = '-';
    }

    // Terminate the string
    str[i] = '\0';

    // Reverse the string
    int j = 0;
    char temp;
    for (j = 0; j < i / 2; j++) {
        temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

/**
 * Converts a string to an integer.
 * @param str The string to convert.
 * @return The integer value.
 */
int atoi(const char *str) {
    int result = 0;
    unsigned int i = 0;
    bool is_negative = 0;

    // Handle negative numbers
    if (str[0] == '-') {
        is_negative = true;
        i++;
    }

    // Convert the string to an integer
    while (str[i] != '\0') {
        result = result * 10 + str[i] - '0';
        i++;
    }

    return is_negative ? -result : result;
}