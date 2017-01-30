/*
 * All memory allocation functions are here
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdalign.h>

#include <kernel/mm.h>
#include <kernel/sched.h>
#include <kernel/panic.h>

#define CANARY 0xCAFEBABE

/* Chunck anatomy:
 * 
 * chunck_marker_start_t (free? + usable size)
 * padding? for alignment
 * USEABLE MEMORY
 * chunck_marker_end_t (usable size)
 * padding? for alignment
 */


// at the begining of each chunck
typedef struct chunck_marker_start {
	bool free;
	size_t size; // of available memory, don't forget to add sizeof chunck_marker_start_t + sizeof chunck_marker_end at the end
} chunck_marker_start_t;

typedef struct chunck_marker_end {
	size_t size; // of available memory, see above
	uint32_t canary;
} chunck_marker_end_t;

static const size_t max_align = alignof( max_align_t);
static size_t aligned_size(const size_t size) {
	size_t modulo = size%max_align;
	return size + (modulo!=0 ? max_align-modulo : 0);
}

/* Puts chunck_marker_start_t and chunck_marker_end_t.
 * insertion_point and useable_size must already be aligned.
 */
static void mark_chunck(const uintptr_t insertion_point, const size_t useable_size, const bool isFree) {
	chunck_marker_start_t* start = (chunck_marker_start_t*)insertion_point;
	start->free = isFree;
	start->size = useable_size;
	chunck_marker_end_t* end = (chunck_marker_end_t*)(insertion_point + useable_size + aligned_size(sizeof(chunck_marker_start_t)));
	end->size = useable_size;
	end->canary = CANARY;
}

//FIXME must be made thread-safe
void* malloc( size_t size ) {
	if(size == 0) {
		return NULL;
	}
	//FIXME const for useable_size + start_size + end_size?
	size_t useable_size = aligned_size(size);
	const size_t start_size = aligned_size(sizeof(chunck_marker_start_t));
	const size_t end_size = aligned_size(sizeof(chunck_marker_end_t));

	mm_info_t* mm_info = current_process_control_block()->mm_info;
	uintptr_t best_ptr = 0;
	chunck_marker_start_t* last_marker = NULL;
	size_t best_size;
	uintptr_t ptr = mm_info->heap_start;
	while(ptr < mm_info->heap_end) {
		last_marker = (chunck_marker_start_t*)ptr;
		if(last_marker->free && last_marker->size >= useable_size && (best_ptr == 0 || last_marker->size < best_size)) {
			// found a better match
			best_ptr = ptr;
			best_size = last_marker->size;
			if(last_marker->size == useable_size) {
				// and we can't find a better one
				break;
			}
		}
		ptr += last_marker->size + start_size + end_size;
	}

	if(best_ptr != 0) {
		if(best_size > useable_size + start_size + end_size) {
			// split it
			uintptr_t free_chunck_insertion_point = best_ptr + useable_size + start_size + end_size;
			size_t free_chunck_size = best_size - useable_size - start_size - end_size;
			mark_chunck(free_chunck_insertion_point, free_chunck_size, true);
		} else {
			// use the sligthly bigger size
			useable_size = best_size;
		}
		// use it
		mark_chunck(best_ptr, useable_size, false);
		return (void*)(best_ptr + start_size);
	} else {
		// need to allocate some more memory
		uintptr_t insertion_point;
		size_t free_size;
		if(last_marker == NULL || !last_marker->free) {
			// we must start at the begining of a new page
			insertion_point = mm_info->heap_end;
			free_size = 0;
		} else {
			// let's merge with the last free chunck
			insertion_point = (uintptr_t)last_marker;
			free_size = last_marker->size + start_size + end_size;
		}
		while(free_size < useable_size + start_size + end_size) {
			// increase the heap
			// TODO check we don't overwrite the stack
			uintptr_t new_page = mm_alloc_physical_page(true);
			if(new_page == 0) {
				panic("failed to increase heap size (alloc)");
			}
			if(mm_map_page(new_page, mm_info->heap_end)) {
				panic("failed to increase heap size (map)");
			}
			mm_info->heap_end += MM_PAGE_SIZE;
			free_size += MM_PAGE_SIZE;
		}
		if(free_size > useable_size + 2*start_size + 2*end_size) {
			// split it
			uintptr_t free_chunck_insertion_point = insertion_point + useable_size + start_size + end_size;
			size_t free_chunck_size = free_size - useable_size - 2*start_size - 2*end_size;
			mark_chunck(free_chunck_insertion_point, free_chunck_size, true);
		} else {
			// use the sligthly bigger size
			useable_size = free_size - start_size - end_size;
		}
		mark_chunck(insertion_point, useable_size, false);
		return (void*)(insertion_point + start_size);
	}
}

void free( void* ptr ) {
	if(ptr == NULL) {
		return;
	}
	const size_t start_size = aligned_size(sizeof(chunck_marker_start_t));
	const size_t end_size = aligned_size(sizeof(chunck_marker_end_t));

	chunck_marker_start_t* start = (chunck_marker_start_t*)(ptr - start_size);
	chunck_marker_end_t* end = (chunck_marker_end_t*)(ptr + start->size);
	if(end->canary != CANARY) {
		panic("Dead heap canary");
	}
	start->free = true;
	
	// merge with chuncks before/after
	mm_info_t* mm_info = current_process_control_block()->mm_info;
	
	chunck_marker_start_t* start_after = (chunck_marker_start_t*)(ptr + start->size + end_size);
	if((uintptr_t)start_after < mm_info->heap_end && start_after->free) {
		mark_chunck((uintptr_t)start, start->size + start_after->size + start_size + end_size, true);
	}

	chunck_marker_end_t* end_before = (chunck_marker_end_t*)(ptr - start_size - end_size);
	if((uintptr_t)end_before > mm_info->heap_start) {
		if(end_before->canary != CANARY) {
			panic("Dead heap canary");
		}
chunck_marker_start_t* start_before = (chunck_marker_start_t*)(ptr - start_size - end_size - end_before->size - start_size);
		if(start_before->free) {
			mark_chunck((uintptr_t)start_before, start->size + start_before->size + start_size + end_size, true);
		}
	}

}