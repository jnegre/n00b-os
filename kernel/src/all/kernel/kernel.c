#include <stdint.h>

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

// Defined by the linker script
// Use only their address, as their value is meaningless
extern int kernel_start;
extern int kernel_end;

void kernel_log_u32(const char* msg, const uint32_t u32);

void kernel_main(uint32_t mmap_length, multiboot_memory_map_t* mmap) {
	/* Initialize terminal interface */
	terminal_initialize();
 
	terminal_writestring("In kernel");
	
	kernel_log_u32("kernel_start", (uint32_t)&kernel_start);
	kernel_log_u32("kernel_end", (uint32_t)&kernel_end);
	
	kernel_log_u32("mmap_length", mmap_length);
	kernel_log_u32("mmap_addr", (uint32_t)mmap);
	
	for(multiboot_memory_map_t* mm = mmap;
			(uint32_t)mm < (uint32_t)(mmap)+mmap_length;
			mm = (multiboot_memory_map_t*)((uint32_t)mm+mm->size+sizeof(mm->size))) {
		terminal_writestring("\nSize:");
		terminal_writeu32(mm->size);
		terminal_writestring(" addr:");
		terminal_writeu64(mm->addr);
		terminal_writestring(" to:");
		terminal_writeu64(mm->addr + mm->len - 1);
		terminal_writestring(" type:");
		terminal_writeu32(mm->type);
	}
}

void kernel_log_u32(const char* msg, const uint32_t u32) {
	terminal_writestring("\n");
	terminal_writestring(msg);
	terminal_writestring(": ");
	terminal_writeu32(u32);
}

