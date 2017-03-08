#include <threads.h>
#include <stddef.h>
#include <kernel/sched.h>

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg) {
	sched_new_thread(thr, func, arg, PRIORITY_NORMAL);
	return thrd_success; // we panicked otherwise
}