bootcd_all: bootcd/build/n00b-os.iso

bootcd/build/n00b-os.iso: bootcd/src/grub.cfg kernel/build/n00b-os.kernel
	mkdir -p bootcd/build/isodir/boot/grub/
	cp bootcd/src/grub.cfg bootcd/build/isodir/boot/grub/grub.cfg
	cp kernel/build/n00b-os.kernel bootcd/build/isodir/boot/n00b-os.kernel
	grub-mkrescue -o bootcd/build/n00b-os.iso bootcd/build/isodir
	

.PHONY: demo debug

debug: bootcd/build/n00b-os.iso
	qemu-system-i386 -cdrom $< -s -S

demo: bootcd/build/n00b-os.iso
	qemu-system-i386 -cdrom $<
