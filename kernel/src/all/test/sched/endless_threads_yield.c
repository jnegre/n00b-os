#include <kernel/sched.h>
#include <test.h>
#include <stdio.h>
#include <stdlib.h>

static void thread(void* integer) {
	uint32_t i = (uint32_t)integer;
	while(true) {
		printf("[%u]", i);
		sched_yield();
	}
}

void test_sched_endless_threads_yield(void) {
	printf("Lauching 5 test threads\n");
	for(int i=1; i<=5; i++) {
		sched_new_thread(&thread, (void*)i, PRIORITY_NORMAL);
	}
}