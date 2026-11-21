#include "../src/var.c"

static STATUS state;
static RTMCLASS vars, first, second, alias;
static LIST head, first_link, second_link, alias_head, alias_link;
static int reset_enabled, first_true, first_false, second_true, second_false;
static int alias_true, alias_false;

static void first_receiver (RTMCLASSP module, WORD type, VOID *data)
{
	MSG_SET_VAR *msg = data;
	if (msg->value)
	{
		first_true++;
		/* Bound the old restart behavior so the regression fails instead of hanging. */
		if (reset_enabled && first_true <= 12)
			update_var (&vars, 1, FALSE, FALSE);
	}
	else
		first_false++;
}

static void second_receiver (RTMCLASSP module, WORD type, VOID *data)
{
	MSG_SET_VAR *msg = data;
	if (msg->value) second_true++;
	else second_false++;
}

static void alias_receiver (RTMCLASSP module, WORD type, VOID *data)
{
	MSG_SET_VAR *msg = data;
	if (msg->value) alias_true++;
	else alias_false++;
}

int test_var (FILE *out)
{
	int failures = 0;

	vars.status = &state;
	first.message = first_receiver;
	second.message = second_receiver;
	alias.message = alias_receiver;
	head.next = &first_link;
	first_link.next = &second_link;
	first_link.key = &first;
	second_link.next = &head;
	second_link.key = &second;
	alias_head.next = &alias_link;
	alias_link.next = &alias_head;
	alias_link.key = &alias;
	state.sysvar[1].header = &head;
	state.sysvar[1].max = 1;
	state.sysvar[1].varnumber = 2;
	state.sysvar[2].header = &alias_head;
	state.sysvar[2].max = 1;

	update_var (&vars, 1, 1, FALSE);
	update_var (&vars, 1, 1, FALSE);
	if (first_true != 1 || second_true != 1 || alias_true != 1)
	{
		fputs ("FAIL variable update and unchanged value\n", out);
		failures++;
	}

	update_var (&vars, 1, 1, TRUE);
	if (first_true != 2 || second_true != 2 || alias_true != 2)
	{
		fputs ("FAIL forced variable resend and alias\n", out);
		failures++;
	}

	state.sysvar[1].value = 0;
	first_true = first_false = second_true = second_false = 0;
	alias_true = alias_false = 0;
	reset_enabled = 1;
	update_var (&vars, 1, 1, FALSE);
	if (state.sysvar[1].value != 0 || state.sysvar[2].value != 0 ||
	    first_true != 1 || first_false != 1 ||
	    second_true != 0 || second_false != 1 ||
	    alias_true != 0 || alias_false != 1)
	{
		fputs ("FAIL recursive variable reset delivered an obsolete value\n", out);
		failures++;
	}

	return failures;
}
