/*****************************************************************************/
/*                                                                           */
/* Modul: PUF.C                                                              */
/*                                                                           */
/* Puffersequencer fÅr RTM                                                   */
/*                                                                           */
/*****************************************************************************/
#define PUFVERSION "V 1.12"
#define PUFDATE "19.02.95"

/*****************************************************************************
V 1.13
- Umstellung auf RTM_POSIT etc., 19.02.95
- ClickSetupField eingebaut, 30.01.95
- Bug in find_event beseitigt, 08.01.95
- Speicherverwaltung bereinigt, 04.01.95
V 1.12 20.12.94
- in send_event Bug in midi-key berechnung korrigiert
- create_puf in create umbenannt
V 1.11 18.11.94
- OUTPUT FALSE
- ret in send_event beseitigt
V 1.10 23.07.94
- Window-Breite und Hîhe standardmÑûig kleiner
V 1.09 15.07.94
- load_create_infos und instance_count eingebaut
- storing of Koordinates in VAR
- send_event sends VAR's now
- send_event prepared for OUTPUT module
V 1.08
- mem_free ausgebaut in destroy_mod
- MIDI-Ausgabe angeschaltet
- MS-Namen auf TRA geÑndert
- Update-Zeit-Messung eingebaut
- PUFEVP in REALTSPC.H ausgelagert, wg. PUF-BIG öbertragung
- Bug in Window-Timer beseitigt
V 1.07
- ifdef TEST eingebaut
V 1.06
- ch_channels und ports aus dem Status herausgenommen
- msh_available eingebaut
- set_event_puf und get_event_puf repariert
- play_task_puf erweitert auf Flags-Verwendung
- try_all_connect eingebaut
V 1.05
- MODULE_OTHER fÅr setup
- Umbau auf Modul-Zeiger im status
- destroy_mod eingebaut
- Umbau auf create_window_obj
V 1.04
- Abmelden alter MS-Applikationen
- PUF-Setup open jetzt immer mit define_setup
V 1.03
- Anpassung auf neue RTMCLASS-Struktur
- set_daktstat bei Verkettung von PUF-Events
V 1.02
- Aufnahmeverhinderung bei Header-Event
- Midi-Ausgabe repariert
V 1.01
- doppelte add_rcv fÅr VAR_SET_REC entfernt
- erlauben von Spur 1 .. 64 bei Midi-Out
V 1.00, 17.04.93
- Synchronisation an/aus eingebaut
- Midi-Ausgabe auf Midi-Share eingebaut
- BerÅcksichtigung von CMI Port und Channel
- note_off in status eingesetzt
- Fehler in send_messages() beseitigt
*****************************************************************************/
#ifndef XRSC_CREATE
/*#define XRSC_CREATE 1 */                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"
#include "time.h"

#include "realtim4.h"
#include "puf_mod.h"
#include "realtspc.h"
#include "man.h"
#include "tra.h"
#include "gen.h"
#include "var.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"

#include "msh_unit.h"
#include "msh.h"

#include "export.h"
#include "puf.h"

#if XRSC_CREATE
#include "puf_mod.rsh"
#include "puf_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                 /* X-Faktor */
#define YFAC   gl_hbox                 /* Y-Faktor */
#define XUNITS 1                       /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                       /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)          /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)          /* Y-Anfangsposition */
#define INITW  (20 * gl_wbox)          /* Anfangsbreite in Pixel */
#define INITH  (10 * gl_hbox)          /* Anfangshîhe in Pixel */
#define MILLI  0								/* Millisekunden fÅr Zeitablauf */
#define PUF_RSC_NAME "PUF_MOD.RSC"		/* Name der Resource-Datei */
#define MAXSETUPS 20l						/* Anzahl der PUF-Setups */
enum REIHENFOLGE {RSIGNALE, RKOOR};		/* Anzeige-Reihenfolge */

enum DTaskTypes {PUFDTaskReset, PUFDTaskPrecalc};	/* Delayed Task Typen */

/* Umwandlung von interner Koordinatendarstellung auf Midi-Format */
#define INTERN_TO_MIDI(koor) 64 + koor

#define NO_REFNUM		-1
#define OUTPUT_MODULE FALSE					/* Output through OUTPUT-Module or direct MIDI */

/****** TYPES ****************************************************************/

typedef	struct status *STAT_P;

typedef	struct status
{
	UINT	play			: 1	;	/* PLAY gedrÅckt */
	UINT	record		: 1	;	/* RTM Record an/aus */
	UINT	puf_record	: 1	;	/* PUF Record an/aus */
	UINT	cycle			: 1	;	/* Cycle-Modus an/aus*/
	UINT	sync			: 1	;	/* synchron mitlaufen */
	UINT	note_off		: 1	;	/* Note-Off Events schicken */
	LONG	leftloc;					/* Linker Locator */
	LONG	rightloc;				/* Rechter Locator */
	LONG	posit;					/* SMPTE Zeit	*/
	BOOLEAN	new;					/* Koordinaten neu ausgeben */
	BOOLEAN	zeitl;				/* Zeitlupe */
	BOOLEAN	pause;				/* Pause: Sequencer anhalten */
	PUFEVP	header;				/* Kopf fÅr Event-Liste */
	PUFEVP	locator[10];		/* Locators */
	ULONG		max_events;			/* Maximale Event-Anzahl */
	PUF_INF		tmp_event;		/* TemporÑrer Event fÅr play_task */
	KOOR_ALL		tmp_koors;		/*     "      Koordinaten */	
	TRACK_ALL	tmp_tracks;		/*     "      Tracks */
	VOL_ALL		tmp_vols;		/*     "      BIG-Volumes */
	TFilter	filter;				/* Midi-In-Filter	*/
	RTMCLASSP	manmodule,		/* Zugehîriges MAN-Modul */
					tramodule,		/* Zugehîriges TRA-Modul */
					varmodule;		/* Zugehîriges VAR-Modul */
	PUFEVP		events_p;		/* Merker fÅr Speicherfreigabe */
	KOOR_ALL		*koors_p;		/* Merker fÅr Speicherfreigabe */	
	TRACK_ALL	*tracks_p;		/* Merker fÅr Speicherfreigabe */
	VOL_ALL		*vols_p;			/* Merker fÅr Speicherfreigabe */
	clock_t		start_cl,		/* Messung: Interrupt-Start */
 					stop_cl,			/* 			Interrupt-Ende */
					last_update;	/* Messung fÅr Intervalldauer fÅr Fenster-Update */
} STATUS;

