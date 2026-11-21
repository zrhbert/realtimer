#define update_var test_cmi_update_var
#include "../src/cmi.c"

static STATUS state;
static SETUP setup;
static EDIT actual;
static RTMCLASS module;
static int loads, echo_count, inject_external;
static LONG last_loaded;

static BOOLEAN load_probe (RTMCLASSP module, LONG number)
{
	loads++;
	last_loaded = number;
	return TRUE;
}

VOID test_cmi_update_var (RTMCLASSP var, UWORD variable, LONG value, BOOL resend)
{
	MSG_SET_VAR msg;

	if (variable != VAR_SET_CMI) return;
	msg.variable = variable;
	msg.value = value;
	echo_count++;
	message (&module, SET_VAR, &msg);
	if (inject_external)
	{
		msg.value++;
		message (&module, SET_VAR, &msg);
	}
}

int test_cmi (FILE *out)
{
	MSG_SET_VAR msg;
	int failures = 0;

	module.status = &state;
	module.actual = &actual;
	module.set_setnr = load_probe;
	actual.setup = &setup;
	actual.number = 7;

	send_messages (&module);
	if (echo_count != 1 || loads != 0)
	{
		fputs ("FAIL CMI reloaded its own setup update\n", out);
		failures++;
	}

	msg.variable = VAR_SET_CMI;
	msg.value = 7;
	message (&module, SET_VAR, &msg);
	msg.value = 8;
	message (&module, SET_VAR, &msg);
	if (loads != 2 || last_loaded != 8)
	{
		fputs ("FAIL external or forced same-value CMI setup update\n", out);
		failures++;
	}

	loads = 0;
	inject_external = TRUE;
	send_messages (&module);
	if (loads != 1 || last_loaded != 8)
	{
		fputs ("FAIL external CMI update during setup publication\n", out);
		failures++;
	}
	return failures;
}
