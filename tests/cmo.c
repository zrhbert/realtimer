#define update_var test_cmo_update_var
#define var_get_value test_cmo_var_get_value
#define hndl_alert test_cmo_hndl_alert
#define redraw_window test_cmo_redraw_window
#define draw_dispobjs test_cmo_draw_dispobjs
#define hide_mouse test_cmo_hide_mouse
#define show_mouse test_cmo_show_mouse
#include "../src/cmo.c"

static STATUS states[2];
static SETUP setups[2], standards[2];
static EDIT actuals[2];
static RTMCLASS modules[2];
static WINDOW windows[2];
static LIST head, links[MAXINPUTS];
static DISPOBJ displays[MAXINPUTS];
static DISPOBJSTATUS display_states[MAXINPUTS];
static int loads, echoes, inject_external, alert_count, display_resets;
static LONG last_loaded;
static WORD last_alert;
static WORD cmo_channels[2] = {14, 15}, cmo_ports[2] = {0, 0};
static SHORT redraw_source;
static int redraw_inject, redraw_injected, redraw_count, incremental_count;
static int mouse_hidden, unprotected_draw;

#if defined(__HAVE_68881__)
static volatile WORD fpu_task_done;
static volatile WORD fpu_task_armed;
static volatile WORD fpu_task_early;
static volatile unsigned long fpu_task_entry_control;
static const float fpu_task_value = -17.25f;
static const float fpu_task_value2 = 9.5f;
static const unsigned long fpu_task_control = 0x30;
static const unsigned long fpu_task_status = 0x01000008;
static const unsigned long fpu_task_address = 0x87654320;
static const unsigned long fpu_expected_control = 0x20;
static const unsigned long fpu_expected_status = 0x08000008;
static const unsigned long fpu_expected_address = 0x12345678;
static unsigned long fpu_saved_frame[54], fpu_seen_frame[54];
static unsigned long fpu_saved_registers[24], fpu_saved_controls[3];
static const unsigned long fpu_null_frame = 0;

static VOID fpu_task (long date, short refNum, long a1, long a2, long a3)
{
	if (!fpu_task_armed)
	{
		fpu_task_early = TRUE;
		fpu_task_done = TRUE;
		return;
	}
	__asm__ volatile (
		"fmove.l %%fpcr,%0\n\t"
		"fmove.s %1,%%fp0\n\t"
		"fmove.s %2,%%fp1\n\t"
		"fmove.l %3,%%fpcr\n\t"
		"fmove.l %4,%%fpsr\n\t"
		"fmove.l %5,%%fpiar"
		: "=m" (fpu_task_entry_control)
		: "m" (fpu_task_value), "m" (fpu_task_value2), "m" (fpu_task_control),
		  "m" (fpu_task_status), "m" (fpu_task_address)
		: "fp0", "fp1", "memory");
	fpu_task_done = TRUE;
}

