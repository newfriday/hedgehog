#
# os bootloader (updated for new tree layout)
#

# toolchain (can be overridden in environment)
CC ?= gcc
AS ?= as
AR ?= ar
LD ?= ld
OBJCOPY ?= objcopy

# flags
CFLAGS :=
ASFLAGS :=
LDFLAGS :=
ARFLAGS :=
ARCH_CFLAGS :=
ARCH_ASFLAGS :=
ARCH_ARFLAGS :=
ARCH_LDFLAGS :=

ARFLAGS += crs

CFLAGS += -Wall -W
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fshort-wchar -ffreestanding
CFLAGS += -fsigned-char
CFLAGS += -m64 -mno-mmx -mno-sse -mno-sse2 -mno-80387 -mno-fp-ret-in-387
CFLAGS += -mno-red-zone
CFLAGS += -nostdinc -nostdlib -fno-common
CFLAGS += -Werror
CFLAGS += -static

# disable stack-protector in final (kept as in original)
CFLAGS += -fstack-protector-strong
CFLAGS += -fno-stack-protector

ASFLAGS += -m64 -nostdinc -nostdlib

LDFLAGS += -Wl,--gc-sections -nostartfiles -nostdlib
LDFLAGS += -Wl,-n,-z,max-page-size=0x1000
LDFLAGS += -static

ARCH_CFLAGS += -gdwarf-2
ARCH_ASFLAGS += -gdwarf-2 -DASSEMBLER=1

ARCH_LDSCRIPT_ORI = link_ram.ld.S
ARCH_LDSCRIPT = link_ram.ld
ARCH_LDSCRIPT_IN = link_ram.ld.in

CFLAGS += -DCONFIG_RETPOLINE
CFLAGS += -DHV_DEBUG -DPROFILING_ON -fno-omit-frame-pointer

# include paths
INCLUDE_PATH := . include

# convenience: convert INCLUDE_PATH -> -I flags
INCLUDE_FLAGS := $(patsubst %,-I%,$(INCLUDE_PATH))

# Source / object lists (match new layout)
OBJ_S := start.o

OBJ_C := init.o \
         utils/string.o \
         early/early_print.o \
         early/early_console.o \
         reboot.o \
         boot_info.o

OBJ := $(OBJ_S) $(OBJ_C)

# dependency files
DEPS := $(OBJ:.o=.d)

# default target
all: bootloader.elf

# link
bootloader.elf: $(ARCH_LDSCRIPT) $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $(ARCH_LDFLAGS) -T$(ARCH_LDSCRIPT) -Wl,--start-group $(OBJ) -Wl,--end-group

# preprocess linker script if necessary
$(ARCH_LDSCRIPT): $(ARCH_LDSCRIPT_ORI)
	$(CC) -E -P $< -o $@

# include dependency files if present; don't fail if they're missing
-include $(DEPS)

# C compilation rule: produce .o and .d (dependency) reliably
%.o: %.c
	[ ! -e $@ ] && mkdir -p $(dir $@);
	$(CC) $(INCLUDE_FLAGS) -I. -c $(CFLAGS) $(ARCH_CFLAGS) $< -o $@ -MMD -MF $(@:.o=.d) -MT $@

# assembly rule for start.S (also generate .d)
start.o: start.S
	[ ! -e $@ ] && mkdir -p $(dir $@);
	$(CC) $(INCLUDE_FLAGS) -I. $(ASFLAGS) $(ARCH_ASFLAGS) -c $< -o $@ -MMD -MF $(@:.o=.d) -MT $@

# clean
clean:
	rm -f $(OBJ) $(DEPS) $(ARCH_LDSCRIPT) *.elf

.PHONY: all clean
