ifdef OS
	WINDOWS = 1
endif

SDK ?= /Users/bertramd/.vscode/extensions/dgis.atari-st-dev-0.2.1/sdk/darwin
TOOLCHAIN = $(SDK)/opt/cross-mint
CC = $(TOOLCHAIN)/bin/m68k-atari-mintelf-gcc
SDK_ROOT = $(TOOLCHAIN)/m68k-atari-mintelf/sys-root
SDK_USR = $(SDK_ROOT)/usr
COMPAT_INCLUDE ?= gcc/include
GEM_INCLUDE ?= PUREC/INCLUDE

CFLAGS = --sysroot=$(SDK_ROOT) -D__GEMLIB_OLDNAMES \
	-ISRC -I$(COMPAT_INCLUDE) -idirafter $(GEM_INCLUDE) -std=c99 -g \
	-Wno-incompatible-pointer-types -x c
LDFLAGS = $(SDK_USR)/lib/crt0.o -nostdlib -L$(SDK_USR)/lib \
	-lgem -lm -lc -lgcc

TARGET = realtim5gcc.app

SRCS = \
	SRC/DESKTOP.C SRC/DIALOG.C SRC/DISK.C SRC/EVENT.C SRC/GEMAIN.C \
	SRC/GLOBAL.C SRC/INITERM.C SRC/MENU.C SRC/RCM.C SRC/RESOURCE.C \
	SRC/WINDOWS.C SRC/XRSRC.C \
	SRC/REALTSPC.C SRC/LISTS.C SRC/OBJECTS.C SRC/init_rtm.c \
	SRC/DISPOBJ.C SRC/a3d.c SRC/CMI.C SRC/CMO.C SRC/GEN.C SRC/GMI.C \
	SRC/KOO.C SRC/LFO.C SRC/MAE.C SRC/MAN.C SRC/MSH.C SRC/MTR.C \
	SRC/PAR.C SRC/PUF.C SRC/SPG.C SRC/SPO.C SRC/SPS.C SRC/SYN.C \
	SRC/TRA.C SRC/VAR.C

OBJS = $(SRCS:.C=.o)
OBJS := $(OBJS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(info Linking $(TARGET))
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.C
	$(info Compiling $<)
	$(CC) $(CFLAGS) -c $< -o $@

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
