#define mem_avail test_puf_mem_avail
#define mem_free test_puf_mem_free
#define daktstatus test_puf_daktstatus
#define set_daktstat test_puf_set_daktstat
#define close_daktstat test_puf_close_daktstat
#define destroy_obj test_puf_destroy_obj
#define var_get_value test_puf_var_get_value
#define update_var test_puf_update_var
#include "../src/puf.c"

static STATUS state;
static SETUP setup;
static EDIT actual;
static RTMCLASS module;
static WINDOW window;
static PUFEVENT head, events[5];
static LONG output_variables[MAXSYSVARS];
static int resets, freed_buffers, freed_locator, destroyed;

LONG test_puf_var_get_value (RTMCLASSP var, UWORD variable)
{
	return output_variables[variable];
}

VOID test_puf_update_var (RTMCLASSP var, UWORD variable, LONG value, BOOL resend)
{
	output_variables[variable] = value;
}

VOID test_puf_prepare_output (SHORT source, WORD channel1, WORD port1,
                              WORD channel2, WORD port2)
{
	mem_set (&state, 0, (UWORD)sizeof (state));
	mem_set (output_variables, 0, (UWORD)sizeof (output_variables));
	module.status = &state;
	module.special = source;
	puf_module = &module;
	state.new = TRUE;
	output_variables[VAR_CMI_CHANNEL1] = channel1;
	output_variables[VAR_CMI_PORT1] = port1;
	output_variables[VAR_CMI_CHANNEL2] = channel2;
	output_variables[VAR_CMI_PORT2] = port2;
}

BOOLEAN test_puf_output (PUF_INF *current, PUF_INF *previous, BOOLEAN force)
{
	return midi_out_puf (&module, current, previous, force);
}

LONG test_puf_output_value (UWORD variable)
{
	return output_variables[variable];
}

LONG test_puf_mem_avail (VOID)
{
	return 1000;
}

VOID test_puf_mem_free (VOID *ptr)
{
	if (ptr == state.locator)
	{
		freed_locator++;
		return;
	}
	if (ptr == state.events_p || ptr == state.koors_p ||
	    ptr == state.tracks_p || ptr == state.vols_p)
		freed_buffers++;
	free (ptr);
}

VOID test_puf_daktstatus (const BYTE *title, const BYTE *text) {}
VOID test_puf_set_daktstat (WORD percent) {}
VOID test_puf_close_daktstat (VOID) {}
VOID test_puf_destroy_obj (RTMCLASSP module) { destroyed++; }

static VOID reset_probe (RTMCLASSP module)
{
	resets++;
}

int test_puf (FILE *out)
{
	union
	{
		MSG_RTM_POSIT posit;
		MSG_SET_VAR variable;
	} msg = {0};
	int i, failures = 0;

	module.status = &state;
	module.actual = &actual;
	module.window = &window;
	module.reset = reset_probe;
	actual.setup = &setup;
	state.header = &head;
	state.max_events = 5;
	head.next = &events[0];
	head.prev = &events[4];
	for (i = 0; i < 4; i++) events[i].next = &events[i + 1];
	events[4].next = &head;

	state.sync = TRUE;
	msg.posit.posit = 250;
	message (&module, RTM_POSIT, &msg);
	if (state.posit != 250 || state.locator[0] != &events[1] ||
	    !state.new || window.milli != 1 || resets != 1)
	{
		fputs ("FAIL synchronized PUF seek while stopped\n", out);
		failures++;
	}

	state.play = TRUE;
	msg.posit.posit = 500;
	message (&module, RTM_POSIT, &msg);
	if (state.posit != 500 || state.locator[0] != &events[3] ||
	    !state.play || resets != 2)
	{
		fputs ("FAIL synchronized PUF forward seek while playing\n", out);
		failures++;
	}
	msg.posit.posit = 125;
	message (&module, RTM_POSIT, &msg);
	if (state.posit != 125 || state.locator[0] != &events[0] ||
	    !state.play || resets != 3)
	{
		fputs ("FAIL synchronized PUF reverse seek while playing\n", out);
		failures++;
	}

	state.sync = FALSE;
	state.play = FALSE;
	msg.posit.posit = 375;
	message (&module, RTM_POSIT, &msg);
	message (&module, RTM_CONT, &msg);
	if (state.posit != 125 || state.locator[0] != &events[0] ||
	    state.play || resets != 3)
	{
		fputs ("FAIL unsynchronized PUF ignored transport position\n", out);
		failures++;
	}

	state.sync = TRUE;
	message (&module, RTM_CONT, &msg);
	if (state.posit != 375 || state.locator[0] != &events[2] || !state.play)
	{
		fputs ("FAIL synchronized PUF continued at transport position\n", out);
		failures++;
	}

	mem_set (&state, 0, (UWORD)sizeof (state));
	init_events (&module);
	for (i = 0; i < 10; i++)
	{
		if (state.locator[i] != state.header)
		{
			fputs ("FAIL PUF locator initialization\n", out);
			failures++;
			break;
		}
	}
	msh_available = FALSE;
	destroy_mod (&module);
	if (freed_locator || freed_buffers != 4 || destroyed != 1)
	{
		fputs ("FAIL PUF buffer ownership during shutdown\n", out);
		failures++;
	}

	return failures;
}
