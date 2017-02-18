/*
 * See http://wiki.osdev.org/PIT for all the nasty details.
 */
#include <stdint.h>
#include <arch/i386/pit.h>

#define DATA_PORT_0 0x40
#define DATA_PORT_2 0x42
#define COMMAND_PORT 0x43



void pit_configure(enum pit_channel channel, enum pit_mode mode, uint16_t reload) {
	uint8_t command = channel << 6 | mode << 1 | 3 << 4; // Access mode: lobyte/hibyte
	asm volatile (
		"outb %%al, %0;"
		"mov %%bl, %%al;"
		"outb %%al, %%dx;"
		"mov %%bh, %%al;"
		"outb %%al, %%dx;"
		:
		: [input] "N"(COMMAND_PORT), "a" (command), "b" (reload), "d" (DATA_PORT_0 + channel)
	);
}