#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>
#include <stdint.h>

//TODO terminal -> tty
void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writeu32(const uint32_t hex);
void terminal_writeu64(const uint64_t hex);
void terminal_writestring(const char* data);

#endif

