/*****************************************************************************/
/*                                                                           */
/* Modul: ED4.C                                                              */
/*                                                                           */
/* 4D-Editor                                                                 */
/*                                                                           */
/*****************************************************************************/
#define ED4VERSION "V 1.00"
#define ED4DATE "04.03.95"

/*****************************************************************************
V 1.00
- VAR_MAE_SPERRE_AUSSEN FALSE, 04.03.95
- umstellung auf RTM_POSIT etc., 19.02.95
V 0.03
- ClickSetupField eingebaut, 30.01.95
- MAXPERCENT ausgebaut, 03.01.95
- CreateStandardEvent eingebaut und Standard-Event neu positioniert
V 0.02	
- Message-Funktionen erweitert
V 0.01	28.11.94
*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"
#include "time.h"

#include "realtim4.h"
#include "ed4_mod.h"
#include "realtspc.h"
#include "var.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"
#include "dispobj.h"

#include "msh_unit.h"
#include "msh.h"

#include "export.h"
#include "ed4.h"

#if XRSC_CREATE
#include "ed4_mod.rsh"
#include "ed4_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|SIZER)
#define FLAGS  (WI_RESIDENT | WI_MOUSE | WI_NOSCROLL)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (620)				         /* Anfangsbreite in Pixel */
#define INITH  (400)         				/* Anfangshîhe in Pixel */
#define MILLI  100                    	/* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "ED4_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 100L					/* Anzahl der ED4-Setups */

#define DIRECT_CONTROL 1					/* Steuerung direkt statt Åber VAR an/aus */

enum koor_states 
{USED, NEW, FREE};		/* Zustand der Koordinaten in den zwei Gruppen */

enum DTaskTypes {ED4DTaskReset, ED4DTaskPrecalc};	/* Delayed Task Typen */

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "ED4";		/* Name, fÅr Extension etc. */

/* Umwandlung von interner Koordinatendarstellung auf Midi-Format */
#define INTERN_TO_MIDI(koor) 64 + koor

/****** TYPES ****************************************************************/

typedef struct status *STAT_P;

typedef struct status
{
	UINT	editing		: 1	;	/* Event wird gerade eingefÅgt */
	UINT	play			: 1	;	/* PLAY gedrÅckt */
	UINT	record		: 1	;	/* RTM Record an/aus */
	UINT	ed4_record	: 1	;	/* ED4 Record an/aus */
	UINT	cycle			: 1	;	/* Cycle-Modus an/aus*/
	UINT	sync			: 1	;	/* synchron mitlaufen */
	UINT	note_off		: 1	;	/* Note-Off Events schicken */
	LONG	leftloc;					/* Linker Locator */
	LONG	rightloc;				/* Rechter Locator */
	LONG	posit;					/* SMPTE Zeit	*/
	BOOLEAN	new;					/* Anzeige neu aufbauen */
	BOOLEAN	midi_new;			/* Koordinaten neu ausgeben */
	BOOLEAN	zeitl;				/* Zeitlupe */
	BOOLEAN	pause;				/* Pause: Sequencer anhalten */
	BOOLEAN	reset_flag;	/* Parameter wurden zurueckgesetzt */
	WINDOWP		refwindow;
	RTMCLASSP	refmodule;
	SO_P			cue_list;		/* Kopf fÅr Liste der Sound-Events */

	WORD			z_cursor;		/* Durchlaufender Cursor */

	POS_3D		speed;			/* Koor/Quant */
	WORD			pos_x,			/* Vorwahl fÅr X-Pos des nÑchsten Objektes */
					pos_y;			/* Vorwahl fÅr Y-Pos des nÑchsten Objektes */
	WORD			volume;			/* Vorwahl fÅr Volume des nÑchsten Objektes */
	WORD			input_ch;		/* Vorwahl fÅr Input-Ch am CM des nÑchsten Objektes */

	BOOLEAN	anzeige[MAXSIGNALS],		/* Flags fÅr Anzeige an/aus */
				fadenkreuz[MAXSIGNALS]; /* Flags fÅr Fadenkreuz pro Signal */

	PUF_INF		tmp_event;		/* TemporÑrer Event fÅr play_task */
	KOOR_ALL		tmp_koors;		/*     "      Koordinaten */	
	TRACK_ALL	tmp_tracks;		/*     "      Tracks */
	VOL_ALL		tmp_vols;		/*     "      BIG-Volumes */
	PUF_INF		old_event;		/* Voriger Event fÅr play_task */
	KOOR_ALL		old_koors;		/*     "      Koordinaten */	
	TRACK_ALL	old_tracks;		/*     "      Tracks */
	VOL_ALL		old_vols;		/*     "      BIG-Volumes */
	TFilter		filter;				/* Midi-In-Filter	*/
	clock_t		start_cl,		/* Messung: Interrupt-Start */
 					stop_cl,			/* 			Interrupt-Ende */
					last_update;	/* Messung fÅr Intervalldauer fÅr Fenster-Update */
	RTMCLASSP	manmodule,		/* Zugehîriges MAN-Modul */
					tramodule,		/* Zugehîriges TRA-Modul */
					varmodule;		/* Zugehîriges VAR-Modul */
} STATUS;

typedef struct setup *SET_P;
typedef struct setup
{
	WORD		rot_x,						/* Grafik-Parameter: Rotationen */
				rot_y,
				rot_z,
				distanz,						/* Distanz */
				persp;						/* Perspektive */
	WORD		modus;				/* In diesem F. benutzte Darstellung */
	BOOLEAN	innenraum;					/* Nur Innenraum anzeigen */
	BOOLEAN	pfeile;						/* Nur Innenraum anzeigen */
	WORD		raumform;					/* Raumform fÅr dieses Fenster */
	RTMCLASSP	refmodule;				/* Das Bezugsmodul */
} SETUP;	/* EnthÑlt alle Parameter einer kompletten ED4-Einstellung */

/****** VARIABLES ************************************************************/
PRIVATE WORD	ed4_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*ed4_rsc_ptr = &ed4_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *ed4_menu;
PRIVATE OBJECT *ed4_setup;
PRIVATE OBJECT *ed4_shelp;
PRIVATE OBJECT *ed4_help;
PRIVATE OBJECT *ed4_desk;
PRIVATE OBJECT *ed4_text;
PRIVATE OBJECT *ed4_info;
PRIVATE OBJECT *ed4_raum;

PRIVATE RTMCLASSP	modulep[MAXMSAPPLS];			/* Zeiger auf Modul-Strukturen */
PRIVATE WORD		refNums[1];						/* Referenznummern */

/****** FUNCTIONS ************************************************************/

/* MidiShare Funktionen */
PUBLIC VOID			cdecl	receive_evts_ed4	_((SHORT refNum));
PUBLIC VOID			cdecl play_task_ed4		_((LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3));
PUBLIC VOID			cdecl delayed_task_ed4	_((LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3));
PRIVATE VOID		InstallFilter				_((SHORT refNum));

/* Interne ED4-Funktionen */
PRIVATE VOID		dsetup			_((WINDOWP refwindow));
PRIVATE VOID		click_setup		_((WINDOWP window, MKINFO *mk));
PRIVATE RTMCLASSP define_setup	_((WINDOWP window, RTMCLASSP refmodule));
PUBLIC VOID    	get_edit_setup		_((RTMCLASSP module));
PUBLIC BOOLEAN		set_setnr_setup	_((RTMCLASSP module, LONG setupnr));
PRIVATE BOOLEAN midi_out_ed4 (RTMCLASSP module, PUF_INF *akt, PUF_INF *alt, BOOLEAN force);

PRIVATE VOID 	position					_((RTMCLASSP module, LONG smpte));
PRIVATE VOID	start_record 		_((RTMCLASSP module));
PRIVATE VOID	tempo_up				_((RTMCLASSP module));
PRIVATE VOID	tempo_down			_((RTMCLASSP module));
PRIVATE BOOLEAN midi_out			_((RTMCLASSP module));
PRIVATE BOOLEAN send_event			_((RTMCLASSP module, INT miditrack, INT vel));
PRIVATE BOOLEAN send_stop 			_((RTMCLASSP module));
PRIVATE VOID	init_messages 		_((RTMCLASSP module));
PRIVATE VOID	init_standard 		_((RTMCLASSP module));
PRIVATE VOID	init_events 		_((RTMCLASSP module));
PRIVATE SHORT	init_midishare 	_((VOID));

PRIVATE VOID create_displayobs (WINDOWP window);

PRIVATE BOOLEAN	init_rsc			_((VOID));
PRIVATE BOOLEAN	term_rsc			_((VOID));

/* Sound Object Handling */
PRIVATE SO_P PrevSO (SO_P event);
PRIVATE SO_P NextSO (SO_P event);
PRIVATE SO_P CreateSO (VOID);
PRIVATE VOID DestroySO (SO_P event);
PRIVATE SO_P RemoveSO (SO_P event);
PRIVATE SO_P InsertSO (SO_P prev, SO_P event);
PRIVATE SO_P CopySO (SO_P dest, SO_P source);
PRIVATE SO_P CreateCopySO (SO_P source);
PRIVATE LONG CountSO (SO_P header);

PRIVATE SO_P FindSOByTime (LONG location, SO_P header);
PRIVATE SO_P NextSOByTime (LONG location, SO_P header);
PRIVATE SO_P PrevSOByTime (LONG location, SO_P header);
PRIVATE SO_P InsertSOByTime (LONG location, SO_P event, SO_P header);
PRIVATE SO_P InsertSOByOwnTime (SO_P event, SO_P header);

