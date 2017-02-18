# handles irq0, switches to a new task
.global irq0
.type irq0, @function

irq0:
	pusha
	push %ds
	push %es
	push %fs
	push %gs

	// acknowledge the irq
	pushl $0
	call pic_send_eoi
	popl %eax

	push %esp
	push %ss
	call irq0_C
	// the 2 parameters now contain the new values
	pop %ebx
	pop %ecx
	// set new GDT
	lgdt (%eax)
	// set new SS
	mov %bx, %ss
	// set new ESP
	mov %ecx, %esp

	// We are now in the next task's stack
	pop %gs
	pop %fs
	pop %es
	pop %ds
	popa
	iret
