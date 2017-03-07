#include <kernel/sched.h>
#include <test.h>
#include <stdio.h>
#include <stdlib.h>

static void thread(void* ch) {
	char* c = (char*)ch;
	int dummy = 0;
	for(int i=0; i<200; i++) {
		for(int j= 0; j<500000; j++) {
			dummy ++;
		}
		printf("%c", *c, dummy);
	}
}

void test_sched_threads_basic(void) {
	printf("Lauching 3 test threads\n");
	char* lnh = "LNH";
	sched_new_thread(&thread, lnh, PRIORITY_LOW);
	sched_new_thread(&thread, lnh+1, PRIORITY_NORMAL);
	sched_new_thread(&thread, lnh+2, PRIORITY_HIGH);
}