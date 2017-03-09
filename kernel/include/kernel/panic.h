#ifndef _KERNEL_PANIC_H
#define _KERNEL_PANIC_H
#include <stdnoreturn.h>

#define panic(msg) kernel_panic(__FILE__, __LINE__, msg)

/* Prints a message and stops the computer.
 */
noreturn void kernel_panic(const char* file, unsigned int line, const char* msg);

/* Stops the computer.
 */
noreturn void halt(void);

 #endif