#include <kernel/sched.h>
#include <test.h>
#include <stdio.h>
#include <stdlib.h>

// thread 1
static void thread1(void) {
	while(true) {
		//printf("1");
		int *p = malloc(sizeof(int));
		assert_not_null("malloc returned non NULL", (uintptr_t)p);
		*p = 1;
		assert_uint("Thread malloc", 1, p[0]);
		free(p);
	}
}
// thread 2
static void thread2(void) {
	while(true) {
		//printf("2");
		int *p = malloc(sizeof(int));
		assert_not_null("malloc returned non NULL", (uintptr_t)p);
		*p = 2;
		assert_uint("Thread malloc", 2, p[0]);
		free(p);
	}
}

// FIXME causes Page Faults
void test_sched_endless_threads_malloc(void) {
	printf("Lauching 2 test threads\n");
	sched_new_thread(&thread1);
	sched_new_thread(&thread2);
}