static int test_timer_fpu_state (FILE *out, SHORT source)
{
	const float expected = 123.5f;
	const float expected2 = -321.75f;
	float observed_value, observed_value2;
	unsigned long saved_control, saved_status, saved_address;
	unsigned long observed_control, observed_status, observed_address;
	LONG remaining = 10000000L;

	fpu_task_done = FALSE;
	fpu_task_armed = FALSE;
	fpu_task_early = FALSE;
	if (!MidiTask (fpu_task, MidiGetTime () + 20, source, 0, 0, 0))
	{
		fputs ("FAIL scheduling FPU interrupt regression\n", out);
		return 1;
	}
	__asm__ volatile (
		"fmove.l %%fpcr,%[saved_control]\n\t"
		"fmove.l %%fpsr,%[saved_status]\n\t"
		"fmove.l %%fpiar,%[saved_address]"
		: [saved_control] "=m" (saved_control),
		  [saved_status] "=m" (saved_status),
		  [saved_address] "=m" (saved_address)
		: : "memory");
	__asm__ volatile (
		"fmove.s %[expected],%%fp0\n\t"
		"fmove.s %[expected2],%%fp1\n\t"
		"fmove.l %[control],%%fpcr\n\t"
		"fmove.l %[status],%%fpsr\n\t"
		"fmove.l %[address],%%fpiar\n\t"
		"move.w #1,%[armed]\n\t"
		"1: tst.w %[done]\n\t"
		"bne.s 2f\n\t"
		"subq.l #1,%[remaining]\n\t"
		"bne.s 1b\n\t"
		"2: fmove.l %%fpcr,%[observed_control]\n\t"
		"fmove.l %%fpsr,%[observed_status]\n\t"
		"fmove.l %%fpiar,%[observed_address]\n\t"
		"fmove.s %%fp0,%[observed_value]\n\t"
		"fmove.s %%fp1,%[observed_value2]"
		: [remaining] "+d" (remaining),
		  [armed] "=m" (fpu_task_armed),
		  [observed_control] "=m" (observed_control),
		  [observed_status] "=m" (observed_status),
		  [observed_address] "=m" (observed_address),
		  [observed_value] "=m" (observed_value),
		  [observed_value2] "=m" (observed_value2)
		: [expected] "m" (expected), [expected2] "m" (expected2),
		  [control] "m" (fpu_expected_control),
		  [status] "m" (fpu_expected_status),
		  [address] "m" (fpu_expected_address), [done] "m" (fpu_task_done)
		: "fp0", "fp1", "cc", "memory");
	__asm__ volatile (
		"fmove.l %[saved_control],%%fpcr\n\t"
		"fmove.l %[saved_status],%%fpsr\n\t"
		"fmove.l %[saved_address],%%fpiar"
		: : [saved_control] "m" (saved_control),
		    [saved_status] "m" (saved_status),
		    [saved_address] "m" (saved_address)
		: "memory");
	if (!fpu_task_done)
	{
		fputs ("FAIL waiting for FPU interrupt regression\n", out);
		return 1;
	}
	if (fpu_task_early)
	{
		fputs ("FAIL FPU interrupt fired before the regression was ready\n", out);
		return 1;
	}
	if (observed_value != expected || observed_value2 != expected2 ||
	    observed_control != fpu_expected_control ||
	    observed_status != fpu_expected_status ||
	    observed_address != fpu_expected_address)
	{
		fprintf (out, "FAIL MIDI timer FPU state: value %s, FPCR %08lx, "
		         "FPSR %08lx, FPIAR %08lx\n",
		         observed_value == expected && observed_value2 == expected2 ? "ok" : "changed",
		         observed_control, observed_status, observed_address);
		return 1;
	}
	if (fpu_task_entry_control != 0)
	{
		fprintf (out, "FAIL MIDI timer callback inherited FPCR %08lx\n",
		         fpu_task_entry_control);
		return 1;
	}
	return 0;
}

static long test_null_fpu_supervisor (void)
{
	long remaining = 10000000L;

	__asm__ volatile (
		"fsave %[saved_frame]\n\t"
		"fmovem.x %%fp0-%%fp7,%[saved_registers]\n\t"
		"fmovem.l %%fpcr/%%fpsr/%%fpiar,%[saved_controls]\n\t"
		"frestore %[null_frame]\n\t"
		"move.w #1,%[armed]\n\t"
		"1: tst.w %[done]\n\t"
		"bne.s 2f\n\t"
		"subq.l #1,%[remaining]\n\t"
		"bne.s 1b\n\t"
		"2: fsave %[seen_frame]\n\t"
		"fmovem.x %[saved_registers],%%fp0-%%fp7\n\t"
		"fmovem.l %[saved_controls],%%fpcr/%%fpsr/%%fpiar\n\t"
		"frestore %[saved_frame]"
		: [remaining] "+d" (remaining),
		  [armed] "=m" (fpu_task_armed),
		  [saved_frame] "+m" (fpu_saved_frame),
		  [saved_registers] "+m" (fpu_saved_registers),
		  [saved_controls] "+m" (fpu_saved_controls),
		  [seen_frame] "=m" (fpu_seen_frame)
		: [null_frame] "m" (fpu_null_frame), [done] "m" (fpu_task_done)
		: "cc", "memory");
	return remaining;
}

