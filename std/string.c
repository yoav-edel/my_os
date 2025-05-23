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

/**
 * @brief Compares up to n characters of two strings lexicographically.
 *
 * Compares the first n characters of the null-terminated strings str1 and str2.
 * Returns the difference between the first pair of differing characters, or zero if no difference is found within n characters.
 *
 * @param str1 First string to compare.
 * @param str2 Second string to compare.
 * @param n Maximum number of characters to compare.
 * @return int Negative value if str1 < str2, zero if equal up to n characters, positive value if str1 > str2.
 */
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

/**
 * @brief Copies a null-terminated string to a destination buffer.
 *
 * Copies the string pointed to by src, including the terminating null byte, to the buffer pointed to by dest.
 * The destination buffer must be large enough to receive the copy.
 *
 * @param dest Pointer to the destination buffer.
 * @param src Pointer to the null-terminated source string.
 * @return Pointer to the destination buffer dest.
 */
char *strcpy(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/**
 * @brief Copies up to n characters from the source string to the destination buffer.
 *
 * Copies at most n characters from the null-terminated string src to dest. If src is shorter than n characters, the remainder of dest is padded with null bytes. If src is longer than or equal to n, no null terminator is added to dest.
 *
 * @param dest Destination buffer to copy to.
 * @param src Source null-terminated string to copy from.
 * @param n Maximum number of characters to copy.
 * @return Pointer to the destination buffer dest.
 */
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
