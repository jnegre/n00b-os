#include <test.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static noreturn int thread(void* integer) {
	uint32_t i = (uint32_t)integer;
	while(true) {
		printf("[%u]", i);
		thrd_yield();
	}
}

void test_sched_endless_threads_yield(void) {
	printf("Launching 5 test threads\n");
	for(int i=1; i<=5; i++) {
		thrd_create(NULL, &thread, (void*)i);
	}
}