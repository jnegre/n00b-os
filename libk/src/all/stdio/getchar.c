#include <stdio.h>
#include <kernel/input/keyboard.h>

int getchar() {
	return kbd_getchar();
}