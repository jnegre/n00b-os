#
# Can't be named interrupt.S because there's also an interrupt.c,
# and they would both generate an interrupt.o.
#

.set NB_OF_INTER, 256

.section .bss
.align 4
idt:
	.skip (NB_OF_INTER * 8)
idt_info:
	.word 0 # size -1
	.long 0 # idt


.section .text

#
# Inits the idt and configure the PIC.
# Adds some initial interrupt handlers.
#
.global itr_init
.type itr_init, @function
itr_init:
	# fill idt_info
	movw $NB_OF_INTER * 8 -1, idt_info
	movl $idt, idt_info+2
	lidt (idt_info)

	# assign IRQs to their interrupts (none are enabled yet)
	call pic_init

	# add initial handlers
	call itr_set_initial_handlers

	ret

#
# Sets an interrupt handler.
# [BP+20] handler (32)
# [BP+16] code segment (16)
# [BP+12] type (8)
# [BP+8]  interrupt number (8)
#
.global itr_set_handler
.type itr_set_handler, @function
itr_set_handler:
	push %ebp
	mov %esp, %ebp
	push %eax
	push %ebx
	push %ecx
	push %edx

	# low 32 bits in eax
	mov 16(%ebp), %eax
	mov $16, %cl
	shl %cl, %eax
	mov 20(%ebp), %ebx
	and $0xFFFF, %ebx
	or %ebx, %eax

	# high 32 bits in ebx
	mov 20(%ebp), %ebx
	and $0xFFFF0000, %ebx
	mov 12(%ebp), %edx
	mov $8, %cl
	shl %cl, %edx
	or %edx, %ebx

	# idt index
	mov 8(%ebp), %ecx

	pushf
	cli #FIXME won't be enough on multiprocessor
	mov %eax, idt(, %ecx, 8)
	mov %ebx, idt+4(, %ecx, 8)
	popf

	pop %edx
	pop %ecx
	pop %ebx
	pop %eax
	pop %ebp
	ret

.global itr_enable
.type itr_enable, @function
itr_enable:
	sti
	ret

.global itr_disable
.type itr_disable, @function
itr_disable:
	cli
	ret
	