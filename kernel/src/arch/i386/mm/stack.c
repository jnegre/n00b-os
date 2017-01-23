#include <stdbool.h>

#include <kernel/mm.h>
#include <kernel/panic.h>
#include <kernel/tty.h> //FIXME remove

extern int stack_bottom;

/*
 * Extends the bootstrap stack to its final size.
 * Add the missing top canari.
 */
void mm_init_stack(void) {
	// add 3 more pages for the stack
	for(int i=0; i<3; i++) {
		uintptr_t page = mm_alloc_physical_page(true);
		if(page == 0) {
			panic("Failed to allocate a page to extend the stack");
		}
		if(mm_map_page(page, 0xFFBFE000 - 0x1000*i)) {
			panic("Can't map new page for the stack");
		}
	}
	//add the canari at the top
	*(uint32_t*)(0xFFBFC00C) = 0xDEADBEEF;
	terminal_writestring("\nStack is now 16k long");
}