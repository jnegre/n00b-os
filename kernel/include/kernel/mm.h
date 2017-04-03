#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H
#include <stdint.h>
#include <stdbool.h>
#include <kernel/sync.h>
#include <kernel/sched.h>
#include <arch/x86/multiboot.h> //FIXME remove this dependency

#define MM_PAGE_SIZE 4096
#define MM_STACK_PAGE_SIZE 4

typedef struct mm_ring_info {
	semaphore_t heap_semaphore;
	uintptr_t heap_start;
	uintptr_t heap_end;
	semaphore_t stacks_semaphore;
	uintptr_t stacks_start; // the address where stacks start, going toward lower addresses. 0xFFC00000 in kernel land.
	unsigned int stacks_bitmaps_length; //the number of stacks_bitmaps
	uint32_t* stacks_bitmaps; // array of bitfields, indicates allocated stacks
} mm_ring_info_t;

typedef struct mm_info {
	uintptr_t page_dir; //cr3
	mm_ring_info_t* kernel_info;
} mm_info_t;

void mm_init_stack(void);
uintptr_t mm_alloc_stack(mm_ring_info_t *info, process_control_block_t* new_pcb);
void mm_free_stack(mm_ring_info_t *info, uintptr_t sp);

void mm_init_page_allocator(uint32_t mmap_length, multiboot_memory_map_t* mmap);

uintptr_t mm_alloc_physical_page(const bool zero);
void mm_free_physical_page(uintptr_t pa);

int mm_map_page(uintptr_t phys_address, uintptr_t virt_address);
uintptr_t mm_unmap_page(uintptr_t virt_address);

uintptr_t mm_new_page_directory(void);

#endif
