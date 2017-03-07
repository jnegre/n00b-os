#include <threads.h>
#include <kernel/sched.h>

void thrd_yield() {
	sched_yield();
}