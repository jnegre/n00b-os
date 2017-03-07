#ifndef _THREADS_H
#define _THREADS_H

#include <stdnoreturn.h>
#include <stdint.h>

typedef uint32_t thrd_t;

int thrd_equal(thrd_t lhs, thrd_t rhs);
thrd_t thrd_current(void);
void thrd_yield();
noreturn void thrd_exit(int res);

#endif 