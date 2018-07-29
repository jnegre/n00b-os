#ifndef _STDLIB_H
#define _STDLIB_H
#include <stddef.h>

void abort(void) __attribute__((__noreturn__));

void* malloc( size_t size );
void free( void* ptr );
void* realloc (void* ptr, size_t size);

#endif 