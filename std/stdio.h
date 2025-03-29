//
// Created by Yoav on 3/29/2025.
//

#ifndef STDIO_H
#define STDIO_H
#include "../drivers/screen.h"
#include "stdbool.h"
//printf function

/**
 * @brief Converts and prints a 32-bit unsigned integer as hexadecimal
 * @param num The number to be converted and printed
 */
void put_hexa(uint32_t num);

/**
 * @brief Converts and prints a 32-bit address as hexadecimal
 * @param addr The address to be printed
 */
void put_addr(uint32_t addr);

/**
 * @brief Prints boolean value as "true" or "false" string
 * @param b The boolean value to print
 */
void put_bool(bool b);

/**
 * @brief Formatted print function similar to standard printf
 * @param format Format string with the following specifiers:
 *        %s - string
 *        %d - integer
 *        %p - pointer (address with 0x prefix)
 *        %x - hexadecimal (with 0x prefix)
 *        %c - character
 *        %b - boolean (prints "true" or "false")
 *        %% - percent sign
 * @param ... Variable arguments corresponding to format specifiers
 */
void printf(const char *format, ...);

#endif //STDIO_H
