/*****************************************************************************/
/*                                                                           */
/* Modul: MIDIKERN.C                                                         */
/*                                                                           */
/* Internal MIDI kernel: replaces the resident MidiShare kernel             */
/* (MIDSHARE.PRG) with an implementation of the API subset RTM uses,        */
/* linked into the program itself. External MIDI synchronisation (MTC/clock */
/* slaving) is not supported; the sync calls report internal sync.          */
/*                                                                           */
/* Services                                                                  */
/*   - millisecond timebase (MFP Timer A at ~1 kHz, drift-compensated)      */
/*   - MidiTask scheduler running at interrupt level on a private stack     */
/*     (the small system supervisor stack overflowed with earlier tasks)    */
/*   - deferred tasks (MidiDTask) executed from the main loop               */
/*   - interrupt-safe 16-byte cell pool for events and tasks                */
/*   - event routing between the in-process clients (TRA/VAR/PUF/...) with  */
/*     MidiShare connection and filter semantics                            */
/*   - physical MIDI out/in on the ACIA 6850 at $FFFC04/06 (port 0); the    */
/*     input hooks the system midivec and parses bytes into events          */
/*                                                                           */
/* Concurrency: the program runs in user mode and cannot mask interrupts,  */
/* so a busy/pending discipline is used instead. Every API call brackets    */
/* itself with mk_enter()/mk_leave() (an atomic counter). The timer         */
/* interrupt only touches the shared structures when the counter is zero;  */
/* otherwise it sets mk_pending and the work is run by mk_leave() in the    */
/* interrupted context. Byte rings to and from the ACIA are single-reader/ */
/* single-writer and need no locking.                                       */
/*                                                                           */
/*****************************************************************************/

#include <string.h>
#include <mint/osbind.h>

#include "import.h"
#include "msh_unit.h"

/****** TUNING ***************************************************************/

#define MK_CELLS      4096         /* 16-byte cells: events, tasks, sysex   */
#define MK_APPS       16           /* client slots incl. pseudo-app 0       */
#define MK_IRQ_STACK  8192         /* private supervisor stack for tasks    */
#define MK_STR_(x)    #x
#define MK_STR(x)     MK_STR_ (x)
#define MK_TXLEN      256          /* MIDI out ring (power of two)          */
#define MK_RXLEN      256          /* MIDI in ring (power of two)           */
#define MK_MAX_TASKS_PER_TICK 64   /* runaway brake                         */
#define MK_MAX_RX_PER_TICK    128
#define MK_SYSEX_MAX  256          /* longest accepted incoming sysex       */

/* MFP Timer A: 2457600 / 10 / 246 = 999.024 Hz. The ~0.0976% shortfall is
	almost exactly 1/1024, so every 1024th tick advances the clock by 2 ms. */
#define MK_TIMER_CTRL 2            /* prescaler /10                         */
#define MK_TIMER_DATA 246
#define MK_MFP_TIMER_A 13          /* MFP interrupt number for Jenabint     */
#define MK_VEC_TIMER_A 0x4D        /* exception vector number (0x134/4)     */

/****** HARDWARE *************************************************************/

#define ACIA_MIDI_CTRL (*(volatile unsigned char *) 0xFFFC04L)
#define ACIA_MIDI_DATA (*(volatile unsigned char *) 0xFFFC06L)
#define ACIA_TDRE      0x02

/****** TYPES ****************************************************************/

typedef union mkcell
{
	TMidiEv       ev;
	TMidiST       st;
	TMidiSEX      sex;
	union mkcell *next;
} MKCELL;

/* Header cell of a sysex/stream chain, viewed through info.linkSE */
typedef struct
{
	MidiSEXPtr first;              /* first data cell (12 bytes each)       */
	MidiSEXPtr last;               /* last data cell, for O(1) append       */
	long       count;              /* number of data bytes (fields)         */
	long       unused;
} MKSEXHEAD;

typedef struct
{
	Boolean         used;
	char            name [32];
	MkRcvAlarmPtr   rcvAlarm;
	MkApplAlarmPtr  applAlarm;
	FilterPtr       filter;
	unsigned short  dests;         /* bit per destination refNum            */
	MidiEvPtr       rxHead, rxTail;
	long            rxCount;
	MidiEvPtr       dtHead, dtTail;
	long            dtCount;
} MKAPP;

/****** VARIABLES ************************************************************/

static MKCELL  mk_pool [MK_CELLS];
static MKCELL *mk_free;
static long    mk_freecount;

static MKAPP   mk_app [MK_APPS];

static volatile long  mk_time;          /* milliseconds since start         */
static unsigned short mk_tickmod;       /* drift compensation counter       */

static volatile short mk_busy;          /* main context inside the kernel   */
static volatile short mk_pending;       /* ISR deferred its service work    */
static Boolean        mk_inited;

