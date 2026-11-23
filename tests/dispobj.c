#include "../src/dispobj.c"

static WORD space_pixel (WORD x, WORD y)
{
	WORD pixel, color;

	v_get_pixel (vdi_handle, x, y, &pixel, &color);
	return pixel;
}

static void save_space_pixels (CONST RECT *work, WORD *pixels)
{
	WORD x, y;

	for (y = 0; y < work->h; y++)
		for (x = 0; x < work->w; x++)
			pixels[y * work->w + x] = space_pixel (work->x + x, work->y + y);
}

static int space_pixel_differences (CONST RECT *work, CONST RECT *area, WORD *pixels)
{
	WORD x, y;
	int differences = 0;

	for (y = area->y; y < area->y + area->h; y++)
		for (x = area->x; x < area->x + area->w; x++)
			if (pixels[(y - work->y) * work->w + x - work->x] != space_pixel (x, y))
				differences++;
	return differences;
}

static void space_clip (CONST RECT *area)
{
	WORD clip[4];

	clip[0] = area->x;
	clip[1] = area->y;
	clip[2] = area->x + area->w - 1;
	clip[3] = area->y + area->h - 1;
	vs_clip (vdi_handle, TRUE, clip);
}

static int test_top_view (FILE *out)
{
	static WORD pixels[160 * 112];
	RECT work = {64, 64, 160, 112};
	RECT left, right, gap;
	RECT labels[MAXSIGNALS - 1];
	TOP_LAYOUT layout, label_layout;
	SPACESTATUS label_state;
	WORD work_in[11], work_out[57], clip[4] = {0, 0, 0, 0};
	WORD width, height, cell_width, cell_height, old_wchar, old_hchar;
	WORD xpositive, xnegative, zpositive, znegative, ypositive, ynegative;
	WORD marker_x, marker_z, absent, single, duplicate;
	WORD label_x[MAXSIGNALS], label_z[MAXSIGNALS];
	WORD i, signal;
	DISPOBJP object;
	SPACESTATP state;
	int failures = 0, differences;

	if (appl_init () < 0)
	{
		fputs ("FAIL initializing AES for A3D top-view test\n", out);
		return 1;
	}
	old_wchar = gl_wchar;
	old_hchar = gl_hchar;
	vdi_handle = graf_handle (&width, &height, &cell_width, &cell_height);
	gl_wchar = width;
	gl_hchar = height;
	for (i = 0; i < 10; i++) work_in[i] = 1;
	work_in[10] = RC;
	v_opnvwk (work_in, &vdi_handle, work_out);
	if (!vdi_handle)
	{
		fputs ("FAIL opening VDI workstation for A3D top-view test\n", out);
		gl_wchar = old_wchar;
		gl_hchar = old_hchar;
		appl_exit ();
		return 1;
	}
	v_hide_c (vdi_handle);
	space_clip (&work);
	init_dispobj ();
	object = CreateSpaceDispobj (NULL, WUERFEL, 0, &work);
	object->set_type (object, DOTypeSpaceMode, MONO);
	object->set_uni (object, DOUniSpaceCrosshair, 1);
	object->set_uni (object, DOUniSpaceDisplay, 2);
	state = &object->status->space;
	state->position[0].x = 70;
	state->position[0].y = 40;
	state->position[0].z = 60;
	state->position[1].x = -50;
	state->position[1].y = -20;
	state->position[1].z = -30;
	InvalidVdiTextState ();
	DrawSpace (object);
	FinishSpace (object);
	save_space_pixels (&work, pixels);
	object->set_type (object, DOTypeSpaceForm, DRAUFSICHT);
	DrawSpace (object);
	FinishSpace (object);
	object->set_type (object, DOTypeSpaceForm, WUERFEL);
	InvalidVdiTextState ();
	DrawSpace (object);
	differences = space_pixel_differences (&work, &work, pixels);
	if (differences)
	{
		fprintf (out, "FAIL returning from A3D top view to 3D: %d pixels\n", differences);
		failures++;
	}
	FinishSpace (object);

	object->set_type (object, DOTypeSpaceForm, DRAUFSICHT);
	object->set_uni (object, DOUniSpaceDisplay, 0);
	DrawSpace (object);
	FinishSpace (object);
	if (!top_layout (object, &layout))
	{
		fputs ("FAIL A3D top-view layout\n", out);
		failures++;
		goto cleanup;
	}
	xpositive = top_coordinate (70, layout.plot.x, layout.plot.w, FALSE, FALSE);
	xnegative = top_coordinate (-70, layout.plot.x, layout.plot.w, FALSE, FALSE);
	zpositive = top_coordinate (60, layout.plot.y, layout.plot.h, TRUE, FALSE);
	znegative = top_coordinate (-60, layout.plot.y, layout.plot.h, TRUE, FALSE);
	ypositive = top_coordinate (40, layout.plot.y, layout.plot.h, TRUE, FALSE);
	ynegative = top_coordinate (-40, layout.plot.y, layout.plot.h, TRUE, FALSE);
	if (xpositive <= xnegative || zpositive >= znegative || ypositive >= ynegative ||
	    space_pixel (xpositive, layout.plot.y + 1) == space_pixel (xnegative, layout.plot.y + 1) ||
	    space_pixel (layout.plot.x + 1, zpositive) == space_pixel (layout.plot.x + 1, znegative) ||
	    space_pixel (layout.gauge_x - 2, ypositive) == space_pixel (layout.gauge_x - 2, ynegative))
	{
		fputs ("FAIL A3D top-view axis orientation or crosshair\n", out);
		failures++;
	}
	if (top_coordinate (40, layout.plot.x, layout.plot.w, FALSE, TRUE) !=
	    top_coordinate (100, layout.plot.x, layout.plot.w, FALSE, FALSE) ||
	    top_coordinate (-200, layout.plot.y, layout.plot.h, TRUE, FALSE) !=
	    top_coordinate (-100, layout.plot.y, layout.plot.h, TRUE, FALSE) ||
	    top_coordinate (20, layout.plot.x, layout.plot.w, FALSE, TRUE) <=
	    top_coordinate (20, layout.plot.x, layout.plot.w, FALSE, FALSE))
	{
		fputs ("FAIL A3D top-view inside zoom or coordinate clamping\n", out);
		failures++;
	}

	label_layout = layout;
	label_layout.plot.w = label_layout.plot.h = 200;
	memset (&label_state, 0, sizeof (label_state));
	label_state.crosshair[0] = TRUE;
	label_x[0] = label_layout.plot.x + label_layout.plot.w / 2;
	label_z[0] = label_layout.plot.y + label_layout.plot.h / 2;
	for (signal = 1; signal < MAXSIGNALS; signal++)
	{
		label_x[signal] = label_x[0] - 6 + signal;
		label_z[signal] = label_z[0] - 6 + signal;
		label_state.display[signal] = TRUE;
	}
	for (signal = 1; signal < MAXSIGNALS; signal++)
	{
		WORD other;
		RECT *label = &labels[signal - 1];

		top_place_label (&label_layout, &label_state, label_x, label_z, signal, 1,
		                 labels, signal - 1, label);
		if (label->x <= label_layout.plot.x || label->y <= label_layout.plot.y ||
		    label->x + label->w >= label_layout.plot.x + label_layout.plot.w ||
		    label->y + label->h >= label_layout.plot.y + label_layout.plot.h)
		{
			fputs ("FAIL A3D signal label outside the plot\n", out);
			failures++;
		}
		if ((label_x[0] >= label->x && label_x[0] < label->x + label->w) ||
		    (label_z[0] >= label->y && label_z[0] < label->y + label->h))
		{
			fputs ("FAIL A3D crosshair through a signal label\n", out);
			failures++;
		}
		for (other = 1; other < MAXSIGNALS; other++)
			if (label->x <= label_x[other] + 2 &&
			    label->x + label->w > label_x[other] - 2 &&
			    label->y <= label_z[other] + 2 &&
			    label->y + label->h > label_z[other] - 2)
			{
				fputs ("FAIL A3D marker through a signal label\n", out);
				failures++;
			}
		for (other = 0; other < signal - 1; other++)
			if (label->x < labels[other].x + labels[other].w &&
			    label->x + label->w > labels[other].x &&
			    label->y < labels[other].y + labels[other].h &&
			    label->y + label->h > labels[other].y)
			{
				fputs ("FAIL overlapping A3D signal labels\n", out);
				failures++;
			}
	}

	memset (&label_state, 0, sizeof (label_state));
	label_state.crosshair[0] = label_state.display[1] = TRUE;
	for (i = 0; i < 4; i++)
	{
		RECT *label = &labels[0];

		label_x[0] = label_x[1] = label_layout.plot.x +
		              (i & 1 ? 3 : label_layout.plot.w - 4);
		label_z[0] = label_z[1] = label_layout.plot.y +
		              (i & 2 ? 3 : label_layout.plot.h - 4);
		top_place_label (&label_layout, &label_state, label_x, label_z, 1,
		                 MAXSIGNALS - 1, labels, 0, label);
		if (label->x <= label_layout.plot.x || label->y <= label_layout.plot.y ||
		    label->x + label->w >= label_layout.plot.x + label_layout.plot.w ||
		    label->y + label->h >= label_layout.plot.y + label_layout.plot.h ||
		    (label_x[0] >= label->x && label_x[0] < label->x + label->w) ||
		    (label_z[0] >= label->y && label_z[0] < label->y + label->h))
		{
			fputs ("FAIL A3D grouped signal labels at a plot edge\n", out);
			failures++;
		}
	}

	label_layout.plot.w = label_layout.plot.h =
		MAXSIGNALS * label_layout.text_height + 2;
	for (signal = 0; signal < MAXSIGNALS; signal++)
	{
		label_x[signal] = label_layout.plot.x + label_layout.plot.w / 2;
		label_z[signal] = label_layout.plot.y + 1 +
		                  signal * label_layout.text_height + label_layout.text_height / 2;
		label_state.crosshair[signal] = TRUE;
		label_state.display[signal] = signal > 0;
	}
	for (signal = 1; signal < MAXSIGNALS; signal++)
	{
		WORD other;
		RECT *label = &labels[signal - 1];

		top_place_label (&label_layout, &label_state, label_x, label_z, signal, 1,
		                 labels, signal - 1, label);
		for (other = 0; other < signal - 1; other++)
			if (label->x < labels[other].x + labels[other].w &&
			    label->x + label->w > labels[other].x &&
			    label->y < labels[other].y + labels[other].h &&
			    label->y + label->h > labels[other].y)
			{
				fputs ("FAIL crowded A3D label fallback\n", out);
				failures++;
			}
	}

	state->position[2] = state->position[1];
	marker_x = top_coordinate (state->position[1].x, layout.plot.x, layout.plot.w, FALSE, FALSE);
	marker_z = top_coordinate (state->position[1].z, layout.plot.y, layout.plot.h, TRUE, FALSE);
	absent = space_pixel (marker_x + 1, marker_z - 1);
	object->set_uni (object, DOUniSpaceDisplay, 2);
	DrawSpace (object);
	FinishSpace (object);
	single = space_pixel (marker_x + 1, marker_z - 1);
	save_space_pixels (&work, pixels);
	state->position[2].x = 30;
	state->kneu = TRUE;
	DrawSpace (object);
	FinishSpace (object);
	differences = space_pixel_differences (&work, &work, pixels);
	if (absent == single || differences)
	{
		fprintf (out, "FAIL A3D enabled and disabled signal markers: %d %d, %d pixels\n",
		         absent, single, differences);
		failures++;
	}
	state->position[2] = state->position[1];
	object->set_uni (object, DOUniSpaceDisplay, 6);
	DrawSpace (object);
	FinishSpace (object);
	duplicate = space_pixel (marker_x + 1, marker_z - 1);
	if (duplicate != single || !space_pixel_differences (&work, &layout.plot, pixels))
	{
		fputs ("FAIL coincident A3D markers and labels\n", out);
		failures++;
	}

	object->set_uni (object, DOUniSpaceDisplay, 0);
	state->position[1] = state->position[0];
	DrawSpace (object);
	FinishSpace (object);
	save_space_pixels (&work, pixels);
	object->set_uni (object, DOUniSpaceCrosshair, 3);
	DrawSpace (object);
	FinishSpace (object);
	if (space_pixel_differences (&work, &layout.plot, pixels))
	{
		fputs ("FAIL coincident A3D crosshairs\n", out);
		failures++;
	}

	{
		static const struct { WORD mode, display, edges; } cases[] = {
			{ MONO,   0x1fe, 0x00 },
			{ STEREO, 0x1fe, 0x55 },
			{ QUADRO, 0x1fe, 0x77 },
			{ OKTO,   0x1fe, 0x77 },
			{ QUADRO, 0x1fa, 0x74 }, /* Signal 2 off: no 1-2, 2-3 or 1-3. */
		};
		WORD projected[MAXSIGNALS], background, z, x;
		RECT gauge = work, gauge_numbers = work;
		WORD tick;

		object->set_uni (object, DOUniSpaceCrosshair, 0);
		object->set_uni (object, DOUniSpaceDisplay, 0x1fe);
		z = top_coordinate (0, layout.plot.y, layout.plot.h, TRUE, FALSE);
		for (signal = 1; signal < MAXSIGNALS; signal++)
		{
			state->position[signal] = (POS_3D){ -100 + (signal - 1) * 28, 0, 0 };
			projected[signal] = top_coordinate (state->position[signal].x,
			                        layout.plot.x, layout.plot.w, FALSE, FALSE);
		}
		DrawSpace (object);
		FinishSpace (object);
		save_space_pixels (&work, pixels);
		gauge.x = layout.gauge_x - 3;
		gauge.w = work.x + work.w - gauge.x;
		gauge_numbers = gauge;
		gauge_numbers.x = layout.gauge_x + 6;
		gauge_numbers.w = work.x + work.w - gauge_numbers.x;
		tick = space_pixel (gauge.x, z);
		object->set_uni (object, DOUniSpaceNumbers, FALSE);
		DrawSpace (object);
		FinishSpace (object);
		background = space_pixel (layout.plot.x + 1, z + 3);
		if (!space_pixel_differences (&work, &layout.plot, pixels) ||
		    !space_pixel_differences (&work, &gauge_numbers, pixels) ||
		    space_pixel (gauge.x, z) != tick ||
		    space_pixel (projected[1] + 1, z - 1) == background)
		{
			fputs ("FAIL A3D hiding signal numbers without hiding the marks\n", out);
			failures++;
		}
		save_space_pixels (&work, pixels);
		for (i = 0; i < sizeof (cases) / sizeof (cases[0]); i++)
		{
			object->set_type (object, DOTypeSpaceMode, cases[i].mode);
			object->set_uni (object, DOUniSpaceDisplay, cases[i].display);
			DrawSpace (object);
			FinishSpace (object);
			for (signal = 1; signal < MAXSIGNALS - 1; signal++)
			{
				x = (projected[signal] + projected[signal + 1]) / 2;
				if ((space_pixel (x, z) != background) !=
				    !!(cases[i].edges & (1 << (signal - 1))))
				{
					fprintf (out, "FAIL A3D connection %d-%d in case %d\n", signal, signal + 1, i);
					failures++;
				}
			}
			if (space_pixel_differences (&work, &gauge, pixels))
			{
				fputs ("FAIL A3D X/Z connections affected the Y gauge\n", out);
				failures++;
			}
		}
	}

	object->set_uni (object, DOUniSpaceCrosshair, 3);
	object->set_uni (object, DOUniSpaceNumbers, TRUE);
	object->set_uni (object, DOUniSpaceDisplay, 0x1fe);
	DrawSpace (object);
	FinishSpace (object);
	left = work;
	left.w = work.w / 2;
	right = work;
	right.x += left.w;
	right.w -= left.w;
	for (i = 0; i < 12; i++)
	{
		for (signal = 0; signal < MAXSIGNALS; signal++)
		{
			WORD source = i % 4 == 0 ? 0 : signal;
			state->position[signal].x = i % 4 == 3 ? source * 3 - 12 :
			                            (i * 19 + source * 29) % 201 - 100;
			state->position[signal].y = i % 4 == 3 ? source * 3 - 12 :
			                            (i * 31 + source * 17) % 201 - 100;
			state->position[signal].z = i % 4 == 3 ? source * 2 - 8 :
			                            (i * 13 + source * 37) % 201 - 100;
		}
		state->kneu = TRUE;
		space_clip (&left);
		DrawSpace (object);
		space_clip (&right);
		DrawSpace (object);
		FinishSpace (object);
	}
	space_clip (&work);
	save_space_pixels (&work, pixels);
	object->new = TRUE;
	DrawSpace (object);
	FinishSpace (object);
	differences = space_pixel_differences (&work, &work, pixels);
	if (differences)
	{
		fprintf (out, "FAIL A3D incremental redraw: %d pixels\n", differences);
		failures++;
	}

	save_space_pixels (&work, pixels);
	gap = work;
	gap.x += 60;
	gap.w = 30;
	left.w = 60;
	right.x = gap.x + gap.w;
	right.w = work.x + work.w - right.x;
	state->position[0].x = -70;
	state->position[0].y = -60;
	state->position[0].z = -50;
	state->position[1] = state->position[0];
	state->kneu = TRUE;
	space_clip (&left);
	DrawSpace (object);
	space_clip (&right);
	DrawSpace (object);
	FinishSpace (object);
	if (space_pixel_differences (&work, &gap, pixels))
	{
		fputs ("FAIL A3D redraw touched an occluded region\n", out);
		failures++;
	}
	object->new = TRUE;
	space_clip (&gap);
	DrawSpace (object);
	FinishSpace (object);
	space_clip (&work);
	save_space_pixels (&work, pixels);
	object->new = TRUE;
	DrawSpace (object);
	FinishSpace (object);
	differences = space_pixel_differences (&work, &work, pixels);
	if (differences)
	{
		fprintf (out, "FAIL A3D redraw after exposure: %d pixels\n", differences);
		failures++;
	}

	object->set_uni (object, DOUniSpaceCrosshair, 1);
	object->set_uni (object, DOUniSpaceDisplay, 0);
	state->position[0].x = state->position[0].y = state->position[0].z = 100;
	DrawSpace (object);
	FinishSpace (object);
	save_space_pixels (&work, pixels);
	object->set_uni (object, DOUniSpaceInside, TRUE);
	state->position[0].x = state->position[0].y = state->position[0].z = 40;
	DrawSpace (object);
	FinishSpace (object);
	if (space_pixel_differences (&work, &work, pixels))
	{
		fputs ("FAIL A3D inside-view rendering and clamping\n", out);
		failures++;
	}

	work.w = 58;
	work.h = 40;
	space_clip (&work);
	object->set_work (object, &work);
	DrawSpace (object);
	FinishSpace (object);
	if (!top_layout (object, &layout) || layout.labels)
	{
		fputs ("FAIL compact A3D top-view layout\n", out);
		failures++;
	}
	state->position[0].x = -20;
	state->position[0].y = 10;
	state->position[0].z = -25;
	state->kneu = TRUE;
	DrawSpace (object);
	FinishSpace (object);
	save_space_pixels (&work, pixels);
	object->new = TRUE;
	DrawSpace (object);
	FinishSpace (object);
	differences = space_pixel_differences (&work, &work, pixels);
	if (differences)
	{
		fprintf (out, "FAIL compact A3D incremental redraw: %d pixels\n", differences);
		failures++;
	}

cleanup:
	object->delete (object);
	vs_clip (vdi_handle, FALSE, clip);
	v_show_c (vdi_handle, 1);
	v_clsvwk (vdi_handle);
	gl_wchar = old_wchar;
	gl_hchar = old_hchar;
	appl_exit ();
	return failures;
}