PRIVATE VOID ComputeSOTimes (SO_P event);
PRIVATE SO_P CreateStandardEvent (RTMCLASSP module);

PRIVATE VOID init_standard (RTMCLASSP module);
/*** Sound-Objekt Handling ***********************************************************/

PRIVATE SO_P PrevSO (SO_P event)
{
	return event->prev;
} /* PrevSO */

PRIVATE SO_P NextSO (SO_P event)
{
	return event->next;
} /* NextSO */

PRIVATE SO_P CreateSO ()
{
	SO_P	event, next;

	event = new(SOUNDOBJ);

	if(!event)
	{
	  	hndl_alert (ERR_NOMEMORY);
	  	return FALSE;
	} /* if */

	mem_set(event, 0, (UWORD)sizeof(SOUNDOBJ));		
	return event;

} /* CreateSO */

PRIVATE VOID DestroySO (SO_P event)
{
	if (event)
	{
		/* Noch ausklinken ? */
		if (event->next)
		{
			RemoveSO (event);
		} /* if next */
		
		/* Speicher freigeben */
		mem_free (event);
	} /* if event */
} /* DestroySO */
		
PRIVATE SO_P RemoveSO (SO_P event)
{
	SO_P	next,	prev;
	
	if (event)
	{
		prev = event->prev;
		next = event->next;

		/* Nachbarn verketten */
		prev->next = next;
		next->prev = prev;
		
		/* Event ausklinken */
		event->prev = NULL;
		event->next = NULL;
		return event;
	} /* if event */
	
	return NULL;
} /* RemoveSO */

PRIVATE SO_P InsertSO (SO_P prev, SO_P event)
{
	SO_P next = prev->next;
	
	/* Event einklinken */
	event->prev	= prev;
	event->next	= next;
	prev->next	= event;
	next->prev	= event;

	return event;
	
} /* InsertSO */

PRIVATE SO_P CopySO (SO_P dest, SO_P source)
{
	/* Kopiert alle Infos auûer Verkettung */
	SO_P prev = dest->prev, next = dest->next;
	
	/* Info komplett kopieren */
	*dest = *source;
	
	/* Verkettung wiederherstellen */
	dest->prev = prev;
	dest->next = next;

	return dest;
} /* CopySO */

PRIVATE SO_P CreateCopySO (SO_P source)
{
 	/* Kopiert alle Infos in einen neuen Event */
	SO_P event;
	
	event = CreateSO();
	if (event) CopySO (event, source);
	
	return event;
} /* CreateCopySO */

PRIVATE LONG CountSO (SO_P header)
{
	SO_P event = NextSO(header);
	LONG num = 1;
	
	while (event != header)
	{
		num++;
		event = NextSO(event);
	} /* while */
	
	return num;
} /* CountSO */

/****** By-Time Funktionen ********/

PRIVATE SO_P FindSOByTime (LONG location, SO_P header)
{
	/* Findet letzten Event vor oder auf angegebener Zeit */
	SO_P event, next;
	
	if (location <= header->cue_time)
		return header;

	event = header;
	next = NextSO (event);

	/* Noch nicht am Ende und immer noch nicht da? */
	while ((next != header) &&  (next->cue_time <= location))
	{
		event = next;
		next = NextSO (event);
	}
	
	return event;
} /* FindSOByTime */

PRIVATE SO_P NextSOByTime (LONG location, SO_P header)
{
	SO_P event;

	event = FindSOByTime (location, header);
	event = NextSO (event);
	
	return event;
} /* NextSOByTime */

PRIVATE SO_P PrevSOByTime (LONG location, SO_P header)
{
	SO_P event;

	event = FindSOByTime (location, header);
	event = PrevSO (event);
	
	return event;
} /* PrevSOByTime */

PRIVATE SO_P InsertSOByTime (LONG location, SO_P event, SO_P header)
{
	SO_P	prev;
	
	prev = FindSOByTime (location, header);
	InsertSO (prev, event);
	
	return event;
} /* InsertSOByTime */

PRIVATE SO_P InsertSOByOwnTime (SO_P event, SO_P header)
{
	return InsertSOByTime (event->cue_time, event, header);
} /* InsertSOByOwnTime */

PRIVATE VOID ComputeSOTimes (SO_P event)
{
	/* Berechnet entry und exit time fÅr 
		gegebenen CUE-Punkt und Geschwindigkeit */
		
	LONG duration;
	
	duration = 2*MAXKOOR / abs(event->speed.z) * QUANT;
	event->entry_time = event->cue_time - duration/2;
	event->exit_time	= event->cue_time + duration/2;

} /* ComputeSOTimes */

PRIVATE SO_P CreateStandardEvent (RTMCLASSP module)
{
	STAT_P		status = module->status;
	SO_P	event;
	WORD			signal;
	
	event = CreateSO ();
	if (event)
	{
		/* Auf akt. Werte setzen */
		event->entry_time 	= status->posit;
		/* DIV/0 verhindern durch Default Speed */
		event->speed.z			= (status->speed.z!=0) ? status->speed.z : 1;
		event->position[0].x = status->pos_x - 15;
		event->position[0].y = status->pos_y;
		event->position[1].x = 30;
		event->position[1].y = 0;
		event->position[1].z = 0;
		event->volume 			= status->volume;
		event->input_ch[0]	= status->input_ch;
		event->input_ch[1]	= status->input_ch+1;
		/* Restl. Signale abschalten */
		for (signal = 2; signal < MAXSIGNALS; signal++)
			event->input_ch[signal]	= -1;

		/* Cue-time ist erwarteter Nulldurchgang bei momentaner Geschw. */
		event->cue_time 		= MAXKOOR / abs(event->speed.z) * QUANT + status->posit;

		ComputeSOTimes (event);					
		
		/* Semaphore setzen */
		status->editing = TRUE;
		InsertSOByOwnTime (event, status->cue_list);
		status->input_ch = status->input_ch++%MAXINPUTS;
		/* Semaphore freigeben */
		status->editing = FALSE;

		return event;
	} /* if event */
	return NULL;
} /* CreateStandardEvent */

/*** Midi-Event-Verwaltung **************************************************************/

PUBLIC VOID cdecl receive_evts_ed4 (int refNum)
{
	MidiEvPtr	event;
	LONG 			n;
	INT 			r;
	MidiEvPtr	myTask;
	RTMCLASSP	module = modulep[refNum], man = module->status->manmodule;
	STAT_P		status = module->status;
	WINDOWP		window = module->window;
	
	r = refNum;
	for (n = MidiCountEvs(r); n > 0; --n) 	/* Alle empfangenen Events abarbeiten */
	{
		event = MidiGetEv (r);				/*  Information holen */
		switch (EvType(event))
		{
/*
			case typeRTMPosit:
				if (status->sync)
				{
					status->posit 		= get_posit((MidiSTPtr)event);
					position(module, status->posit);    /* find actual position */
					module->reset(module);
					status->new	  		= TRUE;
					status->midi_new  = TRUE;
					window->milli = 1;
				} /* if */
				break;
			case typeRTMCont:
				if (status->sync)
				{
					status->posit 		= get_posit((MidiSTPtr)event);
					status->play = TRUE;
					myTask = MidiTask(play_task_ed4, MidiGetTime(), refNum, 0, 0, 0);
					window->milli = 1;
					status->new	  = TRUE;
					status->midi_new  = TRUE;
				} /* if */
				break;
			case typeRTMStop:
				if (status->sync)
				{
					module->reset(module);
					status->posit 		= get_posit((MidiSTPtr)event);
					if (status->play)
					{
						if (man>0)						/* MAN-Modul vorhanden ? */
							if (man->reset >0)		/* MAN reset-Funktion da? */
								(man->reset)(man);
							send_stop (module);
					}
					status->play = FALSE;
					window->milli = 1;
					status->new	  = TRUE;
					status->midi_new  = TRUE;
				} /* if */
				break;
			case typeRTMCycleSet:
				status->leftloc 		= get_cycle_start((MidiSTPtr)event);
				status->rightloc		= get_cycle_end((MidiSTPtr)event);
				window->milli = 1;
				break;
			case typeRTMCycleOnOff:
				status->cycle 			= (UWORD) get_cycle_on((MidiSTPtr)event);
				window->milli = 1;
				break;
			case typeRTMRecordOnOff:
				status->record 		= (BOOLEAN) get_record_on((MidiSTPtr)event);
				status->new	  = TRUE;
				status->midi_new  = TRUE;
				window->milli = 1;
				break;
*/
		} /* switch */
		MidiFreeEv (event);
	} /* for */
} /* receive_evts_ed4 */

PUBLIC VOID cdecl delayed_task_ed4 (LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3)
{
	/* Wird aufgerufen, um nicht EchtzeitfÑhige Funktionen auszufÅhren */
	RTMCLASSP	module 	= modulep[refNum];
	WORD action = (WORD)a1;

	switch (action)
	{
		case ED4DTaskReset:
			module->reset (module);
			break;
		case ED4DTaskPrecalc:
			module->precalc (module);
			break;
	} /* switch */
} /* delayed_task_ed4 */