typedef	struct setup
{
	/* Signal Record/Play Parameter */
	BOOLEAN	rec_an[MAXSIGNALS];		/* Aufnahme an/aus */
	BOOLEAN	rec_x[MAXSIGNALS];		/* Aufnahme X-Koor */
	BOOLEAN	rec_y[MAXSIGNALS];		/* Aufnahme Y-Koor */
	BOOLEAN	rec_z[MAXSIGNALS];		/* Aufnahme Z-Koor */
	BOOLEAN	rec_vol[MAXSIGNALS];		/* Aufnahme Volume */
	BOOLEAN	rec_dub[MAXSIGNALS];		/* Overdub-Modus */
	/* Anzeige-Parameter */
	BOOLEAN	anz_an[MAXSIGNALS];	/* Anzeige fÅr Signal x an*/
	BOOLEAN	anz_x;					/* Anzeige X-Koordinaten*/
	BOOLEAN	anz_y;					/* Anzeige Y-Koordinaten*/
	BOOLEAN	anz_z;					/* Anzeige Z-Koordinaten*/
	BOOLEAN	anz_vol;					/* Anzeige Volume */
	BOOLEAN	ausgabe[MAXSIGNALS];	/* Ausgabe */
	WORD		breite;					/* Breite eines Events */
	WORD		hoehe;					/* Hîhe eines Feldes */
	WORD		reihenfolge;			/* nach Koordinaten oder nach Signalen sortiert */
} SETUP;

typedef struct setup *SET_P;

/*
PUF-Events liegen als zyklische, doppelt verkettete Liste mit Kopf
im Speicher. Jeder Event trÑgt einen Zeiger auf den nÑchsten und
den vorherigen Event. Der Kopf zeigt auf den ersten Ev.. Der letzte
Ev. zeigt wieder auf den Kopf.
*/
/****** VARIABLES ************************************************************/
PRIVATE WORD	puf_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*puf_rsc_ptr = &puf_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *puf_setup;
PRIVATE OBJECT *puf_shelp;
PRIVATE OBJECT *puf_help;
PRIVATE OBJECT *puf_desk;
PRIVATE OBJECT *puf_menu;
PRIVATE OBJECT *puf_text;
PRIVATE OBJECT *puf_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "PUF";		/* Name, fÅr Extension etc. */

PRIVATE RTMCLASSP	modulep[MAXMSAPPLS];			/* Zeiger auf Modul-Strukturen */
PRIVATE WORD		refNums[1];						/* Referenznummern */

PRIVATE	PUF_INF		tmp_event;
PRIVATE	KOOR_ALL		tmp_koors;			/* Koordinaten-Struktur */
PRIVATE	TRACK_ALL	tmp_tracks;			/* Spur-Zuweisung */
PRIVATE	VOL_ALL		tmp_volumes;		/* General-Volume aus BIG */

/****** FUNCTIONS ************************************************************/

/* MidiShare Funktionen */
PUBLIC VOID			cdecl	receive_evts_puf	_((SHORT refNum));
PUBLIC VOID			cdecl play_task_puf		_((LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3));
PUBLIC VOID			cdecl delayed_task_puf	_((LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3));
PRIVATE VOID		InstallFilter				_((SHORT refNum));

/* Interne PUF-Funktionen */
PRIVATE VOID	dsetup				_((WINDOWP refwindow));
PRIVATE VOID	click_setup			_((WINDOWP window, MKINFO *mk));
PRIVATE RTMCLASSP define_setup	_((WINDOWP window, RTMCLASSP refmodule));
PRIVATE VOID 	position				_((RTMCLASSP module, LONG smpte));

PRIVATE VOID	start_record 		_((RTMCLASSP module));
PRIVATE VOID	tempo_up				_((RTMCLASSP module));
PRIVATE VOID	tempo_down			_((RTMCLASSP module));
PRIVATE PUFEVP	find_event			_((RTMCLASSP module, LONG smpte));
PRIVATE BOOLEAN midi_out_puf		_((RTMCLASSP module, PUF_INF *akt, PUF_INF *alt, BOOLEAN force));
PRIVATE BOOLEAN send_event			_((RTMCLASSP module, INT miditrack, INT vel));
PRIVATE BOOLEAN send_stop 			_((RTMCLASSP module));
PRIVATE VOID	init_messages 		_((RTMCLASSP module));
PRIVATE VOID	init_standard 		_((RTMCLASSP module));
PRIVATE VOID	init_events 		_((RTMCLASSP module));
PRIVATE SHORT	init_midishare 	_((VOID));

PRIVATE BOOL RemoveEvent (PUFEVP event);
PRIVATE PUFEVP CreateEvent (VOID);
PRIVATE VOID DestroyEvent (PUFEVP event);
PRIVATE BOOL InsertEvent (PUFEVP prev, PUFEVP event);
PRIVATE BOOL RemoveEvent (PUFEVP event);
PRIVATE PUFEVP NextEvent (PUFEVP event);
PRIVATE PUFEVP PrevEvent (PUFEVP event);

PRIVATE BOOLEAN init_rsc			_((VOID));
PRIVATE BOOLEAN term_rsc			_((VOID));

/*****************************************************************************/

PUBLIC VOID cdecl receive_evts_puf (int refNum)
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
					posit(module, status->posit);    /* find actual position */
					myTask = MidiDTask(delayed_task_puf, MidiGetTime(), refNum, (LONG)PUFDTaskReset, 0, 0);
					/*
					if (status->locator[0])						/* Position plausibel? */
						if (man>0)					/* MAN-Modul vorhanden ? */
							if (man->apply >0)	/* MAN apply-Funktion da? */
								(man->apply)(man, &(status->locator[0]->event)); /* Play this event */
					*/
					status->new	  = TRUE;
					window->milli = 1;
				} /* if */
				break;
			case typeRTMCont:
				if (status->sync)
				{
					status->posit 		= get_posit((MidiSTPtr)event);
					status->play = TRUE;
					myTask = MidiTask(play_task_puf, MidiGetTime() + QUANT, refNum, 0, 0, 0);
					window->milli = 1;
					status->new	  = TRUE;
				} /* if */
				break;
			case typeRTMStop:
				if (status->sync)
				{
					myTask = MidiDTask(delayed_task_puf, MidiGetTime(), refNum, (LONG)PUFDTaskReset, 0, 0);
					/*
					if (man>0)						/* MAN-Modul vorhanden ? */
						if (man->reset >0)		/* MAN reset-Funktion da? */
							(man->reset)(man);
					*/
					status->posit 		= get_posit((MidiSTPtr)event);
					if (status->play) send_stop (module);
					status->play = FALSE;
					window->milli = 1;
					status->new	  = TRUE;
				} /* if */
				break;
*/
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
				window->milli = 1;
				break;
		} /* switch */
		MidiFreeEv (event);
	} /* for */
} /* receive_evts_puf */

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

/*****************************************************************************/

PRIVATE PUFEVP insert_ev_puf(PUFEVP location)
{
	PUFEVP	new_event;
	
	new_event = (PUFEVP) mem_alloc(sizeof(PUFEVENT));
	if(new_event)
	{
		mem_set(new_event, 0, (UWORD)sizeof(PUFEVENT));
	
		/* Event einklinken */
		new_event->prev	= location;
		new_event->next	= location->next;
		location->next		= new_event;
		(new_event->next)->prev	= new_event;
		
		return (new_event);
	} /* if */
	else
	{
   	hndl_alert (ERR_NOMEMORY);
   	return (FALSE);
   } /* else */

} /* insert_ev_puf */

PUBLIC VOID cdecl delayed_task_puf (LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3)
{
	/* Wird aufgerufen, um nicht EchtzeitfÑhige Funktionen auszufÅhren */
	RTMCLASSP	module 	= modulep[refNum];
	WORD action = (WORD)a1;

	switch (action)
	{
		case PUFDTaskReset:
			module->reset (module);
			break;
		case PUFDTaskPrecalc:
			module->precalc (module);
			break;
	} /* switch */
} /* delayed_task_puf */

