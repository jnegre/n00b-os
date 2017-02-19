#include <kernel/sync.h>
#include <kernel/sched.h>

void mutex_acquire(mutex_t* mutex) {
	uint32_t tid = current_process_control_block()->tid;
	asm(
		"spin_lock:"
		"	cmp %2, %0;" // do we already have the lock?
		"	je got_lock;"
		"	cmp $0, %0;" // is the lock free?
		"	je get_lock;"
		"	call sched_yield;"
		"	jmp spin_lock;"
		"get_lock:"
		"	xchg %%eax, %0;" // try to get lock
		"	cmp $0, %%eax;" // test if successful
		"	jne spin_lock;"
		"	movw $0, %1;" // should be useless, but who knows ¯\(°_o)/¯
		"got_lock:"
		"	incw %1;"
		: [output] "=m" (mutex->lock), "=m" (mutex->level)
		: [input] "a"(tid)
	);
}

void mutex_release(mutex_t* mutex) {
	if(--mutex->level == 0) {
		mutex->lock = 0;
	}
}
