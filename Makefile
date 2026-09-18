ifdef OS
	WINDOWS = 1
endif

SDK ?= /Users/bertramd/.vscode/extensions/dgis.atari-st-dev-0.2.1/sdk/darwin
TOOLCHAIN = $(SDK)/opt/cross-mint
CC = $(TOOLCHAIN)/bin/m68k-atari-mintelf-gcc
SDK_ROOT = $(TOOLCHAIN)/m68k-atari-mintelf/sys-root
SDK_USR = $(SDK_ROOT)/usr
COMPAT_INCLUDE ?= gcc/include

CFLAGS = --sysroot=$(SDK_ROOT) -D__GEMLIB_OLDNAMES \
	-Isrc -I$(COMPAT_INCLUDE) -std=c99 -g \
	-Wno-incompatible-pointer-types -x c
LDFLAGS = $(SDK_USR)/lib/crt0.o -nostdlib -L$(SDK_USR)/lib \
	-lgem -lm -lc -lgcc

TARGET = src/realtim5.prg

SRCS = \
	src/desktop.c src/dialog.c src/disk.c src/event.c src/gemain.c \
	src/global.c src/initerm.c src/menu.c src/rcm.c src/resource.c \
	src/windows.c src/xrsrc.c \
	src/realtspc.c src/lists.c src/objects.c src/init_rtm.c \
	src/dispobj.c src/a3d.c src/cmi.c src/cmo.c src/gen.c src/gmi.c \
	src/koo.c src/lfo.c src/mae.c src/man.c src/msh.c src/mtr.c \
	src/par.c src/puf.c src/spg.c src/spo.c src/sps.c src/syn.c \
	src/tra.c src/var.c src/midishare_stub.c

OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(info Linking $(TARGET))
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(info Compiling $<)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(info Cleaning...)
ifdef WINDOWS
	@del /q $(OBJS)
else
	rm -f $(OBJS) $(TARGET)
endif