PUBLIC VOID cdecl play_task_puf (LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3)
{
	/* Wird soundso oft aufgerufen, um neue Daten in
		das Fenster einzublenden und neue Koordinaten zu berechnen und speichern */

	RTMCLASSP	module 	= modulep[refNum], man = module->status->manmodule;
	STAT_P		status 	= module->status;
	SET_P			akt 		= module->actual->setup;
	WINDOWP		window 	= module->window;
	PUFEVP		location, header = status->header, *locator = status->locator;
	REG KOOR_SINGLE	*koor, *ekoor;				/* Koordinaten-Struktur */
	REG TRACK_SINGLE	*track, *etrack;			/* Spur-Zuweisung */
	REG VOL_SINGLE		*volume, *evolume;		/* General-Volume aus BIG */
	REG WORD 	signal;
	PUF_INF		*event, *tmp = &module->status->tmp_event;
	MidiEvPtr	myTask;
	BOOLEAN		ret,
					record 		= status->record,
					force			= status->new,
					*ausgabe		= akt->ausgabe,
					*rec_an 		= akt->rec_an,
					*rec_x 		= akt->rec_x,
					*rec_y 		= akt->rec_y,
					*rec_z 		= akt->rec_z,
					*rec_vol 	= akt->rec_vol,
					*rec_dub 	= akt->rec_dub;
	
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
			myTask = MidiDTask(delayed_task_puf, MidiGetTime() + QUANT/2, refNum, (LONG)PUFDTaskPrecalc, 0, 0);
			myTask = MidiTask(play_task_puf, MidiGetTime() + QUANT, refNum, 0, 0, 0);
		} /* if play */
		
		/* Zeit weiterzÑhlen */
		status->posit += QUANT;

		/* Die aktuellen Koordinaten feststellen */
		location = locator[0];
		
		/* Zeiger auf nÑchsten Event setzen */
		if (location->next == header)
		{
			/* Kein Event mehr frei, evtl. vorne anfangen ... */
			if (status->cycle)
			{
				location = header->next;
			} /* if */
			else
				rtm_stop(refNum, status->posit); /* ... oder anhalten */
		} /* if */
		else
		{
			/* Sonst: Locator auf nÑchsten Event setzen */
			locator[0] = location->next;	
		} /* else */
		
		/* Zeiger auf Event-Information holen */
		event 	= &location->event;
		ekoor		= event->koors->koor;
		etrack	= event->tracks->track;
		evolume	= event->volumes->volume;

		/* Zeiger auf temporÑren Event setzen */
		/* Zeiger setzen */		
		koor		= tmp->koors->koor;
		track 	= tmp->tracks->track;
		volume	= tmp->volumes->volume;
		
			
		/* TemporÑren Event aufbauen */
		for (signal = 0; signal < MAXSIGNALS; signal++)
		{
			/* Soll dieses Signal aufgenommen werden ? */
			if ( (!record && rec_an[signal]) || (record && ausgabe[signal] && rec_dub[signal]))
			{
				/* Aus Sequenzer holen und in tmp Event kopieren */
				koor[signal]		= ekoor[signal];
				track[signal]		= etrack[signal];
				volume[signal]		= evolume[signal];
			} /* if */
			else
			{
				/* Auf Standard-Wert setzen */
				koor[signal].koor.x = 0;
				koor[signal].koor.y = 0;
				koor[signal].koor.z = 0;
				koor[signal].volume = 127;
				track[signal] 	= signal;
				volume[signal] = 127;
			} /* else */
		} /* for */
	
	
		/* Modul-Manager aufrufen */
		
		if (man>0)					/* MAN-Modul vorhanden ? */
			if (man->apply >0)	/* MAN apply-Funktion da? */
				tmp = man->apply(man, tmp);	/* Werte werden verÑndert */

		if (location != header)
		{
			for (signal = 0; signal < MAXSIGNALS; signal++)
			{
				if (record && rec_an[signal] && ausgabe[signal])
				{
					/* Werte aus Sequenzer kopieren */
					if (rec_x[signal])
						ekoor[signal].koor.x		= koor[signal].koor.x;
					if (rec_y[signal])
						ekoor[signal].koor.y		= koor[signal].koor.y;
					if (rec_z[signal])
						ekoor[signal].koor.z		= koor[signal].koor.z;
					if (rec_vol[signal])
						ekoor[signal].volume =  koor[signal].volume;
					etrack[signal]				= track[signal];
					/* evolume[signal]				= volume[signal]; */
				} /* if */
			} /* for */
			/* Midi-Ausgabe aufrufen mit altem und neuen Event */
			ret = midi_out_puf(module, event, &location->prev->event, force);
		} /* if */
		else	
		{
			mem_set (ekoor, 0, (UWORD) sizeof(KOOR_ALL));
			mem_set (etrack, 0, (UWORD) sizeof(TRACK_ALL));
			mem_set (evolume, 0, (UWORD) sizeof(VOL_ALL));
			/* Midi-Ausgabe aufrufen mit altem und neuen Event */
			ret = midi_out_puf(module, event, NULL, TRUE);
		} /* else */
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
} /* play_task_puf */

PRIVATE PUFEVP find_event (RTMCLASSP module, LONG smpte)
{
	/* Event zu einem SMPTE-Zeitpunkt finden */
	STAT_P		status = module->status;
	REG LONG		pos;
	REG PUFEVP	header = status->header, location = header;
	
	if(smpte <= status->max_events*QUANT)
	{
		/* Bei Null anfangen und vorfahren, bis Position erreicht. */
		for (pos = 0; pos<smpte; pos+= QUANT)
			location = location->next;
	} /* if */
	else
	{
		/* Letzte Position */
		location = header->prev;
	} /* if */
	return (location);
} /* find_event */

PRIVATE VOID position (RTMCLASSP module, LONG posit)
{
	/* Locator auf SMPTE-Zeit setzen */
	STAT_P		status = module->status;

	status->locator[0] = find_event(module, posit);
	status->new = TRUE;					/* Koor neu ausgeben */
	status->posit = posit;
} /* position */

/* Alte PUF-BIG-Routinen
GLOBAL EVENT_INFO *get_event_puf(RTMCLASSP module, LONG smpte, EVENT_INFO *event)
{
	/* Koordinaten zu einem bestimmten Zeitpunkt abfragen */
	
	PUFEVP 	location = find_event(module, smpte);
	PUF_INF	*pevent = &location->event;
	UBYTE track = event->track;
	
	*event->koor_0 = pevent->koors->koor[track];		/* Daten hineinkopieren */
	*event->koor_1 = pevent->koors->koor[track+1];	/* Daten hineinkopieren */
	return (event);	/* Zeiger auf den Event zurÅckgeben */
} /* get_event_puf */

