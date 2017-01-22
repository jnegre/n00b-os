#include <stdlib.h>
#include <kernel/panic.h>

void abort(void) {
    panic("abort");
}