PUBLIC VOID cdecl play_task_ed4 (LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3)
{
	/* Wird soundso oft aufgerufen, um neue Daten in
		das Fenster einzublenden und neue Koordinaten zu berechnen und speichern */

	RTMCLASSP	module 	= modulep[refNum];
	STAT_P		status 	= module->status;
	SET_P			akt 		= module->actual->setup;
	WINDOWP		window 	= module->window;
	REG WORD 	signal;
	PUF_INF		*tmp = &module->status->tmp_event;
	PUF_INF		*old = &module->status->old_event;
	REG KOOR_SINGLE	*koor;			/* Koordinaten-Struktur */
	REG TRACK_SINGLE	*track;			/* Spur-Zuweisung */
	REG TRACK_SINGLE	*vol;				/* Volume-Info */
	MidiEvPtr	myTask;
	BOOLEAN		ret,
					record 		= status->record,
					force			= status->new;

#ifdef TEST
	/* Zeitmessung */
	status->start_cl = MidiGetTime();
#endif

	if (refNum > 0 && status->play)
	{
		window->milli = 1; /* So bald wie mîglich updaten */
	
		/* Wenn weiterhin aufgenommen/gespielt werden soll, 
			muû der Task wieder eingeklinkt werden */
		if (status->play)
		{
			/* Vor dem nÑchsten apply die Daten berechnen */
			myTask = MidiDTask(delayed_task_ed4, MidiGetTime() + QUANT/2, refNum, (LONG)ED4DTaskPrecalc, 0, 0);
			myTask = MidiTask(play_task_ed4, MidiGetTime() + QUANT, refNum, 0, 0, 0);
		} /* if play */
		
		/* Zeit weiterzÑhlen */
		status->posit += QUANT;

/*
		/* Cycle Mode Handling */
		if (status->posit > status->rightloc)
		{
			if (status->cycle)
			{
				status->posit = status->leftloc;
			} /* if */
			else
				rtm_stop(refNum, status->posit); /* ... oder anhalten */
		} /* if */
*/

		/* Zeiger auf Event setzen */
		koor		= tmp->koors->koor;
		track		= tmp->tracks->track;
		vol		= tmp->volumes->volume;

		/* Event aufbauen */
		mem_set (koor,  0, (UWORD) sizeof(KOOR_ALL));
		mem_set (track, -1, (UWORD) sizeof(TRACK_ALL));
		mem_set (vol,   0, (UWORD) sizeof(VOL_ALL));
		
		if (!status->editing)
		{
			module->apply (module, tmp);
			
			if (status->posit != 0)
			{
				/* Midi-Ausgabe aufrufen mit neuem Event */
				ret = midi_out_ed4(module, tmp, old, force);
			} /* if */
			else	
			{
				/* Midi-Ausgabe aufrufen mit NULL Event */
				mem_set (koor, 0, (UWORD) sizeof(KOOR_ALL));
				mem_set (track, 0, (UWORD) sizeof(TRACK_ALL));
				mem_set (vol, 0, (UWORD) sizeof(VOL_ALL));
				/* Midi-Ausgabe aufrufen mit altem und neuen Event */
				ret = midi_out_ed4(module, tmp, NULL, TRUE);
			} /* else */
			
			/* Alte Werte kopieren */
			*old->koors = *tmp->koors;
			*old->tracks = *tmp->tracks;
			*old->volumes = *tmp->volumes;
		} /* if not editing */
	} /* if */

	if (!ret)
	{
		/* hndl_alert_obj (module, ERR_MIDISHAREFULL); */
		status->play = FALSE;
	} /* if */
	
	window->milli = 1;
#ifdef TEST
	/* Zeitmessung */
	status->stop_cl = MidiGetTime();
#endif
} /* play_task_ed4 */

PRIVATE VOID position (RTMCLASSP module, LONG posit)
{
	/* Locator auf SMPTE-Zeit setzen */
	STAT_P		status = module->status;

	status->posit = posit;
	status->new = TRUE;					/* Koor neu ausgeben */
	status->midi_new  = TRUE;
} /* position */

PRIVATE VOID InstallFilter (SHORT refNum)
{
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	TFilter		*filter = &status->filter;

	register int i;

	for (i = 0; i<256; i++)
	{ 										
		AcceptBit(filter->evType,i);		/* accepte tous les types d'ÇvÇnements	*/
		AcceptBit(filter->port,i);		/* en provenance de tous les ports		*/
	} /* for */
											
	for (i = 0; i<16; i++)
		AcceptBit(filter->channel,i);	/* et sur tous les canaux Midi		*/
		
	MidiSetFilter( refNum, filter );   /* installe le filtre				*/
} /* InstallFilter */

PRIVATE	VOID start_record (RTMCLASSP module)
{
	STAT_P		status = module->status;
	SHORT			refNum = (SHORT)module->special;
	
	rtm_pos(refNum, 0L);
	send_variable(VAR_RECORD, TRUE);		/* RECORD  fÅr alle Module */
} /* start_record */

PRIVATE BOOLEAN midi_out_ed4 (RTMCLASSP module, PUF_INF *akt, PUF_INF *alt, BOOLEAN force)
{
	STAT_P		status	= module->status;
	UWORD 		signal;
	KOOR_SINGLE *akt_s, *alt_s;
	POINT_3D		*akt_p, *alt_p;			/* XYZ-Wert eines Signals */
	UBYTE 		trackbase;					/* Basiswert fÅr Key-Berechnung */
	BOOLEAN		xneu = FALSE,				/* X, Y, Z, Vol-Koordinate schicken */
					yneu = FALSE,
					zneu = FALSE,
					vneu = FALSE,
					ret = TRUE,
					new = status->midi_new;	
	WORD			track;
	
	if(alt == 0)
	{
		alt = akt;			/* Noch keine alten Werte vorh. */
		force = new;
	} /* if */
	
	if(force)
	{
		xneu = TRUE;
		yneu = TRUE;
		zneu = TRUE;
		vneu = TRUE;
	} /* if */
				
	for (signal = 1; signal < MAXSIGNALS; signal ++)
	{
		track = akt->tracks->track[signal];
		if(track >= 0 && track < 2*MAXINPUTS)
		{
			akt_s = &(akt->koors->koor[signal]);
			alt_s = &(alt->koors->koor[signal]);

			if (new || (track != alt->tracks->track[signal]))
			{
				/* Wenn sich Spur geÑndert hat, alles neu schicken */
				xneu |= TRUE;
				yneu |= TRUE;
				zneu |= TRUE;
				vneu |= TRUE;
			} /* if */

			akt_p = &(akt_s->koor);
			alt_p = &(alt_s->koor);
			
			xneu |= (akt_p->x !=	alt_p->x);
			yneu |= (akt_p->y !=	alt_p->y);
			zneu |= (akt_p->z !=	alt_p->z);
			vneu |= (akt_s->volume !=	alt_s->volume);
			
			trackbase = 4 * track;
			
			if (xneu)
			{
				if (signal>0) ret &= send_event (module, trackbase, INTERN_TO_MIDI(akt_p->x));
				send_variable(VAR_PUF_KOORX0 + signal, akt_p->x);
			} /* if */
			trackbase++;						/* nÑchste Koor */
			
			if (yneu)
			{
				if (signal>0) ret &= send_event (module, trackbase, INTERN_TO_MIDI(akt_p->y));
				send_variable(VAR_PUF_KOORY0 + signal, akt_p->y);
			} /* if */
			trackbase++;						/* nÑchste Koor */

			if (zneu)
			{
				if (signal>0) ret &= send_event (module, trackbase, INTERN_TO_MIDI(akt_p->z));
				send_variable(VAR_PUF_KOORZ0 + signal, akt_p->z);
			} /* if */
			trackbase++;						/* nÑchste Koor */
			
			if (vneu)
			{
				if (signal>0) ret &= send_event (module, trackbase, akt_s->volume);
				send_variable(VAR_PUF_VOL0 + signal, akt_s->volume);
			} /* if */
		} /* if */
	} /* for */
	
	status->midi_new = FALSE;					/* Koor ausgegeben */
	return (ret);
} /* midi_out_ed4 */

PRIVATE BOOLEAN	send_event (RTMCLASSP module, INT miditrack, INT vel)
{
	/* Track 0 ..63 */
	/* Einen Event rausschicken */
#if OUTPUT_MODULE
#else
	MidiEvPtr	e;
	WORD 		channel, port;
	STAT_P	status	= module->status;
	SHORT		refNum = (SHORT)module->special;
	
	if (refNum > 0)
	{
		if (miditrack < 128 )
		{
			/* Ausgabe auf das erste CMI-System */
			channel	= 15 /*var_get_value (var_module, VAR_CMI_CHANNEL1)*/;
			port		= 0 /* var_get_value (var_module, VAR_CMI_PORT1) */;
			miditrack	-= 0;
		} /* if */
		else
		{
			/* Ausgabe auf das zweite CMI-System */
			channel	= 14 /*var_get_value (var_module, VAR_CMI_CHANNEL2)*/;
			port		= 0 /* var_get_value (var_module, VAR_CMI_PORT2) */;
			/* Spur zurÅckrechnen (33..64->1..32) */
			miditrack	-= 128;
		} /* else */
		
		if (miditrack >= 0 && miditrack < 128
				&& vel >= 0 && vel < 128
				&& channel >= 0 && channel < 16
				&& port>=0 && port < 256)
		{
			if (status->note_off)
			{
				e = MidiNewEv (typeNote);
				if (e)
				{
					Chan (e)	= channel;
					Port (e) = port;
					Pitch (e) = miditrack;
					Vel (e)	= vel;
					Dur (e)	= QUANT/2;		/* Genug Zeit lassen vor Midi-Off */
					MidiSendIm (refNum, e);
				} /* if */
			} /* if */
			else 
			{
				e = MidiNewEv (typeKeyOn);
				if (e)
				{
					Chan (e)	= channel;
					Port (e) = port;
					Pitch (e) = miditrack;
					Vel (e)	= vel;
					MidiSendIm (refNum, e);
				} /* if */
			} /* else */
		} /* if */
	} /* if */
#endif /* OUTPUT_MODULE */
	return TRUE;
} /* send_event */

