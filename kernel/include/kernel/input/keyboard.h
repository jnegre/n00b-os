#ifndef _KERNEL_INPUT_KEYBOARD_H
#define _KERNEL_INPUT_KEYBOARD_H
#include <stdint.h>

void kbd_handle_key(uint8_t scan);
int kbd_getchar();

enum kbd_key_type {NONE, MOD, LOCK, REGULAR};

typedef struct kbd_key_spec {
	uint8_t type:2;
	//uint8_t caps:1; // 1 if affected by CAPS LOCK
	//uint8_t num:1; // 1 id affected by NUM LOCK
} kbd_key_spec_t;

typedef struct kbd_scan_mod {
	uint8_t shift:1;
	//uint8_t ctrl:1;
	//uint8_t alt:1;
	//uint8_t altgr:1;
} kbd_scan_mod_t;

typedef struct kbd_scan_reg {
	unsigned char ascii;
} kbd_scan_reg_t;

union kbd_scan {
	kbd_scan_mod_t mod;
	kbd_scan_reg_t reg;
};

typedef struct kbd_keyboard_definition {
	kbd_key_spec_t specs[128];
	union kbd_scan scans[128][2];
	//union kbd_scan e0_scans[128][2];
} kbd_keyboard_definition_t;

#define KBD_KEY_SPEC_NONE {.type=NONE}
#define KBD_KEY_SPEC_MOD {.type=MOD}
#define KBD_KEY_SPEC_REG {.type=REGULAR}

#define KBD_SCAN_NONE {{0}}
#define KBD_SCAN_REG(c) {.reg={.ascii=c}}
#define KBD_MOD_SHIFT .shift=1
#define KBD_SCAN_MOD(mods) {.mod={mods}}

#endif