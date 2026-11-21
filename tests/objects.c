#define draw_object test_setup_draw_object
#define redraw_window test_setup_redraw_window
#define get_border test_setup_get_border
#define mem_alloc test_setup_mem_alloc
#include "../src/objects.c"
#undef draw_object
#undef redraw_window
#undef get_border
#undef mem_alloc

extern VOID draw_object (WINDOWP window, WORD object);
extern VOID redraw_window (WINDOWP window, CONST RECT *area);
extern VOID get_border (WINDOWP window, WORD object, RECT *border);
extern VOID *mem_alloc (LONG size);

typedef struct
{
	const char *text;
	const char *transparent;
	BOOL selected;
	WORD color;
	ICONBLK *icon;
	BOOL move_field;
	BOOL add_ted;
} SETUP_VALUES;

static OBJECT tree[7];
static TEDINFO fields[3];
static ICONBLK icons[2];
static char text[16], fill_text[] = "2", transparent_text[16];
static SETUP_VALUES values[2];
static SETUP presets[2], current, edited_setup;
static EDIT actual, edited;
static RTMCLASS module;
static WINDOW window;
static int observing, grouped_regions, fail_allocation;
static int draws[7], regions[7], region_widths[7];

VOID *test_setup_mem_alloc (LONG size)
{
	if (observing && fail_allocation) return NULL;
	return mem_alloc (size);
}

VOID test_setup_draw_object (WINDOWP target, WORD object)
{
	if (observing && target == &window && object >= 0 && object < 7)
		draws[object]++;
	else
		draw_object (target, object);
}

VOID test_setup_get_border (WINDOWP target, WORD object, RECT *border)
{
	if (observing && target == &window)
	{
		border->x = object * 3;
		if (grouped_regions == 1 && object == 4) border->x = 10;
		if (grouped_regions == 2)
		{
			if (object == 1) border->x = 2;
			if (object == 3) border->x = 1;
			if (object == 4) border->x = 0;
		}
		border->y = 0;
		border->w = border->h = 1;
	}
	else
		get_border (target, object, border);
}

VOID test_setup_redraw_window (WINDOWP target, CONST RECT *area)
{
	if (observing && target == &window && area->x >= 0 && area->x < 21)
	{
		regions[area->x / 3]++;
		region_widths[area->x / 3] = area->w;
	}
	else
		redraw_window (target, area);
}

static VOID setup_fields (RTMCLASSP target)
{
	SETUP_VALUES *value = target->edited->setup->dummy;

	strcpy (text, value->text);
	strcpy (transparent_text, value->transparent);
	tree[2].ob_state &= ~SELECTED;
	if (value->selected) tree[2].ob_state |= SELECTED;
	fields[1].te_color = value->color;
	tree[4].ob_spec.iconblk = value->icon;
	if (value->move_field) tree[1].ob_x++;
	if (value->add_ted) tree[6].ob_spec.tedinfo = &fields[2];
}

static VOID clear_draws (VOID)
{
	memset (draws, 0, sizeof (draws));
	memset (regions, 0, sizeof (regions));
	memset (region_widths, 0, sizeof (region_widths));
}

static int draw_count (VOID)
{
	int i, count = 0;
	for (i = 0; i < 7; i++) count += draws[i] + regions[i];
	return count;
}

