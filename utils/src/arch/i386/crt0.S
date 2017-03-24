# This should be part of libc, but we don't have one. Yet?
# TODO move to libc
.section .text

.global _start
_start:

# run main
call main

# TODO call exit instead
1:
	hlt;
	jmp 1b