static MidiEvPtr      mk_tasks;         /* pending MidiTasks, sorted by date */

/* MIDI out/in byte rings (single producer/single consumer each) */
static volatile unsigned char  mk_tx [MK_TXLEN];
static volatile unsigned short mk_txw, mk_txr;
static volatile unsigned char  mk_rx [MK_RXLEN];
static volatile unsigned short mk_rxw, mk_rxr;

/* MIDI input parser state */
static unsigned char mk_in_status;      /* running status                   */
static unsigned char mk_in_data [2];
static short         mk_in_have, mk_in_need;
static MidiEvPtr     mk_in_sysex;       /* sysex being accumulated          */

/* saved system state */
static long  mk_old_timer_vec;
static void *mk_old_midivec;
static _KBDVECS *mk_kbdvecs;

/* referenced from the assembly stubs */
long          mk_saved_sp;
unsigned char mk_irq_stack [MK_IRQ_STACK];

static void mk_service (void);

/****** ATOMIC BRACKETS ******************************************************/

static void mk_enter (void)
{
	__asm__ volatile ("addq.w #1,%0" : "+m" (mk_busy));
} /* mk_enter */

static void mk_leave (void)
{
	for (;;)
	{
		__asm__ volatile ("subq.w #1,%0" : "+m" (mk_busy));
		if (mk_busy != 0 || !mk_pending)
			return;
		mk_pending = 0;                 /* run the deferred interrupt work  */
		__asm__ volatile ("addq.w #1,%0" : "+m" (mk_busy));
		mk_service ();
	} /* for */
} /* mk_leave */

/****** CELL POOL ************************************************************/

static MKCELL *mk_alloc (void)
{
	MKCELL *c = mk_free;
	if (c)
	{
		mk_free = c->next;
		mk_freecount--;
		memset (c, 0, sizeof (MKCELL));
	} /* if */
	return c;
} /* mk_alloc */

static void mk_release (MKCELL *c)
{
	c->next = mk_free;
	mk_free = c;
	mk_freecount++;
} /* mk_release */

/****** EVENTS ***************************************************************/

#define MK_IS_PRIVATE(t) ((t) >= typePrivate && (t) < 128)
#define MK_IS_SEX(t)     ((t) == typeSysEx || (t) == typeStream)

static MidiEvPtr mk_new_ev (short type)
{
	MKCELL *c = mk_alloc ();
	MidiEvPtr e;

	if (!c) return NULL;
	e = &c->ev;
	e->evType = (Byte) type;
	e->date   = mk_time;
	if (MK_IS_PRIVATE (type))
	{
		MKCELL *st = mk_alloc ();
		if (!st) { mk_release (c); return NULL; }
		e->info.linkST = &st->st;
	} /* if */
	else if (MK_IS_SEX (type))
	{
		MKCELL *h = mk_alloc ();
		if (!h) { mk_release (c); return NULL; }
		e->info.linkSE = (MidiSEXPtr) h;    /* MKSEXHEAD, zeroed            */
	} /* else if */
	return e;
} /* mk_new_ev */

static void mk_free_ev (MidiEvPtr e)
{
	if (!e) return;
	if (MK_IS_PRIVATE (e->evType))
	{
		if (e->info.linkST) mk_release ((MKCELL *) e->info.linkST);
	} /* if */
	else if (MK_IS_SEX (e->evType))
	{
		MKSEXHEAD *h = (MKSEXHEAD *) e->info.linkSE;
		if (h)
		{
			MidiSEXPtr s = h->first, n;
			while (s) { n = s->link; mk_release ((MKCELL *) s); s = n; }
			mk_release ((MKCELL *) h);
		} /* if */
	} /* else if */
	mk_release ((MKCELL *) e);
} /* mk_free_ev */

static void mk_add_field (MidiEvPtr e, long v)
{
	if (MK_IS_SEX (e->evType))
	{
		MKSEXHEAD *h = (MKSEXHEAD *) e->info.linkSE;
		short fill = (short) (h->count % 12);
		if (fill == 0)
		{
			MKCELL *c = mk_alloc ();
			if (!c) return;                  /* pool exhausted: drop byte   */
			if (h->last) h->last->link = &c->sex; else h->first = &c->sex;
			h->last = &c->sex;
		} /* if */
		h->last->data [fill] = (Byte) v;
		h->count++;
	} /* if */
	else if (MK_IS_PRIVATE (e->evType))
	{
		/* not used by RTM for private events; keep fields 0..3 reachable */
	} /* else if */
} /* mk_add_field */

