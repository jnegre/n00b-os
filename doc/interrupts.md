Vector | Type | Handled? | Description
-------|------|----------|-------------
0 | Fault | Y | Divide Error
1 | Fault/Trap | N | Debug Exception
2 | Interrupt | Y | NMI Interrupt
3 | Trap | N | Breakpoint
4 | Trap | Y | Overflow
5 | Fault | N | BOUND Range Exceeded
6 | Fault | N | Invalid Opcode (Undefined Opcode)
7 | Fault | N | Device Not Available (No Math Coprocessor)
8 | Abort | Y | Double Fault
9 | Fault | N | Coprocessor Segment Overrun (reserved)
10 | Fault | Y | Invalid TSS
11 | Fault | Y | Segment Not Present
12 | Fault | Y | Stack-Segment Fault
13 | Fault | Y | General Protection
14 | Fault | Y | Page Fault
15 | | N | (Intel reserved. Do not use.)
16 | Fault | Y | x87 FPU Floating-Point Error
17 | Fault | Y | Alignment Check
18 | Abort | Y | Machine Check
19 | Fault | Y | SIMD Floating-Point Exception
20 | Fault | N | Virtualization Exception
21-31 | | N | Intel reserved
32 | IRQ | N | 0 - Programmable Interrupt Timer
33 | IRQ | N | 1 - Keyboard
34 | IRQ | N | 2 - Cascade (used internally, never raised)
35 | IRQ | N | 3 - COM2
36 | IRQ | N | 4 - COM1
37 | IRQ | N | 5 - LPT2
38 | IRQ | N | 6 - Floppy Disk
39 | IRQ | N | 7 - LPT1 (spurious interrupt)
40 | IRQ | N | 8 - CMOS real-time clock
41 | IRQ | N | 9 - Free for peripherals / SCSI / NIC 
42 | IRQ | N | 10 - Free for peripherals / SCSI / NIC 
43 | IRQ | N | 11 - Free for peripherals / SCSI / NIC 
44 | IRQ | N | 12 - PS2 Mouse
45 | IRQ | N | 13 - FPU
46 | IRQ | N | 14 - Primary ATA Hard Disk 
47 | IRQ | N | 15 - Secondary ATA Hard Disk