GLOBAL BOOLEAN set_event_puf (RTMCLASSP module, LONG smpte, EVENT_INFO *event)
{
	/* Koordinaten eines Zeipunktes setzen */
	
	PUFEVP	location =  find_event(module, smpte);
	PUF_INF	*pevent = &location->event;
	STAT_P	status = module->status;
	UBYTE 	track = event->track;
	
	if (location != status->header)
	{
		pevent->koors->koor[track]		= *event->koor_0; /* Koordinaten Åbernehmen */
		pevent->koors->koor[track+1]	= *event->koor_1; /* Koordinaten Åbernehmen */
		return (TRUE);		/* Hat geklappt! */
	} /* if */
	else
		return (FALSE);	/* Kein Event vorhanden fÅr diesen Zeitpunkt */
} /* set_event_puf */
*/

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	return event;
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	RTMCLASSP man = module->status->manmodule;
	STAT_P	status = module->status;
		
	/* ZurÅcksetzen von Werten */
	if (man>0)					/* MAN-Modul vorhanden ? */
		if (man->reset >0)	/* MAN reset-Funktion da? */
			(man->reset)(man);
	status->new	  = TRUE;
} /* reset */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	RTMCLASSP man = module->status->manmodule;

	/* Vorausberechnung */
	if (man>0)					/* MAN-Modul vorhanden ? */
		if (man->precalc >0)	/* MAN precalc-Funktion da? */
			(man->precalc)(man);
} /* precalc */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	MidiEvPtr	myTask;
	UWORD		variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	LONG		posit 	= ((MSG_RTM_POSIT *)msg)->posit;
	STAT_P	status	= module->status;
	SET_P		akt		= module->actual->setup;
	WINDOWP	window	= module->window;
	WORD		refNum = (WORD)module->special;

	switch(type)
	{
		case RTM_POSIT:
			if (status->sync)
			{
				position(module, status->posit);    /* find actual position */
				module->reset (module);
				status->new	  		= TRUE;
				window->milli = 1;
			} /* if */
			break;
		case RTM_CONT:
			if (status->sync)
			{
				position(module, status->posit);    /* find actual position */
				status->play = TRUE;
				status->new	  = TRUE;
				window->milli = 1;
				myTask = MidiTask(play_task_puf, MidiGetTime(), refNum, 0, 0, 0);
			} /* if */
			break;
		case RTM_STOP:
			if (status->sync)
			{
				module->reset (module);
				status->posit 			= posit;
				if (status->play)	send_stop (module);
				status->play = FALSE;
				window->milli = 1;
				status->new	  = TRUE;
			} /* if */
			break;
		case SET_VAR:
			switch (variable)
			{
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
					if(variable>=VAR_PUF_REC_SIG0 && variable<VAR_PUF_REC_SIG0 + MAXSIGNALS)
						akt->rec_an[variable - VAR_PUF_REC_SIG0] = (BOOLEAN) value;
					else if(variable>=VAR_PUF_PLAY_SIG0 && variable<VAR_PUF_PLAY_SIG0 + MAXSIGNALS)
						akt->ausgabe[variable - VAR_PUF_PLAY_SIG0] = (BOOLEAN) value;
			} /* switch */
			window->milli = 1;
			status->new	  = TRUE;
		break;
	} /* switch */
} /* message */

PRIVATE BOOLEAN midi_out_puf (RTMCLASSP module, PUF_INF *akt, PUF_INF *alt, BOOLEAN force)
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
					new = status->new;	
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
				
	for (signal = 0; signal < MAXSIGNALS; signal ++)
	{
		track = akt->tracks->track[signal];
		if(track >= 0 && track < 2*MAXINPUTS)
		{
			akt_s = &(akt->koors->koor[signal]);
			alt_s = &(alt->koors->koor[signal]);

			if (status->new || (track != alt->tracks->track[signal]))
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
	
	status->new = FALSE;					/* Koor ausgegeben */
	return (ret);
} /* midi_out_puf */

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
			channel	= var_get_value (var_module, VAR_CMI_CHANNEL1);
			port		= var_get_value (var_module, VAR_CMI_PORT1);
			miditrack	-= 0;
		} /* if */
		else
		{
			/* Ausgabe auf das zweite CMI-System */
			channel	= var_get_value (var_module, VAR_CMI_CHANNEL2);
			port		= var_get_value (var_module, VAR_CMI_PORT2);
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

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STR128 	s;
	WORD		signal, offset;

	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFANZ1-PUFANZ0)*signal;
		ed->anz_an[signal] = get_checkbox (puf_setup, PUFANZ0 + offset);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECON1-PUFRECON0)*signal;
		ed->rec_an[signal] = get_checkbox (puf_setup, PUFRECON0 + offset);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECX1-PUFRECX0)*signal;
		ed->rec_x[signal] = get_checkbox (puf_setup, PUFRECX0 + offset);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECY1-PUFRECY0)*signal;
		ed->rec_y[signal] = get_checkbox (puf_setup, PUFRECY0 + offset);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECZ1-PUFRECZ0)*signal;
		ed->rec_z[signal] = get_checkbox (puf_setup, PUFRECZ0 + offset);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECVOL1-PUFRECVOL0)*signal;
		ed->rec_vol[signal] = get_checkbox (puf_setup, PUFRECVOL0 + offset);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECDUB1-PUFRECDUB0)*signal;
		ed->rec_dub[signal] = get_checkbox (puf_setup, PUFRECDUB0 + offset);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFAUSG1-PUFAUSG0)*signal;
		ed->ausgabe[signal] = get_checkbox (puf_setup, PUFAUSG0 + offset);
	} /* for */

	ed->anz_x = get_checkbox (puf_setup, PUFANZX);
	ed->anz_y = get_checkbox (puf_setup, PUFANZY);
	ed->anz_z = get_checkbox (puf_setup, PUFANZZ);
	ed->anz_vol = get_checkbox (puf_setup, PUFANZVOL);
	
	get_ptext (puf_setup, PUFBREITE, s);
	sscanf (s, "%d", &ed->breite);
	get_ptext (puf_setup, PUFHOEHE, s);
	sscanf (s, "%d", &ed->hoehe);

	if (get_checkbox (puf_setup, PUFREIHENFSIG + 0)) ed->reihenfolge = RSIGNALE;
	if (get_checkbox (puf_setup, PUFREIHENFKOOR + 0)) ed->reihenfolge = RKOOR;

} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STR128 	s;
	WORD		signal, offset;

	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFANZ1-PUFANZ0)*signal;
		set_checkbox (puf_setup, PUFANZ0 + offset, ed->anz_an[signal]);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECON1-PUFRECON0)*signal;
		set_checkbox (puf_setup, PUFRECON0 + offset, ed->rec_an[signal]);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECX1-PUFRECX0)*signal;
		set_checkbox (puf_setup, PUFRECX0 + offset, ed->rec_x[signal]);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECY1-PUFRECY0)*signal;
		set_checkbox (puf_setup, PUFRECY0 + offset, ed->rec_y[signal]);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECZ1-PUFRECZ0)*signal;
		set_checkbox (puf_setup, PUFRECZ0 + offset, ed->rec_z[signal]);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECVOL1-PUFRECVOL0)*signal;
		set_checkbox (puf_setup, PUFRECVOL0 + offset, ed->rec_vol[signal]);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFRECDUB1-PUFRECDUB0)*signal;
		set_checkbox (puf_setup, PUFRECDUB0 + offset, ed->rec_dub[signal]);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(PUFAUSG1-PUFAUSG0)*signal;
		set_checkbox (puf_setup, PUFAUSG0 + offset, ed->ausgabe[signal]);
	} /* for */

	set_checkbox (puf_setup, PUFANZX, ed->anz_x);
	set_checkbox (puf_setup, PUFANZY, ed->anz_y);
	set_checkbox (puf_setup, PUFANZZ, ed->anz_z);
	set_checkbox (puf_setup, PUFANZVOL, ed->anz_vol);
	
	sprintf (s, "%d", ed->breite);
	set_ptext (puf_setup, PUFBREITE, s);
	sprintf (s, "%d", ed->hoehe);
	set_ptext (puf_setup, PUFHOEHE, s);

	set_checkbox (puf_setup, PUFREIHENFSIG + 0, ed->reihenfolge = RSIGNALE);
	set_checkbox (puf_setup, PUFREIHENFKOOR + 0, ed->reihenfolge = RKOOR);

	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (puf_setup, PUFSETNR, s);
	
} /* set_dbox */

