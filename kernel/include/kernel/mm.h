#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H
#include <stdint.h>
#include <stdbool.h>
#include <arch/x86/multiboot.h> //FIXME remove this dependency

void mm_init_page_allocator(uint32_t mmap_length, multiboot_memory_map_t* mmap);

uintptr_t mm_alloc_physical_page(const bool zero);
void mm_free_physical_page(uintptr_t pa);

int mm_map_page(uintptr_t phys_address, uintptr_t virt_address);
void mm_unmap_page(uintptr_t virt_address);

#endif
