#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <test/mm.h>
#include <test/sched.h>

#include <kernel/tty.h>
#include <kernel/panic.h>
#include <kernel/mm.h>
#include <kernel/sched.h>
#include <arch/x86/multiboot.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

// Defined by the linker script
// Use only their address, as their value is meaningless
extern int kernel_start;
extern int kernel_end;

void kernel_main(uint32_t mmap_length, multiboot_memory_map_t* mmap) {
	sched_init_process_control_block();
	/* Initialize terminal interface */
	terminal_initialize();

	process_control_block_t *pcb = current_process_control_block();
 
	printf("In %c kernel (tgid=%u tid=%u)\n", 'C', pcb->tgid, pcb->tid);
	printf("Kernel start: 0x%X end: 0x%X (%u kb)\n", &kernel_start, &kernel_end, (&kernel_end - &kernel_start)/1024);
	printf("Memory map length %u at addr 0x%X\n", mmap_length, mmap);

	for(multiboot_memory_map_t* mm = mmap;
			(uint32_t)mm < (uint32_t)(mmap)+mmap_length;
			mm = (multiboot_memory_map_t*)((uint32_t)mm+mm->size+sizeof(mm->size))) {

		printf(" - size:%u addr: 0x%J to: 0x%J type: %u\n", mm->size, mm->addr, mm->addr + mm->len - 1, mm->type);
	}
	mm_init_page_allocator(mmap_length, mmap);
	mm_init_stack();

	sched_setup_tick();

	printf("Heap from 0x%X to 0x%X (%u bytes)\n",
		pcb->mm_info->heap_start,
		pcb->mm_info->heap_end,
		(pcb->mm_info->heap_end - pcb->mm_info->heap_start)
		);

	//let's test the mm
	//test_mm_page_allocator();
	//and malloc
	//test_malloc();
	printf("Heap from 0x%X to 0x%X (%u bytes)\n",
		pcb->mm_info->heap_start,
		pcb->mm_info->heap_end,
		(pcb->mm_info->heap_end - pcb->mm_info->heap_start)
		);

	//test_sched_endless_threads_malloc();
	//test_sched_endless_threads_basic();
	//test_sched_endless_threads_yield();
	//test_sched_endless_threads_sleep();
	//test_sched_threads_exit();
	test_sched_threads_basic();

	printf("Main kernel thread will sleep from now on.\n");
	while(true) {
		sched_sleep(UINT32_MAX);
	}
}
