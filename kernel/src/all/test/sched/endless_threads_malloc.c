#include <test.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static noreturn int thread(void* integer) {
	uint32_t i = (uint32_t)integer;
	while(true) {
		//printf("%u", i);
		int *p = malloc(sizeof(int));
		assert_not_null("malloc returned non NULL", (uintptr_t)p);
		*p = i;
		assert_uint("Thread malloc", i, p[0]);
		free(p);
	}
}

void test_sched_endless_threads_malloc(void) {
	thrd_t one, two, three, four;
	thrd_create(&one, &thread, (void*)1);
	thrd_create(&two, &thread, (void*)2);
	thrd_create(&three, &thread, (void*)3);
	thrd_create(&four, &thread, (void*)4);
	printf("Launched 4 test threads with ids %u, %u, %u and %u.\n", one, two, three, four);
}