static MidiEvPtr mk_copy_ev (MidiEvPtr e)
{
	MidiEvPtr c;

	if (!e) return NULL;
	c = mk_new_ev (e->evType);
	if (!c) return NULL;
	c->date = e->date;
	c->refNum = e->refNum;
	c->port = e->port;
	c->chan = e->chan;
	if (MK_IS_PRIVATE (e->evType))
		*c->info.linkST = *e->info.linkST;
	else if (MK_IS_SEX (e->evType))
	{
		MKSEXHEAD *h = (MKSEXHEAD *) e->info.linkSE;
		MidiSEXPtr s;
		long n;
		for (s = h->first, n = h->count; s && n > 0; s = s->link)
		{
			short i, lim = (short) (n > 12 ? 12 : n);
			for (i = 0; i < lim; i++) mk_add_field (c, s->data [i]);
			n -= lim;
		} /* for */
	} /* else if */
	else
		c->info = e->info;
	return c;
} /* mk_copy_ev */

/****** MIDI OUT (serializer + ACIA ring) ************************************/

static short mk_tx_room (void)
{
	return (short) (MK_TXLEN - 1 - ((mk_txw - mk_txr) & (MK_TXLEN - 1)));
} /* mk_tx_room */

static void mk_tx_put (unsigned char b)
{
	mk_tx [mk_txw & (MK_TXLEN - 1)] = b;
	mk_txw++;
} /* mk_tx_put */

static void mk_drain_tx (void)
{
	while (mk_txr != mk_txw && (ACIA_MIDI_CTRL & ACIA_TDRE))
	{
		ACIA_MIDI_DATA = mk_tx [mk_txr & (MK_TXLEN - 1)];
		mk_txr++;
	} /* while */
} /* mk_drain_tx */

static void mk_wire_out (MidiEvPtr e)
{
	unsigned char status = 0, d [2];
	short ndata = 0;
	unsigned char chan = (unsigned char) (e->chan & 15);

	switch (e->evType)
	{
		case typeNote:                       /* approximated as KeyOn       */
		case typeKeyOn:      status = 0x90 | chan; d[0] = e->info.note.pitch; d[1] = e->info.note.vel; ndata = 2; break;
		case typeKeyOff:     status = 0x80 | chan; d[0] = e->info.note.pitch; d[1] = e->info.note.vel; ndata = 2; break;
		case typeKeyPress:   status = 0xA0 | chan; d[0] = e->info.data[0]; d[1] = e->info.data[1]; ndata = 2; break;
		case typeCtrlChange: status = 0xB0 | chan; d[0] = e->info.data[0]; d[1] = e->info.data[1]; ndata = 2; break;
		case typeProgChange: status = 0xC0 | chan; d[0] = e->info.data[0]; ndata = 1; break;
		case typeChanPress:  status = 0xD0 | chan; d[0] = e->info.data[0]; ndata = 1; break;
		case typePitchWheel: status = 0xE0 | chan; d[0] = e->info.data[0]; d[1] = e->info.data[1]; ndata = 2; break;
		case typeSongPos:    status = 0xF2; d[0] = e->info.data[0]; d[1] = e->info.data[1]; ndata = 2; break;
		case typeSongSel:    status = 0xF3; d[0] = e->info.data[0]; ndata = 1; break;
		case typeClock:      status = 0xF8; break;
		case typeStart:      status = 0xFA; break;
		case typeContinue:   status = 0xFB; break;
		case typeStop:       status = 0xFC; break;
		case typeTune:       status = 0xF6; break;
		case typeActiveSens: status = 0xFE; break;
		case typeReset:      status = 0xFF; break;
		case typeQuarterFrame:
			status = 0xF1;
			d[0] = (unsigned char) (((e->info.note.pitch & 7) << 4) | (e->info.note.vel & 15));
			ndata = 1;
			break;
		case typeSysEx:
		case typeStream:
		{
			MKSEXHEAD *h = (MKSEXHEAD *) e->info.linkSE;
			MidiSEXPtr s;
			long n;
			short framed = (e->evType == typeSysEx);
			if (!h || mk_tx_room () < h->count + 2) return;   /* all or nothing */
			if (framed) mk_tx_put (0xF0);
			for (s = h->first, n = h->count; s && n > 0; s = s->link)
			{
				short i, lim = (short) (n > 12 ? 12 : n);
				for (i = 0; i < lim; i++) mk_tx_put (s->data [i]);
				n -= lim;
			} /* for */
			if (framed) mk_tx_put (0xF7);
			return;
		}
		default:
			return;                          /* internal event: not wire data */
	} /* switch */

	if (mk_tx_room () < 1 + ndata) return;
	mk_tx_put (status);
	if (ndata > 0) mk_tx_put (d[0]);
	if (ndata > 1) mk_tx_put (d[1]);
} /* mk_wire_out */

/****** ROUTING **************************************************************/

static Boolean mk_filter_ok (MKAPP *a, MidiEvPtr e)
{
	if (!a->filter) return TRUE;
	if (!IsAcceptedBit (a->filter->evType, (unsigned char) e->evType)) return FALSE;
	if (!IsAcceptedBit (a->filter->port, e->port)) return FALSE;
	if (e->evType < typeSongPos &&
	    !IsAcceptedBit (a->filter->channel, e->chan & 15)) return FALSE;
	return TRUE;
} /* mk_filter_ok */

