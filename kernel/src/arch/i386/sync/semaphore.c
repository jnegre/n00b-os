#include <kernel/sync.h>
#include <kernel/sched.h>

void semaphore_acquire(semaphore_t* semaphore) {
	uint32_t tid = current_process_control_block()->tid;
	asm(
		"spin_lock:"
		"	cmp $0, %0;" // is the lock free?
		"	je get_lock;"
		"	call sched_yield;"
		"	jmp spin_lock;"
		"get_lock:"
		"	xchg %%eax, %0;" // try to get lock
		"	cmp $0, %%eax;" // test if successful
		"	jne spin_lock;"
		: [output] "=m" (semaphore->lock)
		: [input] "a"(tid)
	);
}

void semaphore_release(semaphore_t* semaphore) {
	semaphore->lock = 0;
}
