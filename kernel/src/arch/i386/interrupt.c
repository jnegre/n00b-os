#include <arch/i386/interrupt.h>
#include <kernel/panic.h>

/*
 * Initial handlers
 */

static void de_handler() {
	panic("(0) #DE Divide Error");
}
static void nmi_handler() {
	panic("(2) NMI Interrupt");
}
static void of_handler() {
	panic("(4) #OF Overflow");
}
static void df_handler() {
	panic("(8) #DF Double Fault");
}
static void ts_handler() {
	panic("(10) #TS Invalid TSS");
}
static void np_handler() {
	panic("(11) #NP Segment Not Present");
}
static void ss_handler() {
	panic("(12) #SS Stack-Segment Fault");
}
static void gp_handler() {
	panic("(13) #GP General Protection");
}
static void pf_handler() {
	panic("(14) #PF Page Fault");
}
static void mf_handler() {
	panic("(16) #MF x87 FPU Floating-Point Error");
}
static void ac_handler() {
	panic("(17) #AC Alignment Check");
}
static void mc_handler() {
	panic("(18) #MC Machine Check");
}
static void xm_handler() {
	panic("(19) #XM SIMD Floating-Point Exception");
}

/*
 * Sets up a minimal set of interrupt handlers.
 */
void itr_set_initial_handlers(void) {
	itr_set_handler(0, 0x8E, 8, &de_handler);
	itr_set_handler(2, 0x8E, 8, &nmi_handler);
	itr_set_handler(4, 0x8E, 8, &of_handler);
	itr_set_handler(8, 0x8E, 8, &df_handler);
	itr_set_handler(10, 0x8E, 8, &ts_handler);
	itr_set_handler(11, 0x8E, 8, &np_handler);
	itr_set_handler(12, 0x8E, 8, &ss_handler);
	itr_set_handler(13, 0x8E, 8, &gp_handler);
	itr_set_handler(14, 0x8E, 8, &pf_handler);
	itr_set_handler(16, 0x8E, 8, &mf_handler);
	itr_set_handler(17, 0x8E, 8, &ac_handler);
	itr_set_handler(18, 0x8E, 8, &mc_handler);
	itr_set_handler(19, 0x8E, 8, &xm_handler);
}