//
// Created by Yoav on 1/6/2025.
//

#ifndef MYKERNELPROJECT_ASSERT_H
#define MYKERNELPROJECT_ASSERT_H
#define assert(expr) ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__, __func__))
//todo optimize this function
void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function);
#endif //MYKERNELPROJECT_ASSERT_H
