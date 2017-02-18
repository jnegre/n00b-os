.set MASTER_COMMAND, 0x20
.set MASTER_DATA, 0X21
.set SLAVE_COMMAND, 0xA0
.set SLAVE_DATA, 0XA1

.set PIC_EOI, 0x20 # End-of-interrupt command code

.section .text

#
# Reinitialize the PIC controllers and set new vector offsets
#
.global pic_init
.type pic_init, @function
pic_init:
	push %eax

	# Disable all IRQs
	call pic_disable_all
	# starts the initialization sequence (in cascade mode)
	mov $0x11, %al
	out %al, $MASTER_COMMAND
	out %al, $SLAVE_COMMAND
	# ICW2: Master PIC vector offset
	mov $32, %al
	out %al, $MASTER_DATA
	# ICW2: Slave PIC vector offset
	mov $40, %al
	out %al, $SLAVE_DATA
	# ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	mov $0b100, %al
	out %al, $MASTER_DATA
	# ICW3: tell Slave PIC its cascade identity (0000 0010)
	mov $0b10, %al
	out %al, $SLAVE_DATA
	# ICW4: set as Master in 8086/88 Mode
	mov $0b101, %al
	out %al, $MASTER_DATA
	# ICW4: set as Slave in 8086/88 Mode
	mov $0b001, %al
	out %al, $SLAVE_DATA

	# Disable all IRQs (again)
	call pic_disable_all

	pop %eax
	ret


#
# Sends EOI for an IRQ
# [BP+8] IRQ (long)
#
.global pic_send_eoi
.type pic_send_eoi, @function
pic_send_eoi:
	push %ebp
	mov %esp, %ebp
	push %eax
	push %edx

	mov $PIC_EOI, %al

	cmpb $8, 8(%ebp)
	jl 1f # if IRQ < 8
	# PIC 2
	mov $SLAVE_COMMAND, %dx
	jmp 2f
1:
	# PIC 1
	mov $MASTER_COMMAND, %dx
2:
	out %al, %dx

	pop %edx
	pop %eax
	pop %ebp
	ret

#
# Disable all IRQs
#
.global pic_disable_all
.type pic_disable_all, @function
pic_disable_all:
	push %eax

	mov $0xFF, %al
	out %al, $MASTER_DATA
	out %al, $SLAVE_DATA

	pop %eax
	ret

#
# Enables an IRQ
# [BP+8] IRQ (long)
#
.global pic_enable
.type pic_enable, @function
pic_enable:
	push %ebp
	mov %esp, %ebp
	push %eax
	push %ebx
	push %edx;


	mov 8(%ebp), %ebx
	cmpb $8, 8(%ebp)
	jl 1f # if IRQ < 8
	# PIC 2
	mov $SLAVE_DATA, %dx
	sub $8, %ebx
	jmp 2f
1:
	# PIC 1
	mov $MASTER_DATA, %dx
2:
	# get current mask
	in %dx, %al
	btr %bx, %ax
	out %al, %dx

	pop %edx
	pop %ebx
	pop %eax
	pop %ebp
	ret
