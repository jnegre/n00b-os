#ifndef _KERNEL_SYNC_H
#define _KERNEL_SYNC_H

#include <stdint.h>

/*
 * Semaphore
 */
typedef struct semaphore {
	volatile uint32_t lock;
} semaphore_t;

#define SEMAPHORE_INIT \
	{ \
		.lock = 0, \
	}

void semaphore_acquire(semaphore_t* semaphore);
void semaphore_release(semaphore_t* semaphore);

/*
 * Reentrant mutex
 */
typedef struct mutex {
	volatile uint32_t lock;
	uint16_t level;
} mutex_t;

#define MUTEX_INIT \
	{ \
		.lock = 0, \
		.level = 0 \
	}

void mutex_acquire(mutex_t* mutex);
void mutex_release(mutex_t* mutex);

#endif
