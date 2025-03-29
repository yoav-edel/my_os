//
// Created by Yoav on 3/29/2025.
//

#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"

#define HEXA_SIZE 8
static void int_to_hex_string(uint32_t num, char *buffer) {
    static const char *hex_chars = "0123456789ABCDEF";
    for (int i = HEXA_SIZE; i >= 0; i--) {
        buffer[i] = hex_chars[num & 0xF];
        num >>= 4;
    }
    buffer[8] = '\0'; // Null-terminate the string
}

void put_hexa(uint32_t num) {
    char buffer[HEXA_SIZE + 1]; // 8 hex digits + null terminator
    int_to_hex_string(num, buffer);
    put_string(buffer);
}

void put_addr(uint32_t addr) {
    char buffer[HEXA_SIZE + 1]; // 8 hex digits + null terminator
    int_to_hex_string(addr, buffer);
    put_string(buffer);
}

void put_bool(bool b) {
    if (b)
        put_string("true");
    else
        put_string("false");
}


void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    for (; *format != '\0'; format++) {
        if (*format == '%') {
            format++; // Move past '%'
            if (*format == '\0')
                break; // premature end
            switch (*format) {
                case 's': {
                    char *s = va_arg(args, char*);
                    put_string(s);
                    break;
                }
                case 'd': {
                    int d = va_arg(args, int);
                    put_int(d);
                    break;
                }
                case 'p': {
                  void *p = va_arg(args, void*);
                  put_string("0x");
                  put_addr((uint32_t)p);
                  break;
                }
                case 'x': {
                    unsigned int x = va_arg(args, unsigned int);
                    put_string("0x");
                    put_hexa(x);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, char);
                    put_char(c);
                    break;
                }
                case 'b': {
                    bool b = va_arg(args, int);
                    put_bool(b);
                    break;
                }

                case '%': {
                    put_char('%');
                    break;
                }
                default: {
                    put_char('%');
                    put_char(*format);
                    break;
                }
            }
        } else {
            put_char(*format);
        }
    }

    va_end(args);
}