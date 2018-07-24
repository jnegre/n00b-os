#include <kernel/input/keyboard.h>
#include <kernel/ringbuffer.h>
#include <kernel/sync.h>
#include <kernel/sched.h>
#include <stdio.h>
#include <threads.h>

#define KEY_UP(scan) (scan>>7)
#define KEY_CODE(scan) (scan&0x7F)

// for scan code set 1, minimalist set for french keyboard
static const unsigned char kbdfr[128] = {
	/* 00 */ 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0, 0, '\b', '\t',
	/* 10 */ 'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0, 0, '\n', 0, 'Q', 'S',
	/* 20 */ 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 0, 0, 0, 0, 'W', 'X', 'C', 'V',
	/* 30 */ 'B', 'N', 0, 0, 0, 0, 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0,
	/* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*static*/ unsigned char buffer_data[8];

ringbuffer_t ringbuffer = {
	.head = 0,
	.tail = 0,
	.nb_elems = 8,
	.size_elem = sizeof(unsigned char),
	.buffer = buffer_data
};

void kbd_handle_key(uint8_t scan) {
	
	if(!KEY_UP(scan)) {
		unsigned char ascii = kbdfr[KEY_CODE(scan)];
		if(ascii != 0) {
			int res = ringbuffer_put(&ringbuffer, &ascii);
		}
	}
}

int kbd_getchar() {
	static semaphore_t semaphore = SEMAPHORE_INIT;
	unsigned char thechar;
	int error = 0;

	semaphore_acquire(&semaphore);
	while(ringbuffer_empty(&ringbuffer)) {
		sched_sleep(1); //FIXME does not work with thrd_yield() = sched_sleep(0)
	}
	error = ringbuffer_get(&ringbuffer, &thechar);
	semaphore_release(&semaphore);
	return (error!=0) ? EOF : thechar;
}