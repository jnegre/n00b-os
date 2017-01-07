#FIXME should be configured from some external file, with default values
HOST:=i686-elf
HOSTARCH:=i386



AS:=$(HOST)-as
CC:=$(HOST)-gcc

CFLAGS:=-ffreestanding -Wall -Wextra -g -Iinclude
SFLAGS:=-g
LDFLAGS:=-nostdlib -lgcc

ARCHDIR=arch/$(HOSTARCH)


C_SRC_FILES:=$(shell find kernel $(ARCHDIR) -name "*.c" -type f)
S_SRC_FILES:=$(shell find $(ARCHDIR) -name "*.S" -type f)

#FIXME use a build dir
OBJ_FILES:=$(C_SRC_FILES:.c=.o) $(S_SRC_FILES:.S=.o)
D_FILES:=$(C_SRC_FILES:.c=.d)

.PHONY: all clean demo debug todo

all: n00b-os.kernel

n00b-os.kernel: $(OBJ_FILES) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LDFLAGS) $(OBJ_FILES)
	grub-file --is-x86-multiboot n00b-os.kernel

#FIXME should only compile files from kernel + my arch
.c.o:
	$(CC) -MD -MP -c $< -o $@ -std=gnu11 $(CFLAGS)
	
.S.o:
	$(AS) $< -o $@ $(SFLAGS)

clean:
	rm -f n00b-os.kernel
	rm -f $(OBJ_FILES)
	rm -f $(D_FILES)

debug: n00b-os.kernel
	qemu-system-i386 -kernel n00b-os.kernel -s -S

demo: n00b-os.kernel
	qemu-system-i386 -kernel n00b-os.kernel
#FIXME use -cdrom or -hda with grub

todo:
	-@grep --color=always -n -r -e F\IXME -e T\ODO *

#include dependency files
-include $(D_FILES)
