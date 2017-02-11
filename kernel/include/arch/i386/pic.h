#ifndef _ARCH_I386_PIC_H
#define _ARCH_I386_PIC_H
#include <stdint.h>

void pic_disable(void);
void pic_send_eoi(uint8_t irq);

#endif
