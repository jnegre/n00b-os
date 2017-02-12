#include <test.h>
#include <stdint.h>
#include <stdio.h>

void assert_null(const char* msg, const uintmax_t actual) {
	if(actual != 0) {
		printf("\nASSERT %s\n", msg);
	}
}

void assert_not_null(const char* msg, const uintmax_t actual) {
	if(actual == 0) {
		printf("\nASSERT %s\n", msg);
	}
}

void assert_uint(const char* msg, const uintmax_t expected, const uintmax_t actual) {
	if(expected != actual) {
		printf("\nASSERT %s: expected %J, got %J\n", msg, expected, actual);
	}
}
