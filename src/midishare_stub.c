/*
 * MidiShare stub.
 *
 * The original MidiShare client connector (LIBMIDI.LIB / MIDSHARE.O) is a
 * relocatable object in an old DRI/GST format that the modern
 * m68k-atari-mintelf-gcc/ELF toolchain cannot link directly. Reliably
 * reverse-engineering its exact runtime handshake (it pokes at a low-memory
 * CPU vector and installs a "trap #6" trampoline used to talk to the real
 * MidiShare driver/ACC once loaded) was judged too risky to reimplement
 * blindly.
 *
 * All call sites in realtimer already guard their MIDI usage with
 * "if (MidiShare()) { ... }", so this stub simply reports "MidiShare driver
 * not present", which disables the MIDI features without affecting the rest
 * of the application. Replace this with a real MidiShare integration when
 * one becomes available for the modern toolchain.
 */

#include "import.h"
#include "msh_unit.h"

/* Under GNU_C, msh_unit.h declares "extern unsigned long micro_rtx();" (an
 * old-style/K&R function, not a function-pointer variable), so it must be
 * defined here as a real function. It should never actually be called since
 * every call site checks MidiShare() first. */
unsigned long micro_rtx (number)
int number;
{
	number = number;
	return 0L;
} /* micro_rtx */

Boolean MidiShare (void)
{
	return FALSE;
} /* MidiShare */

/* Reads back a MIDI-routing config file written by the external MIDISAVE.PRG
 * helper; only ever invoked after that helper is launched via Pexec(), which
 * never happens on this toolchain. Declared/called implicitly (no
 * prototype) in MSH.C, hence the plain K&R-style definition here. */
int TCMidiRestore ()
{
	return 0;
} /* TCMidiRestore */
