#include <kernel/sched.h>

process_control_block_t** current_process_control_block_ptr(void) {
	process_control_block_t** result;
	asm("movl %%esp, %0;"
		"andl $0xFFFFD000, %0;"
		"orl $0x3FF8, %0;"
		:"=g" (result)
		);
	return result;
}

process_control_block_t* current_process_control_block(void) {
	return *current_process_control_block_ptr();
}