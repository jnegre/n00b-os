#include <kernel/sched.h>
#include <test.h>
#include <stdio.h>
#include <stdlib.h>

static noreturn int thread(void* ch) {
	char* c = (char*)ch;
	int i = 0;
	while(true) {
		if(++i == 100000) {
			i = 0;
			printf("%c", *c);
		}
	}
}

void test_sched_endless_threads_basic(void) {
	printf("Lauching 3 test threads\n");
	char* lnh = "LNH";
	sched_new_thread(NULL, &thread, lnh, PRIORITY_LOW);
	sched_new_thread(NULL, &thread, lnh+1, PRIORITY_NORMAL);
	sched_new_thread(NULL, &thread, lnh+2, PRIORITY_HIGH);
}