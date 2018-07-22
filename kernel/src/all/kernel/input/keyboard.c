#include <kernel/input/keyboard.h>
#include <stdio.h>

#define KEY_UP(scan) (scan>>7)
#define KEY_CODE(scan) (scan&0x7F)

// for scan code set 1, minimalist set for french keyboard
unsigned char kbdfr[128] = {
	/* 00 */ 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0, 0, '\b', '\t',
	/* 10 */ 'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0, 0, '\n', 0, 'Q', 'S',
	/* 20 */ 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 0, 0, 0, 0, 'W', 'X', 'C', 'V',
	/* 30 */ 'B', 'N', 0, 0, 0, 0, 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0,
	/* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void kbd_handle_key(uint8_t scan) {
	
	if(!KEY_UP(scan)) {
		unsigned char ascii = kbdfr[KEY_CODE(scan)];
		if(ascii != 0) {
			printf("%c", kbdfr[KEY_CODE(scan)]);
		}
	}
}