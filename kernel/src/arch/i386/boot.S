# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare constants for the multiboot information
.set BOOTLOADER_MAGIC, 0x2BADB002  # This should be in %eax
.set MAX_MEMORY_MAP_SIZE, 24*20    # Enough for 20 standard multiboot_memory_map_t
.set MAX_INITRD_SIZE, 10*1024

# The kernel is located at 3GiB (+ 1MiB) but loaded at 1MiB.
# Until paging is enabled, we must substract this value to all addresses.
.set KERNEL_BASE_ADDRESS, 0xC0000000

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
msg_early_no_mods_flag:
	.asciz "N\360o\360 \360m\360o\360d\360s\360 \360f\360l\360a\360g\360"
msg_early_not_one_mod:
	.asciz "D\360i\360d\360n\360'\360t\360 \360h\360a\360v\360e\360 \360a\360 \360s\360i\360n\360g\360l\360e\360 \360m\360o\360d\360"
msg_early_too_big_initrd:
	.asciz "T\360o\360o\360 \360b\360i\360g\360 \360i\360n\360i\360t\360r\360d\360"

#other messages
filename:
	.asciz "kernel/src/arch/i386/boot.S" # FIXME would be nicer to use a macro
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
	.align 4096
.global stack_bottom
stack_bottom:
	.skip 4096 # 4 KiB
stack_top:

.section .data
	.align 4 #(see Intel 3A section 3.5.1)
gdt:
	.macro gdt_entry access:req, base=0, limit=0xFFFFF, flags=0b1100
	.byte (\limit & 0xFF), (\limit & 0xFF00) >> 8
	.byte (\base & 0xFF), (\base & 0xFF00) >> 8
	.byte (\base & 0xFF0000) >> 16
	.byte \access
	.byte ((\flags << 4) | ((\limit & 0xF0000) >> 16))
	.byte (\base & 0xFF000000) >> 24
	.endm

	gdt_entry limit=0, flags=0, access=0 # Null descriptor
	gdt_entry 0b10011010 # Code segment, kernel
	gdt_entry 0b10010010 # Data segment, kernel
.global gdt_info
gdt_info:
	.word gdt_info - gdt - 1
	.long gdt

.section .bss
	.align 4096 #required for page directory + page table
boot_pagedir:
	.skip 4096
boot_pagetab1: # For the kernel
	.skip 4096
boot_pagetab2: # For the stack
	.skip 4096
	# Further page tables may be required if the kernel grows beyond 3 MiB.
.global mm_freepage
mm_freepage:
	.skip 4096
memory_map_length:
	.skip 4
memory_map:
	.skip MAX_MEMORY_MAP_SIZE
initrd_length:
	.skip 4
initrd_image:
	.skip MAX_INITRD_SIZE

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded. It
# doesn't make sense to return from this function as the bootloader is gone.
.section .text
.global _start
.type _start, @function
_start:
	# The bootloader has loaded us into 32-bit protected mode on a x86
	# machine. Interrupts are disabled. Paging is disabled. The processor
	# state is as defined in the multiboot standard.
	

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
	movl $msg_early_missing_magic-KERNEL_BASE_ADDRESS, %esi
	cmp $BOOTLOADER_MAGIC, %eax
	jne early_panic

	##
	## Memory map
	##

	# Check we have a memory map
	movl $msg_early_no_mmap_flag-KERNEL_BASE_ADDRESS, %esi
	bt $6, (%ebx) # bit 6 in the flags entry
	jnc early_panic
	
	# get the size of the memory map and check it's not too big
	movl 44(%ebx), %ecx
	movl $msg_early_too_big_mmap-KERNEL_BASE_ADDRESS, %esi
	cmp $MAX_MEMORY_MAP_SIZE, %ecx
	jg early_panic

	# copy memory map
	# TODO only copy the fields we actually need
	movl %ecx, memory_map_length-KERNEL_BASE_ADDRESS
	movl $memory_map-KERNEL_BASE_ADDRESS, %edi
	movl 48(%ebx), %esi
	rep movsl

	##
	## initrd
	##

	# Check we have mods
	movl $msg_early_no_mods_flag-KERNEL_BASE_ADDRESS, %esi
	bt $3, (%ebx) # bit 3 in the flags entry
	jnc early_panic

	# only 1 mod is expected: initrd
	movl $msg_early_not_one_mod-KERNEL_BASE_ADDRESS, %esi
	cmp $1, 20(%ebx)
	jne early_panic

	# compute the size of initrd znd check it's not too big
	movl $msg_early_too_big_initrd-KERNEL_BASE_ADDRESS, %esi
	movl 24(%ebx), %edx # mods_addr
	movl 4(%edx), %ecx  # mod_end
	subl (%edx), %ecx   # mod_start
	cmp $MAX_INITRD_SIZE, %ecx
	jg early_panic

	# copy initrd
	movl %ecx, initrd_length-KERNEL_BASE_ADDRESS
	movl (%edx), %esi
	movl $initrd_image-KERNEL_BASE_ADDRESS, %edi
	rep movsl
	
	##
	## Now let's enable paging
	##
	
	# Check the kernel is small enough to enable paging with a single
	# Page table of 1024 entries to 4Kb of physical memory
	movl $msg_early_too_big-KERNEL_BASE_ADDRESS, %esi
	movl $kernel_end-KERNEL_BASE_ADDRESS, %eax
	#subl $kernel_start, %eax
	cmp $4*1024*1024, %eax #4mb
	jge early_panic
	
	# Now fill the page table
	movl $boot_pagetab1-KERNEL_BASE_ADDRESS, %edi
	movl $0, %esi #map from the 1st byte of RAM
	movl $1024, %ecx #map 1024 pages.
	
