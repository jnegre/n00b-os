#include <kernel/ringbuffer.h>
#include <string.h>
#include <stdio.h>

//TODO optimize
#define OFFSET(rb, position) ( (rb->position % rb->nb_elems) * rb-> size_elem )

bool ringbuffer_empty(struct ringbuffer *rb) {
	return rb->head == rb->tail;
}

bool ringbuffer_full(struct ringbuffer *rb) {
	return rb->head - rb->tail == rb->nb_elems;
}

int ringbuffer_put(struct ringbuffer *rb, const void* data) {
	if(!ringbuffer_full(rb)) {
		memcpy(&(rb->buffer[OFFSET(rb, head)]), data, rb->size_elem);
		rb->head++;
		return 0;
	} else {
		return -1;
	}
}

int ringbuffer_get(struct ringbuffer *rb, void* data) {
	if(!ringbuffer_empty(rb)) {
		memcpy(data, &(rb->buffer[OFFSET(rb, tail)]), rb->size_elem);
		rb->tail++;
		return 0;
	} else {
		return -1;
	}
}
