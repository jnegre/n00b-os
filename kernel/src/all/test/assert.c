#include <test.h>
#include <stdint.h>
#include <kernel/tty.h>

void assert_null(const char* msg, const uintmax_t actual) {
    if(actual != 0) {
        terminal_writestring("\nASSERT ");
        terminal_writestring(msg);
        terminal_writestring("\n");        
    }
}

void assert_not_null(const char* msg, const uintmax_t actual) {
    if(actual == 0) {
        terminal_writestring("\nASSERT ");
        terminal_writestring(msg);
        terminal_writestring("\n");        
    }
}

void assert_uint(const char* msg, const uintmax_t expected, const uintmax_t actual) {
    if(expected != actual) {
        terminal_writestring("\n ASSERT");
        terminal_writestring(msg);
        terminal_writestring(": ");
        terminal_writeu64(actual);
        terminal_writestring(" vs ");
        terminal_writeu64(expected);
        terminal_writestring("\n");
    }
}
