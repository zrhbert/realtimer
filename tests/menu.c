#define is_state test_is_state
#define menu_normal test_menu_normal
#define find_top test_find_top
#define hndl_alert test_hndl_alert
#include "../src/menu.c"

static WINDOW window;
static RTMCLASS module;
static RTMCLASSP modules[1];
static int help_calls, fallback_calls, alert_count;
static BOOLEAN help_succeeds;

BOOLEAN test_is_state (OBJECT *tree, WORD object, UWORD state)
{
	return FALSE;
}

VOID test_menu_normal (WINDOWP target, WORD title, BOOLEAN normal)
{
}

WINDOWP test_find_top (VOID)
{
	return &window;
}

WORD test_hndl_alert (WORD alert_id)
{
	alert_count++;
	return 1;
}

static BOOLEAN module_help (RTMCLASSP target)
{
	help_calls++;
	if (!help_succeeds) alert_count++;
	return help_succeeds;
}

static BOOLEAN fallback_help (WINDOWP target, WORD icon)
{
	fallback_calls++;
	return FALSE;
}

int test_menu (FILE *out)
{
	int failures = 0;

	window.opened = 1;
	window.module = &module;
	window.showhelp = fallback_help;
	module.help = module_help;
	modules[0] = &module;
	rtmmodules = modules;
	rtmtop = 1;
	sel_window = NULL;

	hndl_menu (NULL, MFILE, MHELP);
	if (help_calls != 1 || alert_count != 1 || fallback_calls != 0)
	{
		fputs ("FAIL failed module help was dispatched more than once\n", out);
		failures++;
	}

	help_calls = fallback_calls = alert_count = 0;
	help_succeeds = TRUE;
	hndl_menu (NULL, MFILE, MHELP);
	if (help_calls != 1 || alert_count != 0 || fallback_calls != 0)
	{
		fputs ("FAIL successful module help\n", out);
		failures++;
	}

	help_calls = fallback_calls = alert_count = 0;
	module.help = NULL;
	hndl_menu (NULL, MFILE, MHELP);
	if (help_calls != 0 || alert_count != 1 || fallback_calls != 1)
	{
		fputs ("FAIL help fallback for a window without module help\n", out);
		failures++;
	}
	return failures;
}