/* Route one event from refNum to its destinations; consumes the event.
	Receive alarms fire synchronously, exactly like the resident kernel. */
static void mk_deliver (short refNum, MidiEvPtr e)
{
	unsigned short dests;
	unsigned short alarms = 0;
	short d;

	if (refNum < 0 || refNum >= MK_APPS || !mk_app [refNum].used)
	{
		mk_free_ev (e);
		return;
	} /* if */
	e->refNum = (Byte) refNum;
	dests = mk_app [refNum].dests;

	if (dests & 1)
		mk_wire_out (e);                     /* destination 0: the MIDI port */

	for (d = 1; d < MK_APPS; d++)
	{
		MKAPP *a = &mk_app [d];
		MidiEvPtr c;
		if (!(dests & (1 << d)) || !a->used) continue;
		if (!mk_filter_ok (a, e)) continue;
		c = mk_copy_ev (e);
		if (!c) break;                       /* pool exhausted               */
		c->link = NULL;
		if (a->rxTail) a->rxTail->link = c; else a->rxHead = c;
		a->rxTail = c;
		a->rxCount++;
		if (a->rcvAlarm) alarms |= (1 << d);
	} /* for */
	mk_free_ev (e);

	for (d = 1; d < MK_APPS; d++)
		if (alarms & (1 << d))
			mk_app [d].rcvAlarm ((short) d);
} /* mk_deliver */

/****** MIDI IN (parser) *****************************************************/

static void mk_in_event (MidiEvPtr e)
{
	if (e) mk_deliver (0, e);                /* from the port, app 0         */
} /* mk_in_event */

static void mk_in_voice (void)
{
	unsigned char st = mk_in_status;
	MidiEvPtr e = NULL;

	switch (st & 0xF0)
	{
		case 0x80: e = mk_new_ev (typeKeyOff); break;
		case 0x90: e = mk_new_ev (typeKeyOn); break;
		case 0xA0: e = mk_new_ev (typeKeyPress); break;
		case 0xB0: e = mk_new_ev (typeCtrlChange); break;
		case 0xC0: e = mk_new_ev (typeProgChange); break;
		case 0xD0: e = mk_new_ev (typeChanPress); break;
		case 0xE0: e = mk_new_ev (typePitchWheel); break;
	} /* switch */
	if (!e) return;
	e->chan = st & 0x0F;
	e->info.data [0] = mk_in_data [0];
	e->info.data [1] = (mk_in_need > 1) ? mk_in_data [1] : 0;
	mk_in_event (e);
} /* mk_in_voice */

static void mk_in_byte_parse (unsigned char b)
{
	if (b >= 0xF8)                           /* realtime, may interleave     */
	{
		MidiEvPtr e = NULL;
		switch (b)
		{
			case 0xF8: e = mk_new_ev (typeClock); break;
			case 0xFA: e = mk_new_ev (typeStart); break;
			case 0xFB: e = mk_new_ev (typeContinue); break;
			case 0xFC: e = mk_new_ev (typeStop); break;
			case 0xFE: e = mk_new_ev (typeActiveSens); break;
			case 0xFF: e = mk_new_ev (typeReset); break;
		} /* switch */
		mk_in_event (e);
		return;
	} /* if */

	if (b & 0x80)                            /* new status byte              */
	{
		if (mk_in_sysex) { mk_in_event (mk_in_sysex); mk_in_sysex = NULL; }
		mk_in_status = b;
		mk_in_have = 0;
		switch (b & 0xF0)
		{
			case 0xC0: case 0xD0: mk_in_need = 1; break;
			case 0xF0:
				switch (b)
				{
					case 0xF0: mk_in_sysex = mk_new_ev (typeSysEx); mk_in_status = 0; break;
					case 0xF1: mk_in_need = 1; break;
					case 0xF2: mk_in_need = 2; break;
					case 0xF3: mk_in_need = 1; break;
					case 0xF7: mk_in_status = 0; break;
					default:   mk_in_status = 0; break;
				} /* switch */
				break;
			default: mk_in_need = 2; break;
		} /* switch */
		return;
	} /* if */

	if (mk_in_sysex)                         /* sysex data byte              */
	{
		MKSEXHEAD *h = (MKSEXHEAD *) mk_in_sysex->info.linkSE;
		if (h->count < MK_SYSEX_MAX)
			mk_add_field (mk_in_sysex, b);
		return;
	} /* if */

	if (!mk_in_status) return;               /* data without status: drop    */
	mk_in_data [mk_in_have++] = b;
	if (mk_in_have < mk_in_need) return;
	mk_in_have = 0;

	if (mk_in_status < 0xF0)
		mk_in_voice ();                      /* running status stays armed   */
	else
	{
		MidiEvPtr e = NULL;
		switch (mk_in_status)
		{
			case 0xF1:
				e = mk_new_ev (typeQuarterFrame);
				if (e)
				{
					e->info.note.pitch = (mk_in_data [0] >> 4) & 7;
					e->info.note.vel = mk_in_data [0] & 15;
				} /* if */
				break;
			case 0xF2:
				e = mk_new_ev (typeSongPos);
				if (e) { e->info.data [0] = mk_in_data [0]; e->info.data [1] = mk_in_data [1]; }
				break;
			case 0xF3:
				e = mk_new_ev (typeSongSel);
				if (e) e->info.data [0] = mk_in_data [0];
				break;
		} /* switch */
		mk_in_event (e);
		mk_in_status = 0;
	} /* else */
} /* mk_in_byte_parse */

