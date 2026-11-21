#include <mint/osbind.h>

static void *test_dosound (void *sound);
static short test_giaccess (short value, short reg);

#undef Dosound
#undef Giaccess
#define Dosound(sound) test_dosound ((void *)(sound))
#define Giaccess(value, reg) test_giaccess ((short)(value), (short)(reg))
#define MidiGetTime test_midi_time
#include "../src/tra.c"

static STATUS state;
static RTMCLASS module;
static LONG now;
static unsigned char psg[16];
static unsigned short periods[16];
static int clicks;

long test_midi_time (void)
{
	return now;
}

static void *test_dosound (void *sound)
{
	unsigned char *script = sound;

	if (sound == (void *)DS_INQUIRE || !sound) return NULL;
	psg[PSG_APITCHLOW] = script[1];
	psg[PSG_APITCHHIGH] = script[3];
	psg[PSG_AVOLUME] = 0;
	if (clicks < 16)
		periods[clicks] = script[1] | (script[3] << 8);
	clicks++;
	return NULL;
}

static short test_giaccess (short value, short reg)
{
	short index = reg & 0x7f;
	if (reg & 0x80) psg[index] = (unsigned char)value;
	return psg[index];
}

static void service_at (LONG time)
{
	now = time;
	state.posit = time - time % QUANT;
	state.position_time = state.posit;
	service_click (&module);
}

int test_tra (FILE *out)
{
	int failures = 0;

	module.status = &state;
	state.play = TRUE;
	state.click = TRUE;
	psg[PSG_APITCHLOW] = 0x11;
	psg[PSG_APITCHHIGH] = 0x02;
	psg[PSG_MODE] = 0xff;
	psg[PSG_AVOLUME] = 5;

	service_at (0);
	service_at (100);
	if (clicks != 1 || psg[PSG_APITCHLOW] != 0x11 ||
	    psg[PSG_APITCHHIGH] != 0x02 || psg[PSG_MODE] != 0xff ||
	    psg[PSG_AVOLUME] != 5)
	{
		fputs ("FAIL first click or PSG restoration\n", out);
		failures++;
	}

	service_at (499);
	if (clicks != 1)
	{
		fputs ("FAIL premature click\n", out);
		failures++;
	}
	service_at (500);
	service_at (1000);
	service_at (1500);
	service_at (2000);
	if (clicks != 5 || periods[0] >= periods[1] ||
	    periods[1] != periods[2] || periods[2] != periods[3] ||
	    periods[4] != periods[0])
	{
		fputs ("FAIL 120 BPM click cadence or four-beat accent\n", out);
		failures++;
	}

	service_at (3100);
	if (clicks != 5)
	{
		fputs ("FAIL stale clicks after a stall\n", out);
		failures++;
	}
	service_at (3500);
	if (clicks != 6)
	{
		fputs ("FAIL click after a stall\n", out);
		failures++;
	}

	now = 4000;
	state.posit = 750;
	state.position_time = now;
	advance_generation (&state.click_generation);
	service_click (&module);
	if (clicks != 6)
	{
		fputs ("FAIL click immediately after an off-beat seek\n", out);
		failures++;
	}
	now = 4250;
	state.posit = 1000;
	state.position_time = now;
	service_click (&module);
	if (clicks != 7)
	{
		fputs ("FAIL click after a backward seek\n", out);
		failures++;
	}

	state.play = FALSE;
	service_at (4500);
	state.click = FALSE;
	state.play = TRUE;
	service_at (5000);
	if (clicks != 7)
	{
		fputs ("FAIL click while stopped or disabled\n", out);
		failures++;
	}

	return failures;
}
