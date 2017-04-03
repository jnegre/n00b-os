#include <stdbool.h>
#include <stdlib.h>

#include <kernel/mm.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/sync.h>

// TODO have no magic numbers

extern int stack_bottom; // TODO useless?


/*
 * Extends the bootstrap stack to its final size.
 * Add the missing top canary.
 */
void mm_init_stack(void) {
	// add enough pages for the bootstrap stack
	for(int i=0; i<3; i++) {
		uintptr_t page = mm_alloc_physical_page(true);
		if(page == 0) {
			panic("Failed to allocate a page to extend the stack");
		}
		if(mm_map_page(page, 0xFFBFE000 - 0x1000*i)) {
			panic("Can't map new page for the stack");
		}
	}
	// add the canary at the top
	*(uint32_t*)(0xFFBFC00C) = 0xDEADBEEF;
	// complete pcb
	mm_ring_info_t *kernel_info = current_process_control_block()->mm_info->kernel_info;
	uint32_t* bitmap = malloc(sizeof(uint32_t));
	if(!bitmap) {
		panic("Failed to init kernel mm_ring_info_t");
	}
	bitmap[0] = 1;
	kernel_info->stacks_bitmaps = bitmap;
	kernel_info->stacks_bitmaps_length = 1;
}

/* returns the 1st 0 bit, or -1 if none */
static int find_free_bit(uint32_t bitfield) {
	if(bitfield == UINT32_MAX) {
		return -1;
	} else {
		for(int i=0; i<32; i++) {
			uint32_t test = 1<<i;
			if((bitfield & test) != test) {
				return i;
			}
		}
		//we hould never, ever, get there
		panic("Logic error");
	}
}

/*
 * Allocates a new stack.
 * Sets up the canaries and pcb
 * Returns the stack pointer, or 0 if OOM
 */
uintptr_t mm_alloc_stack(mm_ring_info_t *info, process_control_block_t* new_pcb) {
	// allocate pages before semaphore_acquire
	uintptr_t pages[MM_STACK_PAGE_SIZE];
	for(int i=0; i<MM_STACK_PAGE_SIZE; i++) {
		pages[i] = mm_alloc_physical_page(true);
		if(pages[i] == 0) {
			for(int j=0; j<i; j++) {
				mm_free_physical_page(pages[j]);
			}
			return 0;
		}
	}
	// update pcb
	semaphore_acquire(&info->stacks_semaphore);
	// find somewhere to put our stack
	unsigned int bitmap_id;
	int bitmap_field = -1;
	for(bitmap_id = 0; bitmap_id<info->stacks_bitmaps_length; bitmap_id++) {
		bitmap_field = find_free_bit(info->stacks_bitmaps[bitmap_id]);
		if(bitmap_field != -1) {
			// got one!
			info->stacks_bitmaps[bitmap_id] |= 1<<bitmap_field;
			break;
		}
	}
	if(bitmap_field == -1) {
		// TODO implement
		panic("not implemented: increase kernel_info->stacks_bitmaps");
	}
	semaphore_release(&info->stacks_semaphore);
	// TODO check we don't collide with the heap
	// maps + puts canary and pcb
	uintptr_t stack_top = info->stacks_start - ((32*bitmap_id + bitmap_field)*MM_STACK_PAGE_SIZE*MM_PAGE_SIZE);
	for(int i=1; i<=MM_STACK_PAGE_SIZE; i++) {
		if(mm_map_page(pages[i-1], stack_top - 0x1000*i)) {
			for(int j=0; j<MM_STACK_PAGE_SIZE; j++) {
				mm_free_physical_page(pages[j]);
			}
			return 0;
		}
	}
	// add the canary at the top
	uintptr_t base = stack_top - 4*4096;
	*(uint32_t*)(base | 0xC) = 0xDEADBEEF;
	// and bottom
	*(uint32_t*)(base | 0x3FF0) = 0xDEADBEEF;
	// pcb
	*(process_control_block_t**)(base | 0x3FF8) = new_pcb;
	uintptr_t sp = base | 0x3FF0;

	return sp;
}

void mm_free_stack(mm_ring_info_t *info, uintptr_t sp) {
	unsigned int bitmap_field = (info->stacks_start - sp)/(MM_STACK_PAGE_SIZE*MM_PAGE_SIZE);
	unsigned int bitmap_id = bitmap_field/32;
	bitmap_field = bitmap_field%32;
	// unmap + free memory
	uintptr_t stack_top = info->stacks_start - ((32*bitmap_id + bitmap_field)*MM_STACK_PAGE_SIZE*MM_PAGE_SIZE);
	for(int i=1; i<=MM_STACK_PAGE_SIZE; i++) {
		mm_free_physical_page(mm_unmap_page(stack_top - 0x1000*i));
	}
	// update info
	semaphore_acquire(&info->stacks_semaphore);
	info->stacks_bitmaps[bitmap_id] &= ~(1<<bitmap_field);
	semaphore_release(&info->stacks_semaphore);
}