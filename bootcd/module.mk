bootcd_all: bootcd/build/n00b-os.iso

bootcd/build/n00b-os.iso: bootcd/build/initrd bootcd/src/grub.cfg kernel/build/n00b-os.kernel
	mkdir -p bootcd/build/isodir/boot/grub/
	cp bootcd/src/grub.cfg bootcd/build/isodir/boot/grub/grub.cfg
	cp bootcd/build/initrd bootcd/build/isodir/boot/initrd
	cp kernel/build/n00b-os.kernel bootcd/build/isodir/boot/n00b-os.kernel
	grub-mkrescue -o bootcd/build/n00b-os.iso bootcd/build/isodir

bootcd/build/initrd: $(shell find -H bootcd/src/initrd -type f 2>/dev/null) utils/build/bin/true utils/build/bin/false
	mkdir -p bootcd/build/initrd.directory/bin
	cp -r bootcd/src/initrd/* bootcd/build/initrd.directory
	cp -r utils/build/bin/* bootcd/build/initrd.directory/bin
	tar --create --format=ustar --owner=root --group=root --file bootcd/build/initrd --directory bootcd/build/initrd.directory `ls bootcd/build/initrd.directory`

.PHONY: demo debug

debug: bootcd/build/n00b-os.iso
	qemu-system-i386 -cdrom $< -s -S -no-reboot -no-shutdown

demo: bootcd/build/n00b-os.iso
	qemu-system-i386 -cdrom $< -no-reboot -no-shutdown
