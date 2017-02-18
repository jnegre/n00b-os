#include <kernel/sched.h>
#include <test.h>
#include <stdio.h>
#include <stdlib.h>

// thread 1
static void thread1(void) {
	while(true) {
		//printf("1");
		asm("pause");
	}
}
// thread 2
static void thread2(void) {
	while(true) {
		//printf("2");
		asm("pause");
	}
}

void test_sched_endless_threads_basic(void) {
	printf("Lauching 2 test threads\n");
	sched_new_thread(&thread1);
	sched_new_thread(&thread2);
}