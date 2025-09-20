//
// Created by Yoav on 1/8/2025.
//

#ifndef MYKERNELPROJECT_STRING_H
#define MYKERNELPROJECT_STRING_H
#include "stdint.h"

int strlen(const char *str);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, int n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, int n);

int memcmp(const void *_Buf1, const void *_Buf2, const size_t _Size);


#endif //MYKERNELPROJECT_STRING_H
