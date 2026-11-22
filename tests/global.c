#include "../src/import.h"

static short test_popup_graf_mkstate (short *x, short *y, short *buttons, short *keys);
static short test_popup_evnt_multi (short type, short clicks, short button_mask,
                                    short button_state, short leave1, short x1,
                                    short y1, short w1, short h1, short leave2,
                                    short x2, short y2, short w2, short h2,
                                    short *message, unsigned long interval,
                                    short *x, short *y, short *buttons,
                                    short *keys, short *key, short *count);

#undef graf_mkstate
#undef evnt_multi
#define graf_mkstate test_popup_graf_mkstate
#define evnt_multi test_popup_evnt_multi
#include "../src/global.c"

typedef struct
{
	WORD event, x, y;
} POPUP_EVENT;

static POPUP_EVENT events[4];
static WORD initial_x, initial_y, event_count, event_index;
static BOOL popup_active, events_exhausted, invalid_event;

static short test_popup_graf_mkstate (short *x, short *y, short *buttons, short *keys)
{
	if (!popup_active) return mt_graf_mkstate (x, y, buttons, keys, aes_global);
	*x = initial_x;
	*y = initial_y;
	*buttons = 1;
	*keys = 0;
	return 1;
}

static short test_popup_evnt_multi (short type, short clicks, short button_mask,
                                   short button_state, short leave1, short x1,
                                   short y1, short w1, short h1, short leave2,
                                   short x2, short y2, short w2, short h2,
                                   short *message, unsigned long interval,
                                   short *x, short *y, short *buttons,
                                   short *keys, short *key, short *count)
{
	POPUP_EVENT *event;

	if (!popup_active)
		return mt_evnt_multi (type, clicks, button_mask, button_state,
		                      leave1, x1, y1, w1, h1, leave2, x2, y2, w2, h2,
		                      message, interval, x, y, buttons, keys, key, count,
		                      aes_global);
	if (event_index >= event_count)
	{
		events_exhausted = TRUE;
		return MU_BUTTON;
	}
	event = &events[event_index++];
	if (event->event == MU_M1)
	{
		BOOL inside = event->x >= x1 && event->x < x1 + w1 &&
		              event->y >= y1 && event->y < y1 + h1;
		if (!(type & MU_M1) || (leave1 ? inside : !inside)) invalid_event = TRUE;
	}
	*x = event->x;
	*y = event->y;
	*buttons = event->event == MU_BUTTON ? 0 : 1;
	*keys = *key = 0;
	*count = 1;
	return event->event;
}

static WORD run_popup (OBJECT *tree, WORD y, BOOL guarded)
{
	initial_x = 100;
	initial_y = y;
	event_index = 0;
	events_exhausted = invalid_event = FALSE;
	popup_active = TRUE;
	if (guarded)
		y = popup_menu_guarded (tree, ROOT, 0, 0, 2, TRUE, 1);
	else
		y = popup_menu (tree, ROOT, 0, 0, 2, TRUE, 1);
	popup_active = FALSE;
	return y;
}

int test_global (FILE *out)
{
	OBJECT tree[3];
	RECT old_desk = desk;
	WORD old_blinkrate = blinkrate;
	WORD work_in[11], work_out[57], width, height, cell_width, cell_height;
	WORD i, selected;
	int failures = 0;

	if (appl_init () < 0)
	{
		fputs ("FAIL initializing AES for popup test\n", out);
		return 1;
	}
	vdi_handle = graf_handle (&width, &height, &cell_width, &cell_height);
	for (i = 0; i < 10; i++) work_in[i] = 1;
	work_in[10] = RC;
	v_opnvwk (work_in, &vdi_handle, work_out);
	if (!vdi_handle)
	{
		fputs ("FAIL opening VDI workstation for popup test\n", out);
		appl_exit ();
		return 1;
	}
	desk.x = desk.y = 0;
	desk.w = 640;
	desk.h = 400;
	blinkrate = 0;
	memset (tree, 0, sizeof (tree));
	tree[0].ob_next = NIL;
	tree[0].ob_head = 1;
	tree[0].ob_tail = 2;
	tree[0].ob_type = G_BOX;
	tree[0].ob_spec.index = 0x00ff1100L;
	tree[0].ob_width = 100;
	tree[0].ob_height = 100;
	tree[1].ob_next = 2;
	tree[1].ob_head = tree[1].ob_tail = NIL;
	tree[1].ob_type = G_BOX;
	tree[1].ob_flags = SELECTABLE|EXIT;
	tree[1].ob_spec.index = 0x00ff1100L;
	tree[1].ob_x = 10;
	tree[1].ob_width = 80;
	tree[1].ob_height = 25;
	tree[2] = tree[1];
	tree[2].ob_next = ROOT;
	tree[2].ob_flags |= LASTOB;
	tree[2].ob_y = 70;
	tree[2].ob_height = 20;

	events[0] = (POPUP_EVENT){MU_BUTTON, 100, 10};
	event_count = 1;
	selected = run_popup (tree, 10, TRUE);
	if (selected != NIL || events_exhausted || invalid_event)
	{
		fputs ("FAIL shifted popup selected an item without pointer movement\n", out);
		failures++;
	}

	events[0] = (POPUP_EVENT){MU_M1, 100, 75};
	events[1] = (POPUP_EVENT){MU_BUTTON, 100, 75};
	event_count = 2;
	selected = run_popup (tree, 10, TRUE);
	if (selected != 2 || events_exhausted || invalid_event)
	{
		fputs ("FAIL selecting a shifted popup item by dragging\n", out);
		failures++;
	}

	events[0] = (POPUP_EVENT){MU_M1, 100, 75};
	events[1] = (POPUP_EVENT){MU_M1, 100, 10};
	events[2] = (POPUP_EVENT){MU_BUTTON, 100, 10};
	event_count = 3;
	selected = run_popup (tree, 10, TRUE);
	if (selected != 1 || events_exhausted || invalid_event)
	{
		fputs ("FAIL selecting the initially covered popup item after moving away\n", out);
		failures++;
	}

	events[0] = (POPUP_EVENT){MU_M1, 100, 40};
	events[1] = (POPUP_EVENT){MU_M1, 100, 10};
	events[2] = (POPUP_EVENT){MU_BUTTON, 100, 10};
	event_count = 3;
	selected = run_popup (tree, 10, TRUE);
	if (selected != 1 || events_exhausted || invalid_event)
	{
		fputs ("FAIL selecting the initially covered popup item after leaving for the background\n", out);
		failures++;
	}

	events[0] = (POPUP_EVENT){MU_BUTTON, 100, 100};
	event_count = 1;
	selected = run_popup (tree, 100, TRUE);
	if (selected != 2 || events_exhausted || invalid_event)
	{
		fputs ("FAIL selecting the centered popup item\n", out);
		failures++;
	}

	events[0] = (POPUP_EVENT){MU_BUTTON, 100, 10};
	selected = run_popup (tree, 10, FALSE);
	if (selected != 1 || events_exhausted || invalid_event)
	{
		fputs ("FAIL preserving the original popup behavior\n", out);
		failures++;
	}

	desk = old_desk;
	blinkrate = old_blinkrate;
	v_clsvwk (vdi_handle);
	appl_exit ();
	return failures;
}