PRIVATE BOOLEAN send_stop (RTMCLASSP module)
{
	/* Midi-Stop Nachricht senden */
	MidiEvPtr	e;
	STAT_P		status	= module->status;
	SHORT			refNum = (SHORT)module->special;
	BOOLEAN 		ret = TRUE;
	
	if (refNum > 0)
	{
		e = MidiNewEv (typeStop);
		if (e)
		{
			MidiSendIm (refNum, e);
			Port (e) = var_get_value (var_module, VAR_CMI_PORT1);
			ret &= TRUE;
		} /* if */
		e = MidiNewEv (typeStop);
		if (e)
		{
			MidiSendIm (refNum, e);
			Port (e) = var_get_value (var_module, VAR_CMI_PORT2);
			ret &= TRUE;
		} /* if */
		/* if (!ret)	hndl_alert_obj (module, ERR_MIDISHAREFULL); */
	} /* if */
	return ret;
} /* send_stop */

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	STAT_P		status = module->status;
	RTMCLASSP 	man = status->manmodule;
	SO_P			header = status->cue_list, object;
	LONG			posit = status->posit, obj_quants;
	WORD			signal = 1, z_cursor = status->z_cursor, 
					x, y, z, obj_signal, volume,
					*input_ch;
	TRACK_SINGLE	*track 	= event->tracks->track;
	KOOR_SINGLE		*koor 	= event->koors->koor;
	VOL_SINGLE		*vol 		= event->volumes->volume;
	

	/* Modul-Manager aufrufen */	
	if (man>0)					/* MAN-Modul vorhanden ? */
		if (man->apply >0)	/* MAN apply-Funktion da? */
			event = man->apply(man, event);	/* Werte werden verÑndert */

	/* Position des durchlaufenden Cursors:
		immer genau so viele Punkte weitergehen wie Speed vorgibt */
	z_cursor += status->speed.z;
	if (z_cursor > MAXKOOR) z_cursor = -MAXKOOR;
	if (z_cursor < -MAXKOOR) z_cursor = MAXKOOR;
	status->z_cursor = z_cursor;
	
	koor[0].koor.x = status->pos_x;
	koor[0].koor.y = status->pos_y;
	koor[0].koor.z = z_cursor;
	track[0] 		= -1;				/* Keine Ausgabe fÅr FÅhrungspunkt */

	send_variable(VAR_PUF_KOORX0, koor[0].koor.x);
	send_variable(VAR_PUF_KOORY0, koor[0].koor.y);
	send_variable(VAR_PUF_KOORZ0, koor[0].koor.z);

	/* Default: Anzeige aus */
	for (signal = 0; signal < MAXSIGNALS; signal++)
		status->anzeige[signal] = FALSE;

	signal = 1;
	/* Liste der Events durchgehen */
	object = header->next;
	while (object != header)
	{
		if ((object->entry_time <= posit) && (object->exit_time >= posit))
		{
			if ( !object->active)
			{
				/* Objekt aktivieren */
				object->active = TRUE;
				status->new = TRUE;
				status->midi_new  = TRUE;
			} /* if not active */
			
			/* Objekt-Position berechnen */
			obj_quants = (posit - object->cue_time) / QUANT;
			x = object->position[0].x + object->speed.x * obj_quants;
			y = object->position[0].y + object->speed.y * obj_quants;
			z = object->speed.z * obj_quants;

			volume = object->volume;
			input_ch = object->input_ch;
			for (obj_signal = 0; obj_signal < MAXSIGNALS; obj_signal++)
			{
							
				/* Max. 8 Objekte gleichzeitig herstellen */
				if (signal < MAXSIGNALS)
				{
					if (input_ch[obj_signal] > -1)
					{
		
						status->anzeige[signal] = TRUE;						
						
						if (obj_signal == 0)
						{
							koor[signal].koor.x 	= x;
							koor[signal].koor.y 	= y;
							koor[signal].koor.z 	= z;
						}
						else
						{
							/* Offset fÅr jedes Signal zu Objekt-Koordinaten addieren */
							koor[signal].koor.x 	= x + object->position[obj_signal].x;
							koor[signal].koor.y 	= y + object->position[obj_signal].y;
							koor[signal].koor.z 	= z + object->position[obj_signal].z;
						}
						koor[signal].volume 	= volume;
						vol[signal]				= volume;
						track[signal]			= input_ch[obj_signal];
						/* SignalzÑhler erhîhen */
						signal++;
					} /* if input_ch */
				} /* if MAXSIGNALS */
			} /* for obj_signal */
		} /* if entry_time and exit_time */
		else if (object->active)
		{
			/* Objekt deaktivieren */
			object->active = FALSE;
			for (obj_signal = 0; obj_signal < MAXSIGNALS; obj_signal++)
			{
				
				/* Max. 8 Objekte gleichzeitig herstellen */
				if (signal < MAXSIGNALS)
				{
					if (object->input_ch[obj_signal] > -1)
					{
						koor[signal].koor.x 	= 0;
						koor[signal].koor.y 	= 0;
						koor[signal].koor.z 	= 0;
						koor[signal].volume 	= 0;
						vol[signal]				= 0;
						track[signal]			= object->input_ch[obj_signal];
						status->anzeige[signal] = TRUE;	
						/* SignalzÑhler erhîhen */
						signal++;
					} /* if input_ch */
				} /* if MAXSIGNALS */
			} /* for obj_signal */
		} /* else if still active */
			
		
		object = NextSO (object);
	} /* while */
	
	return event;
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	/* ZurÅcksetzen von Werten */
	STAT_P	status	= module->status;
	RTMCLASSP man = status->manmodule;
	WINDOWP	window = module->window;
	SO_P			header = status->cue_list, object;

	status->reset_flag = TRUE;
	status->new = TRUE;
	status->midi_new  = TRUE;
	if (window) window->milli = 1;

	/* Liste der Events durchgehen */
	object = NextSO(header);
	while (object != header)
	{
		/* Objekt aktivieren */
		object->active = FALSE;
		object = NextSO (object);
	} /* while */

	if (man>0)					/* MAN-Modul vorhanden ? */
		if (man->reset)		/* MAN reset-Funktion da? */
			man->reset(man);

} /* reset */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	/* Vorausberechnung */
	STAT_P	status	= module->status;
	RTMCLASSP man = status->manmodule;

	/* Vorausberechnung */
	if (man>0)					/* MAN-Modul vorhanden ? */
		if (man->precalc >0)	/* MAN precalc-Funktion da? */
			(man->precalc)(man);

#if DIRECT_CONTROL
	status->pos_x 		= var_get_value(var_module, VAR_MAEX)*MAXKOOR/MAXPERCENT;
	status->pos_y 		= -var_get_value(var_module, VAR_MAEZ)*MAXKOOR/MAXPERCENT;
	status->speed.z	= var_get_value(var_module, VAR_MAEY)/3;
	status->volume 	= 127;
#else
	status->pos_x 		= var_get_value(var_module, VAR_VAR0+0)*MAXKOOR/MAXPERCENT;
	status->pos_y 		= var_get_value(var_module, VAR_VAR0+1)*MAXKOOR/MAXPERCENT;
	status->speed.x	= var_get_value(var_module, VAR_VAR0+2)- MAXPERCENT/2;
	status->speed.y	= var_get_value(var_module, VAR_VAR0+3)- MAXPERCENT/2;
	status->speed.z	= var_get_value(var_module, VAR_VAR0+4)- MAXPERCENT/2;
	status->volume 	= var_get_value(var_module, VAR_VAR0+6);
#endif

	send_variable(VAR_ED4_PRESEL_POSX, 	status->pos_x);
	send_variable(VAR_ED4_PRESEL_POSY, 	status->pos_y);
	send_variable(VAR_ED4_PRESEL_VOL,  	status->volume);
	send_variable(VAR_ED4_SPEED, 			status->speed.z);
	
} /* precalc */

/*****************************************************************************/

PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	SET_P	ed = module->edited->setup;
	STRING 	s;

	get_ptext (ed4_setup, ED4GRAFIKX, s);
	sscanf (s, "%d", &ed->rot_x);
	get_ptext (ed4_setup, ED4GRAFIKY, s);
	sscanf (s, "%d", &ed->rot_y);
	get_ptext (ed4_setup, ED4GRAFIKZ, s);
	sscanf (s, "%d", &ed->rot_z);
	get_ptext (ed4_setup, ED4GRAFIKPERSP, s);
	sscanf (s, "%d", &ed->persp);

	ed->innenraum = get_checkbox (ed4_setup, ED4INNENRAUM);

} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited 	= module->edited;
	SET_P		ed = edited->setup;
	STRING 	s;

	sprintf (s, "%d", ed->rot_x);
	set_ptext (ed4_setup, ED4GRAFIKX, s);
	sprintf (s, "%d", ed->rot_y);
	set_ptext (ed4_setup, ED4GRAFIKY, s);
	sprintf (s, "%d", ed->rot_z);
	set_ptext (ed4_setup, ED4GRAFIKZ, s);
	sprintf (s, "%d", ed->persp);
	set_ptext (ed4_setup, ED4GRAFIKPERSP, s);

	set_checkbox (ed4_setup, ED4INNENRAUM, ed->innenraum);
	set_checkbox (ed4_setup, ED4AUSSENWELT, !(ed->innenraum));

	copy_icon (&ed4_setup[ED4RAUMFORM], &ed4_raum[ed->raumform+1]);  /* Holen der Iconstruktur aus MTR_EBENEN Popup */
	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (ed4_setup, ED4SETNR , s);
} /* set_dbox */

