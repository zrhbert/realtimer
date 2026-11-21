#include "../src/dispobj.c"

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
	return failures + test_cmo_redraw (out);
}
