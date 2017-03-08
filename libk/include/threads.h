#ifndef _THREADS_H
#define _THREADS_H

#include <stdnoreturn.h>
#include <stdint.h>

typedef uint32_t thrd_t;
typedef int(*thrd_start_t)(void*);

enum {
	thrd_success,
	thrd_nomem,
	thrd_timedout,
	thrd_busy,
	thrd_error
};

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
int thrd_equal(thrd_t lhs, thrd_t rhs);
thrd_t thrd_current(void);
void thrd_yield();
noreturn void thrd_exit(int res);

#endif 