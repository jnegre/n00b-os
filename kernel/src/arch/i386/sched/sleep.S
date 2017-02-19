# Sleeps/yields the current thread
# [BP+8] sleep duration in ms (8)
.global sched_sleep
.type sched_sleep, @function
sched_sleep:
	push %ebp
	mov %esp, %ebp

	mov 8(%ebp), %eax
	int $50

	pop %ebp
	ret
