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
 
 
void kernel_main(void* start, void* end,multiboot_info_t* multiboot_info) {
	/* Initialize terminal interface */
	terminal_initialize();
 
	terminal_writestring("In kernel");
	
	kernel_log_u32("start", start);
	kernel_log_u32("end", end);
	
	kernel_log_u32("mem_lower", multiboot_info->mem_lower);
	kernel_log_u32("mem_upper", multiboot_info->mem_upper);
	kernel_log_u32("mmap_length", multiboot_info->mmap_length);
	kernel_log_u32("mmap_addr", multiboot_info->mmap_addr);
	
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

void kernel_log_u32(const char* msg, const uint32_t u32) {
	terminal_writestring("\n");
	terminal_writestring(msg);
	terminal_writestring(": ");
	terminal_writeu32(u32);
}

