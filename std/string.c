//
// Created by Yoav on 1/8/2025.
//

#include "string.h"

int strlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char *str1, const char *str2) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return str1[i] - str2[i];
        }
        i++;
    }
    return str1[i] - str2[i];
}

int strncmp(const char *str1, const char *str2, int n) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0' && i < n) {
        if (str1[i] != str2[i]) {
            return str1[i] - str2[i];
        }
        i++;
    }
    if (i == n) {
        return 0;
    }
    return str1[i] - str2[i];
}

char *strcpy(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

char *strncpy(char *dest, const char *src, int n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    // Null-pad the remainder of dest if src is shorter than n
    for ( ; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

int memcmp(const void *_Buf1, const void *_Buf2, const size_t _Size) {
    const uint8_t *buf1 = (const uint8_t *) _Buf1;
    const uint8_t *buf2 = (const uint8_t *) _Buf2;
    for (size_t i = 0; i < _Size; i++) {
        if (buf1[i] != buf2[i]) {
            return buf1[i] - buf2[i];
        }
    }
    return 0;
}
