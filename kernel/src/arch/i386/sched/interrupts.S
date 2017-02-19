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
.global irq0_handler
.type irq0_handler, @function
irq0_handler:
	save_task

	# acknowledge the irq
	pushl $0
	call pic_send_eoi
	popl %eax

	# update the counters
	call sched_ms_tick

	switch_task

# handles int 50, sleeps/yields the current task
.global int50_handler
.type int50_handler, @function
int50_handler:
	save_task

	# park the current task
	pushl %eax
	call sched_park_task
	popl %eax

	switch_task
