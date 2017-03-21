#include <kernel/sched.h>
#include <kernel/sync.h>
#include <kernel/vfs.h>
#include <kernel/mm.h>
#include <stddef.h>

void sched_init_process_control_block(void) {
	extern int kernel_end;
	extern int gdt_info;
	static mm_info_t init_mm_info = {
		.gdt_info = (uintptr_t)&gdt_info,
		.heap_semaphore = SEMAPHORE_INIT,
		.heap_start = (uintptr_t)&kernel_end,
		.heap_end = (uintptr_t)&kernel_end
	};
	static vfs_open_files_t init_vfs_open_files = {
		.handles = {},//file_handle_t* [VFS_OPEN_FILES_LENGTH];
		.next = NULL
	};
	for(int i=0; i<VFS_OPEN_FILES_LENGTH; i++) {
		init_vfs_open_files.handles[i] = NULL;
	}
	static vfs_info_t init_vfs_info = {
		.root = NULL,
		.cwd = NULL,
		.files = &init_vfs_open_files
	};
	static process_control_block_t init_pcb = {
		.tgid = 1,
		.tid = 1,
		.priority = PRIORITY_NORMAL,
		.mm_info = &init_mm_info,
		.vfs_info = &init_vfs_info
	};
	*current_process_control_block_ptr() = &init_pcb;
}