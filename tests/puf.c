#include "../src/puf.c"

static STATUS state;
static SETUP setup;
static EDIT actual;
static RTMCLASS module;
static WINDOW window;
static PUFEVENT head, events[5];
static int resets;

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

	return failures;
}
