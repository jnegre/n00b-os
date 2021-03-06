#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <kernel/mm.h>
#include <kernel/panic.h>
#include <kernel/sync.h>
#include <arch/x86/multiboot.h>

extern int kernel_physical_end;
extern volatile uintptr_t mm_freepage; // virtual address to temporary map to
                                       // TODO take care of the physical memory at this addr at boot
static mutex_t mutex = MUTEX_INIT;
static uintptr_t *kernel_page_dir = (uintptr_t*) 0xFFFFF000; // last page in vmem is mapped to the page dir
static uintptr_t first_free_page = 0;

static uint32_t stat_mark = 0;

typedef struct __attribute__ ((packed)){
	int pagetable;
	int page;
}pageinfo;

static pageinfo mm_virtaddrtopageindex(uintptr_t addr){
	pageinfo pginf;

	// align address to 4k (highest 20-bits of address)
	addr &= ~0xFFF;
	pginf.pagetable = addr / 0x400000; // each page table covers 0x400000 bytes in memory
	pginf.page = (addr % 0x400000) / 0x1000; //0x1000 = page size
	return pginf;
}

static uintptr_t* mm_freepage_pte;

#define MAP_TO_FREEPAGE(page_table_entry) {*mm_freepage_pte = page_table_entry; invlpg((void*)&mm_freepage);}

//TODO move elsewhere
static inline uint64_t max(uint64_t a, uint64_t b) {
	return a<b ? b : a;
}
static inline uint64_t min(uint64_t a, uint64_t b) {
	return a<b ? a : b;
}

static void mark_page(uintptr_t physical_addr, uintptr_t next);

/*
 * Called during init of the kernel.
 * No need to be thread safe.
 * We still have the entire memory outside of the kernel that booted for us.
 */
void mm_init_page_allocator(uint32_t mmap_length, multiboot_memory_map_t* mmap) {
	pageinfo mm_freepage_info = mm_virtaddrtopageindex((uintptr_t)&mm_freepage);
	uintptr_t *page_table = (uintptr_t *) (0xFFC00000 + (mm_freepage_info.pagetable * 0x1000));
	mm_freepage_pte = &page_table[mm_freepage_info.page];
	uintptr_t page_after_kernel = (((uintptr_t) &kernel_physical_end >> 12) + 1)*4096;
	uintptr_t previous = 0;
	
	// go through all unused pages of physical memory and mark them with the addr of the next one
	// but do not go over 4G
	for(multiboot_memory_map_t* mm = mmap;
			(uint32_t)mm < (uint32_t)(mmap)+mmap_length;
			mm = (multiboot_memory_map_t*)((uint32_t)mm+mm->size+sizeof(mm->size))) {
		if(mm->type == 1) {
			for(uint64_t next = max(page_after_kernel, mm->addr); next < min(mm->addr + mm->len, 0xFFFFFFFF); next += 4096) {
				if(first_free_page == 0) {
					first_free_page = (uintptr_t)next;
				} else {
					mark_page(previous, (uintptr_t)next);
				}
				previous = (uintptr_t)next;
			}
		}
	}
	mark_page(previous, 0);// close the linked list
	// stats
	printf("Marked physical pages: %u\n", stat_mark);
}

/*
 * Invalidates the TLB (Translation Lookaside Buffer) for one specific virtual address.
 * The next memory reference for the page will be forced to re-read PDE and PTE from main memory.
 * Must be issued every time you update one of those tables.
 * The m pointer points to a logical address, not a physical or virtual one: an offset for your ds segment. 
 */
static inline void invlpg(void* m) {
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
    asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}

static void mark_page(uintptr_t physical_addr, uintptr_t next) {
	// map
	MAP_TO_FREEPAGE(physical_addr | 3);
	// write
	mm_freepage = next;
	// for stats
	stat_mark++;
}


uintptr_t mm_alloc_physical_page(const bool zero) {
	mutex_acquire(&mutex);
	uintptr_t page = first_free_page;
	if(page != 0) {
		// map
		MAP_TO_FREEPAGE(page | 3);
		// read next
		first_free_page = mm_freepage;
		// zero page if requested
		if(zero) {
			memset((void*)&mm_freepage, 0, 4096);
		}
		// unmap
		MAP_TO_FREEPAGE(0);
	}
	mutex_release(&mutex);
	return page;
}

