#include <threads.h>
#include <kernel/sched.h>

thrd_t thrd_current(void) {
	return current_process_control_block()->tid;
}