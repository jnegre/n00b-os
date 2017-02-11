#ifndef _ARCH_I386_INTERRUPT_H
#define _ARCH_I386_INTERRUPT_H
#include <stdint.h>

/*
 * Sets an interrupt handler.
 *
 * itr: interrupt number
 * type: [P DPL S GT]
 *  - P: Present, set to 0 for unused interrupts. (1 bit)
 *  - DPL: Descriptor Privilege Level, specifies which privilege Level the calling Descriptor minimum should have. (2 bits)
 *  - S: Storage Segment, set to 0 for interrupt gates. (1 bit)
 *  - GT: Gate Type, 0x5=32-bit task gate, 0xE=32-bit interrupt gate, 0xF=32-bit trap gate. (4 bits)
 * cs: code segment (8 = kernel)
 * handler: pointer to interrupt handler
 */
void itr_set_handler(uint8_t itr, uint8_t type, uint16_t cs, void* handler);

#endif
