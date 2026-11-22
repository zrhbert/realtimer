#define popup_menu_guarded test_a3d_popup_menu
#define draw_object test_a3d_draw_object
#define redraw_window test_a3d_redraw_window
#define draw_dispobjs test_a3d_draw_dispobjs
#define hide_mouse test_a3d_hide_mouse
#define show_mouse test_a3d_show_mouse
#include "../src/a3d.c"

static WORD popup_choice, popup_center;
static int mouse_hidden, unprotected_draw, redraw_count, inject_update;
static RTMCLASS module;

WORD test_a3d_popup_menu (OBJECT *tree, WORD obj, WORD x, WORD y,
                         WORD center, BOOLEAN relative, WORD buttons)
{
	popup_center = center;
	return popup_choice;
}

VOID test_a3d_draw_object (WINDOWP window, WORD object)
{
}

VOID test_a3d_redraw_window (WINDOWP window, CONST RECT *area)
{
	redraw_count++;
}

VOID test_a3d_hide_mouse (VOID)
{
	mouse_hidden++;
}

VOID test_a3d_show_mouse (VOID)
{
	mouse_hidden--;
}

VOID test_a3d_draw_dispobjs (WINDOWP window)
{
	if (!mouse_hidden) unprotected_draw++;
	if (inject_update)
	{
		inject_update = FALSE;
		apply (&module, NULL);
	}
}

static BOOL tree_contains (OBJECT *tree, WORD parent, WORD wanted)
{
	WORD object = tree[parent].ob_head;
	WORD count = 0;

	while (object != NIL && object != parent && count++ < NUM_OBS)
	{
		if (object == wanted) return TRUE;
		object = tree[object].ob_next;
	}
	return FALSE;
}

static int test_setup_file (FILE *out, RTMCLASSP module)
{
	static CONST CHAR filename[] = "A3DTEST.TMP";
	static CONST CHAR header[] = "A3D V 1.00\n";
	SET_P actual = module->actual->setup;
	FILE *file = fopen (filename, "w+b");
	int failures = 0;

	if (!file)
	{
		fputs ("FAIL creating the A3D setup test file\n", out);
		return 1;
	}
	module->location = SETUPS_EXTERN;
	module->file_pointer = file;
	module->file_header_len = strlen (header);
	if (fputs (header, file) == EOF ||
	    fwrite (actual, sizeof (*actual), 1, file) != 1 ||
	    !get_setnr_obj (module, 1))
	{
		fputs ("FAIL writing A3D top-view setup to disk\n", out);
		failures++;
	}
	if (fclose (file))
	{
		fputs ("FAIL closing the A3D setup test file\n", out);
		failures++;
	}

	file = fopen (filename, "r+b");
	if (!file)
	{
		fputs ("FAIL reopening the A3D setup test file\n", out);
		failures++;
	}
	else
	{
		module->file_pointer = file;
		actual->raumform = WUERFEL;
		if (!set_setnr_obj (module, 1) || actual->raumform != DRAUFSICHT)
		{
			fputs ("FAIL loading A3D top-view setup from disk\n", out);
			failures++;
		}
		if (fclose (file))
		{
			fputs ("FAIL closing the reopened A3D setup test file\n", out);
			failures++;
		}
	}
	module->file_pointer = NULL;
	module->location = SETUPS_INTERN;
	if (remove (filename))
	{
		fputs ("FAIL removing the A3D setup test file\n", out);
		failures++;
	}
	return failures;
}

