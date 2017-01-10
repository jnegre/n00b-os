#include <kernel/tty.h>

void panic(const char* msg) {
	terminal_writestring("\n\nKernel panic: ");
	terminal_writestring(msg);
	halt();
}