int test_objects (FILE *out)
{
	int failures = 0;

	memset (tree, 0, sizeof (tree));
	memset (fields, 0, sizeof (fields));
	memset (&module, 0, sizeof (module));
	memset (&window, 0, sizeof (window));
	tree[0].ob_type = G_BOX;
	tree[1].ob_type = G_FBOXTEXT;
	tree[1].ob_spec.tedinfo = &fields[0];
	fields[0].te_ptext = text;
	fields[0].te_txtlen = sizeof (text);
	fields[0].te_color = 0x1180;
	tree[2].ob_type = G_USERDEF;
	tree[3].ob_type = G_BOXTEXT;
	tree[3].ob_spec.tedinfo = &fields[1];
	fields[1].te_ptext = fill_text;
	fields[1].te_txtlen = sizeof (fill_text);
	tree[4].ob_type = G_ICON;
	tree[5].ob_type = G_BUTTON;
	tree[5].ob_spec.free_string = "Next";
	tree[6].ob_type = G_FTEXT;
	tree[6].ob_flags = LASTOB;
	tree[6].ob_spec.tedinfo = &fields[2];
	fields[2].te_ptext = transparent_text;
	fields[2].te_txtlen = sizeof (transparent_text);
	fields[2].te_color = 0x1100;

	values[0].text = "1234";
	values[0].transparent = "same";
	values[0].selected = TRUE;
	values[0].color = 0x1180;
	values[0].icon = &icons[0];
	values[1].text = "9";
	values[1].transparent = "new";
	values[1].selected = TRUE;
	values[1].color = 0x11a0;
	values[1].icon = &icons[1];
	presets[0].dummy = &values[0];
	presets[1].dummy = &values[1];
	current = edited_setup = presets[0];
	actual.setup = &current;
	edited.setup = &edited_setup;
	edited.modified = FALSE;
	module.actual = &actual;
	module.edited = &edited;
	module.standard = &presets[0];
	module.setups = presets;
	module.setup_length = sizeof (SETUP);
	module.max_setups = 2;
	module.location = SETUPS_INTERN;
	module.window = &window;
	module.set_edit = set_edit_obj;
	module.set_dbox = setup_fields;
	module.set_setnr = set_setnr_obj;
	window.module = &module;
	window.object = tree;
	window.opened = 1;
	window.exit_obj = 5;
	setup_fields (&module);
	observing = TRUE;

	clear_draws ();
	set_setnr_obj (&module, 1);
	if (draws[0] || !regions[1] || !regions[3] || !regions[4] || !regions[6] ||
	    draws[2] || draws[5] || draw_count () != 4)
	{
		fputs ("FAIL setup did not redraw only changed text, color and icon\n", out);
		failures++;
	}

	clear_draws ();
	set_setnr_obj (&module, 1);
	if (draw_count ())
	{
		fputs ("FAIL unchanged setup redrew controls\n", out);
		failures++;
	}

	grouped_regions = TRUE;
	clear_draws ();
	set_setnr_obj (&module, 0);
	if (!regions[3] || regions[4] || region_widths[3] != 2 || draw_count () != 3)
	{
		fputs ("FAIL adjacent changed setup regions were not combined\n", out);
		failures++;
	}
	grouped_regions = 2;
	clear_draws ();
	set_setnr_obj (&module, 1);
	if (!regions[0] || region_widths[0] != 3 || draw_count () != 2)
	{
		fputs ("FAIL bridging changed setup regions were not combined\n", out);
		failures++;
	}
	grouped_regions = FALSE;

	clear_draws ();
	tree[5].ob_state |= SELECTED;
	edited.modified = TRUE;
	set_nr_obj (&window, 0);
	if (draws[0] || !regions[1] || !regions[3] || !regions[4] || !regions[6] ||
	    !regions[5] || tree[5].ob_state & SELECTED || draws[2] ||
	    draw_count () != 5)
	{
		fputs ("FAIL edited setup change or pressed-button redraw\n", out);
		failures++;
	}

	values[1].selected = FALSE;
	clear_draws ();
	set_setnr_obj (&module, 1);
	if (!draws[2] || tree[2].ob_state & SELECTED || draws[0])
	{
		fputs ("FAIL changed setup checkbox was not redrawn\n", out);
		failures++;
	}

	values[1].move_field = TRUE;
	clear_draws ();
	set_setnr_obj (&module, 1);
	if (draws[0] != 1)
	{
		fputs ("FAIL setup tree geometry change did not force a full redraw\n", out);
		failures++;
	}

	values[1].move_field = FALSE;
	values[1].add_ted = TRUE;
	tree[6].ob_spec.tedinfo = NULL;
	fields[2].te_color = 0x1180;
	clear_draws ();
	set_setnr_obj (&module, 1);
	if (regions[6] != 1 || draws[6] || draws[0])
	{
		fputs ("FAIL new text descriptor did not restore its background\n", out);
		failures++;
	}

	tree[3].ob_flags |= INDIRECT;
	clear_draws ();
	set_setnr_obj (&module, 0);
	if (draws[0] != 1 || draw_count () != 1)
	{
		fputs ("FAIL indirect setup object did not force a full redraw\n", out);
		failures++;
	}

	tree[3].ob_flags &= ~INDIRECT;
	fail_allocation = TRUE;
	clear_draws ();
	set_setnr_obj (&module, 1);
	if (draws[0] != 1 || draw_count () != 1)
	{
		fputs ("FAIL setup snapshot allocation fallback\n", out);
		failures++;
	}
	fail_allocation = FALSE;
	observing = FALSE;
	return failures;
}