PRIVATE VOID    send_messages	(RTMCLASSP module)
{
	ED_P		actual = module->actual;
	SET_P		akt = actual->setup;
	WORD 		signal;

	send_variable(VAR_SET_PUF, actual->number);
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		send_variable (VAR_PUF_REC_SIG0 + signal, akt->rec_an[signal]);
		send_variable (VAR_PUF_PLAY_SIG0 + signal, akt->ausgabe[signal]);
	} /* for */

} /* send_messages */
/*****************************************************************************/

LOCAL VOID click_setup (window, mk)
WINDOWP window;
MKINFO  *mk;

{
	RTMCLASSP	module = Module(window);
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	WORD		signal, offset;
	STRING	s;
	LONG		x;

	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case PUFSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case PUFSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case PUFSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case PUFSETSTORE:
			module->set_store (window, edited->number);
			break;
		case PUFSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case PUFOK   :
			module->set_ok (window);
			break;
		case PUFCANCEL:
			module->set_cancel (window);
		   break;
		case PUFHELP :
			module->help (module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case PUFSTANDARD:
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
				set_ptext (puf_setup, PUFSETNR, s);
				draw_object(window, PUFSETNR);
			} /* if */
			switch (window->exit_obj)
			{
				case PUFANZKOORALL:
					ed->anz_x 	= !ed->anz_x;
					ed->anz_y 	= !ed->anz_x;
					ed->anz_z 	= !ed->anz_x;
					ed->anz_vol = !ed->anz_vol;
					set_checkbox (puf_setup, PUFANZX, ed->anz_x);
					draw_object(window, PUFANZX);
					set_checkbox (puf_setup, PUFANZY, ed->anz_y);
					draw_object(window, PUFANZY);
					set_checkbox (puf_setup, PUFANZZ, ed->anz_z);
					draw_object(window, PUFANZZ);
					set_checkbox (puf_setup, PUFANZVOL, ed->anz_vol);
					draw_object(window, PUFANZVOL);
               undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case PUFANZALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(PUFANZ1-PUFANZ0)*signal;
						ed->anz_an[signal] = !ed->anz_an[signal];
						set_checkbox (puf_setup, PUFANZ0 + offset, ed->anz_an[signal]);
						draw_object(window, PUFANZ0 + offset);
					} /* for */
               undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case PUFRECONALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(PUFRECON1-PUFRECON0)*signal;
						ed->rec_an[signal] = !ed->rec_an[signal];
						set_checkbox (puf_setup, PUFRECON0 + offset, ed->rec_an[signal]);
						draw_object(window, PUFRECON0 + offset);
					} /* for */
               undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case PUFRECXALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(PUFRECX1-PUFRECX0)*signal;
						ed->rec_x[signal] = !ed->rec_x[signal];
						set_checkbox (puf_setup, PUFRECX0 + offset, ed->rec_x[signal]);
						draw_object(window, PUFRECX0 + offset);
					} /* for */
               undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case PUFRECYALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(PUFRECY1-PUFRECY0)*signal;
						ed->rec_y[signal] = !ed->rec_y[signal];
						set_checkbox (puf_setup, PUFRECY0 + offset, ed->rec_y[signal]);
						draw_object(window, PUFRECY0 + offset);
					} /* for */
               undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case PUFRECZALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(PUFRECZ1-PUFRECZ0)*signal;
						ed->rec_z[signal] = ! ed->rec_z[signal];
						set_checkbox (puf_setup, PUFRECZ0 + offset, ed->rec_z[signal]);
						draw_object(window, PUFRECZ0 + offset);
					} /* for */
               undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case PUFRECVOLALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(PUFRECVOL1-PUFRECVOL0)*signal;
						ed->rec_vol[signal] = !ed->rec_vol[signal];
						set_checkbox (puf_setup, PUFRECVOL0 + offset, ed->rec_vol[signal]);
						draw_object(window, PUFRECVOL0 + offset);
					} /* for */
               undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case PUFRECDUBALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(PUFRECDUB1-PUFRECDUB0)*signal;
						ed->rec_dub[signal] = !ed->rec_dub[signal];
						set_checkbox (puf_setup, PUFRECDUB0 + offset, ed->rec_dub[signal]);
						draw_object(window, PUFRECDUB0 + offset);
					} /* for */
               undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case PUFAUSGALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(PUFAUSG1-PUFAUSG0)*signal;
						ed->ausgabe[signal] = !ed->ausgabe[signal];
						set_checkbox (puf_setup, PUFAUSG0 + offset, ed->ausgabe[signal]);
						draw_object(window, PUFAUSG0 + offset);
					} /* for */
               undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
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
	WINDOWP window;
	WORD    ret;
	
	window = search_window (CLASS_DIALOG, SRCH_ANY, PUF_SETUP);
	
	if (window == NULL)
	{
		form_center (puf_setup, &ret, &ret, &ret, &ret);
		
		window = crt_dialog (puf_setup, NULL, PUF_SETUP, (BYTE *)puf_text [FPUFSN].ob_spec, WI_MODELESS);
		
	} /* if */
		
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (puf_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			
			module = define_setup(window, (RTMCLASSP)refwindow->module); 
			window->module = (VOID*) module;

			module->set_edit (module);
			module->set_dbox (module);
			undo_state (window->object, PUFHELP, DISABLED);
		} /* if */
		
		if (! open_dialog (PUF_SETUP)) hndl_alert (ERR_NOOPEN);
	
	} /* if */
} /* dsetup */

PRIVATE RTMCLASSP define_setup (WINDOWP window, RTMCLASSP refmodule)
{
	RTMCLASSP module;
	
	if (window != NULL)
	{
		module = create_module(module_name, instance_count);
		/* Informationen kopieren */
		mem_move (module, refmodule, (UWORD)sizeof (RTMCLASS));
		window->click    = click_setup;
		module->apply		= 0;
		module->reset		= 0;
		module->precalc	= 0;
		module->object_type	= MODULE_OTHER;
		sprintf(module->object_name, "PUF Setups");
	} /* if */
	return module;
} /* define_setup */

/*****************************************************************************/

