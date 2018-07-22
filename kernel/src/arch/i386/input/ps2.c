#include <arch/i386/interrupt.h>
#include <arch/i386/pic.h>
#include <arch/i386/input/ps2.h>
#include <stdio.h>

void irq1_handler(void); // defined in interrupts.S

void ps2_init() {
	//TODO do not rely on someone else configuring the keyboard for us

	// Setting IRQ1 handler
	itr_set_handler(33, 0x8E, 8, &irq1_handler); //TODO check it's the correct type
	// Enabling IRQ 1
	pic_enable(1);

}


