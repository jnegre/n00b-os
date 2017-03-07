#include <threads.h>
#include <kernel/sched.h>

void thrd_exit(int res) {
	sched_exit(res);
}