PRIVATE VOID handle_menu_mod (window, title, item)
WINDOWP window;
WORD    title, item;

{
	RTMCLASSP	module 	= Module(window);
	STAT_P		status 	= module->status;
	SET_P			akt 		= module->actual->setup;
	PUFEVP		location, header = status->header, *locator = status->locator;
	SHORT			refnum = (LONG)module->special;
	
	if (window != NULL)
		menu_normal (window, title, FALSE);         /* Titel invers darstellen */
	
	switch (title)
	{
		case MPUFINFO:
			switch (item)
			{
				case MPUFINFOANZEIG:
					info_mod(window, NIL);
					break;
			} /* switch */
			break;
		case MPUFCOMMANDS:
			switch (item)
			{
			} /* switch */
			break;
		case MPUFANZEIGEN:
			switch (item)
			{
				case MPUFSETUPS:
					dsetup(window);
					break;
			} /* switch */
			break;
		case MPUFCOMM:
			switch (item)
			{
				case MPUFCOMMBIG:
					send_part_big (refnum, locator[0]);
					break;
			} /* switch */
			break;
		case MPUFOPTIONS:
			switch (item)
			{
				case MPUFSETUPS:
					dsetup(window);
					break;
			} /* switch */
			break;
	} /* switch */
	
	if (window != NULL)
		menu_normal (window, title, TRUE);          /* Titel wieder normal darstellen */
} /* handle_menu_mod */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

GLOBAL VOID wi_draw_mod (window)
WINDOWP window;
{
	RTMCLASSP	module = Module (window);
	RECT		r;
	WORD		h = gl_hbox, w = gl_wbox, x0, y0, offset;
	STRING	s;
	STAT_P	status;

	clr_scroll (window);
#ifdef TEST
	if (module)
	{
		status = module->status;
		/* X-Y-Offsets holen */
		rc_copy (&window->scroll, &r);
		x0 = r.x;
		y0 = r.y;
	
		text_default(vdi_handle);
		/* Zeitmessung */
		sprintf(s, "Interrupt:     %8ld msec", (status->stop_cl - status->start_cl));
		v_gtext (vdi_handle, x0 + 2 * w, y0 + 5*h, s);
		sprintf(s, "Window-Update: %8ld msec", (MidiGetTime() - status->last_update) );
		v_gtext (vdi_handle, x0 + 2 * w, y0 + 6*h, s);
		status->last_update = MidiGetTime ();
	} /* if */
#endif
} /* wi_draw_mod */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (window)
WINDOWP window;
{
	RTMCLASSP	module = Module (window);
	SHORT			refNum = (SHORT)module->special;
	LONG			numtasks;		
	
/* BD 2012_01_21: Midi blockieren
	
	/* Max. einen Event ausfÅhren */
	numtasks = MidiCountDTasks(refNum);
	if (numtasks > 0)
	{
		MidiExec1DTask(refNum);
		/* Den Rest wegschmeissen */
		if (numtasks > 1)
			MidiFlushDTasks(refNum);
	} /* if numtasks */
*/

/*
	/* Alle Delayed-Tasks ausfÅhren */
	for (numtasks = MidiCountDTasks (refNum); numtasks > 0; numtasks--)
	{
		MidiExec1DTask(refNum);
	} /* for numtasks */
*/	
	window->milli = 0; /* Timer b.a.w. abschalten */

	if (window->opened >0)
		redraw_window(window, &window->scroll);

} /* wi_timer_mod */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
	WINDOWP window;
	WORD    menu_height, inx;
	
	inx    = num_windows (CLASS_PUF, SRCH_ANY, NULL);
	window = create_window_obj (KIND, CLASS_PUF);
	
	if (window != NULL)
	{
		
		WINDOW_INITOBJ_OBJ

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
		window->bg_color  = -1;
		window->mousenum  = ARROW;
		window->mouseform = NULL;
		window->milli     = MILLI;
		window->special   = 0;
		window->edit_obj  = 0;
		window->edit_inx  = 0;
		window->exit_obj  = 0;
		window->object    = 0;
		window->menu      = menu;
		window->hndl_menu = handle_menu_mod;
		window->draw		= wi_draw_mod;
		window->timer     = wi_timer_mod;
		window->timer     = wi_timer_mod;
		window->showinfo  = info_mod;
		
		sprintf (window->name, (BYTE *)puf_text [FPUFN].ob_spec);
		sprintf (window->info, (BYTE *)puf_text [FPUFI].ob_spec, 0);
	} /* if */
	
	return (window);                      /* Fenster zurÅckgeben */
} /* crt_mod */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN ok;
	WINDOWP window;
	
	window = search_window (CLASS_PUF, SRCH_ANY, icon);
			
	if (window != NULL)
	{
		if (window->opened == 0)
		{	
			if (! open_window (window)) hndl_alert (ERR_NOOPEN);
		} /* if */
		else top_window (window);
	} /* if */
			
	ok= window !=0;
			
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
	WORD			ret;
	STRING		s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, IPUF);
		
	if (window == NULL)
	{
		 form_center (puf_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (puf_info, NULL, IPUF, (BYTE *)puf_text [FPUFIN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		sprintf(s, "%-20s", PUFDATE);
		set_ptext (puf_info, PUFIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (puf_info, PUFCOMPILE, s);
		sprintf(s, "%-20s", PUFVERSION);
		set_ptext (puf_info, PUFIVERNR, s);
		if (module)
			sprintf(s, "%-20ld", module->status->max_events);
		else		
			sprintf(s, "(kein Modul selektiert)    ");
		set_ptext (puf_info, PUFIEVENTS, s);
			
		/* sprintf(s, "%-20ld           ", puf_free); */
		sprintf(s, "(ohne Funktion)");
		set_ptext (puf_info, PUFIFREE, s);

		if (! open_dialog (IPUF)) hndl_alert (ERR_NOOPEN);
	}
	
	return (window != NULL);
} /* info_mod */

/*****************************************************************************/
/* Kreiere Modul                                                             */
/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	RTMCLASSP 	module;
	WINDOWP		window;
	STAT_P		status;
	FILE			*fp;
	SHORT			refNum;

	module = create_module (module_name, instance_count);
		
	if (module != NULL)
	{	
		module->class_number	= CLASS_PUF;
		module->icon			= &puf_desk[PUFICON];
		module->icon_position= IPUF;
		module->icon_number	= IPUF;	/* Soll bei Init vergeben werden */
		module->menu_title	= MPUFS;
		module->menu_position= MPUF;
		module->menu_item		= MPUF;	/* Soll bei Init vergeben werden */
		module->multiple		= FALSE;
		
		module->crt				= crt_mod;
		module->open			= open_mod;
		module->info			= info_mod;
		module->init			= init_puf;
		module->term			= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_OTHER;
		module->apply				= apply;
		module->reset				= reset;
		module->precalc			= precalc;
		module->message			= message;
		module->create				= create;
		module->destroy			= destroy_mod;
		module->test				= test_obj;
	
		module->file_pointer			= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->file_pointer, 0, (UWORD)sizeof(FILE));
		module->import_pointer		= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->import_pointer, 0, (UWORD)sizeof(FILE));
		module->setup_length			= sizeof(SETUP);
		module->location				= SETUPS_EXTERN;
		module->ram_modified 		= FALSE;
		module->max_setups		 		= MAXSETUPS;
		sprintf(module->file_name, 		"%sDEFAULT.PUF", setup_path);
		sprintf(module->file_extension,	"PUF");
		sprintf(module->file_version,		"PUF V 1.00\n");
		sprintf(module->import_name, 		"%sPUF.EXP", import_path);
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
		status->play = FALSE;
		status->note_off = FALSE;

		/* TemporÑrer Event */
		status->tmp_event.koors 	= &status->tmp_koors;
		status->tmp_event.tracks	= &status->tmp_tracks;
		status->tmp_event.volumes	= &status->tmp_vols;

		/* Standard-Module */
		status->varmodule = var_module;
		status->manmodule = man_module;
		status->tramodule = tra_module; 
		init_events(module);
		
		/* Setup-Strukturen initialisieren */
		init_standard(module);
		
		/* Fenster generieren */
		window = crt_mod (puf_setup, puf_menu, IPUF);
		/* Modul-Struktur einbinden */
		window->module		= (VOID*) module;
		module->window		= window;
		refNum				= init_midishare();
		module->special	= (LONG) refNum;
		modulep[refNum]	= module;
		if (refNum>0)
			InstallFilter(refNum);									/* Midi-Input-Filter einbauen */	
		init_messages(module);

		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		puf_module = module;	/* globaler Zeiger auf PUF-Modulparameter */
	} /* if */
	
	return module;
} /* create */

