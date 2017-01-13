#include <kernel/tty.h>
extern void halt(void);
void panic(const char* msg) {
	terminal_writestring("\n\nKernel panic: ");
	terminal_writestring(msg);
	halt(); //FIXME not declared
}
