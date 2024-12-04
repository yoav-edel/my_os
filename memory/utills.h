//
// Created by Yoav on 11/30/2024.
//

#ifndef MYKERNELPROJECT_UTILLS_H
#define MYKERNELPROJECT_UTILLS_H
#include <stdint.h>

void memset(void *ptr, uint8_t value, size_t num);
void memcpy(void *dest, const void *src, size_t num);

#endif //MYKERNELPROJECT_UTILLS_H
