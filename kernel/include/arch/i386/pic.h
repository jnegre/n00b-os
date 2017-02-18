#ifndef _ARCH_I386_PIC_H
#define _ARCH_I386_PIC_H
#include <stdint.h>

/*
 * Enables an IRQ. Does not cli/sti, beware!
 */
void pic_enable(uint8_t irq);

void pic_disable_all(void);
void pic_send_eoi(uint8_t irq);

#endif
