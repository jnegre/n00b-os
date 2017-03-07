# Exits the current thread
# [BP+8] the result value to return (8)
.global sched_exit
.type sched_exit, @function
sched_exit:
	push %ebp
	mov %esp, %ebp

	mov 8(%ebp), %eax
	int $51

	pop %ebp
	ret