static int test_timer_null_fpu_state (FILE *out, SHORT source)
{
	fpu_task_done = FALSE;
	fpu_task_armed = FALSE;
	fpu_task_early = FALSE;
	if (!MidiTask (fpu_task, MidiGetTime () + 20, source, 0, 0, 0))
	{
		fputs ("FAIL scheduling null FPU interrupt regression\n", out);
		return 1;
	}
	Supexec (test_null_fpu_supervisor);
	if (!fpu_task_done || fpu_task_early)
	{
		fputs ("FAIL exercising the null FPU state during a timer interrupt\n", out);
		return 1;
	}
	if (fpu_seen_frame[0] >> 24)
	{
		fprintf (out, "FAIL MIDI timer null FPU frame: %08lx\n",
		         fpu_seen_frame[0]);
		return 1;
	}
	return 0;
}
#endif

VOID test_puf_prepare_output (SHORT source, WORD channel1, WORD port1,
                              WORD channel2, WORD port2);
BOOLEAN test_puf_output (PUF_INF *current, PUF_INF *previous, BOOLEAN force);
LONG test_puf_output_value (UWORD variable);

LONG test_cmo_var_get_value (RTMCLASSP var, UWORD variable)
{
	switch (variable)
	{
		case VAR_CMI_CHANNEL1: return cmo_channels[1];
		case VAR_CMI_CHANNEL2: return cmo_channels[0];
		case VAR_CMI_PORT1: return cmo_ports[1];
		case VAR_CMI_PORT2: return cmo_ports[0];
	}
	return -1;
}

VOID test_cmo_update_var (RTMCLASSP var, UWORD variable, LONG value, BOOL resend)
{
	MSG_SET_VAR msg;

	if (variable != VAR_SET_CMO) return;
	msg.variable = variable;
	msg.value = value;
	echoes++;
	message (&modules[0], SET_VAR, &msg);
	if (inject_external)
	{
		msg.value++;
		message (&modules[0], SET_VAR, &msg);
	}
}

WORD test_cmo_hndl_alert (WORD alert_id)
{
	alert_count++;
	last_alert = alert_id;
	return 1;
}

static BOOLEAN load_probe (RTMCLASSP module, LONG number)
{
	loads++;
	last_loaded = number;
	return TRUE;
}

static BOOLEAN display_uni (DISPOBJP object, INT id, LONG value)
{
	return TRUE;
}

static BOOLEAN display_type (DISPOBJP object, INT id, INT value)
{
	return TRUE;
}

static BOOLEAN display_work (DISPOBJP object, RECT *work)
{
	return TRUE;
}

static VOID display_reset (DISPOBJP object)
{
	display_resets++;
}

static BOOLEAN send_note (SHORT source, WORD type, WORD channel, WORD port,
                         WORD pitch, WORD value)
{
	MidiEvPtr event = MidiNewEv (type);
	if (!event) return FALSE;
	Chan(event) = channel;
	Port(event) = port;
	Pitch(event) = pitch;
	Vel(event) = value;
	MidiSendIm (source, event);
	return TRUE;
}

VOID test_cmo_redraw_window (WINDOWP window, CONST RECT *area)
{
	redraw_count++;
}

VOID test_cmo_hide_mouse (VOID)
{
	mouse_hidden++;
}

VOID test_cmo_show_mouse (VOID)
{
	mouse_hidden--;
}

VOID test_cmo_draw_dispobjs (WINDOWP window)
{
	incremental_count++;
	if (!mouse_hidden) unprotected_draw++;
	if (redraw_inject)
	{
		redraw_inject = FALSE;
		redraw_injected = send_note (redraw_source, typeKeyOn, 15, 0, 108, 100);
	}
}

