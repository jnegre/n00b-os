#include <test.h>
#include <stdlib.h>

void test_malloc(void) {

	// souldn't do anything
	free(NULL);

	int *p1 = malloc(4*sizeof(int));  // allocates enough for an array of 4 int

	assert_not_null("malloc returned non NULL", (uintptr_t)p1);
	for(int n=0; n<4; ++n) // populate p1
		p1[n] = n*n;
	for(int n=0; n<4; ++n) // check p1
		assert_uint("reading from p1", n*n, p1[n]);

	int *p2 = malloc(4*sizeof(int));
	assert_not_null("malloc returned non NULL", (uintptr_t)p2);
	for(int n=0; n<4; ++n) // populate p2
		p2[n] = n*n*n;
	for(int n=0; n<4; ++n) // check p2
		assert_uint("reading from p2", n*n*n, p2[n]);
	for(int n=0; n<4; ++n) // check p1
		assert_uint("reading from p1 again", n*n, p1[n]);

	int *p3 = malloc(5000*sizeof(int));
	assert_not_null("malloc returned non NULL", (uintptr_t)p3);
	for(int n=0; n<5000; ++n) // populate p3
		p3[n] = n*2;
	for(int n=0; n<5000; ++n) // check p3
		assert_uint("reading from p3", n*2, p3[n]);
	for(int n=0; n<4; ++n) // check p2
		assert_uint("reading from p2 again", n*n*n, p2[n]);
	for(int n=0; n<4; ++n) // check p1
		assert_uint("reading from p1 again", n*n, p1[n]);

	free(p1);
	free(p2);

	int *p4 = malloc(4*sizeof(int));
	assert_not_null("malloc returned non NULL", (uintptr_t)p4);
	for(int n=0; n<4; ++n) // populate p4
		p4[n] = n*n*n*n;
	for(int n=0; n<4; ++n) // check p4
		assert_uint("reading from p4", n*n*n*n, p4[n]);
	for(int n=0; n<5000; ++n) // check p3
		assert_uint("reading from p3", n*2, p3[n]);

	free(p4);
	free(p3);

	// stress test
	int *p5;
	int *p6;
	int *p7;
	for(int i=0; i<1000000; i++) {
		p5 = malloc(5001*sizeof(int));
		assert_not_null("malloc returned non NULL", (uintptr_t)p5);
		p5[0] = i;
		assert_uint("reading from p5", i, p5[0]);

		p6 = malloc(1*sizeof(int));
		assert_not_null("malloc returned non NULL", (uintptr_t)p6);
		p6[0] = 2*i;
		assert_uint("reading from p6", 2*i, p6[0]);

		free(p5);

		p7 = malloc(1*sizeof(int));
		assert_not_null("malloc returned non NULL", (uintptr_t)p7);
		p7[0] = 3*i;
		assert_uint("reading from p7", 3*i, p7[0]);

		free(p6);
		free(p7);
	}

	// more stress test
	int *p8;
	for(int i=1; i<1000000; i++) {
		p8 = malloc(i*sizeof(int));
		assert_not_null("malloc returned non NULL", (uintptr_t)p8);
		p8[0] = i;
		assert_uint("reading from p8", i, p8[0]);
		free(p8);
	}
}