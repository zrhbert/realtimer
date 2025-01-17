CC=/opt/cross-mint/bin/m68k-atari-mint-gcc
CFLAGS=-I. -ansi -Wno-nonportable-include-path -DDISABLE_CDECL -Wno-pointer-sign -Wno-deprecated-non-prototype -Wno-strict-prototypes -Wno-unused-variable
DEPS =

BASEOBJS = \
desktop.o \
dialog.o \
disk.o \
event.o \
gemain.o \
global.o \
initerm.o \
menu.o \
rcm.o \
resource.o \
windows.o \
xrsrc.o

RTMOBJS = \
realtspc.o \
lists.o \
objects.o \
init_rtm.o \
dispobj.o \
a3d.o \
cmi.o \
cmo.o \
gen.o \
gmi.o \
koo.o \
lfo.o \
mae.o \
man.o \
msh.o \
mtr.o \
par.o \
puf.o \
spg.o \
spo.o \
sps.o \
syn.o \
tra.o \
var.o


LIBS = \
vdicall.o \
libmidi.lib \
pcfltlib.lib \
pcstdlib.lib \
pcextlib.lib \
pctoslib.lib \
pcgemlib.lib
       
%.o: %.c $(DEPS)
	$(CC) -c -o gcc68k/$@ $< $(CFLAGS)

realtim5gcc.app: $(BASEOBJS) $(RTMOBJS) $(LIBS)
	$(CC)  -o realtim5gcc.app $(BASEOBJS) $(RTMOBJS) $(LIBS)
	
