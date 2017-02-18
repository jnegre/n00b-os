#include <test.h>
#include <kernel/mm.h>
#include <kernel/tty.h>

#define NUMBER_OF_LOOPS 10000
#define NUMBER_OF_PAGES 10

void test_mm_page_allocator(void) {
    terminal_writestring("\ntest_mm_page_allocator()");
	uintptr_t pages[NUMBER_OF_PAGES];

	for(int loop=0; loop<NUMBER_OF_LOOPS; loop++) {
		//terminal_writestring("\nLoop ");
		//terminal_writeu32(loop);
		// allocate & map NUMBER_OF_PAGES pages
		for(int i = 0; i<NUMBER_OF_PAGES; i++) {
			pages[i] = mm_alloc_physical_page(true);
			assert_not_null("Page allocation failed", pages[i]);
			uintptr_t virt_addr = 0x200000 + i*0x100000;
			int error = mm_map_page(pages[i], virt_addr);
			assert_null("mm_map_page error", error);
			if(!error) {
				*((uint32_t*)virt_addr) = i;
			}
		}
		//terminal_writestring("\nMapped pages");

		// unmap & free even pages
		for(int i = 0; i<NUMBER_OF_PAGES; i+=2) {
			uintptr_t virt_addr = 0x200000 + i*0x100000;
			assert_uint("Value from mapped page", i, *((uint32_t*)virt_addr));
			mm_unmap_page(virt_addr);
			mm_free_physical_page(pages[i]);
		}
		// unmap & free odd pages in reverse
		for(int i = NUMBER_OF_PAGES-1; i>0; i-=1) {
			if(i%2 != 1) {
				continue;
			}
			uintptr_t virt_addr = 0x200000 + i*0x100000;
			assert_uint("Value from mapped page", i, *((uint32_t*)virt_addr));
			mm_unmap_page(virt_addr);
			mm_free_physical_page(pages[i]);
		}
		//terminal_writestring("\nUnmapped pages: ");
	}
}