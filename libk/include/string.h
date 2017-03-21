#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

int memcmp(const void*, const void*, size_t);
void* memcpy(void* restrict, const void* restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);

char *strcpy(char *restrict, const char *restrict);

int strcmp(const char*, const char*);
size_t strlen(const char*);

#endif
