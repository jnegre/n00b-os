#include <kernel/sched.h>
#include <test.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static noreturn int thread(void* integer) {
	uint32_t i = (uint32_t)integer;
	while(true) {
		printf("[%u]", i);
		sched_sleep(i * 1000);
	}
}

void test_sched_endless_threads_sleep(void) {
	printf("Launching 3 test threads\n");
	for(int i=1; i<=3; i++) {
		sched_new_thread(NULL, &thread, (void*)i, PRIORITY_NORMAL);
	}
}