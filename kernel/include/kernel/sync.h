#ifndef _KERNEL_SYNC_H
#define _KERNEL_SYNC_H

#include <stdint.h>

/*
 * Spin lock
 */

typedef struct spinlock {
	volatile uint32_t lock;
} spinlock_t;

#define SPIN_LOCK_INIT \
	{ \
		.lock = 0 \
	}

void spin_lock(spinlock_t* lock);
void spin_unlock(spinlock_t* lock);

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