/*****************************************************************************/
PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	MidiEvPtr	myTask;
	UWORD		variable;
	LONG		value;
	LONG		posit 	= ((MSG_RTM_POSIT *)msg)->posit;
	SO_P		tmp,
				*in1 = &((ED4_MSG *)msg)->in1,
				*in2 = &((ED4_MSG *)msg)->in2,
				*out1 = &((ED4_MSG *)msg)->out1,
				*out2 = &((ED4_MSG *)msg)->out2;
	STAT_P	status	= module->status;
	SET_P		akt		= module->actual->setup;
	WINDOWP	window	= module->window;
	WORD		refNum = (WORD)module->special;
	RTMCLASSP	 man = module->status->manmodule;

	switch(type)
	{
		case RTM_POSIT:
			if (status->sync)
			{
				status->posit 			= posit;
				status->new	  		= TRUE;
				status->midi_new  = TRUE;
				module->reset(module);
				window->milli = 1;
			} /* if */
			break;
		case RTM_CONT:
			if (status->sync)
			{
				status->posit 			= posit;
				status->play = TRUE;
				myTask = MidiTask(play_task_ed4, MidiGetTime(), refNum, 0, 0, 0);
				window->milli = 1;
				status->new	  = TRUE;
				status->midi_new  = TRUE;
			} /* if */
			break;
		case RTM_STOP:
			if (status->sync)
			{
				status->posit 			= posit;
				module->reset(module);
				status->play = FALSE;
				window->milli = 1;
				status->new	  = TRUE;
				status->midi_new  = TRUE;
			} /* if */
			break;
		case SET_VAR:
			variable = ((MSG_SET_VAR *)msg)->variable;
			value		= ((MSG_SET_VAR *)msg)->value;
			switch (variable)
			{
				case VAR_SET_ED4:
					/* module->set_setnr(module, value); */
					break;
				case VAR_RECORD:
					status->record		= (BOOLEAN)value;
					break;
				case VAR_PUF_PLAY:
					status->sync		= (BOOLEAN)value;
					break;
				case VAR_PUF_PAUSE:
					status->pause		= (BOOLEAN)value;
					break;
				case VAR_PUF_ZEITL:
					status->zeitl		= (BOOLEAN)value;
					break;
				default:
				break;
			} /* switch */
			window->milli = 1;
			status->new	  = TRUE;
			status->midi_new  = TRUE;
			break;
		case GET_FIRST_EVENT:
			((ED4_MSG *)msg)->out1 = FindSOByTime (0L, status->cue_list);
			break;
		case GET_NEXT_EVENT:
			((ED4_MSG *)msg)->out1 = NextSOByTime((*in1)->cue_time, status->cue_list);
			break;
		case GET_PREV_EVENT:
			((ED4_MSG *)msg)->out1 = PrevSOByTime((*in1)->cue_time, status->cue_list);
			break;
		case MODIFY_EVENT:
			/* Ersetze alten Event durch neuen */
			tmp = FindSOByTime ((*in1)->cue_time, status->cue_list);
			if (tmp != status->cue_list)
			{
				RemoveSO (tmp);
				CopySO (tmp, *in1);
				ComputeSOTimes (tmp);
				InsertSOByOwnTime (tmp, status->cue_list);
			}
			break;
		case INSERT_EVENT:
			tmp = CreateCopySO (*in1);
			ComputeSOTimes (tmp);
			InsertSOByOwnTime(tmp, status->cue_list);
			break;
		case DELETE_EVENT:
			tmp = FindSOByTime ((*in1)->cue_time, status->cue_list);
			if (tmp != status->cue_list)
			{
				DestroySO(tmp);
			}
			break;
		case GET_NUM_EVENTS:
			((ED4_MSG *)msg)->long_out = CountSO (status->cue_list);
			break;
	} /* switch */
} /* message */

PRIVATE VOID    send_messages	(RTMCLASSP module)
{
	ED_P		actual = module->actual;
	SET_P		akt = actual->setup;
	WORD 		signal;
	STAT_P	status	= module->status;

	send_variable(VAR_SET_ED4, actual->number);

} /* send_messages */

/*****************************************************************************/
/* Setup-Behandlung                                                          */
/*****************************************************************************/

PUBLIC VOID    send_messages_setup	(RTMCLASSP module)
{
	RTMCLASSP refmodule = module->status->refmodule;
	
	if (refmodule)
		if (refmodule->send_messages)
			(refmodule->send_messages) (refmodule); 
} /* send_messages */

LOCAL VOID click_setup (window, mk)
WINDOWP window;
MKINFO  *mk;
{
	RTMCLASSP	module = Module(window);
	ED_P			edited 	= module->edited;
	SET_P			ed  = edited->setup;
	STRING 		s;
	WORD 			signal, offset;
	WORD			i, item;
	static		LONG x = 0;	

	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case ED4SETINC:
			module->set_nr (window, edited->number+1);
			break;
		case ED4SETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case ED4SETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case ED4SETSTORE:
			module->set_store (window, edited->number);
			break;
		case ED4SETRECALL:
			module->set_recall (window, edited->number);
			break;
		case ED4OK   :
			module->set_ok (window);
			break;
		case ED4CANCEL:
			module->set_cancel (window);
			break;
		case ED4HELP :
			module->help (module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case ED4STANDARD:
			module->set_standard (window);
			break;
		case ROOT:
		case NIL:
			break;
		default :	
			if(edited->modified == FALSE)
			{
				edited->modified = TRUE;
				sprintf(s, "%ld*", edited->number);
				set_ptext (ed4_setup, ED4SETNR, s);
				draw_object(window, ED4SETNR);
			} /* if */
			switch (window->exit_obj)
			{
/*
				case ED4LINIEALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(ED4LINIE1-ED4LINIE0)*signal;
						ed->anzeige[signal] = !ed->anzeige[signal];
						set_checkbox (ed4_setup, ED4LINIE0 + offset, ed->anzeige[signal]);
						draw_object(window, ED4LINIE0 + offset);
					} /* for */
					undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case ED4FADENALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(ED4FADEN1-ED4FADEN0)*signal;
						ed->fadenkreuz[signal] = !ed->fadenkreuz[signal];
						set_checkbox (ed4_setup, ED4FADEN0 + offset, ed->fadenkreuz[signal]);
						draw_object(window, ED4FADEN0 + offset);
					} /* for */
					undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
*/
				case ED4RAUMFORM:	
					i = ed->raumform +1;
					item = popup_menu (ed4_raum, ROOT, 0, 0, i, TRUE, mk->momask);
					if ((item != NIL) && (item != i))
					{
						ed->raumform = item-1;
						/* Holen der Iconstruktur aus ED4_RAUMFORM Popup */
						copy_icon (&ed4_setup[ED4RAUMFORM], &ed4_raum[ed->raumform+1]);
					} /* if */
	            undo_state (window->object, window->exit_obj, SELECTED);
					draw_object (window, ROOT);
					break;
				default:
					module->get_dbox(module);
					break;
			} /* switch */
		break;
	} /* switch */
} /* click_setup */

/*****************************************************************************/

PRIVATE VOID dsetup (WINDOWP refwindow)
{
	RTMCLASSP	module;
	WINDOWP		window;
	WORD			ret;
	
	window = search_window (CLASS_DIALOG, SRCH_CLOSED, ED4_SETUP);
	
	if (window == NULL)
	{
		form_center (ed4_setup, &ret, &ret, &ret, &ret);
		
		window = crt_dialog (ed4_setup, NULL, ED4_SETUP, (BYTE *)ed4_text [FED4SN].ob_spec, WI_MODELESS);
	} /* if */
		
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			module = define_setup(window, (RTMCLASSP)refwindow->module); 
			window->edit_obj = find_flags (ed4_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			
			module->set_edit (module);
			module->set_dbox (module);
			undo_state (window->object, ED4HELP, DISABLED);
		} /* if */
		else
			window->opened = 1;
			
		if (! open_window (window)) hndl_alert (ERR_NOOPEN);
	
	} /* if */
} /* dsetup */

PRIVATE RTMCLASSP define_setup (WINDOWP window, RTMCLASSP refmodule)
{
	RTMCLASSP module;

	if (window != NULL)
	{
		window->click    = click_setup;
		window->key      = wi_key_obj;
		window->showinfo = info_mod;
		window->showhelp = showhelp_obj;
		module = create_module(module_name, instance_count);
		/* Informationen kopieren */
		mem_move (module, refmodule, (UWORD)sizeof (RTMCLASS));
		module->object_type	= MODULE_OTHER;
		module->load		= refmodule->load;
		module->save		= refmodule->save;
		module->import		= refmodule->import;
		module->export		= refmodule->export;
		module->apply		= 0;
		module->reset		= 0;
		module->precalc	= 0;
		module->send_messages	= send_messages_setup;
		module->get_edit	= get_edit_setup;
		module->set_setnr	= set_setnr_setup;
		module->window		= window;
		window->module		= (VOID*) module;
		
		module->status->refmodule = refmodule;
		sprintf(module->object_name, "%s", refmodule->object_name);
		sprintf(window->name, " %s Setups ", refmodule->object_name);
	} /* if */
	return module;
} /* define_setup */

