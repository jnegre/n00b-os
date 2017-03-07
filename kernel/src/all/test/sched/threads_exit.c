#include <kernel/sched.h>
#include <kernel/panic.h>
#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static void thread(void* integer) {
	uint32_t i = (uint32_t)integer;
	for(int j=0; j<5; j++) {
		printf("[%u]", i);
		sched_sleep(i * 1000);
	}
	thrd_exit(i);
	panic("After sched_exit()");
}

void test_sched_threads_exit(void) {
	printf("Lauching 3 test threads, 5 logs each\n");
	for(int i=1; i<=3; i++) {
		sched_new_thread(&thread, (void*)i, PRIORITY_NORMAL);
	}
}