static int test_redraw_scheduling (FILE *out, SHORT source)
{
	int failures = 0;

	windows[0].milli = windows[1].milli = 0;
	if (!send_note (source, typeKeyOn, 15, 0, 108, 65))
	{
		fputs ("FAIL allocating CMO redraw scheduling event\n", out);
		return 1;
	}
	if (windows[0].milli != MILLI || windows[1].milli != 0)
	{
		fputs ("FAIL CMO MIDI redraw did not use the bounded interval\n", out);
		failures++;
	}

	windows[0].milli = 0;
	states[0].kneu[1][27] = FALSE;
	if (!send_note (source, typeKeyOn, 15, 0, 108, 65))
	{
		fputs ("FAIL allocating unchanged CMO MIDI event\n", out);
		return failures + 1;
	}
	if (windows[0].milli != 0 || states[0].kneu[1][27])
	{
		fputs ("FAIL unchanged CMO MIDI event scheduled a redraw\n", out);
		failures++;
	}

	windows[0].milli = MILLI;
	windows[0].count = 70;
	if (!send_note (source, typeKeyOn, 15, 0, 108, 70))
	{
		fputs ("FAIL allocating repeated CMO MIDI event\n", out);
		return failures + 1;
	}
	if (windows[0].milli != MILLI || windows[0].count != 70 ||
	    states[0].koor[1][27].koor.x != 6)
	{
		fputs ("FAIL repeated CMO MIDI event changed the pending deadline\n", out);
		failures++;
	}

	windows[0].milli = 1;
	if (!send_note (source, typeKeyOn, 15, 0, 108, 71))
	{
		fputs ("FAIL allocating urgent CMO redraw event\n", out);
		return failures + 1;
	}
	if (windows[0].milli != 1)
	{
		fputs ("FAIL CMO MIDI event delayed an urgent redraw\n", out);
		failures++;
	}

	windows[0].milli = 0;
	apply (&modules[0], NULL);
	if (windows[0].milli != 0)
	{
		fputs ("FAIL CMO apply scheduled a redraw without MIDI changes\n", out);
		failures++;
	}
	windows[0].milli = 1;
	apply (&modules[0], NULL);
	if (windows[0].milli != 1)
	{
		fputs ("FAIL CMO apply delayed an urgent redraw\n", out);
		failures++;
	}

	windows[0].milli = MILLI;
	redraw_source = source;
	redraw_count = incremental_count = redraw_injected = 0;
	mouse_hidden = unprotected_draw = 0;
	states[0].new = states[0].reset_flag = FALSE;
	redraw_inject = TRUE;
	wi_timer_mod (&windows[0]);
	if (redraw_count != 0 || incremental_count != 1 || !redraw_injected ||
	    mouse_hidden || unprotected_draw || windows[0].milli != MILLI ||
	    !states[0].kneu[1][27] || states[0].koor[1][27].koor.x != 36)
	{
		fputs ("FAIL CMO incremental redraw or update during redraw\n", out);
		failures++;
	}
	wi_start_mod (&windows[0]);
	if (display_states[27].space.position[0].x != 57 ||
	    states[0].kneu[1][27])
	{
		fputs ("FAIL CMO did not display the latest deferred position\n", out);
		failures++;
	}
	wi_timer_mod (&windows[0]);
	if (windows[0].milli != 0)
	{
		fputs ("FAIL CMO redraw timer did not stop when idle\n", out);
		failures++;
	}
	states[0].new = TRUE;
	wi_timer_mod (&windows[0]);
	if (redraw_count != 1 || incremental_count != 2)
	{
		fputs ("FAIL CMO full redraw after display invalidation\n", out);
		failures++;
	}
	states[0].new = FALSE;
	states[0].reset_flag = TRUE;
	wi_timer_mod (&windows[0]);
	windows[0].flags |= WI_JUNK;
	states[0].reset_flag = FALSE;
	wi_timer_mod (&windows[0]);
	windows[0].flags &= ~WI_JUNK;
	if (redraw_count != 3 || incremental_count != 2)
	{
		fputs ("FAIL CMO full redraw after reset or window exposure\n", out);
		failures++;
	}
	return failures;
}