PUBLIC VOID    get_edit_setup	(RTMCLASSP module)
{
	RTMCLASSP refmodule = module->status->refmodule;

	get_edit_obj (module);
	refmodule->reset (refmodule);
		
} /* get_edit_obj */

PUBLIC BOOLEAN	set_setnr_setup	(RTMCLASSP module, LONG setupnr)
{
	RTMCLASSP refmodule = module->status->refmodule;
	BOOLEAN ret;
	
	ret = set_setnr_obj (module, setupnr);
	refmodule->reset (refmodule);
	return ret;
} /* set_setnr_setup */

/*****************************************************************************/
/* MenÅbehandlung                                                            */
/*****************************************************************************/

PRIVATE VOID update_menu_mod (window)
WINDOWP window;
{
	SET_P		akt = Akt(window);

/*
	menu_check(ed4_menu, MED4MONO, akt->modus == MONO);
	menu_check(ed4_menu, MED4STEREO, akt->modus == STEREO);
	menu_check(ed4_menu, MED4QUADRO, akt->modus == QUADRO);
	menu_check(ed4_menu, MED4OKTO, akt->modus == OKTO);
	menu_check(ed4_menu, MED4INNENRAUM, akt->innenraum);
*/
	menu_check(ed4_menu, MED4TETRAEDER, akt->raumform == TETRAEDER);
	menu_check(ed4_menu, MED4SECHSKANAL, akt->raumform == SECHSKANAL);
	menu_check(ed4_menu, MED4OKTAEDER, akt->raumform == OKTAEDER);
	menu_check(ed4_menu, MED4WUERFEL, akt->raumform == WUERFEL);
	menu_check(ed4_menu, MED4WUERFELLANG, akt->raumform == WUERFELLANG);
	menu_check(ed4_menu, MED4WUERFELHOCH, akt->raumform == WUERFELHOCH);
	menu_check(ed4_menu, MED4WUERFELMITTE, akt->raumform == WUERFELMITTE);
	menu_check(ed4_menu, MED4WUERFELDOPP, akt->raumform == WUERFELDOPP);
	menu_check(ed4_menu, MED4KREISFORM, akt->raumform == KREISFORM);
	menu_check(ed4_menu, MED4QUADROPHON, akt->raumform == QUADROPHON);
} /* update_menu_mod */

/*****************************************************************************/

PRIVATE VOID handle_menu_mod (window, title, item)
WINDOWP window;
WORD    title, item;

{
	SET_P		akt = Akt(window);
	STAT_P	status = Status(window);
	SO_P		event;
	WORD		signal;
	
	if (window != NULL)
		menu_normal (window, title, FALSE);         /* Titel invers darstellen */

	switch (title)
	{
		case MED4INFO:
			switch (item)
			{
				case MED4INFOANZEIG: info_mod(window, NIL);
											break;
			} /* switch */
			break;
	
		case MED4EVENTS:
			switch (item)
			{
				case MED4NEW:		
					if (status->record)
					{
						CreateStandardEvent (Module(window));
					} /* if record */
					break;
				case MED4EDIT:
					break;
				case MED4DEL:
					break;
			} /* switch */
			status->new = TRUE;
			status->midi_new  = TRUE;
			window->milli = 1;
			break;

		case MED4FORMS:
			switch (item)
			{
				case MED4TETRAEDER:		akt->raumform = TETRAEDER;		break;
				case MED4SECHSKANAL:		akt->raumform = SECHSKANAL;	break;
				case MED4OKTAEDER:		akt->raumform = OKTAEDER;		break;
				case MED4WUERFEL:			akt->raumform = WUERFEL;		break;
				case MED4WUERFELLANG:	akt->raumform = WUERFELLANG;	break;
				case MED4WUERFELHOCH:	akt->raumform = WUERFELHOCH;	break;
				case MED4WUERFELMITTE:	akt->raumform = WUERFELMITTE;	break;
				case MED4WUERFELDOPP:	akt->raumform = WUERFELDOPP;	break;
				case MED4QUADROPHON:		akt->raumform = QUADROPHON;	break;
				case MED4KREISFORM:		akt->raumform = KREISFORM;		break;
			} /* switch */
			status->new = TRUE;
			window->milli = 1;
			break;
		case MED4OPTIONS:
			switch (item)
			{
				case MED4SETUPS	:
					dsetup(window);
					break;
				case MED4INNENRAUM		:
					akt->innenraum = !akt->innenraum;
					window->milli = 1;
					status->new = TRUE;
					break;
				case MED4PERSPEKTIVE 	:
					break;
			} /* switch */
	} /* switch */
		
	if (window != NULL)
		menu_normal (window, title, TRUE);          /* Titel wieder normal darstellen */
} /* handle_menu_mod */

/*****************************************************************************/
/* Taste fÅr Fenster                                                         */
/*****************************************************************************/

PRIVATE BOOLEAN wi_key_mod (window, mk)
WINDOWP window;
MKINFO  *mk;

{
	RTMCLASSP	module = Module(window);
	WORD			refNum = (WORD)module->special;
	STAT_P		status = module->status;
	BOOLEAN		ok = FALSE;

	switch (mk->scan_code)
	{
		case INSERT:
			handle_menu_mod (window, MED4EVENTS, MED4NEW);
			break;
	} /* switch scancode */
	
	ok |= (menu_key (window, mk));
	return (ok);
} /* wi_key_mod */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

PRIVATE VOID wi_draw_mod (window)
WINDOWP window;

{	
	RTMCLASSP	module = Module(window);
	STAT_P		status = module->status;
	
	if (status->new)
	{
		clr_scroll (window);
	} /* if */

} /* wi_draw_mod */

/*****************************************************************************/
/* Vor zeichnen Status verÑndern                                             */
/*****************************************************************************/

PRIVATE VOID wi_start_mod (window)
WINDOWP window;

{	
	RTMCLASSP	module = Module(window);
	STAT_P	status = Status(window);
	SET_P		akt = module->actual->setup;
	DISPOBJP	dispobj;
	LIST_P	header, element;
	BOOLEAN	new = status->new || (window->flags & WI_JUNK);
	BOOLEAN	newkoor = FALSE;
	KOOR_ALL	*koor;	
	POINT_3D	*point;
	POS_3D	position;
	WORD		index, x, obj_nr = 0;
	RECT		a;
	
	status->new = new;
	
	/* Alle Display-Objekt Parameter updaten */
	header = window->dispobjs;
	element = list_next(header);
	while (element != header) {
		dispobj = (DISPOBJP) element->key;
		if (new) {
			dispobj->new	= TRUE;
			/* Nur fÅr 3D-Anzeige */
			if (obj_nr == 0)
			{
				dispobj->set_uni  (dispobj, DOUniSpaceInside, akt->innenraum);
				dispobj->set_uni  (dispobj, DOUniSpaceArrows, akt->pfeile);
				dispobj->set_type (dispobj, DOTypeSpaceForm, akt->raumform);
				dispobj->set_type (dispobj, DOTypeSpaceMode, akt->modus);
				/* Hîhe fÅr Space korrigieren */
				a = window->scroll;
				a.h -= gl_hbox*2;
				dispobj->set_work (dispobj, &a);
	
				dispobj->set_uni  (dispobj, DOUniRotationX, akt->rot_x);
				dispobj->set_uni  (dispobj, DOUniRotationY, akt->rot_y);
				if (akt->rot_z !=0)
					dispobj->set_uni  (dispobj, DOUniRotationZ, akt->rot_z);
				dispobj->set_uni  (dispobj, DOUniSpacePerspective, akt->persp);
	
				/* Compute and send bitflags for display and crosshair */
				x = 0;
				for (index = 1; index < MAXSIGNALS; index ++)
					x = x | (status->anzeige[index] << index);
				dispobj->set_uni (dispobj, DOUniSpaceCrosshair, x);

		 		/* Fadenkreuz fÅr 0 bei Record anzeigen (durchl. Linien + Kreuz) */
				if (status->record)
					dispobj->set_uni (dispobj, DOUniSpaceDisplay, 1);
				else
					dispobj->set_uni (dispobj, DOUniSpaceDisplay, 0);
					
				dispobj->set_uni (dispobj, DOUniSpaceInputBase, 0);
			} /* if obj_nr == 0 */
			else
			{
				/* Hîhe fÅr Texte berechnen */
				a = dispobj->work;
				a.y = window->scroll.h - gl_hbox;
				dispobj->set_work (dispobj, &a);
			} /* else */
			
			obj_nr++;
		} /* if new */

		if (status->reset_flag)
			if (dispobj->reset) (*dispobj->reset) (dispobj);
		element = list_next(element);
	} /* while */
	status->reset_flag = FALSE;	
} /* wi_start_mod */

/*****************************************************************************/
/* Nach zeichnen Status verÑndern                                            */
/*****************************************************************************/

PRIVATE VOID wi_finished_mod (window)
WINDOWP window;