static void mk_parse_input (void)
{
	short n = MK_MAX_RX_PER_TICK;
	while (mk_rxr != mk_rxw && n-- > 0)
	{
		unsigned char b = mk_rx [mk_rxr & (MK_RXLEN - 1)];
		mk_rxr++;
		mk_in_byte_parse (b);
	} /* while */
} /* mk_parse_input */

/****** TASKS ****************************************************************/

/* A task is an event cell (date, refNum) with an ST cell holding
	proc, a1, a2, a3. */

static void mk_run_tasks (void)
{
	short n = MK_MAX_TASKS_PER_TICK;

	while (mk_tasks && mk_tasks->date <= (unsigned long) mk_time && n-- > 0)
	{
		MidiEvPtr t = mk_tasks;
		TMidiST *st = t->info.linkST;
		MkTaskPtr proc = (MkTaskPtr) st->ptr1;
		long date = (long) t->date;
		short ref = (short) t->refNum;
		long a1 = (long) st->ptr2, a2 = (long) st->ptr3, a3 = (long) st->ptr4;

		mk_tasks = t->link;
		mk_release ((MKCELL *) st);
		mk_release ((MKCELL *) t);
		proc (date, ref, a1, a2, a3);
	} /* while */
} /* mk_run_tasks */

static void mk_service (void)
{
	mk_run_tasks ();
	mk_parse_input ();
	mk_drain_tx ();
} /* mk_service */

/****** TIMER INTERRUPT ******************************************************/

/* C part of the Timer A interrupt, entered on the private stack. */
void mk_timer_tick (void)
{
	mk_time++;
	if (++mk_tickmod >= 1024)                /* 999.024 Hz -> add the lost ms */
	{
		mk_tickmod = 0;
		mk_time++;
	} /* if */

	mk_drain_tx ();                          /* byte rings are always safe   */

	if (mk_busy)
		mk_pending = 1;                      /* main is inside: defer        */
	else
		mk_service ();
} /* mk_timer_tick */

/* Interrupt entry: full register save, switch to the private stack (the
	system supervisor stack is small and the tasks need real stack room),
	call the C handler, end-of-interrupt for MFP Timer A. Timer A cannot
	re-enter itself (same IPL), so one saved stack pointer suffices. */
__asm__ (
	".text\n\t.even\n"
	".globl mk_timer_isr\n"
	"mk_timer_isr:\n\t"
	"movem.l %d0-%d7/%a0-%a6,-(%sp)\n\t"
	"move.l %sp,mk_saved_sp\n\t"
	"lea mk_irq_stack+" MK_STR (MK_IRQ_STACK) "-4,%sp\n\t"
	"jsr mk_timer_tick\n\t"
	"move.l mk_saved_sp,%sp\n\t"
	"movem.l (%sp)+,%d0-%d7/%a0-%a6\n\t"
	"bclr #5,0xfffffa0f\n\t"
	"rte\n");
extern void mk_timer_isr (void);

/* midivec hook: the system keyboard/MIDI interrupt calls this with the
	received byte in d0. Push it into the ring; parsing happens later. */
void mk_midi_in_byte (long b)
{
	if (((mk_rxw - mk_rxr) & (MK_RXLEN - 1)) < MK_RXLEN - 1)
	{
		mk_rx [mk_rxw & (MK_RXLEN - 1)] = (unsigned char) b;
		mk_rxw++;
	} /* if */
} /* mk_midi_in_byte */

__asm__ (
	".text\n\t.even\n"
	".globl mk_midivec\n"
	"mk_midivec:\n\t"
	"movem.l %d0-%d1/%a0-%a1,-(%sp)\n\t"
	"and.l #0xff,%d0\n\t"
	"move.l %d0,-(%sp)\n\t"
	"jsr mk_midi_in_byte\n\t"
	"addq.l #4,%sp\n\t"
	"movem.l (%sp)+,%d0-%d1/%a0-%a1\n\t"
	"rts\n");
extern void mk_midivec (void);