int test_a3d (FILE *out)
{
	static STATUS state;
	static SETUP actual_setup, edited_setup, setups[2];
	static EDIT actual, edited;
	static WINDOW window;
	WINDOWP previous_selected = sel_window;
	ICONBLK *previous_icon;
	MKINFO mouse = {0};
	int failures = 0;

	a3d_menu = rs_trindex[A3D_MENU];
	a3d_setup = rs_trindex[A3D_SETUP];
	a3d_raum = rs_trindex[A3D_RAUM];
	previous_icon = a3d_setup[A3DRAUMFORM].ob_spec.iconblk;

	if (NUM_OBS != sizeof (rs_object) / sizeof (rs_object[0]) ||
	    NUM_IB != sizeof (rs_iconblk) / sizeof (rs_iconblk[0]) ||
	    !tree_contains (a3d_menu, 15, MA3DDRAUFSICHT) ||
	    !tree_contains (a3d_menu, 30, MA3DSIGNALNUMMERN) ||
	    !tree_contains (a3d_raum, ROOT, RAUMDRAUFSICHT) ||
	    !is_flags (a3d_raum, RAUMDRAUFSICHT, LASTOB) ||
	    a3d_raum[RAUMDRAUFSICHT].ob_spec.iconblk != &rs_iconblk[14] ||
	    sizeof (RSIB14MASK) != 32 * 30 / 8 ||
	    sizeof (RSIB14DATA) != 32 * 30 / 8)
	{
		fputs ("FAIL A3D top-view resource tree\n", out);
		failures++;
	}

	module.status = &state;
	module.actual = &actual;
	module.edited = &edited;
	module.window = &window;
	module.setup_length = sizeof (SETUP);
	module.location = SETUPS_INTERN;
	module.max_setups = 2;
	module.setups = setups;
	module.flags = FLAG_IMPORTING;
	actual.setup = &actual_setup;
	edited.setup = &edited_setup;
	edited.modified = TRUE;
	window.module = &module;
	window.object = a3d_setup;
	sel_window = &window;

	handle_menu_mod (&window, MA3DFORMS, MA3DDRAUFSICHT);
	update_menu_mod (&window);
	if (actual_setup.raumform != DRAUFSICHT || !state.new ||
	    window.milli != 1 || !is_state (a3d_menu, MA3DDRAUFSICHT, CHECKED))
	{
		fputs ("FAIL selecting A3D top view from the menu\n", out);
		failures++;
	}
	state.new = FALSE;
	window.milli = 0;
	handle_menu_mod (&window, MA3DOPTIONS, MA3DSIGNALNUMMERN);
	update_menu_mod (&window);
	if (!state.hide_top_numbers || !state.new || window.milli != 1 ||
	    is_state (a3d_menu, MA3DSIGNALNUMMERN, CHECKED | DISABLED))
	{
		fputs ("FAIL hiding A3D top-view signal numbers\n", out);
		failures++;
	}
	handle_menu_mod (&window, MA3DOPTIONS, MA3DSIGNALNUMMERN);
	update_menu_mod (&window);
	if (state.hide_top_numbers || !is_state (a3d_menu, MA3DSIGNALNUMMERN, CHECKED))
	{
		fputs ("FAIL restoring A3D top-view signal numbers\n", out);
		failures++;
	}
	handle_menu_mod (&window, MA3DFORMS, MA3DWUERFEL);
	update_menu_mod (&window);
	if (actual_setup.raumform != WUERFEL ||
	    is_state (a3d_menu, MA3DDRAUFSICHT, CHECKED) ||
	    !is_state (a3d_menu, MA3DSIGNALNUMMERN, DISABLED))
	{
		fputs ("FAIL selecting a 3D A3D view after the top view\n", out);
		failures++;
	}

	edited_setup.raumform = WUERFEL;
	window.exit_obj = A3DRAUMFORM;
	popup_choice = RAUMDRAUFSICHT;
	click_setup (&window, &mouse);
	if (popup_center != WUERFEL + 1 || edited_setup.raumform != DRAUFSICHT ||
	    a3d_setup[A3DRAUMFORM].ob_spec.iconblk !=
	    a3d_raum[RAUMDRAUFSICHT].ob_spec.iconblk)
	{
		fputs ("FAIL selecting A3D top view from setup\n", out);
		failures++;
	}
	popup_choice = RAUMWUERFEL;
	click_setup (&window, &mouse);
	if (popup_center != RAUMDRAUFSICHT || edited_setup.raumform != WUERFEL)
	{
		fputs ("FAIL reopening A3D setup with the top view selected\n", out);
		failures++;
	}
	popup_choice = RAUMDRAUFSICHT;
	click_setup (&window, &mouse);

	get_edit_obj (&module);
	if (!get_setnr_obj (&module, 1))
	{
		fputs ("FAIL storing A3D top-view setup\n", out);
		failures++;
	}
	actual_setup.raumform = WUERFEL;
	if (!set_setnr_obj (&module, 1) || actual_setup.raumform != DRAUFSICHT)
	{
		fputs ("FAIL recalling A3D top-view setup\n", out);
		failures++;
	}
	failures += test_setup_file (out, &module);

	state.new = FALSE;
	window.milli = MILLI;
	inject_update = TRUE;
	wi_timer_mod (&window);
	if (mouse_hidden || unprotected_draw || inject_update || window.milli != MILLI)
	{
		fputs ("FAIL A3D protected redraw or update during redraw\n", out);
		failures++;
	}
	state.new = TRUE;
	wi_timer_mod (&window);
	if (redraw_count != 1 || window.milli != 0)
	{
		fputs ("FAIL A3D full redraw after view change\n", out);
		failures++;
	}

	a3d_setup[A3DRAUMFORM].ob_spec.iconblk = previous_icon;
	sel_window = previous_selected;
	return failures;
}