{	
	STAT_P	status = Status(window);

/*	This handling is not necessary, as DISPOBJ take their data
	directly from the VAR's

	/* Neue Koordinaten Åbernehmen */
	if (status->koor_stat1 == USED)
		mem_move(&status->koor_alt, &status->koor_akt1,(UWORD)sizeof(KOOR_ALL)); 
	else
		mem_move(&status->koor_alt, &status->koor_akt2,(UWORD)sizeof(KOOR_ALL)); 

	window->milli = 0; 			/* keine Timer-Funktion mehr bis
										zur nÑchsten énderung */
*/
	status->new = FALSE;
} /* wi_finished_mod */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
WINDOWP window;
MKINFO  *mk;
{
	RTMCLASSP	module = Module(window);
	STAT_P		status = module->status;
	BOOLEAN		new = status->new || (window->flags & WI_JUNK);
	
	/* Sicherheitshalber */
	status->new = new;

	window->milli = 1;
} /* wi_click_mod */

/*****************************************************************************/
/* Einrasten des Fensters                                                    */
/*****************************************************************************/

PRIVATE VOID wi_snap_mod (window, new, mode)
WINDOWP window;
RECT    *new;
WORD    mode;

{
	STAT_P		status = Status(window);
	wi_snap_obj (window, new, mode);
	status->new = TRUE;
	window->milli = 1; 						/* Updaten ! */
} /* wi_snap_mod */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (window)
WINDOWP window;
{
	RTMCLASSP	module = Module (window);
	SHORT			refNum = (SHORT)module->special;
	LONG			numtasks;		

	/* Max. einen Event ausfÅhren */
	numtasks = MidiCountDTasks(refNum);
	if (numtasks > 0)
	{
		MidiExec1DTask(refNum);
		/* Den Rest wegschmeissen */
		if (numtasks > 1)
			MidiFlushDTasks(refNum);
	} /* if numtasks */

	if (window->opened >0)
		redraw_window(window, &window->scroll);

	window->milli = 0; 			/* keine Timer-Funktion mehr bis
										zur nÑchsten énderung */
} /* wi_timer_mod */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;
{
	WINDOWP	window;
	WORD   	menu_height, inx;
	DISPOBJP	dispobj;
	
	inx    = num_windows (CLASS_ED4, SRCH_ANY, NULL);
	window = create_window_obj (KIND, CLASS_ED4);
	
	if (window != NULL)
	{
		WINDOW_INITWIN_OBJ
		
		window->flags     = FLAGS;
		window->icon      = icon;
		window->doc.x     = 0;
		window->doc.y     = 0;
		window->doc.w     = INITW / XFAC;
		window->doc.h     = 0;
		window->xfac      = XFAC;
		window->yfac      = YFAC;
		window->xunits    = XUNITS;
		window->yunits    = YUNITS;
		window->work.x    = window->scroll.x;
		window->work.y    = window->scroll.y - menu_height;
		window->work.w    = window->scroll.w;
		window->work.h    = window->scroll.h + menu_height;
		window->mousenum  = THICK_CROSS;
		window->mouseform = NULL;
		window->milli     = MILLI;
		window->module    = 0;
		window->object    = 0;
		window->menu      = menu;
		window->hndl_menu = handle_menu_mod;
		window->updt_menu = update_menu_mod;
		window->key       = wi_key_mod;
		window->draw		= wi_draw_mod;
		window->snap      = wi_snap_mod;
		window->timer     = wi_timer_mod;
		window->showinfo  = info_mod;
		window->click		= wi_click_mod;
		window->start		= wi_start_mod;
		window->finished  = wi_finished_mod;

		/* Display-Objekte einklinken */
		create_displayobs (window);

		sprintf (window->name, (BYTE *)ed4_text [FED4N].ob_spec, 0);
		sprintf (window->info, (BYTE *)ed4_text [FED4I].ob_spec, 0);
	} /* if */
	
	return (window);                      /* Fenster zurÅckgeben */
} /* crt_mod */

PRIVATE VOID create_displayobs (WINDOWP window)
{	
	WORD		signal, h = gl_hbox, w = gl_wbox, x0, y0;
	LONGSTR	s;
	RECT		a;
	
	if (window->work.h > 400)
		h = 16;
	else
		h = 8;

	/* 3D-Raum */
	CrtED4SpaceDOInsert (window, WUERFEL, 0, &window->scroll);

	a.x = 0 * w;
	a.y = window->scroll.h - h;
	a.h = h;

	strcpy (s, "Input: %ld ");
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, VAR_ED4_PRESEL_INPUT, s);

	a.x = a.x + a.w;
	strcpy (s, "X: %ld ");
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, VAR_ED4_PRESEL_POSX, s);

	a.x = a.x + a.w;
	strcpy (s, "Y: %ld ");
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, VAR_ED4_PRESEL_POSY, s);

	a.x = a.x + a.w;
	strcpy (s, "Vol: %ld ");
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, VAR_ED4_PRESEL_VOL, s);

	a.x = a.x + a.w;
	strcpy (s, "Usr-Spd: %ld ");
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, VAR_ED4_SPEED, s);
	
	a.x = a.x + a.w;
	strcpy (s, "Obj-Spd: %ld ");
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, VAR_ED4_PRESEL_OBJSPD, s);
} /* create_displayobs */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN 	ok;
	WINDOWP 	window;
	
	/* Fenster suchen */
	window = search_window (CLASS_ED4, SRCH_CLOSED, icon);
	/* Wenn nicht gefunden */
	if (window == NULL)
	{
		if (create()>0);	/* Neue Instanz */
			window = search_window (CLASS_ED4, SRCH_CLOSED, icon);
	} /* if */
	
	ok = window != NULL;
	
	if (ok)
	{
		ok = open_window (window);
	} /* if */
	
	return (ok);
} /* open_mod */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

PUBLIC BOOLEAN info_mod (window, icon)
WINDOWP window;
WORD    icon;
{
	RTMCLASSP	module = Module(window);
	WINDOWP		refwindow = window;
	WORD			ret;
	STRING		s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, IED4);
		
	if (window == NULL)
	{
		 form_center (ed4_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (ed4_info, NULL, IED4, refwindow->name, WI_MODELESS);
	} /* if */
		
	if (window != NULL)
	{
		window->object = ed4_info;
		sprintf(s, "%-20s", ED4DATE);
		set_ptext (ed4_info, ED4IVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (ed4_info, ED4COMPILE, s);
		sprintf(s, "%-20s",  ED4VERSION);
		set_ptext (ed4_info, ED4IVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (ed4_info, ED4ISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (ed4_info, ED4IAKT, s);

		if (! open_dialog (IED4)) hndl_alert (ERR_NOOPEN);
	} /* if */

  return (window != NULL);
} /* info_ed4 */

/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	RTMCLASSP 	module;
	WINDOWP		window;
	STAT_P		status;
	SET_P			standard;
	FILE			*fp;
	WORD			signal;
	SHORT			refNum;
	SO_P			header;
	
	module = create_module (module_name, instance_count);
	
	if (module != NULL && instance_count < max_instances)
	{
		module->class_number		= CLASS_ED4;
		module->icon				= &ed4_desk[ED4ICON];
		module->icon_position 	= IED4;
		module->icon_number		= IED4;	/* Soll bei Init vergeben werden */
		module->menu_title		= MED4S;
		module->menu_position	= MED4;
		module->menu_item			= MED4;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
				
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_ed4;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_OTHER;
		module->apply				= apply;
		module->reset				= reset;
		module->precalc			= precalc;
		module->message			= message;
		module->create				= create;
		module->destroy			= destroy_obj;
		module->test				= test_obj;
	
		module->file_pointer			= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->file_pointer, 0, (UWORD)sizeof(FILE));
		module->import_pointer		= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->import_pointer, 0, (UWORD)sizeof(FILE));
		module->setup_length			= sizeof(SETUP);
		module->location				= SETUPS_EXTERN;
		module->ram_modified 		= FALSE;
		module->max_setups	 		= MAXSETUPS;
		module->standard				= (SET_P)mem_alloc(sizeof(SETUP));
		module->actual->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->edited->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->status 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->stat_alt 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->load					= load_obj;
		module->save					= save_obj;
		module->get_dbox				= get_dbox;
		module->set_dbox				= set_dbox;
		module->get_edit				= get_edit_obj;
		module->set_edit				= set_edit_obj;
		module->get_setnr				= get_setnr_obj;
		module->set_setnr				= set_setnr_obj;
		module->set_nr					= set_nr_obj;
		module->set_store				= set_store_obj;
		module->set_recall			= set_recall_obj;
		module->set_ok					= set_ok_obj;
		module->set_cancel			= set_cancel_obj;
		module->set_standard			= set_standard_obj;
		module->send_messages		= send_messages;
		if (module->location == SETUPS_INTERN)
		{
			module->setups	= mem_alloc (sizeof (SETUP) * MAXSETUPS);
			mem_lset (module->setups, 0, sizeof (SETUP) * MAXSETUPS);
		} /* if */
		else
		{
		} /* else */
		/* PrÅfen, ob DEFAULT-Datei vorhanden */
		if((fp=fopen(module->file_name, "rb"))!=0)
		{
			/* Wenn vorhanden, laden */
			fclose(fp);
			module->load(module, module->file_name, FALSE);
		} /* if */

		/* Status-Struktur initialisieren */
		status = module->status;
		mem_set(status, 0, (UWORD) sizeof(STATUS));
		status->speed.z	= 5;
		status->editing	= FALSE;
		status->play 		= FALSE;
		status->note_off 	= FALSE;
		status->volume 	= 127;
		status->input_ch  = 0;
		header 				= CreateSO();
		header->next 		= header;
		header->prev 		= header;
		status->cue_list 	= header;
		
		/* TemporÑrer Event */
		status->tmp_event.koors 	= &status->tmp_koors;
		status->tmp_event.tracks	= &status->tmp_tracks;
		status->tmp_event.volumes	= &status->tmp_vols;

		/* Old Event */
		status->old_event.koors 	= &status->old_koors;
		status->old_event.tracks	= &status->old_tracks;
		status->old_event.volumes	= &status->old_vols;

		/* Standard-Module */
		status->varmodule = var_module;
		status->manmodule = man_module;
		status->tramodule = tra_module; 

		init_standard (module);		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (ed4_setup, ed4_menu, IED4);
		/* Modul-Struktur einbinden */
		window->module	= (VOID*) module;
		sprintf(window->name, " %s ", module->object_name);
		module->window		= window;
		module->status->refmodule = module;	
		module->status->refwindow = window;	
		add_rcv(VAR_SET_ED4,  module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_ED4, MAXSETUPS);
		refNum				= init_midishare();
		module->special	= (LONG) refNum;
		modulep[refNum]	= module;
		if (refNum>0)
			InstallFilter(refNum);									/* Midi-Input-Filter einbauen */	
		init_messages(module);

		ed4_module = module;	/* globaler Zeiger auf ED4-Modulparameter */
		instance_count++;
	} /* if */
	return module;
} /* create */

