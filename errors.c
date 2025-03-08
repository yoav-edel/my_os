//
// Created by Yoav on 3/7/2025.
//
#include "errors.h"
#include "drivers/screen.h"

void panic(const char *msg) {
    put_string("Kernel Panic: ");
    put_string(msg);
    while (1);
}
