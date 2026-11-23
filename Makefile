ifdef OS
	WINDOWS = 1
endif

# Cross toolchain prefix. Defaults to the standalone universal (x86_64/arm64)
# GCC 15 m68k-atari-mintelf toolchain from tho-otto.m68k.eu installed in
# /opt/cross-mint. To build with the compiler bundled in the VS Code
# extension instead (x86_64 only, needs Rosetta on Apple Silicon):
#   make TOOLCHAIN=$(SDK)/opt/cross-mint
SDK ?= /Users/bertramd/.vscode/extensions/dgis.atari-st-dev-0.2.1/sdk/darwin
TOOLCHAIN ?= /opt/cross-mint
CC = $(TOOLCHAIN)/bin/m68k-atari-mintelf-gcc
OBJCOPY = $(TOOLCHAIN)/bin/m68k-atari-mintelf-objcopy
SDK_ROOT = $(TOOLCHAIN)/m68k-atari-mintelf/sys-root
SDK_USR = $(SDK_ROOT)/usr
COMPAT_INCLUDE ?= gcc/include

# Optimization: -O2 for speed (the TT benchmark tools/bench.py measured the
# unoptimized build at ~87% CPU load during record+play). OPT can be
# overridden, e.g. "make OPT=-O0" for debugging convenience.
OPT ?= -O2 -fomit-frame-pointer

# Target CPU: the project is tuned for the Atari TT (68030 + 68882 FPU);
# -m68020-60 makes GCC emit native FPU instructions instead of libgcc
# soft-float calls and picks the matching multilib (crt0/libc/libgem).
# Override with "make CPU=" for a plain-68000 ST build.
CPU ?= -m68020-60

# -MMD -MP: write header dependency files next to the objects, so that a
# changed header recompiles every file that includes it. Without this, the
# MidiShare-removal left modules compiled against the deleted msh_gcc.h
# (inlined trap #5 calls) in build/, which crashed at runtime.
CFLAGS = --sysroot=$(SDK_ROOT) -D__GEMLIB_OLDNAMES \
	-Isrc -I$(COMPAT_INCLUDE) -std=c99 -g $(OPT) $(CPU) \
	-MMD -MP \
	-x c
# crt0.o and -L must follow the multilib selected by $(CPU); the gcc
# driver resolves both when asked.
CRT0 = $(shell $(CC) --sysroot=$(SDK_ROOT) $(CPU) -print-file-name=crt0.o)
LDFLAGS = $(CPU) $(CRT0) -nostdlib -L$(dir $(CRT0)) \
	-lgem -lm -lc -lgcc

BUILD_DIR ?= build
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
	src/midikern.c

SRCS += $(RTM_BASE_SRCS)

RTM_OPT_SRCS = \
	src/a3d.c \
	src/cmi.c \
	src/cmo.c \
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
	src/puf.c \
	src/spg.c \
	src/spo.c \
	src/sps.c \
	src/syn.c

SRCS += $(RTM_OPT_SRCS)

RTM_UNUSED_SRCS = \
	src/pow.c \
	src/big.c \
	src/ed4.c 

#SRCS += $(RTM_UNUSED_SRCS)

OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean st test test-tt

all: $(TARGET) $(TARGET_DEBUG)

# 68000 / no-FPU variant (plain ST and other machines without a 68882).
# Own object directory and PRG names, so it coexists with the TT build.
# Costs ~29 percentage points more CPU than the TT hard-float build in
# the record+play benchmark (tools/bench.py); see the commit history.
ST_BUILD_DIR = build-st
st:
	$(MAKE) CPU= BUILD_DIR=$(ST_BUILD_DIR) TARGET=rt5st.prg TARGET_DEBUG=rt5strun.prg

# Run focused regressions on a plain ST, independently of the TT ROM.
test:
	$(MAKE) CPU= BUILD_DIR=$(ST_BUILD_DIR) $(ST_BUILD_DIR)/regtest.prg
	python3 tools/test_regressions.py $(ST_BUILD_DIR)/regtest.prg

# Exercise native FPU interrupt handling on the TT build as well.
test-tt:
	$(MAKE) CPU=-m68020-60 BUILD_DIR=build build/regtest.prg
	python3 tools/test_regressions.py build/regtest.prg --machine tt --timeout 240

TEST_SRCS = tests/main.c tests/global.c tests/var.c tests/puf.c tests/tra.c tests/a3d.c tests/cmi.c tests/lfo.c tests/objects.c \
	tests/cmo.c tests/dispobj.c tests/menu.c tests/help.c tests/midikern.c
TEST_OBJS = $(patsubst tests/%.c,$(BUILD_DIR)/test-%.o,$(TEST_SRCS))
TEST_APP_OBJS = $(filter-out $(BUILD_DIR)/gemain.o $(BUILD_DIR)/global.o $(BUILD_DIR)/var.o \
	$(BUILD_DIR)/puf.o $(BUILD_DIR)/tra.o $(BUILD_DIR)/a3d.o $(BUILD_DIR)/cmi.o $(BUILD_DIR)/lfo.o $(BUILD_DIR)/menu.o \
	$(BUILD_DIR)/cmo.o $(BUILD_DIR)/dispobj.o $(BUILD_DIR)/objects.o $(BUILD_DIR)/realtspc.o,$(OBJS))

$(BUILD_DIR)/regtest.prg: $(TEST_OBJS) $(TEST_APP_OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/test-%.o: tests/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(info Linking $(TARGET))
	$(CC) $^ $(LDFLAGS) -o $@

$(TARGET_DEBUG): $(TARGET)
	$(info Creating symbol-stripped debug-run copy $(TARGET_DEBUG))
	cp $(TARGET) $(TARGET_DEBUG)
	$(OBJCOPY) --discard-all $(TARGET_DEBUG)

# header dependencies recorded by -MMD
-include $(OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)

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
	rm -rf $(BUILD_DIR) $(ST_BUILD_DIR)
	rm -f $(TARGET) $(TARGET_DEBUG) rt5st.prg rt5strun.prg
endif
