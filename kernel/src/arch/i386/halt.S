# void halt(void)
# halts the cpu, never returns
.global halt
.type halt, @function

# If the system has nothing more to do, put the computer into an
# infinite loop. To do that:
# 1) Disable interrupts with cli (clear interrupt enable in eflags).
#    They are already disabled by the bootloader, so this is not needed.
#    Mind that you might later enable interrupts and return from
#    kernel_main (which is sort of nonsensical to do).
# 2) Wait for the next interrupt to arrive with hlt (halt instruction).
#    Since they are disabled, this will lock up the computer.
# 3) Jump to the cli instruction if it ever wakes up due to a
#    non-maskable interrupt occurring or due to system management mode.
halt:
1:	cli
	hlt
	jmp 1b
