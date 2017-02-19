.macro save_task
	pusha
	push %ds
	push %es
	push %fs
	push %gs
.endm

.macro switch_task
	push %esp
	push %ss
	call sched_switch_next_task
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
.endm


# handles irq0, switches to a new task
.global irq0
.type irq0, @function

irq0:
	save_task

	// acknowledge the irq
	pushl $0
	call pic_send_eoi
	popl %eax

	switch_task
