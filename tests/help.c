#include "../src/import.h"

static short test_appl_find (const char *name);
static short test_appl_write (short id, short length, void *message);

#undef appl_find
#undef appl_write
#define appl_find(name) test_appl_find (name)
#define appl_write(id, length, message) test_appl_write (id, length, message)
#define close_window test_help_close_window
#include "../src/realtspc.c"

static short accessory_available, write_succeeds;
static short writes, destination, message_length;
static int status_closes;
static WORD words[8];

VOID test_help_close_window (WINDOWP window)
{
	status_closes++;
	window->opened = 0;
}

static short test_appl_find (const char *name)
{
	return accessory_available && strcmp (name, AC_NAME) == 0 ? 7 : -1;
}

static short test_appl_write (short id, short length, void *message)
{
	destination = id;
	message_length = length;
	writes++;
	if (length == sizeof (words)) memcpy (words, message, sizeof (words));
	return write_succeeds;
}

int test_help (FILE *out)
{
	WORD old_apid = gl_apid;
	char *keyword;
	WINDOW status_window = {0};
	AKTSTATUS *status;
	int failures = 0;

	status = malloc (sizeof (*status));
	if (!status)
	{
		fputs ("FAIL allocating initialization status\n", out);
		failures++;
	}
	else
	{
		status_window.special = (LONG)status;
		status_window.opened = 1;
		status->close = FALSE;
		timer_daktstat (&status_window);
		if (status_closes || status_window.special != (LONG)status)
		{
			fputs ("FAIL keeping an active initialization status\n", out);
			failures++;
		}
		status->close = TRUE;
		timer_daktstat (&status_window);
		if (status_closes != 1 || status_window.opened || status_window.special)
		{
			fputs ("FAIL releasing the closed initialization status\n", out);
			failures++;
		}
	}

	gl_apid = 42;
	if (help_rtm ("CMI") || writes != 0)
	{
		fputs ("FAIL help request when PC_HELP is absent\n", out);
		failures++;
	}

	accessory_available = TRUE;
	write_succeeds = TRUE;
	if (!help_rtm ("  CMI setup") || writes != 1 || destination != 7 ||
	    message_length != 16 || words[0] != AC_HELP ||
	    words[1] != gl_apid || words[2] != 0)
	{
		fputs ("FAIL PC_HELP message header\n", out);
		failures++;
	}
	else
	{
		memcpy (&keyword, &words[3], sizeof (keyword));
		if (!keyword || strcmp (keyword, "CMI") != 0)
		{
			fputs ("FAIL PC_HELP message keyword\n", out);
			failures++;
		}
	}

	write_succeeds = FALSE;
	if (help_rtm ("CMI") || writes != 2)
	{
		fputs ("FAIL PC_HELP delivery failure\n", out);
		failures++;
	}
	gl_apid = old_apid;
	return failures;
}