static int test_puf_to_cmo (FILE *out, SHORT source, SHORT *second_ref)
{
	KOOR_ALL coordinates = {0}, previous_coordinates = {0};
	TRACK_ALL tracks = {0}, previous_tracks = {0};
	VOL_ALL volumes = {0};
	PUF_INF current = {&coordinates, &tracks, &volumes};
	PUF_INF previous = {&previous_coordinates, &previous_tracks, &volumes};
	int i, failures = 0;

	for (i = 0; i < MAXSIGNALS; i++) tracks.track[i] = -1;
	tracks.track[0] = 4;
	tracks.track[1] = 56;
	tracks.track[2] = 0;
	previous_tracks = tracks;
	coordinates.koor[0].koor.x = 33;
	coordinates.koor[1].koor.x = 19;
	coordinates.koor[1].koor.y = -14;
	coordinates.koor[1].koor.z = -4;
	coordinates.koor[1].volume = 105;
	coordinates.koor[2].koor.x = -19;
	coordinates.koor[2].koor.y = 3;
	coordinates.koor[2].koor.z = 7;
	coordinates.koor[2].volume = 90;

	test_puf_prepare_output (source, 15, 0, 14, 0);
	if (!test_puf_output (&current, &previous, TRUE))
	{
		fputs ("FAIL producing PUF output for CMO\n", out);
		return 1;
	}
	*second_ref = init_midishare (&modules[1]);
	if (*second_ref <= 0)
	{
		fputs ("FAIL registering the second CMO during playback\n", out);
		return 1;
	}
	instance_count++;
	previous_coordinates = coordinates;
	if (!test_puf_output (&current, &previous, FALSE))
	{
		fputs ("FAIL refreshing PUF output for the new CMO\n", out);
		return 1;
	}
	for (i = 0; i < 2; i++)
	{
		if (states[i].koor[0][24].koor.x != 19 ||
		    states[i].koor[0][24].koor.y != -14 ||
		    states[i].koor[0][24].koor.z != -4 ||
		    states[i].koor[0][24].volume != 105 ||
		    states[i].koor[1][0].koor.x != -19 ||
		    states[i].koor[1][0].volume != 90 ||
		    states[i].koor[1][4].koor.x != 0 ||
		    windows[i].milli != MILLI)
		{
			fputs ("FAIL PUF positions routed to both CMO instances\n", out);
			failures++;
			break;
		}
	}
	if (test_puf_output_value (VAR_PUF_KOORX0) != 33 ||
	    test_puf_output_value (VAR_PUF_KOORX0 + 1) != 19 ||
	    test_puf_output_value (VAR_PUF_KOORY0 + 1) != -14)
	{
		fputs ("FAIL PUF position variables for KOO\n", out);
		failures++;
	}

	states[1].new = TRUE;
	wi_start_mod (&windows[1]);
	wi_finished_mod (&windows[1]);
	if (display_states[24].space.position[0].x != 30 ||
	    display_states[24].space.position[0].y != -22 ||
	    strstr ((char *)windows[1].name, "CM 2 (33-64)") == NULL)
	{
		fputs ("FAIL CMO system 2 displayed the recorded input\n", out);
		failures++;
	}

	coordinates.koor[1].koor.x = -32;
	coordinates.koor[1].koor.y = 16;
	if (!test_puf_output (&current, &previous, FALSE))
	{
		fputs ("FAIL producing updated PUF output for CMO\n", out);
		return failures + 1;
	}
	wi_start_mod (&windows[1]);
	if (display_states[24].space.position[0].x != -50 ||
	    display_states[24].space.position[0].y != 25 ||
	    test_puf_output_value (VAR_PUF_KOORX0 + 1) != -32 ||
	    test_puf_output_value (VAR_PUF_KOORY0 + 1) != 16)
	{
		fputs ("FAIL updated PUF positions reached KOO and CMO\n", out);
		failures++;
	}
	return failures;
}

