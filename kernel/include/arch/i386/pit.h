#ifndef _ARCH_I386_PIT_H
#define _ARCH_I386_PIT_H
#include <stdint.h>

#define PIT_FREQUENCY_HZ 1193182

enum pit_channel {
	CHANNEL_0=0,
	CHANNEL_2=2
};

enum pit_mode {
	INTERRUPT_ON_TERMINAL_COUNT = 0,
	HARDWARE_RETRIGGERABLE_ONE_SHOT = 1,
	RATE_GENERATOR = 2,
	SQUARE_WAVE_GENERATOR = 3,
	SOFTWARE_TRIGGERED_STROBE = 4,
	HARDWARE_TRIGGERED_STROBE = 5
};

void pit_configure(enum pit_channel channel, enum pit_mode mode, uint16_t reload);

#endif
