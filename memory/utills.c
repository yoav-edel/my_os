//
// Created by Yoav on 11/30/2024.
//

#include "utills.h"

void memset(void *ptr, uint8_t value, size_t num){
    uint8_t *p = (uint8_t*)ptr;
    for(size_t i = 0; i < num; i++){
        p[i] = value;
    }
}

void memcpy(void *dest, const void *src, size_t num){
    uint8_t *d = (uint8_t*)dest;
    uint8_t *s = (uint8_t*)src;
    for(size_t i = 0; i < num; i++){
        d[i] = s[i];
    }
}
