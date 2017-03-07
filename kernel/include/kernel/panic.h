#ifndef _KERNEL_PANIC_H
#define _KERNEL_PANIC_H
#include <stdnoreturn.h>

/* Prints a message and stops the computer.
 */
noreturn void panic(const char*);

/* Stops the computer.
 */
noreturn void halt(void);

 #endif