#include <stdio.h>
#include <mint/osbind.h>

#include "../src/import.h"
#include "../src/msh_unit.h"

#define IKBD_STATUS (*(volatile unsigned char *)0xfffffc00UL)
#define IKBD_DATA   (*(volatile unsigned char *)0xfffffc02UL)
#define IKBD_TX_READY 0x02
#define IKBD_READ_CLOCK 0x1c

extern volatile unsigned short mk_txw, mk_txr;
extern volatile short mk_timer_slow;

volatile short test_keyboard_reply_seen;
static volatile short keyboard_command_sent, reply_during_task, task_done, task_aborted;
static volatile long timed_task_date, timed_task_now;
static volatile short timed_task_done;
static volatile short repeat_task_count, repeat_task_done;
static volatile short repeat_task_slow, repeat_task_failed;

static long read_hz200 (void)
{
	return *(volatile long *)0x4baL;
}

static void timed_task (long date, short refNum, long a1, long a2, long a3)
{
	timed_task_date = date;
	timed_task_now = MidiGetTime ();
	timed_task_done = TRUE;
}

static void repeating_task (long date, short refNum, long a1, long a2, long a3)
{
	if (repeat_task_count == 0)
	{
		timed_task_date = date;
		timed_task_now = MidiGetTime ();
	} /* if */
	repeat_task_count++;
	if (mk_timer_slow) repeat_task_slow = TRUE;
	if (repeat_task_count < 3)
	{
		if (!MidiTask (repeating_task, MidiGetTime () + 50, refNum, 0, 0, 0))
		{
			repeat_task_failed = TRUE;
			repeat_task_done = TRUE;
		} /* if */
	} /* if */
	else
		repeat_task_done = TRUE;
} /* repeating_task */

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

	/* The Timer A fast path must keep the clock and task dates exact. */
	{
		long start_ms, start_hz, elapsed_ms, elapsed_hz;
		long idle_start_hz, task_start_ms, last_ms;
		short idle_ticks;

		timed_task_done = FALSE;
		start_hz = Supexec (read_hz200);
		start_ms = MidiGetTime ();
		if (!MidiTask (timed_task, start_ms + 100, source, 0, 0, 0))
		{
			fputs ("FAIL scheduling MIDI timing regression\n", out);
			failures++;
		}
		else
		{
			for (remaining = 10000000L; remaining > 0 && !timed_task_done;
			     remaining--) { }
			if (!timed_task_done || timed_task_date != start_ms + 100
			    || timed_task_now < timed_task_date
			    || timed_task_now > timed_task_date + 1)
			{
				fprintf (out, "FAIL MIDI task timing: done=%d date=%ld "
				         "expected=%ld ran at=%ld\n", timed_task_done,
				         timed_task_date, start_ms + 100, timed_task_now);
				failures++;
			}
		}

		/* Once tasks are exhausted, the idle timer should tick at ~100 Hz. */
		idle_start_hz = Supexec (read_hz200);
		last_ms = MidiGetTime ();
		idle_ticks = 0;
		while (Supexec (read_hz200) - idle_start_hz < 40)
		{
			long now = MidiGetTime ();
			if (now != last_ms)
			{
				idle_ticks++;
				last_ms = now;
			} /* if */
		} /* while */
		if (idle_ticks < 8 || idle_ticks > 30)
		{
			fprintf (out, "FAIL idle MIDI timer rate: %d ticks in 200 ms\n",
			         idle_ticks);
			failures++;
		}

		/* Scheduling from idle restores 1 kHz and stays there while tasks recur. */
		task_start_ms = MidiGetTime ();
		repeat_task_count = repeat_task_done = FALSE;
		repeat_task_slow = repeat_task_failed = FALSE;
		if (!MidiTask (repeating_task, task_start_ms + 50, source, 0, 0, 0))
		{
			fputs ("FAIL scheduling task after idle MIDI timer\n", out);
			failures++;
		}
		else
		{
			for (remaining = 10000000L; remaining > 0 && !repeat_task_done;
			     remaining--) { }
			if (!repeat_task_done || repeat_task_count != 3 || repeat_task_failed
			    || repeat_task_slow || timed_task_date != task_start_ms + 50
			    || timed_task_now < timed_task_date
			    || timed_task_now > timed_task_date + 1)
			{
				fprintf (out, "FAIL fast timer restore: count=%d done=%d "
				         "slow=%d failed=%d date=%ld ran at=%ld\n",
				         repeat_task_count, repeat_task_done,
				         repeat_task_slow, repeat_task_failed,
				         timed_task_date, timed_task_now);
				failures++;
			}
		}

		while (Supexec (read_hz200) - start_hz < 200) { }
		elapsed_hz = (Supexec (read_hz200) - start_hz) * 5;
		elapsed_ms = MidiGetTime () - start_ms;
		if (elapsed_ms < elapsed_hz - 20 || elapsed_ms > elapsed_hz + 20)
		{
			fprintf (out, "FAIL MIDI clock drift: %ld ms vs %ld ms system\n",
			         elapsed_ms, elapsed_hz);
			failures++;
		}
	}

	/* A three-byte MIDI message must drain through ACIA transmit interrupts. */
	{
		MidiEvPtr ev;
		long tx_start_hz;

		MidiConnect (source, 0, TRUE);
		ev = MidiNewEv (typeCtrlChange);
		if (!ev)
		{
			fputs ("FAIL allocating MIDI transmit regression event\n", out);
			failures++;
		}
		else
		{
			MidiSetField (ev, 0, 7);
			MidiSetField (ev, 1, 100);
			MidiSendIm (source, ev);
			tx_start_hz = Supexec (read_hz200);
			while (mk_txr != mk_txw &&
			       Supexec (read_hz200) - tx_start_hz < 40) { }
			if (mk_txr != mk_txw)
			{
				fputs ("FAIL MIDI ACIA transmit interrupt did not drain queue\n", out);
				failures++;
			}
		}
		MidiConnect (source, 0, FALSE);
	}

	task_aborted = TRUE;
	MidiClose (source);
	MidiInternalStop ();
	vectors->clockvec = old_clockvec;
	return failures;
}