/****** API: ENVIRONMENT *****************************************************/

Boolean MidiShare (void)
{
	return mk_inited;
} /* MidiShare */

short MidiGetVersion (void)
{
	return 0x0100;                           /* internal kernel              */
} /* MidiGetVersion */

short MidiOpen (const char *name)
{
	short ref, d;

	if (!mk_inited) return MIDIerrSpace;
	mk_enter ();
	for (ref = 1; ref < MK_APPS; ref++)
		if (!mk_app [ref].used)
			break;
	if (ref >= MK_APPS) { mk_leave (); return MIDIerrSpace; }
	memset (&mk_app [ref], 0, sizeof (MKAPP));
	mk_app [ref].used = TRUE;
	strncpy (mk_app [ref].name, name ? name : "", 31);
	mk_leave ();

	for (d = 1; d < MK_APPS; d++)            /* tell the others              */
		if (d != ref && mk_app [d].used && mk_app [d].applAlarm)
			mk_app [d].applAlarm (d, ((long) ref << 16) | MIDIOpenAppl);
	return ref;
} /* MidiOpen */

void MidiClose (short refNum)
{
	short d;

	if (refNum <= 0 || refNum >= MK_APPS || !mk_app [refNum].used) return;
	mk_enter ();
	MidiFlushEvs (refNum);
	MidiFlushDTasks (refNum);
	mk_app [refNum].used = FALSE;
	for (d = 0; d < MK_APPS; d++)
		mk_app [d].dests &= ~(1 << refNum);
	mk_leave ();

	for (d = 1; d < MK_APPS; d++)
		if (mk_app [d].used && mk_app [d].applAlarm)
			mk_app [d].applAlarm (d, ((long) refNum << 16) | MIDICloseAppl);
} /* MidiClose */

short MidiGetNamedAppl (const char *name)
{
	short ref;

	for (ref = 1; ref < MK_APPS; ref++)
		if (mk_app [ref].used && strcmp (mk_app [ref].name, name) == 0)
			return ref;
	return MIDIerrIndex;
} /* MidiGetNamedAppl */

void MidiConnect (short src, short dest, Boolean state)
{
	if (src < 0 || src >= MK_APPS || dest < 0 || dest >= MK_APPS) return;
	if (state)
		mk_app [src].dests |= (1 << dest);
	else
		mk_app [src].dests &= ~(1 << dest);
} /* MidiConnect */

void MidiSetRcvAlarm (short refNum, MkRcvAlarmPtr alarm)
{
	if (refNum > 0 && refNum < MK_APPS) mk_app [refNum].rcvAlarm = alarm;
} /* MidiSetRcvAlarm */

void MidiSetApplAlarm (short refNum, MkApplAlarmPtr alarm)
{
	if (refNum > 0 && refNum < MK_APPS) mk_app [refNum].applAlarm = alarm;
} /* MidiSetApplAlarm */

void MidiSetFilter (short refNum, FilterPtr filter)
{
	if (refNum > 0 && refNum < MK_APPS) mk_app [refNum].filter = filter;
} /* MidiSetFilter */

Boolean MidiGetPortState (short port)
{
	return (port == 0);                      /* one hardware MIDI port       */
} /* MidiGetPortState */

long MidiFreeSpace (void)
{
	return mk_freecount * (long) sizeof (MKCELL);
} /* MidiFreeSpace */

long MidiTotalSpace (void)
{
	return (long) MK_CELLS * (long) sizeof (MKCELL);
} /* MidiTotalSpace */

/****** API: EVENTS **********************************************************/

MidiEvPtr MidiNewEv (short type)
{
	MidiEvPtr e;
	mk_enter ();
	e = mk_new_ev (type);
	mk_leave ();
	return e;
} /* MidiNewEv */

MidiEvPtr MidiCopyEv (void *ev)
{
	MidiEvPtr e = (MidiEvPtr) ev;
	MidiEvPtr c;
	mk_enter ();
	c = mk_copy_ev (e);
	mk_leave ();
	return c;
} /* MidiCopyEv */

void MidiFreeEv (void *ev)
{
	MidiEvPtr e = (MidiEvPtr) ev;
	mk_enter ();
	mk_free_ev (e);
	mk_leave ();
} /* MidiFreeEv */

void MidiSetField (void *ev, long f, long v)
{
	MidiEvPtr e = (MidiEvPtr) ev;
	if (!e || f < 0) return;
	if (MK_IS_PRIVATE (e->evType))
	{
		if (f < 4) ((long *) e->info.linkST) [f] = v;
	} /* if */
	else if (MK_IS_SEX (e->evType))
	{
		MKSEXHEAD *h = (MKSEXHEAD *) e->info.linkSE;
		MidiSEXPtr s = h->first;
		long i = f;
		if (f >= h->count) return;
		while (i >= 12 && s) { s = s->link; i -= 12; }
		if (s) s->data [i] = (Byte) v;
	} /* else if */
	else if (f < 4)
		e->info.data [f] = (Byte) v;
} /* MidiSetField */

