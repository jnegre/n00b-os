#include <kernel/sched.h>
#include <kernel/sync.h>
#include <kernel/mm.h>

void sched_init_process_control_block(void) {
	extern int kernel_end;
	extern int gdt_info;
	static mm_info_t init_mm_info = {
		.gdt_info = (uintptr_t)&gdt_info,
		.heap_semaphore = SEMAPHORE_INIT,
		.heap_start = (uintptr_t)&kernel_end,
		.heap_end = (uintptr_t)&kernel_end
	};
	static process_control_block_t init_pcb = {
		.tgid = 1,
		.tid = 1,
		.priority = PRIORITY_NORMAL,
		.mm_info = &init_mm_info
	};
	*current_process_control_block_ptr() = &init_pcb;
}