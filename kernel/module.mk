#kernel module

kernel_all: kernel/build/n00b-os.kernel

kernel_INCLUDE_DIRS += libk/include

kernel/build/n00b-os.kernel: $(kernel_OBJ_FILES) kernel/src/$(ARCHDIR)/linker.ld libk/build/libk.a
	$(CC) -T kernel/src/$(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LDFLAGS) $(kernel_OBJ_FILES) libk/build/libk.a
	grub-file --is-x86-multiboot $@

.PHONY: demo debug

debug: kernel/build/n00b-os.kernel
	qemu-system-i386 -kernel $< -s -S

demo: kernel/build/n00b-os.kernel
	qemu-system-i386 -kernel $<
#FIXME use -cdrom or -hda with grub