static int test_cmo_redraw (FILE *out)
{
	static WORD pixels[64 * 64];
	RECT work = {64, 64, 64, 64};
	WORD work_in[11], work_out[57], clip[4];
	WORD width, height, cell_width, cell_height, pel, color;
	WORD x, y, i;
	DISPOBJP object;
	SPACESTATP state;
	int failures = 0;

	if (appl_init () < 0)
	{
		fputs ("FAIL initializing AES for CMO redraw test\n", out);
		return 1;
	}
	vdi_handle = graf_handle (&width, &height, &cell_width, &cell_height);
	for (i = 0; i < 10; i++) work_in[i] = 1;
	work_in[10] = RC;
	v_opnvwk (work_in, &vdi_handle, work_out);
	if (!vdi_handle)
	{
		fputs ("FAIL opening VDI workstation for CMO redraw test\n", out);
		appl_exit ();
		return 1;
	}
	v_hide_c (vdi_handle);
	clip[0] = work.x;
	clip[1] = work.y;
	clip[2] = work.x + work.w - 1;
	clip[3] = work.y + work.h - 1;
	vs_clip (vdi_handle, TRUE, clip);

	init_dispobj ();
	object = CreateCMOSpaceDispobj (NULL, WUERFEL, 0, &work, 0);
	object->set_type (object, DOTypeSpaceMode, MONO);
	object->set_uni (object, DOUniSpaceCrosshair, 1);
	object->set_uni (object, DOUniSpaceDisplay, 0);
	state = &object->status->space;
	DrawSpace (object);
	FinishSpace (object);
	for (i = 0; i < 32; i++)
	{
		state->position[0].x = (i * 17) % 161 - 80;
		state->position[0].y = (i * 29) % 161 - 80;
		state->position[0].z = (i * 37) % 161 - 80;
		state->volume[0] = (i * 17) % 128;
		state->kneu = TRUE;
		if (i % 4 == 0) vsl_color (vdi_handle, WHITE);
		DrawSpace (object);
		FinishSpace (object);
	}
	for (y = 0; y < work.h; y++)
		for (x = 0; x < work.w; x++)
		{
			v_get_pixel (vdi_handle, work.x + x, work.y + y, &pel, &color);
			pixels[y * work.w + x] = pel;
		}

	object->new = TRUE;
	DrawSpace (object);
	for (y = 0; y < work.h; y++)
		for (x = 0; x < work.w; x++)
		{
			v_get_pixel (vdi_handle, work.x + x, work.y + y, &pel, &color);
			if (pixels[y * work.w + x] != pel)
				failures++;
		}
	if (failures)
		fprintf (out, "FAIL CMO incremental redraw differs at %d pixels\n", failures);
	object->delete (object);
	vs_clip (vdi_handle, FALSE, clip);
	v_show_c (vdi_handle, 1);
	v_clsvwk (vdi_handle);
	appl_exit ();
	return failures ? 1 : 0;
}

