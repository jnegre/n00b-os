#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <kernel/mm.h>
#include <kernel/sync.h>
#include <arch/x86/multiboot.h>
#include <kernel/tty.h> //FIXME debug

extern int kernel_physical_end;
extern volatile uintptr_t mm_freepage; // virtual address to temporary map to
                                       // FIXME take care of the physical memory at this addr at boot
//TODO static spinlock_t lock = SPIN_LOCK_INIT;
static uintptr_t *kernel_page_dir = (uintptr_t*) 0xFFFFF000; // last page in vmem is mapped to the page dir
static uintptr_t first_free_page = 0;

static uint32_t stat_mark = 0;

typedef struct __attribute__ ((packed)){
	int pagetable;
	int page;
}pageinfo, *ppageinfo;

static pageinfo mm_virtaddrtopageindex(uintptr_t addr){
	pageinfo pginf;

	// align address to 4k (highest 20-bits of address)
	addr &= ~0xFFF;
	//FIXME use bit shifts
	pginf.pagetable = addr / 0x400000; // each page table covers 0x400000 bytes in memory
	pginf.page = (addr % 0x400000) / 0x1000; //0x1000 = page size
	return pginf;
}

static pageinfo mm_freepage_info;

//FIXME move elsewhere
static inline uintptr_t max(uintptr_t a, uintptr_t b) {
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
	mm_freepage_info = mm_virtaddrtopageindex((uintptr_t)&mm_freepage);
	uintptr_t page_after_kernel = (((uintptr_t) &kernel_physical_end >> 12) + 1)*4096; //FIXME rename
	uintptr_t previous = 0;
	
	// go through all unused pages of physical memory and mark them with the addr of the next one
	// but do not go over 4G
	for(multiboot_memory_map_t* mm = mmap;
			(uint32_t)mm < (uint32_t)(mmap)+mmap_length;
			mm = (multiboot_memory_map_t*)((uint32_t)mm+mm->size+sizeof(mm->size))) {
		if(mm->type == 1) {
			//FIXME overflow!?
			for(uintptr_t next = max(page_after_kernel, mm->addr); next < min(mm->addr + mm->len, 0xFFFFFFFF); next += 4096) {
				if(first_free_page == 0) {
					first_free_page = next;
				} else {
					mark_page(previous, next);
				}
				previous = next;
			}
		}
	}
	mark_page(previous, 0);// close the linked list
	// stats
	terminal_writestring("\nMarked physical pages: ");
	terminal_writeu32(stat_mark);

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
	uintptr_t *page_table = (uintptr_t *) (0xFFC00000 + (mm_freepage_info.pagetable * 0x1000));
	page_table[mm_freepage_info.page] = physical_addr | 3;
	invlpg((void*)&mm_freepage);
	// write
	mm_freepage = next;
	// for stats
	stat_mark++;
}


//FIXME sync
uintptr_t mm_alloc_physical_page(const bool zero) {
	uintptr_t page = first_free_page;
	if(page != 0) {
		// map
		uintptr_t *page_table = (uintptr_t *) (0xFFC00000 + (mm_freepage_info.pagetable * 0x1000));
		page_table[mm_freepage_info.page] = page | 3;
		invlpg((void*)&mm_freepage);
		// read next
		first_free_page = mm_freepage;
		// zero page if requested
		if(zero) {
			memset((void*)&mm_freepage, 0, 4096);
		}
		// unmap
		page_table[mm_freepage_info.page] = 0;
		invlpg((void*)&mm_freepage);
	}
	return page;
}

//FIXME sync
void mm_free_physical_page(uintptr_t physical_addr){
	// map
	uintptr_t *page_table = (uintptr_t *) (0xFFC00000 + (mm_freepage_info.pagetable * 0x1000));
	page_table[mm_freepage_info.page] = physical_addr | 3;
	invlpg((void*)&mm_freepage);
	// link to next entry
	mm_freepage = first_free_page;
	// unmap
	page_table[mm_freepage_info.page] = 0;
	invlpg((void*)&mm_freepage);
	// update
	first_free_page = physical_addr;
}

//FIXME sync
/*
 * Does not allow to map 0 or to 0
 */
int mm_map_page(uintptr_t phys_address, uintptr_t virt_address){
	if(phys_address == 0 || virt_address == 0) {
		return -1;
	}
	pageinfo pginf = mm_virtaddrtopageindex(virt_address); // get the PDE and PTE indexes for the addr
	
	if(kernel_page_dir[pginf.pagetable] & 1) {
		// page table exists.
		uintptr_t *page_table = (uintptr_t *) (0xFFC00000 + (pginf.pagetable * 0x1000)); // virt addr of page table
		if(!page_table[pginf.page] & 1) {
			// page isn't mapped
			page_table[pginf.page] = phys_address | 3;
		} else {
			// page is already mapped
			return -1; //FIXME use const
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
	return 0; //FIXME use const
}

//FIXME sync
void mm_unmap_page(uintptr_t virt_address){
	pageinfo pginf = mm_virtaddrtopageindex(virt_address);
	
	if(kernel_page_dir[pginf.pagetable] & 1) {
		int i;
		uintptr_t *page_table = (uintptr_t *) (0xFFC00000 + (pginf.pagetable * 0x1000));
		if(page_table[pginf.page] & 1) {
			// page is mapped, so unmap it
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
}