int test_cmo (FILE *out)
{
	MSG_SET_VAR msg;
	SHORT source, refs[2] = {0, 0}, extra[16], failed;
	int i, count = 0, failures = 0;

	for (i = 0; i < 2; i++)
	{
		modules[i].status = &states[i];
		modules[i].actual = &actuals[i];
		modules[i].window = &windows[i];
		modules[i].standard = &standards[i];
		modules[i].reset = reset;
		modules[i].set_setnr = load_probe;
		actuals[i].setup = &setups[i];
		windows[i].module = &modules[i];
		sprintf ((char *)modules[i].object_name, "CMO_%d", i + 1);
		init_standard (&modules[i], i);
		setups[i] = standards[i];
	}
	if (setups[0].midi_channel != 16 || setups[1].midi_channel != 15)
	{
		fputs ("FAIL CMO windows did not default to different systems\n", out);
		failures++;
	}

	head.next = &links[0];
	for (i = 0; i < MAXINPUTS; i++)
	{
		links[i].next = i + 1 < MAXINPUTS ? &links[i + 1] : &head;
		links[i].key = &displays[i];
		displays[i].status = &display_states[i];
		displays[i].set_uni = display_uni;
		displays[i].set_type = display_type;
		displays[i].set_work = display_work;
		displays[i].reset = display_reset;
	}
	for (i = 0; i < 2; i++)
	{
		windows[i].dispobjs = &head;
		states[i].num_spaces = MAXINPUTS;
	}

	if (!MidiInternalStart ())
	{
		fputs ("FAIL starting MIDI kernel for CMO regression\n", out);
		return 1;
	}
	msh_available = TRUE;
	source = MidiOpen ("PUF");
	if (source > 0) try_all_connect (source);
	refs[0] = init_midishare (&modules[0]);
	if (refs[0] > 0) instance_count++;
	if (source <= 0 || refs[0] <= 0)
	{
		fputs ("FAIL registering MIDI clients for CMO regression\n", out);
		failures++;
		goto cleanup;
	}

#if defined(__HAVE_68881__)
	failures += test_timer_fpu_state (out, source);
	failures += test_timer_null_fpu_state (out, source);
#endif
	failures += test_puf_to_cmo (out, source, &refs[1]);
	if (refs[1] <= 0) goto cleanup;
	failures += test_redraw_scheduling (out, source);
	for (i = 0; i < 2; i++)
	{
		mem_set (states[i].koor, 0, (UWORD)sizeof (states[i].koor));
		mem_set (states[i].kneu, 0, (UWORD)sizeof (states[i].kneu));
		states[i].new = FALSE;
		states[i].reset_flag = FALSE;
		windows[i].milli = 0;
	}
	mem_lset (display_states, 0, sizeof (display_states));

	if (!send_note (source, typeKeyOn, 15, 0, 124, 127) ||
	    !send_note (source, typeKeyOn, 15, 0, 125, 0) ||
	    !send_note (source, typeNote, 15, 0, 126, 96) ||
	    !send_note (source, typeKeyOn, 15, 0, 127, 120))
	{
		fputs ("FAIL allocating CMO MIDI events\n", out);
		failures++;
		goto cleanup;
	}
	for (i = 0; i < 2; i++)
	{
		if (states[i].koor[1][31].koor.x != 63 ||
		    states[i].koor[1][31].koor.y != -64 ||
		    states[i].koor[1][31].koor.z != 32 ||
		    states[i].koor[1][31].volume != 120)
		{
			fputs ("FAIL MIDI coordinates did not reach both CMO instances\n", out);
			failures++;
			break;
		}
	}
	if (windows[0].milli != MILLI || windows[1].milli != 0)
	{
		fputs ("FAIL CMO scheduled redraw for the wrong channel\n", out);
		failures++;
	}

	windows[0].milli = windows[1].milli = 0;
	if (!send_note (source, typeKeyOn, 14, 0, 0, 17) ||
	    !send_note (source, typeKeyOn, 13, 0, 0, 127))
	{
		fputs ("FAIL allocating channel-selection events\n", out);
		failures++;
		goto cleanup;
	}
	if (states[0].koor[0][0].koor.x != -47 ||
	    states[1].koor[0][0].koor.x != -47 ||
	    windows[0].milli != 0 || windows[1].milli != MILLI)
	{
		fputs ("FAIL CMO inactive-channel cache or MIDI channel filter\n", out);
		failures++;
	}

	windows[0].milli = windows[1].milli = 0;
	cmo_channels[1] = 2;
	cmo_ports[1] = 4;
	if (!send_note (source, typeKeyOn, 2, 4, 20, 127) ||
	    !send_note (source, typeKeyOn, 2, 0, 20, 0) ||
	    !send_note (source, typeKeyOn, 15, 0, 20, 0))
	{
		fputs ("FAIL allocating reconfigured CMI events\n", out);
		failures++;
		goto cleanup;
	}
	if (states[0].koor[1][5].koor.x != 63 ||
	    states[1].koor[1][5].koor.x != 63 ||
	    windows[0].milli != MILLI || windows[1].milli != 0)
	{
		fputs ("FAIL CMO followed reconfigured CMI channel and port\n", out);
		failures++;
	}

	windows[0].milli = windows[1].milli = 0;
	cmo_channels[0] = cmo_channels[1];
	cmo_ports[0] = cmo_ports[1];
	if (!send_note (source, typeKeyOn, 2, 4, 40, 99))
	{
		fputs ("FAIL allocating shared-channel CMI event\n", out);
		failures++;
		goto cleanup;
	}
	for (i = 0; i < 2; i++)
	{
		if (states[i].koor[0][10].koor.x != 35 ||
		    states[i].koor[1][10].koor.x != 35 || windows[i].milli != MILLI)
		{
			fputs ("FAIL CMO handled indistinguishable CMI systems\n", out);
			failures++;
			break;
		}
	}

	wi_start_mod (&windows[0]);
	if (display_states[31].space.position[0].x != 100 ||
	    display_states[31].space.position[0].y != -101 ||
	    display_states[31].space.position[0].z != 50 ||
	    display_states[31].space.volume[0] != 120 ||
	    states[0].kneu[1][31])
	{
		fputs ("FAIL CMO display scaling or consumed update\n", out);
		failures++;
	}
	wi_finished_mod (&windows[0]);
	handle_menu_mod (&windows[0], MCMOOPTIONS, MCMOCHANNEL15);
	wi_start_mod (&windows[0]);
	if (setups[0].midi_channel != 15 ||
	    display_states[0].space.position[0].x != -74 ||
	    strstr ((char *)windows[0].name, "CM 2 (33-64)") == NULL ||
	    states[0].kneu[0][0] || windows[0].milli != 1)
	{
		fputs ("FAIL switching CMO views to cached coordinates\n", out);
		failures++;
	}

	reset (&modules[0]);
	wi_start_mod (&windows[0]);
	if (states[0].koor[0][0].koor.x != 0 ||
	    states[0].koor[1][31].koor.x != 0 ||
	    display_states[0].space.position[0].x != 0 ||
	    states[0].reset_flag || display_resets != MAXINPUTS)
	{
		fputs ("FAIL CMO reset or display invalidation\n", out);
		failures++;
	}

	actuals[0].number = 7;
	send_messages (&modules[0]);
	if (echoes != 1 || loads != 0)
	{
		fputs ("FAIL CMO reloaded its own setup update\n", out);
		failures++;
	}
	msg.variable = VAR_SET_CMO;
	msg.value = 7;
	message (&modules[0], SET_VAR, &msg);
	msg.value = 8;
	message (&modules[0], SET_VAR, &msg);
	if (loads != 2 || last_loaded != 8)
	{
		fputs ("FAIL external or forced CMO setup update\n", out);
		failures++;
	}
	loads = 0;
	inject_external = TRUE;
	send_messages (&modules[0]);
	if (loads != 1 || last_loaded != 8)
	{
		fputs ("FAIL external CMO update during setup publication\n", out);
		failures++;
	}

	MidiClose (refs[1]);
	refs[1] = 0;
	while (count < 16 && (failed = MidiOpen ("EXTRA")) > 0)
		extra[count++] = failed;
	instance_count = 1;
	failed = init_midishare (&modules[1]);
	if (failed != MIDIerrSpace || !alert_count || last_alert != ERR_MIDISHAREFULL)
	{
		fputs ("FAIL CMO registration when MIDI clients are full\n", out);
		failures++;
	}
	for (i = 0; i < count; i++) MidiClose (extra[i]);

cleanup:
	for (i = 0; i < 2; i++)
		if (refs[i] > 0) MidiClose (refs[i]);
	if (source > 0) MidiClose (source);
	MidiInternalStop ();
	msh_available = FALSE;
	puf_module = NULL;
	return failures;
}