PRIVATE VOID init_standard (RTMCLASSP module)
{
	/* Setup-Strukturen initialisieren */
	SET_P			standard;

	standard = module->standard;
	mem_set(standard, 0, (UWORD) sizeof(SETUP));
	standard->modus			= MONO;
	standard->raumform		= WUERFEL;
	standard->innenraum		= FALSE;

	/* Fluchtpunkt */
	standard->rot_x			= 60;
	standard->rot_y			= 50;
	standard->rot_z			= 100;
}
/*****************************************************************************/
/* Lîsche Objekt                                                            */
/*****************************************************************************/
PUBLIC VOID destroy_mod (module)
RTMCLASSP module;
{
	STRING 	s;
	INT		refNum;
	STAT_P	status = module->status;
	
	if (msh_available)
	{
		for (refNum = 0; module != modulep[refNum]; refNum++);
	
		if (refNum > 0)
		{
			refNums[instance_count--] = 0;
			MidiClose (refNum);				/* abmelden	*/
		} /* if */
		
	} /* if */
/* Hier Speicher fÅr SO freigeben
	sprintf (s, "%ld Events werden freigegeben ...", status->max_events);
	daktstatus(" ED4-Terminierung ", s);
*/
	set_daktstat(100);
	close_daktstat();
	destroy_obj (module);	/* weiter mit Standard-Routine */
} /* destroy_mod */

/*****************************************************************************/
/* MidiShare initialisieren                                                  */
/*****************************************************************************/

PRIVATE SHORT init_midishare ()
{
	/* Meldet ein neues Modul bei MidiShare an und gibt die refNum zurÅck */
	SHORT		refNum = 0;			/* temporÑre Referenznummer */
	STRING	s;
	
	if (msh_available)
	{
		if (MidiShare())
		{
			if (instance_count <= max_instances)
			{
				if (instance_count == 0)
					sprintf (s, "PUF");
				else
					sprintf (s, "PUF %d", instance_count + 1);
				refNum = MidiGetNamedAppl(s); /* Alte Applikation schliessen */
				if (refNum > 0) MidiClose(refNum);
				refNum = MidiOpen(s);				/* Applikation fÅr MidiShare îffnen	*/
			} /* if */
		} /* if */
	
		if (refNum == 0)
			 hndl_alert (ERR_NOMIDISHARE);
	
		if (refNum == MIDIerrSpace)			/* PrÅfen genug Platz war */
		{
			 hndl_alert (ERR_MIDISHAREFULL);
		} /* if */
	
		if (refNum > 0)							/* PrÅfen ob alles klar */
		{
			instance_count++;
			refNums[instance_count] = refNum;				/* Merken fÅr term_mod */
			MidiSetRcvAlarm(refNum, receive_evts_ed4);	/* Interrupt-Handler */		
			/* An alle anschlieûen */
			try_all_connect (refNum);
		} /* if */
	} /* if */
	
	return refNum;
	
} /* init_midishare */

PRIVATE VOID init_messages (RTMCLASSP module)
{
	WORD	signal;

	add_rcv(VAR_SET_ED4,  module);	/* Message einklinken */
	var_set_max(var_module, VAR_SET_ED4, MAXSETUPS);
	add_rcv(VAR_RECORD, module);	/* Message einklinken */

	add_rcv(VAR_PUF_PLAY, module);	/* Message einklinken */
	add_rcv(VAR_PUF_PAUSE, module);	/* Message einklinken */
	add_rcv(VAR_PUF_ZEITL, module);	/* Message einklinken */

	/* MAE-Sperre verhindern */
	send_variable(VAR_MAE_SPERRE_AUSSEN, FALSE);
} /* init_messages */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

PRIVATE BOOLEAN init_rsc ()

{
  WORD   i, iconw, iconh, iconr;
  STRING s, rsc_name;
#if GEM & (GEM2 | GEM3 | XGEM)
  BYTE   *p;
#endif

#if RSC_CREATE || XRSC_CREATE
#if RSC_CREATE
  rsc_create (gl_wbox, gl_hbox, NUM_TREE, NUM_OBS, NUM_FRSTR, NUM_FRIMG,
              rs_strings, rs_frstr, rs_bitblk, rs_frimg, rs_iconblk,
              rs_tedinfo, rs_object, (OBJECT **)rs_trindex, (RS_IMDOPE *)rs_imdope);
#endif
/*
  alertmsg = &rs_strings [FREESTR];             /* Adresse der Fehlermeldungen */
*/
  ed4_menu  = (OBJECT *)rs_trindex [ED4_MENU];  /* Adresse der ED4-MenÅzeile */
  ed4_setup = (OBJECT *)rs_trindex [ED4_SETUP]; /* Adresse der ED4-Parameter-Box */
  ed4_shelp = (OBJECT *)rs_trindex [ED4_SHELP];	/* Adresse der ED4-Parameter-Hilfe */
  ed4_help  = (OBJECT *)rs_trindex [ED4_HELP];	/* Adresse der ED4-Hilfe */
  ed4_desk  = (OBJECT *)rs_trindex [ED4_DESK];	/* Adresse des ED4-Desktops */
  ed4_text  = (OBJECT *)rs_trindex [ED4_TEXT];	/* Adresse der ED4-Texte */
  ed4_info 	= (OBJECT *)rs_trindex [ED4_INFO];	/* Adresse der ED4-Info-Anzeige */
  ed4_raum	= (OBJECT *)rs_trindex [ED4_RAUM];	/* Adresse der ED4-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (ed4_rsc_ptr, rsc_name))
  {
    strcpy (s, "[3][Resource-File|");
    strcat (s, rsc_name);
    strcat (s, "?][ EXIT ]");
    beep ();
    form_alert (1, s);
    if (! deskacc) return (FALSE);
    menu_unregister (gl_apid);                  /* Wieder abmelden */
    while (TRUE) evnt_timer (0, 1);             /* Lasse andere Prozesse ran */
  } /* if */

  rs_gaddr (ed4_rsc_ptr, R_TREE,  ED4_MENU,	&ed4_menu);    /* Adresse des ED4-MenÅs */
  rs_gaddr (ed4_rsc_ptr, R_TREE,  ED4_SETUP,	&ed4_setup);   /* Adresse der ED4-Parameter-Box */
  rs_gaddr (ed4_rsc_ptr, R_TREE,  ED4_SHELP,	&ed4_shelp);   /* Adresse der ED4-Parameter-Hilfe */
  rs_gaddr (ed4_rsc_ptr, R_TREE,  ED4_HELP,	&ed4_help);    /* Adresse der ED4-Hilfe */
  rs_gaddr (ed4_rsc_ptr, R_TREE,  ED4_DESK,	&ed4_desk);    /* Adresse des ED4-Desktop */
  rs_gaddr (ed4_rsc_ptr, R_TREE,  ED4_TEXT,	&ed4_text);    /* Adresse der ED4-Texte */
  rs_gaddr (ed4_rsc_ptr, R_TREE,  ED4_INFO,	&ed4_info);    /* Adresse der ED4-Info-Anzeige */
  rs_gaddr (ed4_rsc_ptr, R_TREE,  ED4_RAUM,	&ed4_raum);    /* Adresse der ED4-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (ed4_menu, TRUE);
	fix_objs (ed4_setup, TRUE); 
	fix_objs (ed4_shelp, TRUE);
	fix_objs (ed4_help, TRUE);
	fix_objs (ed4_desk, TRUE);
	fix_objs (ed4_text, TRUE);
	fix_objs (ed4_info, TRUE);
	fix_objs (ed4_raum, TRUE);
	
	
	do_flags (ed4_setup, ED4CANCEL, UNDO_FLAG);
	do_flags (ed4_setup, ED4HELP, HELP_FLAG);
	
	menu_enable(menu, MED4, TRUE);

	return (TRUE);
} /* init_rsc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PRIVATE BOOLEAN term_rsc ()

{
  BOOLEAN ok;

  ok = TRUE;

#if ((XRSC_CREATE|RSC_CREATE) == 0)
  ok = rs_free (ed4_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_ed4 ()
{
	BOOLEAN	ok = TRUE;


	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

	return (ok);
} /* init_ed4 */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	return (term_rsc ());
} /* term_mod */