/*****************************************************************************/
/* Lîsche Objekt                                                            */
/*****************************************************************************/
PUBLIC VOID destroy_mod (module)
RTMCLASSP module;
{
	STRING 	s;
	INT		refNum;
	STAT_P	status = module->status;
	WORD		count = 0;
	PUFEVP	event, header;
		
	if (msh_available)
	{
		for (refNum = 0; module != modulep[refNum]; refNum++);
	
		if (refNum > 0)
		{
			refNums[instance_count--] = 0;
			MidiClose (refNum);				/* abmelden	*/
		} /* if */
		
	} /* if */
	sprintf (s, "%ld Events werden freigegeben ...", status->max_events);
	daktstatus(" PUF-Terminierung ", s);

#if FALSE
	header = status->header;
	event = NextEvent (header);
	while (event != header)
	{
		DestroyEvent (event);
		event = NextEvent (header);
		count++;
		if (count % 1000 == 0)
			set_daktstat((WORD)(100L*count/status->max_events));
	} /* while */
#else
	mem_free (status->events_p);
	mem_free (status->koors_p);
	mem_free (status->tracks_p);
	mem_free (status->vols_p);

	mem_free (status->locator);
#endif

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
			MidiSetRcvAlarm(refNum, receive_evts_puf);	/* Interrupt-Handler */		
			/* An alle anschlieûen */
			try_all_connect (refNum);
		} /* if */
	} /* if */
	
	return refNum;
	
} /* init_midishare */

PRIVATE VOID init_messages (RTMCLASSP module)
{
	WORD	signal;

	add_rcv(VAR_SET_PUF,  module);	/* Message einklinken */
	var_set_max(var_module, VAR_SET_PUF, MAXSETUPS);
	add_rcv(VAR_RECORD, module);	/* Message einklinken */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		add_rcv(VAR_PUF_REC_SIG0 + signal, module);	/* Message einklinken */
		add_rcv(VAR_PUF_PLAY_SIG0 + signal, module);	/* Message einklinken */
	}
	add_rcv(VAR_PUF_PLAY, module);	/* Message einklinken */
	add_rcv(VAR_PUF_PAUSE, module);	/* Message einklinken */
	add_rcv(VAR_PUF_ZEITL, module);	/* Message einklinken */

} /* init_messages */

PRIVATE VOID init_standard (RTMCLASSP module)
{
	SET_P			standard = module->standard;
	WORD			signal;

	mem_set(standard, 0, (UWORD) sizeof(SETUP));

	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		standard->anz_an[signal] = TRUE;

		standard->rec_an[signal] = TRUE;
		standard->rec_x[signal] = TRUE;
		standard->rec_y[signal] = TRUE;
		standard->rec_z[signal] = TRUE;
		standard->rec_vol[signal] = TRUE;
		standard->rec_dub[signal] = FALSE;

		standard->ausgabe[signal] = TRUE;
	} /* for */
	standard->anz_x = TRUE;
	standard->anz_y = TRUE;
	standard->anz_z = TRUE;
	standard->anz_vol = TRUE;
	standard->breite = 5;
	standard->hoehe = 5;
	standard->reihenfolge = RSIGNALE;
} /* init_standard */

PRIVATE VOID init_events (RTMCLASSP module)
{
	STAT_P		status = module->status;
	LONG			event, max_events = status->max_events, event_size;
	PUFEVP		location, previous, header = status->header, *locator = status->locator;
	KOOR_ALL		*koors;
	TRACK_ALL	*tracks;
	VOL_ALL		*volumes;
	WORD			x;
	STRING		s;
	BOOL			ok = FALSE;

	/* Die HÑlfte des Speichers reservieren */
	event_size = sizeof(PUFEVENT)
						+sizeof(KOOR_ALL)
						+sizeof(TRACK_ALL)
						+sizeof(VOL_ALL);

	max_events = mem_avail()/(event_size)/2 ;
	/* BD 2012_01_21 reduced max size */
		if (max_events>1000) 
			max_events = 1000;
		
	/* Events verketten */
	sprintf (s, "%ld Events ...", max_events);
	daktstatus(" PUF-Initialisierung", s);

#if FALSE
	header	= CreateEvent();
#else
	do {
		header	= (PUFEVP) mem_alloc(max_events * sizeof(PUFEVENT));
		koors		= (KOOR_ALL*) mem_alloc(max_events * sizeof(KOOR_ALL));
		tracks	= (TRACK_ALL*) mem_alloc(max_events * sizeof(TRACK_ALL));
		volumes	= (VOL_ALL*) mem_alloc(max_events * sizeof(VOL_ALL));

		/* PrÅfen ob Allozieren geklappt hat */
		if(!header || !koors || !tracks || !volumes)
		{
			/* Alles wieder freigeben, wenn nicht ok. */
			mem_free (header);
			mem_free (koors);
			mem_free (tracks);
			mem_free (volumes);
			/* Kleineres StÅck probieren */
			max_events /= 2;
		}
		else
			ok = TRUE;
	} while (!ok);

	/* Speicheradressen merken fÅr mem_free() in destroy */
	status->events_p	= header;
	status->koors_p	= koors;
	status->tracks_p	= tracks;
	status->vols_p		= volumes;

	mem_lset(header, 0, max_events * sizeof(PUFEVENT));
	mem_lset(koors, 0, max_events * sizeof(KOOR_ALL));
	mem_lset(tracks, 0, max_events * sizeof(TRACK_ALL));
	mem_lset(volumes, 0, max_events * sizeof(VOL_ALL));

	/* Header Initialisieren */
	header->next = header;
	header->prev = header;
	header->event.koors	= &koors[0];	/* Zeiger auf zugehîrigen KOOR_ALL Block */
	header->event.tracks	= &tracks[0];	/* Zeiger auf zugehîrigen TRACK_ALL Block */
	header->event.volumes	= &volumes[0];	/* Zeiger auf zugehîrigen VOL_ALL Block */
#endif

	/* Array reservieren fÅr zehn Locator */
	locator = (PUFEVP *) mem_alloc(10 * sizeof(PUFEVP));
	
	for (x = 0; x < 10; x++)
	{
		locator[x] = header;
	} /* for */
	
	event = 0;
	previous = header;
	ok = TRUE;
	while (ok && event<max_events)
	{
#if FALSE
		location = CreateEvent ();
		event++;
#else
		event++;
		location = &header[event];					/* Zeiger auf diesen Event holen */
		location->event.koors	= &koors[event];	/* Zeiger auf zugehîrigen KOOR_ALL Block */
		location->event.tracks	= &tracks[event];	/* Zeiger auf zugehîrigen TRACK_ALL Block */
		location->event.volumes	= &volumes[event];	/* Zeiger auf zugehîrigen VOL_ALL Block */
#endif
		if (location)
		{
			InsertEvent (previous, location);
			previous = location;
			if (event % 1000 == 0)
				set_daktstat((WORD)(100L*event/max_events));
		} /* if event */
		else ok = FALSE;
	} /* while */
	set_daktstat(100);
	status->max_events	= event;
	status->header			= header;		/* Zeiger auf Header-Event Åbernehmen */
	*status->locator		= *locator;		/* Zeiger auf Locator-Array Åbernehmen */
	close_daktstat();
} /* init_events */

