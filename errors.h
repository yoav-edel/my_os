//
// Created by Yoav on 11/30/2024.
//

#ifndef MYKERNELPROJECT_ERRORS_H
#define MYKERNELPROJECT_ERRORS_H
#include "drivers/screen.h"
inline void panic(const char *msg)
{
    put_string("Kernel Panic: ");
    put_string(msg);
    while(1);

}
#endif //MYKERNELPROJECT_ERRORS_H