static DISPOBJP drawn_object;
static int start_calls, draw_calls, finish_calls;

static VOID stub_start (DISPOBJP dispobj)
{
	start_calls++;
}

static VOID stub_draw (DISPOBJP dispobj)
{
	draw_calls++;
	drawn_object = dispobj;
}

static VOID stub_finish (DISPOBJP dispobj)
{
	finish_calls++;
	dispobj->new = FALSE;
}

static int test_update_dispobjs (FILE *out)
{
	static WINDOW window;
	static DISPOBJ unchanged, changed;
	WORD work_in[11], work_out[57], width, height, cell_width, cell_height, i;
	BOOLEAN drew;
	int failures = 0;

	if (appl_init () < 0)
	{
		fputs ("FAIL initializing AES for display update test\n", out);
		return 1;
	}
	vdi_handle = graf_handle (&width, &height, &cell_width, &cell_height);
	for (i = 0; i < 10; i++) work_in[i] = 1;
	work_in[10] = RC;
	v_opnvwk (work_in, &vdi_handle, work_out);

	window.opened = 1;
	window.handle = DESK;
	xywh2rect (64, 64, 64, 64, &window.scroll);
	window.dispobjs = list_create ();
	unchanged.start = changed.start = stub_start;
	unchanged.draw = changed.draw = stub_draw;
	unchanged.finished = changed.finished = stub_finish;
	list_rear_insert (window.dispobjs, list_new_el ((KEYTYPE) &unchanged));
	list_rear_insert (window.dispobjs, list_new_el ((KEYTYPE) &changed));

	drew = update_dispobjs (&window);
	if (drew || start_calls != 2 || draw_calls != 0 || finish_calls != 0)
	{
		fprintf (out, "FAIL unchanged display update drew=%d start=%d draw=%d finish=%d\n",
			drew, start_calls, draw_calls, finish_calls);
		failures++;
	}

	changed.new = TRUE;
	drew = update_dispobjs (&window);
	if (!drew || draw_calls == 0 || drawn_object != &changed || finish_calls != 1
		|| changed.new || unchanged.new)
	{
		fprintf (out, "FAIL changed display update drew=%d draw=%d finish=%d\n",
			drew, draw_calls, finish_calls);
		failures++;
	}

	list_destroy (window.dispobjs);
	if (vdi_handle) v_clsvwk (vdi_handle);
	appl_exit ();
	return failures;
}

