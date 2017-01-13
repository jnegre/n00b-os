#include <kernel/tty.h>
#include <kernel/panic.h>

#include <arch/x86/multiboot.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
 
void kernel_main(multiboot_info_t* multiboot_info) {
	/* Initialize terminal interface */
	terminal_initialize();
 
	terminal_writestring("In kernel");
	
	terminal_writestring("\nmem_lower: ");
	terminal_writeu32(multiboot_info->mem_lower);

	terminal_writestring("\nmem_upper: ");
	terminal_writeu32(multiboot_info->mem_upper);

	terminal_writestring("\nmmap_length: ");
	terminal_writeu32(multiboot_info->mmap_length);

	terminal_writestring("\nmmap_addr: ");
	terminal_writeu32(multiboot_info->mmap_addr);
	
	
	multiboot_memory_map_t* mm = (multiboot_memory_map_t*)multiboot_info->mmap_addr;
	
	while((void*)mm < (void*)(multiboot_info->mmap_addr+multiboot_info->mmap_length)) {
		terminal_writestring("\nSize:");
		terminal_writeu32(mm->size);
		terminal_writestring(" addr:");
		terminal_writeu64(mm->addr);
		terminal_writestring(" to:");
		terminal_writeu64(mm->addr + mm->len - 1);
		terminal_writestring(" type:");
		terminal_writeu32(mm->type);
		
		mm = (multiboot_memory_map_t*)((void*)mm+mm->size+sizeof(mm->size));
	}
}

