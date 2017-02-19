#include <kernel/sched.h>
#include <test.h>
#include <stdio.h>
#include <stdlib.h>

static void thread(void* ch) {
	char c = (char)ch;
	int i = 0;
	while(true) {
		if(++i == 100000) {
			i = 0;
			printf("%c", c);
		}
	}
}

void test_sched_endless_threads_basic(void) {
	printf("Lauching 3 test threads\n");
	sched_new_thread(&thread, (void*)'L', PRIORITY_LOW);
	sched_new_thread(&thread, (void*)'N', PRIORITY_NORMAL);
	sched_new_thread(&thread, (void*)'H', PRIORITY_HIGH);
}