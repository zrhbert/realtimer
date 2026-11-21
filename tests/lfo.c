#define update_var test_lfo_update_var
#include "../src/lfo.c"

static STATUS states[2];
static EDIT actuals[2];
static RTMCLASS modules[2];
static int loads[2], echoes, deliver_peer, inject_external, nest_publication;
static LONG last_loaded[2];

static BOOLEAN load_probe (RTMCLASSP module, LONG number)
{
	int index = module == &modules[0] ? 0 : 1;
	loads[index]++;
	last_loaded[index] = number;
	return TRUE;
}

VOID test_lfo_update_var (RTMCLASSP var, UWORD variable, LONG value, BOOL resend)
{
	MSG_SET_VAR msg;

	if (variable != VAR_SET_LFA) return;
	msg.variable = variable;
	msg.value = value;
	echoes++;
	message (&modules[nest_publication == 2 ? 1 : 0], SET_VAR, &msg);
	if (deliver_peer) message (&modules[1], SET_VAR, &msg);
	if (inject_external)
	{
		msg.value++;
		message (&modules[0], SET_VAR, &msg);
	}
	if (nest_publication == 1)
	{
		nest_publication = 2;
		send_messages (&modules[1]);
		nest_publication = 0;
		message (&modules[0], SET_VAR, &msg);
	}
}

int test_lfo (FILE *out)
{
	MSG_SET_VAR msg;
	int failures = 0, i;

	for (i = 0; i < 2; i++)
	{
		modules[i].status = &states[i];
		modules[i].actual = &actuals[i];
		modules[i].set_setnr = load_probe;
		actuals[i].number = 7;
	}

	deliver_peer = TRUE;
	send_messages (&modules[0]);
	deliver_peer = FALSE;
	if (echoes != 1 || loads[0] != 0 || loads[1] != 1 || last_loaded[1] != 7)
	{
		fputs ("FAIL LFO setup echo or update to another instance\n", out);
		failures++;
	}

	msg.variable = VAR_SET_LFA;
	msg.value = 7;
	message (&modules[0], SET_VAR, &msg);
	msg.value = 8;
	message (&modules[0], SET_VAR, &msg);
	if (loads[0] != 2 || last_loaded[0] != 8)
	{
		fputs ("FAIL external or forced same-value LFO setup update\n", out);
		failures++;
	}

	loads[0] = 0;
	inject_external = TRUE;
	send_messages (&modules[0]);
	inject_external = FALSE;
	if (loads[0] != 1 || last_loaded[0] != 8)
	{
		fputs ("FAIL external LFO update during setup publication\n", out);
		failures++;
	}

	loads[0] = loads[1] = 0;
	nest_publication = 1;
	send_messages (&modules[0]);
	if (loads[0] != 0 || loads[1] != 0)
	{
		fputs ("FAIL nested LFO setup publications\n", out);
		failures++;
	}
	return failures;
}
