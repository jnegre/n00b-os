# handles irq1
.global irq1_handler
.type irq1_handler, @function
irq1_handler:
	# read the scan code and give it to the C code
	xor %eax, %eax
	inb $0x60, %al
	pushl %eax
	call kbd_handle_key
	popl %eax
	# acknowledge the irq
	pushl $1
	call pic_send_eoi
	popl %eax
	iret
