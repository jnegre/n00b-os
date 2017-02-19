#include <kernel/sched.h>
#include <test.h>
#include <stdio.h>
#include <stdlib.h>

static void thread(void* integer) {
	uint32_t i = (uint32_t)integer;
	while(true) {
		printf("%u", i);
		int *p = malloc(sizeof(int));
		assert_not_null("malloc returned non NULL", (uintptr_t)p);
		*p = i;
		assert_uint("Thread malloc", i, p[0]);
		free(p);
	}
}

void test_sched_endless_threads_malloc(void) {
	printf("Lauching 4 test threads\n");
	sched_new_thread(&thread, (void*)1);
	sched_new_thread(&thread, (void*)2);
	sched_new_thread(&thread, (void*)3);
	sched_new_thread(&thread, (void*)4);
}