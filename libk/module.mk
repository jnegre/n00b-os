#libk module

libk_all: libk/build/libk.a

libk/build/libk.a: $(libk_OBJ_FILES)
	$(AR) rcs $@ $(libk_OBJ_FILES)
