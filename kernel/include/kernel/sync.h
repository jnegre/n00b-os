#ifndef _KERNEL_SYNC_H
#define _KERNEL_SYNC_H

#include <stdint.h>

typedef struct spinlock {
	volatile uint32_t lock;
} spinlock_t;

#define SPIN_LOCK_INIT \
	{ \
		.lock = 0 \
	}

void spin_lock(spinlock_t* lock);

void spin_unlock(spinlock_t* lock);

#endif
