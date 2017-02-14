#include <kernel/sched.h>
#include <kernel/mm.h>

void sched_init_process_control_block(void) {
	extern int kernel_end;
	static mm_info_t init_mm_info = {
		.heap_start = (uintptr_t)&kernel_end,
		.heap_end = (uintptr_t)&kernel_end
	};
	static process_control_block_t init_pcb = {
		.tgid = 1,
		.tid = 1,
		.mm_info = &init_mm_info
	};
	*current_process_control_block() = init_pcb;
}