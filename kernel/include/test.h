#ifndef _TEST_H
#define _TEST_H

#include <stdint.h>

void assert_null(const char* msg, const uintmax_t actual);
void assert_not_null(const char* msg, const uintmax_t actual);
void assert_uint(const char* msg, const uintmax_t expected, const uintmax_t actual);

#endif