long MidiGetField (void *ev, long f)
{
	MidiEvPtr e = (MidiEvPtr) ev;
	if (!e || f < 0) return 0;
	if (MK_IS_PRIVATE (e->evType))
		return (f < 4) ? ((long *) e->info.linkST) [f] : 0;
	if (MK_IS_SEX (e->evType))
	{
		MKSEXHEAD *h = (MKSEXHEAD *) e->info.linkSE;
		MidiSEXPtr s = h->first;
		long i = f;
		if (f >= h->count) return 0;
		while (i >= 12 && s) { s = s->link; i -= 12; }
		return s ? (long) (unsigned char) s->data [i] : 0;
	} /* if */
	return (f < 4) ? (long) (unsigned char) e->info.data [f] : 0;
} /* MidiGetField */

void MidiAddField (void *ev, long v)
{
	MidiEvPtr e = (MidiEvPtr) ev;
	if (!e) return;
	mk_enter ();
	mk_add_field (e, v);
	mk_leave ();
} /* MidiAddField */

long MidiCountFields (void *ev)
{
	MidiEvPtr e = (MidiEvPtr) ev;
	if (!e) return 0;
	if (MK_IS_SEX (e->evType))
		return ((MKSEXHEAD *) e->info.linkSE)->count;
	if (MK_IS_PRIVATE (e->evType)) return 4;
	return 2;
} /* MidiCountFields */

/****** API: TIME ************************************************************/

long MidiGetTime (void)
{
	return mk_time;
} /* MidiGetTime */

/****** API: SEND / RECEIVE **************************************************/

void MidiSendIm (short refNum, void *ev)
{
	MidiEvPtr e = (MidiEvPtr) ev;
	if (!e) return;
	mk_enter ();
	mk_deliver (refNum, e);
	mk_leave ();
} /* MidiSendIm */

long MidiCountEvs (short refNum)
{
	if (refNum <= 0 || refNum >= MK_APPS) return 0;
	return mk_app [refNum].rxCount;
} /* MidiCountEvs */

MidiEvPtr MidiGetEv (short refNum)
{
	MidiEvPtr e = NULL;
	MKAPP *a;

	if (refNum <= 0 || refNum >= MK_APPS) return NULL;
	a = &mk_app [refNum];
	mk_enter ();
	e = a->rxHead;
	if (e)
	{
		a->rxHead = e->link;
		if (!a->rxHead) a->rxTail = NULL;
		a->rxCount--;
		e->link = NULL;
	} /* if */
	mk_leave ();
	return e;
} /* MidiGetEv */

void MidiFlushEvs (short refNum)
{
	MidiEvPtr e;
	while ((e = MidiGetEv (refNum)) != NULL)
		MidiFreeEv (e);
} /* MidiFlushEvs */

/****** API: TASKS ***********************************************************/

static MidiEvPtr mk_make_task (MkTaskPtr proc, long date, short refNum,
                               long a1, long a2, long a3, Byte type)
{
	MKCELL *c = mk_alloc (), *st;
	MidiEvPtr t;

	if (!c) return NULL;
	st = mk_alloc ();
	if (!st) { mk_release (c); return NULL; }
	t = &c->ev;
	t->evType = type;
	t->date = date;
	t->refNum = (Byte) refNum;
	t->info.linkST = &st->st;
	st->st.ptr1 = (Ptr) proc;
	st->st.ptr2 = (Ptr) a1;
	st->st.ptr3 = (Ptr) a2;
	st->st.ptr4 = (Ptr) a3;
	return t;
} /* mk_make_task */

MidiEvPtr MidiTask (MkTaskPtr proc, long date, short refNum, long a1, long a2, long a3)
{
	MidiEvPtr t, p;

	mk_enter ();
	t = mk_make_task (proc, date, refNum, a1, a2, a3, typeProcess);
	if (t)
	{
		if (!mk_tasks || mk_tasks->date > t->date)
		{
			t->link = mk_tasks;
			mk_tasks = t;
		} /* if */
		else
		{
			p = mk_tasks;                    /* insert after equal dates     */
			while (p->link && p->link->date <= t->date) p = p->link;
			t->link = p->link;
			p->link = t;
		} /* else */
	} /* if */
	mk_leave ();
	return t;
} /* MidiTask */

MidiEvPtr MidiDTask (MkTaskPtr proc, long date, short refNum, long a1, long a2, long a3)
{
	MidiEvPtr t = NULL;
	MKAPP *a;

	if (refNum <= 0 || refNum >= MK_APPS) return NULL;
	a = &mk_app [refNum];
	mk_enter ();
	t = mk_make_task (proc, date, refNum, a1, a2, a3, typeDProcess);
	if (t)
	{
		t->link = NULL;
		if (a->dtTail) a->dtTail->link = t; else a->dtHead = t;
		a->dtTail = t;
		a->dtCount++;
	} /* if */
	mk_leave ();
	return t;
} /* MidiDTask */

