#libk module

libk_all: libk/build/libk.a

libk_INCLUDE_DIRS += kernel/include

libk/build/libk.a: $(libk_OBJ_FILES)
	$(AR) rcs $@ $(libk_OBJ_FILES)
