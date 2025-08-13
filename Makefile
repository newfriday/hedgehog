#
# os bootloader
#
GCC_MAJOR=$(shell echo __GNUC__ | $(CC) -E -x c - | tail -n 1)
GCC_MINOR=$(shell echo __GNUC_MINOR__ | $(CC) -E -x c - | tail -n 1)


# initialize the flags we used
CFLAGS :=
ASFLAGS :=
LDFLAGS :=
ARFLAGS :=
ARCH_CFLAGS :=
ARCH_ASFLAGS :=
ARCH_ARFLAGS :=
ARCH_LDFLAGS :=

#BASH = $(shell which bash)

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

CFLAGS +=-fstack-protector-strong
#warning...
CFLAGS +=-fno-stack-protector

ASFLAGS += -m64 -nostdinc -nostdlib

LDFLAGS += -Wl,--gc-sections -nostartfiles -nostdlib
LDFLAGS += -Wl,-n,-z,max-page-size=0x1000
LDFLAGS += -static

ARCH_CFLAGS += -gdwarf-2
ARCH_ASFLAGS += -gdwarf-2 -DASSEMBLER=1
ARCH_ARFLAGS +=
ARCH_LDFLAGS +=

ARCH_LDSCRIPT_ORI =link_ram.ld.S
ARCH_LDSCRIPT =link_ram.ld
ARCH_LDSCRIPT_IN =link_ram.ld.in

#CFLAGS += -mindirect-branch=thunk-extern -mindirect-branch-register
CFLAGS += -DCONFIG_RETPOLINE
CFLAGS += -DHV_DEBUG -DPROFILING_ON -fno-omit-frame-pointer

INCLUDE_PATH += ./

CC ?= gcc
AS ?= as
AR ?= ar
LD ?= ld
OBJCOPY ?= objcopy

export CFLAGS ASFLAGS ARFLAGS LDFLAGS ARCH_CFLAGS ARCH_ASFLAGS ARCH_ARFLAGS ARCH_LDFLAGS

OBJ_S +=start.o \

OBJ_C +=init.o\
		string.o\
		early_print.o\
		early_console.o\
		reboot.o\
		boot_info.o

OBJ += $(OBJ_C) $(OBJ_S)

bootloader.elf:$(ARCH_LDSCRIPT) $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $(ARCH_LDFLAGS) -T$(ARCH_LDSCRIPT) -Wl,--start-group $(OBJ) -Wl,--end-group

$(ARCH_LDSCRIPT):$(ARCH_LDSCRIPT_ORI)
	$(CC) -E -P $< -o $@

include $(OBJ_C:.o=.d)

%.o:%.c
	[ ! -e $@ ] && mkdir -p $(dir $@);\
	$(CC) $(patsubst %, -I%, $(INCLUDE_PATH)) -I. -c $(CFLAGS) $(ARCH_CFLAGS) $< -o $@ -MMD -MT $@

%.d:%.c
	@set -e; rm -f $@; $(CC) -MM $< $(INCLUDEFLAGS) > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,\l.o $@ : ,g' < $@.$$$$ > $@;\
	rm -f $@.$$$$

start.o:start.S
	$(CC) $(patsubst %, -I%, $(INCLUDE_PATH)) -I. $(ASFLAGS) $(ARCH_ASFLAG) -c $< -o $@ -MMD -MT $@

clean:
	rm *.d *.o *.elf
