//
// Created by Yoav on 1/6/2025.
//

/*
 * This file is my implementation of the stdint.h header file.
 */
#ifndef MYKERNELPROJECT_STDINT_H
#define MYKERNELPROJECT_STDINT_H
typedef __INT8_TYPE__ int8_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __INT16_TYPE__ int16_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __INT32_TYPE__ int32_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __INT64_TYPE__ int64_t;
typedef __UINT64_TYPE__ uint64_t;
#define NULL ((void *)0)

typedef __SIZE_TYPE__ size_t;
#endif //MYKERNELPROJECT_STDINT_H