long MidiCountDTasks (short refNum)
{
	if (refNum <= 0 || refNum >= MK_APPS) return 0;
	return mk_app [refNum].dtCount;
} /* MidiCountDTasks */

void MidiExec1DTask (short refNum)
{
	MidiEvPtr t = NULL;
	MKAPP *a;
	MkTaskPtr proc = NULL;
	long date = 0, a1 = 0, a2 = 0, a3 = 0;

	if (refNum <= 0 || refNum >= MK_APPS) return;
	a = &mk_app [refNum];
	mk_enter ();
	t = a->dtHead;
	if (t)
	{
		TMidiST *st = t->info.linkST;
		a->dtHead = t->link;
		if (!a->dtHead) a->dtTail = NULL;
		a->dtCount--;
		proc = (MkTaskPtr) st->ptr1;
		date = (long) t->date;
		a1 = (long) st->ptr2; a2 = (long) st->ptr3; a3 = (long) st->ptr4;
		mk_release ((MKCELL *) st);
		mk_release ((MKCELL *) t);
	} /* if */
	mk_leave ();
	if (proc) proc (date, refNum, a1, a2, a3);
} /* MidiExec1DTask */

void MidiFlushDTasks (short refNum)
{
	MKAPP *a;

	if (refNum <= 0 || refNum >= MK_APPS) return;
	a = &mk_app [refNum];
	mk_enter ();
	while (a->dtHead)
	{
		MidiEvPtr t = a->dtHead;
		a->dtHead = t->link;
		mk_release ((MKCELL *) t->info.linkST);
		mk_release ((MKCELL *) t);
	} /* while */
	a->dtTail = NULL;
	a->dtCount = 0;
	mk_leave ();
} /* MidiFlushDTasks */

/****** API: SYNC (internal only) ********************************************/

void MidiGetSyncInfo (SyncInfoPtr p)
{
	memset (p, 0, sizeof (TSyncInfo));
	p->time = mk_time;
	p->syncMode = MIDISyncInternal;
} /* MidiGetSyncInfo */

void MidiSetSyncMode (unsigned short mode)
{
	(void) mode;                             /* external sync not supported  */
} /* MidiSetSyncMode */

long MidiGetExtTime (void)
{
	return mk_time;                          /* internal sync: same clock    */
} /* MidiGetExtTime */

/****** LIFECYCLE ************************************************************/

static long mk_install_sup (void)
{
	mk_old_timer_vec = *(volatile long *) (MK_VEC_TIMER_A * 4L);
	return 0;
} /* mk_install_sup */

short MidiInternalStart (void)
{
	long i;

	if (mk_inited) return TRUE;

	/* cell pool */
	mk_free = NULL;
	for (i = MK_CELLS - 1; i >= 0; i--)
	{
		mk_pool [i].next = mk_free;
		mk_free = &mk_pool [i];
	} /* for */
	mk_freecount = MK_CELLS;

	memset (mk_app, 0, sizeof (mk_app));
	mk_app [0].used = TRUE;                  /* pseudo-app 0: the MIDI port  */
	strcpy (mk_app [0].name, "MidiShare");

	mk_time = 0;
	mk_tasks = NULL;
	mk_busy = mk_pending = 0;
	mk_txw = mk_txr = mk_rxw = mk_rxr = 0;
	mk_in_status = 0; mk_in_have = mk_in_need = 0; mk_in_sysex = NULL;

	/* MIDI input: hook the system midivec */
	mk_kbdvecs = Kbdvbase ();
	mk_old_midivec = (void *) mk_kbdvecs->midivec;
	mk_kbdvecs->midivec = (void (*) (void)) mk_midivec;

	/* timebase: MFP Timer A at ~1 kHz */
	Supexec (mk_install_sup);
	Xbtimer (0 /* Timer A */, MK_TIMER_CTRL, MK_TIMER_DATA, mk_timer_isr);
	Jenabint (MK_MFP_TIMER_A);

	mk_inited = TRUE;
	return TRUE;
} /* MidiInternalStart */

static long mk_remove_sup (void)
{
	*(volatile long *) (MK_VEC_TIMER_A * 4L) = mk_old_timer_vec;
	return 0;
} /* mk_remove_sup */

void MidiInternalStop (void)
{
	if (!mk_inited) return;
	Jdisint (MK_MFP_TIMER_A);
	Supexec (mk_remove_sup);
	if (mk_kbdvecs)
		mk_kbdvecs->midivec = (void (*) (void)) mk_old_midivec;
	mk_inited = FALSE;
} /* MidiInternalStop */
