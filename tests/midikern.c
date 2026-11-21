#include <stdio.h>
#include <mint/osbind.h>

#include "../src/import.h"
#include "../src/msh_unit.h"

#define IKBD_STATUS (*(volatile unsigned char *)0xfffffc00UL)
#define IKBD_DATA   (*(volatile unsigned char *)0xfffffc02UL)
#define IKBD_TX_READY 0x02
#define IKBD_READ_CLOCK 0x1c

volatile short test_keyboard_reply_seen;
static volatile short keyboard_command_sent, reply_during_task, task_done, task_aborted;

__asm__ (
	".text\n\t.even\n\t"
	".globl test_keyboard_clockvec\n"
	"test_keyboard_clockvec:\n\t"
	"move.w #1,test_keyboard_reply_seen\n\t"
	"rts\n");
extern void test_keyboard_clockvec (void *);

static void keyboard_interrupt_task (long date, short refNum,
                                     long a1, long a2, long a3)
{
	long remaining;

	if (task_aborted) return;
	for (remaining = 200000L; remaining > 0 && !(IKBD_STATUS & IKBD_TX_READY);
	     remaining--) { }
	if (remaining > 0)
	{
		/* The IKBD clock reply uses the same interrupt as mouse packets. */
		IKBD_DATA = IKBD_READ_CLOCK;
		keyboard_command_sent = TRUE;
		for (remaining = 200000L; remaining > 0 && !test_keyboard_reply_seen;
		     remaining--) { }
	}
	reply_during_task = test_keyboard_reply_seen;
	task_done = TRUE;
}

int test_midikern (FILE *out)
{
	_KBDVECS *vectors;
	void (*old_clockvec) (void *);
	short source;
	long remaining, tick;
	int failures = 0;

	if (!MidiInternalStart ())
	{
		fputs ("FAIL starting MIDI kernel for interrupt regression\n", out);
		return 1;
	}
	source = MidiOpen ("IRQ test");
	if (source <= 0)
	{
		fputs ("FAIL registering MIDI interrupt regression\n", out);
		MidiInternalStop ();
		return 1;
	}
	vectors = Kbdvbase ();
	old_clockvec = vectors->clockvec;
	test_keyboard_reply_seen = keyboard_command_sent = reply_during_task = FALSE;
	task_done = task_aborted = FALSE;
	vectors->clockvec = test_keyboard_clockvec;

	if (!MidiTask (keyboard_interrupt_task, MidiGetTime () + 50, source, 0, 0, 0))
	{
		fputs ("FAIL scheduling MIDI interrupt regression\n", out);
		failures++;
	}
	else
	{
		for (remaining = 10000000L; remaining > 0 && !task_done; remaining--) { }
		if (!task_done || !keyboard_command_sent || !reply_during_task)
		{
			fprintf (out, "FAIL keyboard interrupt during MIDI task: "
			         "task=%d command=%d reply=%d\n",
			         task_done, keyboard_command_sent, reply_during_task);
			failures++;
		}
		if (task_done)
		{
			tick = MidiGetTime ();
			for (remaining = 10000000L;
			     remaining > 0 && MidiGetTime () == tick; remaining--) { }
			if (!remaining)
			{
				fputs ("FAIL MIDI timer did not resume after callback\n", out);
				failures++;
			}
		}
	}

	task_aborted = TRUE;
	MidiClose (source);
	MidiInternalStop ();
	vectors->clockvec = old_clockvec;
	return failures;
}
