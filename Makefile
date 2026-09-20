ifdef OS
	WINDOWS = 1
endif

SDK ?= /Users/bertramd/.vscode/extensions/dgis.atari-st-dev-0.2.1/sdk/darwin
TOOLCHAIN = $(SDK)/opt/cross-mint
CC = $(TOOLCHAIN)/bin/m68k-atari-mintelf-gcc
OBJCOPY = $(TOOLCHAIN)/bin/m68k-atari-mintelf-objcopy
SDK_ROOT = $(TOOLCHAIN)/m68k-atari-mintelf/sys-root
SDK_USR = $(SDK_ROOT)/usr
COMPAT_INCLUDE ?= gcc/include

CFLAGS = --sysroot=$(SDK_ROOT) -D__GEMLIB_OLDNAMES \
	-Isrc -I$(COMPAT_INCLUDE) -std=c99 -g \
	-Wno-incompatible-pointer-types -x c
LDFLAGS = $(SDK_USR)/lib/crt0.o -nostdlib -L$(SDK_USR)/lib \
	-lgem -lm -lc -lgcc

BUILD_DIR = build
TARGET = realtim5.prg

# A symbol-stripped copy of TARGET, loaded into Hatari during debug sessions
# instead of TARGET itself. Hatari's own debugger parses the running .prg's
# full ELF symbol table for its "monitor symbols"/CPU-view commands, and a
# large table (e.g. with RTM_BASE_SRCS/RTM_OPT_SRCS enabled) overflows its
# remote-protocol response, breaking the gdb connection with repeated
# "Ignoring packet error" messages. TARGET_DEBUG keeps .text/.data/.bss
# byte-identical to TARGET (only .symtab/.strtab/.debug_* are discarded), so
# addresses still match exactly; gdb is pointed at the full TARGET (see
# .vscode/launch.json's "program") for its own symbols/source-level
# debugging, while Hatari loads this stripped copy at runtime. The name
# must fit GEMDOS's 8.3 filename limit (<=8 chars + ".prg") so Hatari's
# GEMDOS HDD emulation doesn't truncate/collide it with another file.
TARGET_DEBUG = rt5run.prg

SRCS = \
	src/desktop.c \
	src/dialog.c \
	src/disk.c \
	src/event.c \
	src/gemain.c \
	src/global.c \
	src/initerm.c \
	src/menu.c \
	src/rcm.c \
	src/resource.c \
	src/windows.c \
	src/lists.c \
	src/xrsrc.c

RTM_BASE_SRCS = \
	src/realtspc.c \
	src/objects.c \
	src/init_rtm.c \
	src/dispobj.c \
	src/msh.c \
	src/tra.c \
	src/var.c \
	src/midishare_stub.c

SRCS += $(RTM_BASE_SRCS)

RTM_OPT_SRCS = \
	src/a3d.c \
	src/big.c \
	src/cmi.c \
	src/cmo.c \
	src/ed4.c \
	src/eff.c \
	src/gen.c \
	src/gmi.c \
	src/koo.c \
	src/lfo.c \
	src/maa.c \
	src/mae.c \
	src/man.c \
	src/mtr.c \
	src/par.c \
	src/pow.c \
	src/puf.c \
	src/spg.c \
	src/spo.c \
	src/sps.c \
	src/syn.c

#SRCS += $(RTM_OPT_SRCS)


OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean

all: $(TARGET) $(TARGET_DEBUG)

$(TARGET): $(OBJS)
	$(info Linking $(TARGET))
	$(CC) $^ $(LDFLAGS) -o $@

$(TARGET_DEBUG): $(TARGET)
	$(info Creating symbol-stripped debug-run copy $(TARGET_DEBUG))
	cp $(TARGET) $(TARGET_DEBUG)
	$(OBJCOPY) --discard-all $(TARGET_DEBUG)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(info Compiling $<)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	$(info Cleaning...)
ifdef WINDOWS
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@del /q $(TARGET) $(TARGET_DEBUG)
else
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(TARGET_DEBUG)
endif