void mm_free_physical_page(uintptr_t physical_addr){
	mutex_acquire(&mutex);
	// map
	MAP_TO_FREEPAGE(physical_addr | 3);
	// link to next entry
	mm_freepage = first_free_page;
	// unmap
	MAP_TO_FREEPAGE(0);
	// update
	first_free_page = physical_addr;
	mutex_release(&mutex);
}

/*
 * Does not allow to map 0 or to 0
 */
int mm_map_page(uintptr_t phys_address, uintptr_t virt_address){
	if(phys_address == 0 || virt_address == 0) {
		return -1;
	}
	pageinfo pginf = mm_virtaddrtopageindex(virt_address); // get the PDE and PTE indexes for the addr
	
	mutex_acquire(&mutex);
	if(kernel_page_dir[pginf.pagetable] & 1) {
		// page table exists.
		uintptr_t *page_table = (uintptr_t *) (0xFFC00000 + (pginf.pagetable * 0x1000)); // virt addr of page table
		if(!page_table[pginf.page] & 1) {
			// page isn't mapped
			page_table[pginf.page] = phys_address | 3;
		} else {
			// page is already mapped
			mutex_release(&mutex);
			return -1;
		}
	} else {
		// doesn't exist, so alloc a page and add into pdir
		uintptr_t new_page_table = mm_alloc_physical_page(true); //FIXME what if OOM?
		uintptr_t *page_table = (uintptr_t *) (0xFFC00000 + (pginf.pagetable * 0x1000)); // virt addr of page tbl

		kernel_page_dir[pginf.pagetable] = new_page_table | 3; // add the new page table into the pdir
		invlpg(page_table);
		page_table[pginf.page] = phys_address | 3; // map the page!
	}
	invlpg((void*)virt_address);
	mutex_release(&mutex);
	return 0;
}

/*
 * Unmap a page and returns its physical address
 */
uintptr_t mm_unmap_page(uintptr_t virt_address){
	uintptr_t phys_address = 0;
	pageinfo pginf = mm_virtaddrtopageindex(virt_address);
	
	mutex_acquire(&mutex);
	if(kernel_page_dir[pginf.pagetable] & 1) {
		int i;
		uintptr_t *page_table = (uintptr_t *) (0xFFC00000 + (pginf.pagetable * 0x1000));
		if(page_table[pginf.page] & 1) {
			// page is mapped, so unmap it
			phys_address = page_table[pginf.page] && ~0xFFF;
			page_table[pginf.page] = 2; // r/w, not present
			invlpg((void*)virt_address);
		}
		
		// check if there are any more present PTEs in this page table
		for(i = 0; i < 1024; i++) {
			if(page_table[i] & 1){
				break;
			}
		}
		
		// if there are none, then free the space allocated to the page table and delete mappings
		if(i == 1024) {
			mm_free_physical_page(kernel_page_dir[pginf.pagetable] & 0xFFFFF000);
			kernel_page_dir[pginf.pagetable] = 2;
			invlpg(page_table);
		}
	}
	mutex_release(&mutex);
	return phys_address;
}

/* Copies the current page directory in a new one, zeroing the user space portion */
//FIXME all further modification in the kernel space portion must be replicated in all other page directories
uintptr_t mm_new_page_directory(void) {
	// get a zeroed page
	uintptr_t new_page_dir = mm_alloc_physical_page(true); //FIXME check if null
	mutex_acquire(&mutex);
	// map it to edit it
	MAP_TO_FREEPAGE(new_page_dir | 3);
	// copy kernel space portion
	memcpy((void*)(&mm_freepage+768), &kernel_page_dir[768], 255 * sizeof(uintptr_t));
	// maps to itself
	((uintptr_t*)&mm_freepage)[1023] = new_page_dir | 3;
	// unmap
	MAP_TO_FREEPAGE(0);
	mutex_release(&mutex);
	return new_page_dir;
}