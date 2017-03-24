#utils module

utils_all: utils/build/bin/true utils/build/bin/false

utils/build/bin/true: utils/build/objs/all/true.o utils/build/objs/arch/i386/crt0.o
	@mkdir -p $(dir $@)
	$(CC) -Wl,--oformat=binary -o $@ $(CFLAGS) $(LDFLAGS) $+

utils/build/bin/false: utils/build/objs/all/false.o utils/build/objs/arch/i386/crt0.o
	@mkdir -p $(dir $@)
	$(CC) -Wl,--oformat=binary -o $@ $(CFLAGS) $(LDFLAGS) $+