PRIVATE PUFEVP CreateEvent ()
{
	PUFEVP 	event;
	BOOL		ok;
	
	event = new(PUFEVENT);
	event->event.koors	= new(KOOR_ALL);		/* Speicher fÅr KOOR_ALL anfordern */
	event->event.tracks	= new(TRACK_ALL);		/* Speicher fÅr KOOR_ALL anfordern */
	event->event.volumes	= new(VOL_ALL);		/* Speicher fÅr KOOR_ALL anfordern */
	event->next = event;
	event->prev = event;
	/* PrÅfen ob alles geklappt hat */
	ok = event && 
			event->event.koors &&
			event->event.tracks &&
			event->event.volumes;
	if (!ok)
	{ /* Speicher wieder freigeben */
		mem_free (event->event.koors);
		mem_free (event->event.tracks);
		mem_free (event->event.volumes);
		mem_free (event);
		return NULL;
	} /* if not ok */
	
	return event;
} /* CreateEvent */

PRIVATE VOID DestroyEvent (PUFEVP event)
{
	mem_free (event->event.koors);
	mem_free (event->event.tracks);
	mem_free (event->event.volumes);
	mem_free (event);
} /* DestroyEvent */

PRIVATE BOOL InsertEvent (PUFEVP prev, PUFEVP event)
{
	PUFEVP	next;
	
	if (prev && event)
	{
		next = prev->next;
		event->prev	= prev;
		event->next	= next;
		prev->next	= event;
		next->prev	= event;
		return TRUE;
	} /* if */
	return FALSE;
} /* InsertEvent */

PRIVATE BOOL RemoveEvent (PUFEVP event)
{
	PUFEVP	prev, next;
	
	if (event)
	{
		next = event->next;
		prev = event->prev;
		
		if (next && prev)
		{
			next->prev = prev;
			prev->next = next;
		} /* if next and prev */
		event->prev	= NULL;
		event->next	= NULL;
		return TRUE;
	} /* if */
	return FALSE;
} /* InsertEvent */

PRIVATE PUFEVP NextEvent (PUFEVP event)
{
	return event->next;
} /* NextEvent */

PRIVATE PUFEVP PrevEvent (PUFEVP event)
{
	return event->prev;
} /* PrevEvent */

/*****************************************************************************/
/* RSC îffnen                                                      		     */
/*****************************************************************************/

PRIVATE BOOLEAN init_rsc ()

{
  WORD   i, y, iconw, iconh, iconr;
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
  puf_setup = (OBJECT *)rs_trindex [PUF_SETUP]; /* Adresse der PUF-Setup-Box */
  puf_shelp = (OBJECT *)rs_trindex [PUF_SHELP];	/* Adresse der PUF-Setup-Hilfe */
  puf_help  = (OBJECT *)rs_trindex [PUF_HELP];	/* Adresse der PUF-Hilfe (allg)*/
  puf_desk  = (OBJECT *)rs_trindex [PUF_DESK];	/* Adresse des PUF-Desktops */
  puf_menu  = (OBJECT *)rs_trindex [PUF_MENU];  /* Adresse der PUF-MenÅzeile */
  puf_text  = (OBJECT *)rs_trindex [PUF_TEXT];  /* Adresse der PUF-Texte */
  puf_info 	= (OBJECT *)rs_trindex [PUF_INFO];	/* Adresse der PUF-Info-Anzeige */
#else

  strcpy (rsc_name, PUF_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (puf_rsc_ptr, rsc_name))
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

  rs_gaddr (puf_rsc_ptr, R_TREE,  PUF_MENU,	&puf_menu);    /* Adresse des PUF-MenÅs */
  rs_gaddr (puf_rsc_ptr, R_TREE,  PUF_SETUP,	&puf_setup);   /* Adresse der PUF-Setup-Box */
  rs_gaddr (puf_rsc_ptr, R_TREE,  PUF_SHELP,	&puf_shelp);   /* Adresse der PUF-Setup-Hilfe */
  rs_gaddr (puf_rsc_ptr, R_TREE,  PUF_HELP,	&puf_help);    /* Adresse der PUF-Hilfe */
  rs_gaddr (puf_rsc_ptr, R_TREE,  PUF_DESK,	&puf_desk);    /* Adresse der PUF-Desktop */
  rs_gaddr (puf_rsc_ptr, R_TREE,  PUF_TEXT,	&puf_text);    /* Adresse der PUF-Texte */
  rs_gaddr (puf_rsc_ptr, R_TREE,  PUF_INFO,	&puf_info);    /* Adresse der PUF-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (puf_menu, TRUE);
	fix_objs (puf_setup, TRUE);
	fix_objs (puf_shelp, TRUE);
	fix_objs (puf_help, TRUE);
	fix_objs (puf_desk, TRUE);
	fix_objs (puf_text, TRUE);
	fix_objs (puf_info, TRUE);
	
	do_flags (puf_setup, PUFCANCEL, UNDO_FLAG);
	do_flags (puf_setup, PUFHELP, HELP_FLAG);

	menu_enable(menu, MPUF, TRUE);

	return (TRUE);
} /* init_rsc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PRIVATE BOOLEAN term_rsc ()
{
  BOOLEAN ok = TRUE;

#if ((XRSC_CREATE|RSC_CREATE) == 0)
  ok = rs_free (puf_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_puf ()

{
	BOOLEAN		ok = TRUE;
	STR128 		s;
	
	ok &= init_rsc ();
	instance_count = load_create_infos (create, "PUF", max_instances);

	return (ok);
} /* init_puf */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;

	ok &= term_rsc ();
	return (ok);
} /* term_mod */
