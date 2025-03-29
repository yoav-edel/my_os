//
// Created by Yoav on 3/29/2025.
//

#ifndef STDARG_H
#define STDARG_H

#include "stdint.h"

typedef char *va_list;

#define va_start(ap, param) (ap = (va_list)&param + sizeof(param))
#define va_arg(ap, type) (*(type *)((ap += sizeof(type)) - sizeof(type)))
#define va_end(ap) (ap = NULL)
#endif //STDARG_H