4:	
	# Map physical address as "present, writable". Note that this maps
	# .text and .rodata as writable.
	# FIXME Mind security and map them as non-writable.

	movl $0, (%edi) # not mapped 
	cmp $kernel_physical_end, %esi
	jge 5f
	# except if under kernel_physical_end
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)
5:
	# Size of page is 4096 bytes.
	addl $4096, %esi
	# Size of entries in boot_pagetab1 is 4 bytes.
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop 4b
	
	# Map the page table to virtual addresses 0x00000000.
	movl $(boot_pagetab1 - KERNEL_BASE_ADDRESS + 0x003), boot_pagedir-KERNEL_BASE_ADDRESS + 0 # +0x003 -> activate RW + Present
	# Map the page table to virtual addresses 0xC0000000 (entry 768).
	movl $(boot_pagetab1 - KERNEL_BASE_ADDRESS + 0x003), boot_pagedir-KERNEL_BASE_ADDRESS + 768*4
	# Map the directory to itself at 0xFFC00000 (Recursive Page Directory).
	movl $(boot_pagedir - KERNEL_BASE_ADDRESS + 0x003), boot_pagedir-KERNEL_BASE_ADDRESS + 1023*4
	
	# Set cr3 to the address of the boot_page_directory.
	movl $(boot_pagedir-KERNEL_BASE_ADDRESS), %ecx
	movl %ecx, %cr3

	# Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0
	
	# Jump to higher half with an absolute jump.
	movl $higher_half, %eax
	jmp *%eax
	
higher_half:
	# Yeah, no more KERNEL_BASE_ADDRESS shenanigans!

	# Remove identity mapping.
	movl $0, boot_pagedir + 0
	invlpg (0)

	##
	## Load GDT
	##
	lgdt (gdt_info)
	# reload segments
	jmp $0x08, $reload_CS # 0x08 points at the new code selector
reload_CS:
	# Reload data segment registers
	mov $0x10, %ax # 0x10 points at the new data selector
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss


	#move the yet single page of the stack to 0xFFBFF000
	movl $(boot_pagetab2-KERNEL_BASE_ADDRESS + 0x003), boot_pagedir + 1022*4
	invlpg (boot_pagetab2)
	movl $(stack_bottom-KERNEL_BASE_ADDRESS + 0x003), boot_pagetab2 + 1023*4
	invlpg (0xFFBFF000)

	# To set up a stack, we set the esp register to point to the top of our
	# stack (as it grows downwards on x86 systems). This is necessarily done
	# in assembly as languages such as C cannot function without a stack.
	mov $0xFFBFFFF0, %esp

	# leave some place for the task_info
	pushl $0
	pushl $0
	# canary
	pushl $0
	pushl $0xDEADBEEF

	# set up minimal interrupts
	call itr_init

	# parameters for kernel_main
	push $initrd_image
	push initrd_length
	push $memory_map
	push memory_map_length

	# Enter the high-level kernel. The ABI requires the stack is 16-byte
	# aligned at the time of the call instruction (which afterwards pushes
	# the return pointer of size 4 bytes). The stack was originally 16-byte
	# aligned above and we've since pushed a multiple of 16 bytes to the
	# stack since (pushed 32 bytes so far) and the alignment is thus
	# preserved and the call is well defined.
	call kernel_main

	# The kernel shouldn't get there
	push $msg_return_from_kernel_main
	push $0 # FIXME use __LINE__
	push $filename
	call kernel_panic
	
# Set the size of the _start symbol to the current location '.' minus its start.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start
