#include <kernel/panic.h>
#include <kernel/tty.h>
#include <stdio.h>

void kernel_panic(const char* file, unsigned int line, const char* msg) {
	printf("\n\nKernel panic: %s (%s line %u)", msg, file, line);
	halt();
}