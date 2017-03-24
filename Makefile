#Master makefile
MODULES:=libk kernel bootcd utils

#FIXME should be configured from some external file, with default values
HOST:=i686-elf
HOSTARCH:=i386

AR:=$(HOST)-ar
AS:=$(HOST)-as
CC:=$(HOST)-gcc


CFLAGS:=-ffreestanding -Wall -Wextra -g
SFLAGS:=-g --warn
LDFLAGS:=-nostdlib -lgcc

ARCHDIR=arch/$(HOSTARCH)

.PHONY: all todo clean

all: $(patsubst %,%_all,$(MODULES))

todo:
	-@grep --color=always -n -r -e F\IXME -e T\ODO .
	
define MODULE_TEMPLATE =
.PHONY: $(1)_all $(1)_clean

$(1)_C_SRC_FILES:=$$(shell find -H $(1)/src/all $(1)/src/$(ARCHDIR) -name "*.c" -type f 2>/dev/null)
$(1)_S_SRC_FILES:=$$(shell find -H $(1)/src/$(ARCHDIR) -name "*.S" -type f 2>/dev/null)

$(1)_OBJ_FILES:=$$(patsubst $(1)/src/%.c,$(1)/build/objs/%.o,$$($(1)_C_SRC_FILES)) \
$$(patsubst $(1)/src/%.S,$(1)/build/objs/%.o,$$($(1)_S_SRC_FILES))

$(1)_INCLUDE_DIRS:=$(1)/include

include $(1)/module.mk
#include dependency files
-include $$($(1)_OBJ_FILES:.o=.d)

$(1)/build/objs/%.o: $(1)/src/%.c
	@mkdir -p $$(dir $$@)
	$$(CC) -MD -MP -c $$< -o $$@ -std=gnu11 $$(CFLAGS) $$(foreach INCLUDE,$$($(1)_INCLUDE_DIRS),-I$$(INCLUDE))

$(1)/build/objs/%.o: $(1)/src/%.S
	@mkdir -p $$(dir $$@)
	$$(AS) --MD $$(@:.o=.d) $$(SFLAGS) -o $$@ $$<

clean: $(1)_clean
$(1)_clean:
	-rm -r $(1)/build

endef

$(foreach MODULE,$(MODULES),$(eval $(call MODULE_TEMPLATE,$(MODULE))))

