#include <kernel/sched.h>
#include <test.h>
#include <stdio.h>
#include <stdlib.h>

static void thread(void* integer) {
	uint32_t i = (uint32_t)integer;
	while(true) {
		printf("[%u]", i);
		//asm("pause");
	}
}

void test_sched_endless_threads_basic(void) {
	printf("Lauching 2 test threads\n");
	sched_new_thread(&thread, (void*)1);
	sched_new_thread(&thread, (void*)2);
}