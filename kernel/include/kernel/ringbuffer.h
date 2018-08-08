#ifndef _KERNEL_RINGBUFFER_H
#define _KERNEL_RINGBUFFER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct ringbuffer {
	const size_t size_elem;
	const size_t nb_elems; // must be a power of two
	void *buffer;
	volatile size_t head;
	volatile size_t tail;
} ringbuffer_t;

bool ringbuffer_empty(struct ringbuffer *rb);
bool ringbuffer_full(struct ringbuffer *rb);

int ringbuffer_put(struct ringbuffer *rb, const void* data);
int ringbuffer_get(struct ringbuffer *rb, void* data);

#endif