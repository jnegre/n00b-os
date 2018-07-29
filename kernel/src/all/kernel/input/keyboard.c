#include <kernel/input/keyboard.h>
#include <kernel/ringbuffer.h>
#include <kernel/sync.h>
#include <kernel/sched.h>
#include <stdio.h>
#include <threads.h>

#define KEY_UP(scan) (scan>>7)
#define KEY_CODE(scan) (scan&0x7F)

// for scan code set 1, minimalist set for french keyboard
static const kbd_keyboard_definition_t default_kbd = {
	.specs = {
		/* 00 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 04 */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 08 */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 0C */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 10 */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 14 */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 18 */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 1C */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 20 */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 24 */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 28 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_MOD, KBD_KEY_SPEC_NONE,
		/* 2C */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 30 */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG,
		/* 34 */ KBD_KEY_SPEC_REG, KBD_KEY_SPEC_REG, KBD_KEY_SPEC_MOD, KBD_KEY_SPEC_NONE,
		/* 38 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_REG,  KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 3C */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 40 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 44 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 48 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 4C */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 50 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 54 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 58 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 5C */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 60 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 64 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 68 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 6C */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 70 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 74 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 78 */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE,
		/* 7C */ KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE, KBD_KEY_SPEC_NONE
	},
	.scans = {
		/* 00 */ {KBD_SCAN_NONE}, {KBD_SCAN_REG(27), KBD_SCAN_REG(27)}, {KBD_SCAN_REG('&'), KBD_SCAN_REG('1')}, {KBD_SCAN_NONE, KBD_SCAN_REG('2')},
		/* 04 */ {KBD_SCAN_REG('"'), KBD_SCAN_REG('3')}, {KBD_SCAN_REG('\''), KBD_SCAN_REG('4')}, {KBD_SCAN_REG('('), KBD_SCAN_REG('5')}, {KBD_SCAN_REG('-'), KBD_SCAN_REG('6')},
		/* 08 */ {KBD_SCAN_NONE, KBD_SCAN_REG('7')}, {KBD_SCAN_REG('_'), KBD_SCAN_REG('8')}, {KBD_SCAN_NONE, KBD_SCAN_REG('9')}, {KBD_SCAN_NONE, KBD_SCAN_REG('0')},
		/* 0C */ {KBD_SCAN_REG(')')}, {KBD_SCAN_REG('='), KBD_SCAN_REG('+')}, {KBD_SCAN_REG('\b'), KBD_SCAN_REG('\b')}, {KBD_SCAN_REG('\t'), KBD_SCAN_REG('\t')},
		/* 10 */ {KBD_SCAN_REG('a'), KBD_SCAN_REG('A')}, {KBD_SCAN_REG('z'), KBD_SCAN_REG('Z')}, {KBD_SCAN_REG('e'), KBD_SCAN_REG('E')}, {KBD_SCAN_REG('r'), KBD_SCAN_REG('R')},
		/* 14 */ {KBD_SCAN_REG('t'), KBD_SCAN_REG('T')}, {KBD_SCAN_REG('y'), KBD_SCAN_REG('Y')}, {KBD_SCAN_REG('u'), KBD_SCAN_REG('U')}, {KBD_SCAN_REG('i'), KBD_SCAN_REG('I')},
		/* 18 */ {KBD_SCAN_REG('o'), KBD_SCAN_REG('O')}, {KBD_SCAN_REG('p'), KBD_SCAN_REG('P')}, {KBD_SCAN_NONE}, {KBD_SCAN_NONE},
		/* 1C */ {KBD_SCAN_REG('\n'), KBD_SCAN_REG('\n')}, {KBD_SCAN_NONE}, {KBD_SCAN_REG('q'), KBD_SCAN_REG('Q')}, {KBD_SCAN_REG('s'), KBD_SCAN_REG('S')},
		/* 20 */ {KBD_SCAN_REG('d'), KBD_SCAN_REG('D')}, {KBD_SCAN_REG('f'), KBD_SCAN_REG('F')}, {KBD_SCAN_REG('g'), KBD_SCAN_REG('G')}, {KBD_SCAN_REG('h'), KBD_SCAN_REG('H')},
		/* 24 */ {KBD_SCAN_REG('j'), KBD_SCAN_REG('J')}, {KBD_SCAN_REG('k'), KBD_SCAN_REG('K')}, {KBD_SCAN_REG('l'), KBD_SCAN_REG('L')}, {KBD_SCAN_REG('m'), KBD_SCAN_REG('M')},
		/* 28 */ {KBD_SCAN_NONE}, {KBD_SCAN_NONE}, {KBD_SCAN_MOD(KBD_MOD_SHIFT)}, {KBD_SCAN_NONE},
		/* 2C */ {KBD_SCAN_REG('w'), KBD_SCAN_REG('W')}, {KBD_SCAN_REG('x'), KBD_SCAN_REG('X')}, {KBD_SCAN_REG('c'), KBD_SCAN_REG('C')}, {KBD_SCAN_REG('v'), KBD_SCAN_REG('V')},
		/* 30 */ {KBD_SCAN_REG('b'), KBD_SCAN_REG('B')}, {KBD_SCAN_REG('n'), KBD_SCAN_REG('N')}, {KBD_SCAN_REG(','), KBD_SCAN_REG('?')}, {KBD_SCAN_REG(';'), KBD_SCAN_REG('.')},
		/* 34 */ {KBD_SCAN_REG(':'), KBD_SCAN_REG('/')}, {KBD_SCAN_REG('!')}, {KBD_SCAN_MOD(KBD_MOD_SHIFT)}, {KBD_SCAN_NONE},
		/* 38 */ {KBD_SCAN_NONE}, {KBD_SCAN_REG(' '), KBD_SCAN_REG(' ')}
	}
};

static unsigned char buffer_data[8];

static ringbuffer_t ringbuffer = {
	.head = 0,
	.tail = 0,
	.nb_elems = 8,
	.size_elem = sizeof(unsigned char),
	.buffer = buffer_data
};

static uint8_t state_shift = 0;

/* Priority : shift(1) none(0) */
static uint8_t get_active_mod() {
	if(state_shift!=0) return 1;
	return 0;
}

static void add_mods(kbd_scan_mod_t mods) {
	state_shift += mods.shift;
}

static void remove_mods(kbd_scan_mod_t mods) {
	if(state_shift!=0) state_shift -= mods.shift;
}

void kbd_handle_key(uint8_t scan) {
	//TODO check for E0/E1
	uint8_t scan_code = KEY_CODE(scan);
	bool key_up = KEY_UP(scan);
	printf("<%X/%X/%X>", scan, scan_code, key_up);
	kbd_key_spec_t spec = default_kbd.specs[scan_code];
	kbd_scan_reg_t scan_reg;
	kbd_scan_mod_t scan_mod;
	switch(spec.type) {
		case REGULAR:
			if(!key_up) {
				scan_reg = default_kbd.scans[scan_code][get_active_mod()].reg;
				if(scan_reg.ascii != 0) {
					/*int res =*/ ringbuffer_put(&ringbuffer, &scan_reg.ascii);
				}
			}
			break;
		case MOD:
			scan_mod = default_kbd.scans[scan_code][0].mod;
			if(key_up) {
				remove_mods(scan_mod);
			} else {
				add_mods(scan_mod);
			}
			break;
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