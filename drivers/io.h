//
// Created by Yoav on 11/27/2024.
//

#ifndef MYKERNELPROJECT_IO_H
#define MYKERNELPROJECT_IO_H


#include "../std/stdint.h"


// Write a byte to a port
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Read a byte from a port
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a 16-bit to a port
static inline void out16(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

// Read a 16-bit from a port
static inline uint16_t in16(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Write a 32-bit from a port
static inline void out32(uint16_t port, uint32_t value) {
    asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Read a 32-bit from a port
static inline uint32_t in32(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}







#endif //MYKERNELPROJECT_IO_H