int test_dispobj (FILE *out)
{
	static WORD xwin[800], ywin[800], xpersp[800], ypersp[800];
	static WORD xzoff[800], yzoff[800];
	static DISPOBJ object;
	static DISPOBJSTATUS state;
	POS_3D point;
	POINT_2D projected;
	int i, failures = 0;

	for (i = 0; i < 800; i++) xwin[i] = ywin[i] = 30000;
	for (i = 0; i < 400; i++)
	{
		xwin[i + 200] = ywin[i + 200] = i - 200;
		xpersp[i + 200] = ypersp[i + 200] = 128;
	}
	object.status = &state;
	object.work.x = object.work.y = -100;
	object.work.w = object.work.h = 200;
	state.space.xwin = &xwin[200];
	state.space.ywin = &ywin[200];
	state.space.xpersp = &xpersp[200];
	state.space.ypersp = &ypersp[200];
	state.space.xzoff = &xzoff[200];
	state.space.yzoff = &yzoff[200];

	point.x = 100;
	point.y = -100;
	point.z = 0;
	state.space.inside = TRUE;
	Project3D (&object, &point, &projected);
	if (projected.x != 100 || projected.y != -100)
	{
		fputs ("FAIL clipping projected coordinates before lookup\n", out);
		failures++;
	}

	point.x = 10;
	point.y = -20;
	point.z = 250;
	state.space.inside = FALSE;
	Project3D (&object, &point, &projected);
	if (projected.x != 10 || projected.y != -20)
	{
		fputs ("FAIL clipping depth before perspective lookup\n", out);
		failures++;
	}
	return failures + test_cmo_redraw (out) + test_top_view (out)
		+ test_update_dispobjs (out);
}
