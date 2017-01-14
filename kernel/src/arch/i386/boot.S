# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare constants for the multiboot information
.set BOOTLOADER_MAGIC, 0x2BADB002  # This should be in %eax
.set MAX_MEMORY_MAP_SIZE, 24*20    # Enough for 20 standard multiboot_memory_map_t

# Declare a multiboot header that marks the program as a kernel. These are magic
# values that are documented in the multiboot standard. The bootloader will
# search for this signature in the first 8 KiB of the kernel file, aligned at a
# 32-bit boundary. The signature is in its own section so the header can be
# forced to be within the first 8 KiB of the kernel file.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .rodata
#messages for early boot phase
msg_early_missing_magic:
	.asciz "M\360i\360s\360s\360i\360n\360g\360 \360m\360a\360g\360i\360c\360"
msg_early_too_big:
	.asciz "K\360e\360r\360n\360e\360l\360 \360>\360 \3604\360M\360b\360"
msg_early_no_mmap_flag:
	.asciz "N\360o\360 \360m\360m\360a\360p\360 \360f\360l\360a\360g\360"
msg_early_too_big_mmap:
	.asciz "T\360o\360o\360 \360b\360i\360g\360 \360m\360m\360a\360p\360"

#other messages
msg_return_from_kernel_main:
	.asciz "returned from kernel_main"

# The multiboot standard does not define the value of the stack pointer register
# (esp) and it is up to the kernel to provide a stack. This allocates room for a
# small stack by creating a symbol at the bottom of it, then allocating 16384
# bytes for it, and finally creating a symbol at the top. The stack grows
# downwards on x86. The stack is in its own section so it can be marked nobits,
# which means the kernel file is smaller because it does not contain an
# uninitialized stack. The stack on x86 must be 16-byte aligned according to the
# System V ABI standard and de-facto extensions. The compiler will assume the
# stack is properly aligned and failure to align the stack will result in
# undefined behavior.
.section .bootstrap_stack, "aw", @nobits
	.align 16
stack_bottom:
	.skip 16384 # 16 KiB
stack_top:

.section .bss
	.align 4096 #required for page directory + page table
boot_pagedir:
	.skip 4096
boot_pagetab1:
	.skip 4096
	# Further page tables may be required if the kernel grows beyond 3 MiB.
memory_map_length:
	.skip 16
memory_map:
	.skip MAX_MEMORY_MAP_SIZE

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded. It
# doesn't make sense to return from this function as the bootloader is gone.
.section .text
.global _start
.type _start, @function
_start:
	# The bootloader has loaded us into 32-bit protected mode on a x86
	# machine. Interrupts are disabled. Paging is disabled. The processor
	# state is as defined in the multiboot standard. The kernel has full
	# control of the CPU. The kernel can only make use of hardware features
	# and any code it provides as part of itself. There's no printf
	# function, unless the kernel provides its own <stdio.h> header and a
	# printf implementation. There are no security restrictions, no
	# safeguards, no debugging mechanisms, only what the kernel provides
	# itself. It has absolute and complete power over the
	# machine.

	# To set up a stack, we set the esp register to point to the top of our
	# stack (as it grows downwards on x86 systems). This is necessarily done
	# in assembly as languages such as C cannot function without a stack.
	mov $stack_top, %esp

	# This is a good place to initialize crucial processor state before the
	# high-level kernel is entered. It's best to minimize the early
	# environment where crucial features are offline. Note that the
	# processor is not fully initialized yet: Features such as floating
	# point instructions and instruction set extensions are not initialized
	# yet. The GDT should be loaded here. Paging should be enabled here.
	# C++ features such as global constructors and exceptions will require
	# runtime support to work as well.
	
	jmp 1f
early_panic:
	# display an error message pointed by %esi
	movl $0xB8000, %edi
	cld
2:
	cmpb $0, (%esi)
	je 3f
	movsw
	jmp 2b
3:	
	cli
	hlt
	jmp early_panic
1:
	# Check eax to verify that we've been loaded by a compatible bootloader
	movl $msg_early_missing_magic, %esi
	cmp $BOOTLOADER_MAGIC, %eax
	jne early_panic
	
	# Check we have a memory map
	movl $msg_early_no_mmap_flag, %esi
	movl (%ebx), %eax
	testl $(1<<6), %eax # bit 6 in the flags entry
	jz early_panic
	
	# get the size of the memory map and check it's not too big
	movl 44(%ebx), %ecx
	movl $msg_early_too_big_mmap, %esi
	cmp $MAX_MEMORY_MAP_SIZE, %ecx
	jg early_panic
	
	# copy memory map
	# TODO only copy the fields we actually need
	movl %ecx, memory_map_length
	movl $memory_map, %edi
	movl 48(%ebx), %esi
	rep movsl
	
	##
	## Now let's enable paging
	##
	
	# Check the kernel is small enough to enable paging with a single
	# Page table of 1024 entries to 4Kb of physical memory
	movl $msg_early_too_big, %esi
	movl $kernel_end, %eax
	#subl $kernel_start, %eax
	cmp $4*1024*1024, %eax #4mb
	jge early_panic
	
	# Now fill the page table
	movl $boot_pagetab1, %edi
	movl $0, %esi #map from the 1st byte of RAM
	movl $1024, %ecx #map 1024 pages
	
4:	
	# Map physical address as "present, writable". Note that this maps
	# .text and .rodata as writable.
	# FIXME Mind security and map them as non-writable.
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)
	# Size of page is 4096 bytes.
	addl $4096, %esi
	# Size of entries in boot_pagetab1 is 4 bytes.
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop 4b
	
	# Map the page table to virtual addresses 0x00000000.
	movl $(boot_pagetab1 + 0x003), boot_pagedir + 0 # +0x003 -> activate RW + Present

	# Set cr3 to the address of the boot_page_directory.
	movl $(boot_pagedir), %ecx
	movl %ecx, %cr3

	# Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0
	
	push $memory_map
	push memory_map_length
	push $kernel_end
	push $kernel_start

	# Enter the high-level kernel. The ABI requires the stack is 16-byte
	# aligned at the time of the call instruction (which afterwards pushes
	# the return pointer of size 4 bytes). The stack was originally 16-byte
	# aligned above and we've since pushed a multiple of 16 bytes to the
	# stack since (pushed 0 bytes so far) and the alignment is thus
	# preserved and the call is well defined.
	call kernel_main

	# The kernel shouldn't get there
	push $msg_return_from_kernel_main
	call panic
	
# Set the size of the _start symbol to the current location '.' minus its start.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start
