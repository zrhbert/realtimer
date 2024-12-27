/*****************************************************************************/
/*                                                                           */
/* Modul: TRA.C                                                              */
/*                                                                           */
/* Transportleiste fÅr RTM                                                   */
/*                                                                           */
/*****************************************************************************/
#define TRAVERSION "V 1.06"
#define TRADATE "19.03.95"

/*****************************************************************************
V 1.06
- send_start_ev_delight und stop eingebaut und wieder auskommentiert, 19.03.95
- Thru fÅr alle event-typen ausser MTC und internen in receive_evts_tra, 18.03.95
- VAR_PUF_OFFSET in message, 14.03.95
- cast for evType to only take lower 4 bits in receive_evts_tra, 14.03.95
- Init-Reihenfolge in init_tra geÑndert, 04.03.95
- SMPTE send aktiviert, 23.02.95
- send_xxxx eingebaut, 19.02.95
- umstellung auf MS-freie start/stop/cont, 19.02.95
- MTC Routinen aus Delight eingebaut, 17.02.95
V 1.05 05.02.95
- Fernsteuerung fÅr REC/MTR/LFO eingebaut, 05.02.95
- send_mtr_accel Bug beseitigt, 03.02.95
- Tastenbedienung fÅr MTR und LFO Beschl. eingebaut, 09.01.95
- Midi-Thru eingebaut
- CNTRL_LEFT und CNTRL_RIGHT fÅr LOC0 und LOC1
V 1.04 23.12.94
- add rcv fÅr cycle on/start/stop
- Bug in FFWD beseitigt, 23.12.94
30.11.94
- Ton an
- create_tra in create umbenannt
15.07.94
- click immernoch nicht synchron
- tra_module eingebaut
- load_create_infos und instance_count eingebaut
- MS-Namen auf TRA geÑndert
- status->new	  = TRUE; bei sync und cycle
- msh_available eingebaut
- big_puf_sync eingebaut
- window->module eingebaut
- try_all_connect eingebaut
- destroy_mod eingebaut
- Umbau auf create_window_obj
V 1.03
- Abmelden alter MS-Applikationen
V 1.02
- Anpassung auf neue RTMCLASS-Struktur
V 1.01
- VAR-Update bei Initialisierung eingebaut
- Register-Variablen ausgebaut
V 1.00, 17.04.93
- Anzeigeformat auf 00.00.00.000 umgebaut fÅr MidiShare
- PUF und BIG an-/ausschalten eingebaut
- Format der Dialogbox umgebaut
- externe Synchronisation -> start/stop eingebaut
- act/inact Elemente ausgebaut
- Punch In/Out repariert
*****************************************************************************/
#ifndef XRSC_CREATE
/*#define XRSC_CREATE 1 */                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "tra_mod.h"
#include "realtspc.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"
#include "dispobj.h"

#include "var.h"
#include <msh_unit.h>		/* Deklarationen fÅr MidiShare Library */
#include "msh.h"

#include "export.h"
#include "tra.h"

#if XRSC_CREATE
#include "tra_mod.rsh"
#include "tra_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT | WI_MOUSE)
#define XFAC   gl_wbox                 /* X-Faktor */
#define YFAC   gl_hbox                 /* Y-Faktor */
#define XUNITS 1                       /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                       /* Y-Einheiten fÅr Scrolling */
#define INITX  1								/* X-Anfangsposition */
#define INITY  ( 3 * gl_hbox)				/* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)          /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)          /* Anfangshîhe in Pixel */
#define MILLI  10000							/* Millisekunden fÅr Zeitablauf */
#define TRA_RSC_NAME "TRA_MOD.RSC"		/* Name der Resource-Datei */

#define NO_REFNUM		-1

#define MAXMODULES 10						/* Max. Anz. Instanzen */
#define MAXSETUPS 0

enum DTaskTypes {TRADTaskReset, TRADTaskBeepLo, TRADTaskBeepHi};	/* Delayed Task Typen */

enum RECEIVE_MTC_SYNC
	{
	NO_RCV_MTC_SYNC,
	RCV_MTC_SYNC,
	TRY_RCV_MTC_SYNC
	};
	
enum RECEIVE_MC_SYNC
	{
	NO_RCV_MC_SYNC,
	RCV_MC_SYNC,
	TRY_RCV_MC_SYNC
	};

#define SYNC_RESOLUTION 1	 				/* Aufruftakt fÅr sync_send/receive in ms,
														muû kleiner sein als 1s/25*4 = 10ms (1QF)  */
#define LOCK_TIME 1000						/* Zeitdifferenz intern/mtc bei der MTC 
														neu initialisiert wird */
/****** TYPES ****************************************************************/
typedef	struct status *STAT_P;
typedef	struct status
{
	UINT	play		: 1	;	/* PLAY gedrÅckt */
	UINT	record	: 1	;	/* RECORD gedrÅckt */
	UINT	ffwd		: 1	;	/* >> gedrÅckt */
	UINT	rew		: 1	;	/* << gedrÅckt */
	UINT	cycle		: 1	;	/* Cycle-Modus an/aus*/
	UINT	master	: 1	;	/* Master-Track an/aus */
	UINT	click		: 1	;	/* Click an/aus */
	UINT	sync_in	: 1	;	/* Sync input an/aus */
	UINT	sync_out	: 1	;	/* Sync output an/aus */
	UINT	locked	: 1	;	/* Sync eingelockt */
	UINT	punch_in	: 1	;	/* Punch in (Aufnahmebeginn am linken Locator) */
	UINT	punch_out: 1	;	/* Punch out (Aufnahmeende am rechten Locator) */
	UINT	puf_sync	: 1	;	/* PUF synchron mitlaufen */
	UINT	big_sync	: 1	;	/* BIG synchron mitlaufen */
	UINT	big_puf_sync	: 1	;	/* BIG + PUF synchron mitlaufen */
	LONG	leftloc;				/* Linker Locator */
	LONG	rightloc;			/* Rechter Locator */
	LONG	posit;				/* SMPTE Zeit	*/
	INT	click_freq;			/* Click-Frequenz */	
	INT	click_duration;	/* Click-Zeit	*/
	INT	click_counter;		/* ZÑhler */
	TFilter	filter;			/* Midi-In-Filter	*/
	BOOLEAN new;				/* komplett neu zeichnen */
	BOOL snd_mtc;				/* Flag ob MTC Senden an */
	WORD snd_mtc_ports;		/* Flag fÅr jeden Port auf dem gesendet werden soll */
	WORD snd_mtc_frames;		/* Sende-Typ (0-4 entspricht 24-100 Frames) */
	LONG snd_mtc_offset;		/* Send Offset ( auch negative Werte ) */
	WORD mtc_type;				/* Sende-Typ in Bits zum "einodern" in die Quarter-Frame Message. */
	WORD ms_frames;  			/* Anzahl der Millisekunden pro Frame */
	WORD ms_quarter;			/* Anzahl der Millisekunden pro Quarter-Frame */
	BOOL rcv_mtc;				/* Flag ob MTC empfangen an. */
	WORD mtc_sync;				/* Flag ob die Synchronisation aus, eingelockt oder ob in Planung. */
	LONG mtc_time;				/* Aktuelle empfangene Zeit, korrigiert um das Åbliche Delay von zwei Frames. */
	LONG rcv_mtc_offset;		/* Receive Offset ( auch negative Werte ) */
} STATUS;

typedef struct setup *SET_P;

typedef struct setup
{
	VOID *dummy;
} SETUP;

typedef void * SmPtr;				/** Delight Event	**/

/****** VARIABLES ************************************************************/
PRIVATE WORD	tra_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*tra_rsc_ptr = &tra_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *transport;
PRIVATE OBJECT *tra_text;						/* TRA-Texte */
PRIVATE OBJECT *tra_info;						/* TRA-Info-Anzeige */
PRIVATE OBJECT *tra_help;						/* TRA-Hilfe-Anzeige */
PRIVATE OBJECT *tra_desk;						/* TRA-Desktop */

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "TRA";		/* Name, fÅr Extension etc. */

PRIVATE RTMCLASSP	modulep[MAXMSAPPLS];		/* Zeiger auf Modul-Strukturen */
PRIVATE WORD		refNums[1];					/* Referenznummern */

/** Send MTC **/
/*
	mtc_count				ZÑhler von 0-7 da 8 Quarter-Frames 
							eine vollstÑndige Message machen. 
	mtc_out_time			Letzte gesendete Zeit, da der Sequenzer 
							die Routine viel îfter anspringt als 
							nîtig. 	
	stimh, stimm, 
	stims, stimf  			Hilfs-Variablen zur Zerlegung 
							der Millisekunden. 

*/
LOCAL INT   stimh, stimm, stims, stimf;
INT	 		 mtc_count;
LONG  		 mtc_out_time;

#ifndef RCV_MTC_BY_MIDI_SHARE

/** Receive MTC **/
/*
	mtc_lock				Flag ob in eine 8-er Sequenz von 
							Quarter-Frames eingelockt. 
	mtc_timer				Aktuelle empfangene Zeit. 
	mtc_dela[]				Zwei Frames Delay-Zeit.
	
*/
LOCAL INT   mtc_val[ 9 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
LOCAL INT	 mtc_lock = 0;
LOCAL LONG  mtc_timer;
LONG  mtc_fr[   10 ] = { 24L, 0L, 25L, 0L, 30L, 0L, 30L, 0L, 25L };
LOCAL LONG  mtc_dela[ 10 ] = { 83L, 0L, 80L, 0L, 67L, 0L, 67L, 0L, 80L };
LOCAL LONG last_mtc_rcv = 0;

#endif RCV_MTC_BY_MIDI_SHARE

/** Sync Receive MTC **/
LONG  		 old_mtc_time;
INT 		 frameval[] = { 24, 25, 30, 30, 100 };

/****** FUNCTIONS ************************************************************/

/* MidiShare Funktionen */
PUBLIC VOID			cdecl	receive_evts_tra	_((INT refNum));
PUBLIC VOID			cdecl play_task_tra		_((LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3));
PUBLIC VOID			cdecl delayed_task_tra	_((LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3));
PUBLIC VOID			cdecl receive_alarm_tra _((SHORT refNum, LONG code));
PRIVATE VOID		InstallFilter				_((SHORT refNum));
PRIVATE WORD		init_midishare 			_((VOID));

/* Interne TRA-Funktionen */
PRIVATE VOID 	send_pos		(LONG posit);
PRIVATE VOID 	send_cont		(LONG posit);
PRIVATE VOID 	send_stop		(LONG posit);
PRIVATE VOID	send_record			_((BOOLEAN flag));

PRIVATE VOID	Beep (RTMCLASSP module, BOOL high);

PRIVATE VOID create_displayobs (WINDOWP window);

PRIVATE BOOLEAN init_rsc_tra	_((VOID));
PRIVATE BOOLEAN term_rsc_tra	_((VOID));


/********************************************************************/
/** Init Send MTC 
	Wird aufgerufen wenn der Sequencer gestartet wird. 
	ms_clock ist die aktuelle Ms-Position, 
	wird aber im Moment nicht benîtigt. 
**/
/********************************************************************/
void StartSendingMtc(RTMCLASSP module, LONG ms_clock )
{
	STAT_P		status = module->status;
	
	status->snd_mtc = TRUE;
	
	/** Local Init **/  
	mtc_count		= 0;
	mtc_out_time	= -LOCK_TIME;

} /* StartSendingMtc */

void StopSendingMtc(RTMCLASSP module)
{
	STAT_P		status = module->status;
	
	status->snd_mtc = FALSE;
	
} /* StopSendingMtc */

/********************************************************************/
/** Send MidiTimeCode 
	Wird nur dann angesprungen, wenn auch wirklich etwas zu senden ist. 
	Details sind aus der Quarter-Frame Definition zu entnehmen. 
	Siehe auch Delight Sync-Dialogbox: 
	Auf bis zu 9 Out-Ports kann gleichzeitig gesendet werden. 
**/
/********************************************************************/
void send_mtc(RTMCLASSP module )
{
	INT			flag, bit, q;
	LONG		lang, clock;
	MidiEvPtr	e, ce;
	STAT_P		status = module->status;
	WORD			refNum = (WORD)module->special;
	
	if ( !status->snd_mtc )  return;
	
	/** Kein Port an, siehe Delight Sync-Dialogbox **/
	if ( status->snd_mtc_ports == 0 )
	  return;
	
	/** QuarterFrame-Event kreiern **/
	e = MidiNewEv( typeQuarterFrame );
	
	/** Speicher voll **/
	if ( !e ) return;
	
	/** Zeit mitzÑhlen **/
	mtc_out_time += status->ms_quarter;
	
	/** Nummer der Message ( 0-7 ) **/
	Pitch( e ) = mtc_count;

	/** Werte eintragen, siehe Defintion der 8 Quarter-Frame Messages **/ 
	switch( mtc_count )
	  {
	  case 0:
	    /** Alle Berechnungen durchfÅhren **/
		
	    clock = mtc_out_time;
		/*
		if ( status->snd_sign_offset )	
		  clock -= status->snd_mtc_offset;
		else  
		*/
	  clock += status->snd_mtc_offset;
		  
		stimh	= (INT)( clock / 3600000L );
		lang	= clock % 3600000L;
		stimm	= (INT)( lang / 60000L );
		lang	= lang % 60000L;
		stimf	= (INT)( lang % 1000L ) / status->ms_frames;
		stims	= (INT)( lang / 1000L );
		
	    Vel( e ) = ( stimf & 15 );
	    mtc_count = 1;
	    break;
	    
	  case 1:
	    Vel( e ) = (( stimf & 16 ) >> 4 );
	    mtc_count = 2;
	    break;
		
	  case 2:
	    Vel( e ) = ( stims & 15 );
	    mtc_count = 3;
	    break;
		
	  case 3:
	    Vel( e ) = (( stims & 48 ) >> 4 );
	    mtc_count = 4;
	    break;
		
	  case 4:
	    Vel( e ) = ( stimm & 15 );
	    mtc_count = 5;
	    break;
		
	  case 5:
	    Vel( e ) = (( stimm & 48 ) >> 4 );
	    mtc_count = 6;
	    break;
		
	  case 6:
	    Vel( e ) = ( stimh & 15 );
	    mtc_count = 7;
	    break;
		
	  case 7:
	    Vel( e )  = (( stimh & 16 ) >> 4 );
	    Vel( e ) |= (INT)status->mtc_type;
	    mtc_count = 0;
	    break;
	  }
	
	/** Quarter Frames auf alle angewÑhlten Ports 
		( auûer dem ersten angewÑhlten ) schicken. 
	**/
	bit  = 1;
	flag = 0;
	for ( q=0; q<9; q++ )
	  {
	  if ( bit & status->snd_mtc_ports )
	    {    
	    if ( flag )
	      {
	      ce = MidiCopyEv( e );
	      if ( ce )
	        MidiSendIm( refNum, ce );
	      }
	
	    Port( e ) = q;
	    flag = 1;
	    }
	  
	  bit <<= 1;  
	  }
	
	/** Quarter Frame auf den ersten angewÑhlten Kanal schicken **/
	MidiSendIm( refNum, e );
	
}

/********************************************************************/
/** Senden synchronisieren. 
	Wird vom Sequenzer jede Millisekunde angesprungen. 
	Hier wird getestet ob etwas gesendet werden soll. 
	Wenn der Sequenzer gespult wurde, wird automatisch wieder 
	die richtige Sync-Zeit eingeklinkt. 
**/
/********************************************************************/
void cdecl sync_send_mtc (LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3 )
{
	RTMCLASSP	module = modulep[refNum];
	MidiEvPtr	myTask;
	STAT_P		status = module->status;
	LONG			ms_clock = status->posit;

	/** Test ob MTC senden an **/
	if ( !status->snd_mtc )  return;

	/** Test ob Sequenzer gespult oder frisch gestartet wurde **/
	/* Lock/Unlock-Zeiten ursprÅnglich auf 100ms */
	if ( ms_clock >= mtc_out_time + LOCK_TIME || ms_clock <= mtc_out_time - LOCK_TIME )
	{
		/** (Re)Initialisieren der Sende-Zeit **/  
		mtc_count	   = 0;
		mtc_out_time = ms_clock;
	} 
	
	/** Sende-Zeit ist eingetreten **/
	if ( ms_clock >= mtc_out_time )
		/** Send **/
		send_mtc(module);  
  
	myTask = MidiTask(sync_send_mtc, MidiGetTime() + SYNC_RESOLUTION, refNum, module, status->posit, 1);
} /* sync_send_mtc */

/********************************************************************/
/** Receive MidiTimeCode 
	Wird immer von der "receive_alarm" Routine angesprungen, 
	wenn ein Quarter Frame emfangen wurde. 
**/
/********************************************************************/
INT receive_mtc(RTMCLASSP module,  MidiEvPtr e, INT type, INT pitch, INT vel )
{
	WORD			refNum = (WORD)module->special;
	STAT_P		status = module->status;
	LONG help, code_type;

#ifndef RCV_MTC_BY_MIDI_SHARE

	if ( !status->sync_in )  return 1;

/*	
	/** Receive MidiTimeCode **/
	if ( status->rcv_mtc == NO_RCV_MTC_SYNC )
	  return 1;
*/	

	/* Wert merken fÅr Timeout */
	last_mtc_rcv = MidiGetTime();
	
	/** Die erste Quarter Message **/
	if ( !pitch )
	  mtc_lock = 1;
	
	/** Quarter-Wert zuweisen **/
	mtc_val[ pitch ] = vel;
	
	/** Die letzte Quarter Message **/
	if ( !(mtc_lock == 1 && pitch == 7) )
	  return 1;
	
	/** Code-Type **/
	code_type = mtc_val[ 7 ] & 6;
	
	/** Frames **/
	mtc_timer  = (LONG)mtc_val[ 0 ];
	mtc_timer += (LONG)(( mtc_val[ 1 ] & 1 ) << 4 );
	mtc_timer *= 1000L;
	mtc_timer /= (mtc_fr[ code_type ]);
	
	/** Sekunden **/
	help  = (LONG)mtc_val[ 2 ];
	help += (LONG)(( mtc_val[ 3 ] & 3 ) << 4 );
	help *= 1000L;
	mtc_timer += help;
	
	/** Minuten **/
	help  = (LONG)mtc_val[4];
	help += (LONG)(( mtc_val[ 5 ] & 3 ) << 4 );
	help *= 60000L;
	mtc_timer += help;
	
	/** Stunden **/
	help  = (LONG)mtc_val[6];
	help += (LONG)(( mtc_val[ 7 ] & 1 ) << 4 );
	if ( help > 20 ) 
	  mtc_timer = 0L;
	else
	{  
		help *= 360000L;
		mtc_timer += help;
	}
	
	status->mtc_time = mtc_timer + mtc_dela[ code_type ];
	status->mtc_time -= status->rcv_mtc_offset;
	
	send_variable(VAR_SYNC_IN_FRAMES, code_type/2);

	mtc_lock = 0;

	/** Test ob schon irgendetwas von auûen Synchronisiert wird **/
	if ( status->mtc_sync == NO_RCV_MTC_SYNC )
	{
		status->mtc_sync = TRY_RCV_MTC_SYNC;

#if FALSE
		/** Test ob Sequenzer spielt **/
		if ( status->play )
		{
			if ( status->mtc_time >= 0L ) 
			{
				status->posit = status->mtc_time;
				/** Non-Realtime Message den Sequenzer zu starten **/
				/* send_cont (0L); */
				send_variable(VAR_SYNC_IN_LOCKED, FALSE);
			}
		}
#endif

	}  
	
#endif RCV_MTC_BY_MIDI_SHARE
	
	return 1;
} /* receive_mtc */

/********************************************************************/
/** Empfangen synchronisieren.
	Wird jede Millisekunde vom Sequenzer aufgerufen. 
	Da der Sequenzer schneller oder langsamer sein kann, 
	oder sogar gespult oder gecycelt werden kann, 
	muû immer das delay in Millisekunden ermittelt werden, 
	sobald ein neues Quarter Frame eingetroffen ist, 
	um den Abstand zu korrigieren. 
	
	posit		ist die aktuelle interne Zeit
	delay			das Åbliche Aufruf-Delay von 1 Millisekunde
**/
/********************************************************************/
void cdecl sync_receive_mtc(LONG date, SHORT refNum, LONG a1, LONG a2, LONG delay )
{
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	MidiEvPtr	myTask;
	LONG			posit;
	
	/* Sanity Check */
	if (refNum > 0 && refNum < MAXMSAPPLS)
	{
		posit = status->posit;

		/** Nun kommen zwei verschiedene Mîglichkeiten der Synchronisation:
		Die eigene Routine und die Routine die die neuen Mîglichkeiten 
		von MidiShare ausnutzt. 
		**/
		
		/***************************/
		/** Die MidiShare Routine, die nicht geht **/
		/***************************/
#ifdef RCV_MTC_BY_MIDI_SHARE
		
		status->mtc_time  = MidiGetExtTime();
		status->mtc_time += 160L;
		status->mtc_time -= status->rcv_mtc_offset;
		
		if ( status->mtc_time < 0L ) 
		status->mtc_time = 0L;
		delay 	       = status->mtc_time - posit;
		
		
		/***************************/
		/** Die eigene Routine    **/
		/***************************/
#else 
		
		/** Ist der Abstand zwischen interner und externer Zeit zu groû, 
		dann wird der Sequenzer gestoppt, 
		da wahrscheinlich auch das externe Signal gestoppt hat. 
		**/

		/* PrÅfen ob Start nîtig */
		if ( status->mtc_sync == TRY_RCV_MTC_SYNC )
		{
		  	status->mtc_sync = RCV_MTC_SYNC;
		  	/* Position ins Zeitraster umsetzen */
		  	posit = status->mtc_time/QUANT;
		  	posit *= QUANT;
			send_variable(VAR_SMPTE, posit);
			send_variable(VAR_SYNC_IN_LOCKED, TRUE);
			send_variable(VAR_TRA_PLAY, TRUE);
		}

		/* Nur stoppen bei aktiver Sync, BD */
		if (status->mtc_sync == RCV_MTC_SYNC)
		{

/*			if ( (status->mtc_time + 500L)  < posit ||
				(status->mtc_time - 1000L) > posit )
*/
			if (MidiGetTime() - last_mtc_rcv > LOCK_TIME)
			{	 
				status->mtc_sync = NO_RCV_MTC_SYNC;
				send_variable(VAR_SYNC_IN_LOCKED, FALSE);
				/* SeSetRltAction( (LONG)SoRemote, (LONG)Z_STOP, 0L, 0L ); */
				send_variable(VAR_TRA_STOP, TRUE);
			}
			/* Play Task aufrufen, wenn es wieder Zeit ist. */
			else if (status->mtc_time - posit > QUANT)
				play_task_tra (posit, refNum, 0L, 0L, 0L);

		} /* RCV_MTC_SYNC */
		
		/** Test ob eine neue Quarter-Frame Message erhalten **/ 
		if ( old_mtc_time != status->mtc_time )
		{
			/** Differenz zwischen interner und externer Zeit berechnen **/
			delay 	   = status->mtc_time - posit;
			old_mtc_time = status->mtc_time;
		}  
		
		myTask = MidiTask(sync_receive_mtc, MidiGetTime() + SYNC_RESOLUTION, refNum, module, status->posit,  1);

#endif RCV_MTC_BY_MIDI_SHARE
	} /* Sanity Check */
	/* return delay; */
} /* sync_receive_mtc */

/********************************************************************/
/** Init Receive MTC 
	Wird aufgerufen, wenn der Sequenzer gestartet wird. 
	Sync-Flag auf an, wenn in der receive_mtv Routine 
	bereits ein Qaurter-Frame empfanegn wurde (TRY_RCV_MTC_SYNC). 
**/
/********************************************************************/
void StartReceivingMtc(RTMCLASSP module)
{
	STAT_P		status = module->status;
	WORD			refNum = (WORD)module->special;
	MidiEvPtr	myTask;

#if FALSE
	if ( status->mtc_sync == TRY_RCV_MTC_SYNC )
	{
		status->mtc_time = 0;
	  	status->mtc_sync = RCV_MTC_SYNC;
		send_variable(VAR_SYNC_IN_LOCKED, TRUE);
	}
#else
	status->mtc_time = 0;
	/* Sync routine einhÑngen */
	myTask = MidiTask(sync_receive_mtc, MidiGetTime(), refNum, module, status->posit, 1);
#endif		
				
	old_mtc_time = -1L;

} /* StartReceivingMtc */

/********************************************************************/
/** Stop Receive MTC 
	Wird aufgerufen, wenn der Sequenzer gestoppt wird. 
	Sync-Flag auf aus.
**/
/********************************************************************/
void StopReceivingMtc(RTMCLASSP module)
{
	STAT_P		status = module->status;
	
	status->mtc_sync = NO_RCV_MTC_SYNC;
	send_variable(VAR_SYNC_IN_LOCKED, FALSE);

} /* StopReceivingMtc */

/********************************************************************/
/** Synchronisations-Modus setzen. 
	Nur vonnîten, wenn die neuen MidiShare-Features angesprochen werden.
**/
/********************************************************************/
INT SetSyncMode(RTMCLASSP module)
{
	STAT_P		status = module->status;

#ifdef RCV_MTC_BY_MIDI_SHARE
	/** Wenn MTC empfangen werden soll **/
	if ( status->rcv_mtc )
	  MidiSetSyncMode( MIDISyncExternal | MIDISyncAnyPort );
	else  
	  MidiSetSyncMode( MIDISyncInternal );
#endif

	return 0;
} /* SetSyncMode */

/********************************************************************/
/** Frames setzen.
	Wird bei jeder énderung der Frame-Art aufgerufen, 
	um die internen Variablen zu setzen. 
**/
/********************************************************************/
INT SetMtcFrames(RTMCLASSP module)
{
	STAT_P		status = module->status;

	/** Anzahl der Millisekunden pro Frame  **/
	status->ms_frames = ( 1000 / frameval[ status->snd_mtc_frames ] );
	
	/** Quarter-Frames werden 4 mal pro Frame gesendet **/
	status->ms_quarter = status->ms_frames / 4;
	
	switch( status->snd_mtc_frames )
	{
		case 0:
		 status->mtc_type = 0;
		 break;
		 
		case 1:
		 status->mtc_type = 2;
		 break;
		 
		case 2:
		 status->mtc_type = 4;
		 break;
		
		case 3:
		 status->mtc_type = 6;
		 break;
		     
		case 4:
		 status->mtc_type = 0;
		 break;
	}
	
	send_variable(VAR_SYNC_OUT_FRAMES, status->snd_mtc_frames);
	return 0;
} /* SetMtcFrames */

/********************************************************************/
/** MTC init. 
	Wird beim Programmstart aufgerufen.
**/
/********************************************************************/
void init_mtc(RTMCLASSP module)
{
	STAT_P		status = module->status;
	
	status->mtc_time	= 0L;
	status->mtc_sync	= NO_RCV_MTC_SYNC;
	/* status->multi_sync_flag = 0; */
	SetMtcFrames(module);

} /* init_mtc */

/*****************************************************************************/

PUBLIC VOID cdecl receive_evts_tra (SHORT refNum)
{
	MidiEvPtr	event, thru_event;
	LONG 			n;
	INT 			r;
	UINT			type;
	MidiEvPtr	myTask;
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	WINDOWP		window = module->window;
	
	r = refNum;
	for (n = MidiCountEvs(r); n > 0; --n) 	/* Alle empfangenen Events abarbeiten */
	{
		event = MidiGetEv (r);				/*  Information holen */
		type = (EvType(event) & 0xFF);
		switch (type)
		{
/*
			case typeRTMPosit:
				status->posit 			= get_posit((MidiSTPtr)event);
				send_variable(VAR_SMPTE, status->posit);
				window->milli			= 1;
				break;
			case typeRTMCont:
				status->posit 			= get_posit((MidiSTPtr)event);
				status->play			= TRUE;
				myTask = MidiTask(play_task_tra, MidiGetTime() + QUANT, refNum, 0, 0, 0);
				if (status->sync_in)
				{
					myTask = MidiTask(sync_receive_mtc, MidiGetTime(), refNum, module, status->posit, 1);
					/* myTask = MidiTask(sync_send_mtc, MidiGetTime(), refNum, module, status->posit, 1); */
				}
				window->milli			= 1;
				break;
			case typeRTMStop:
				status->posit 			= get_posit((MidiSTPtr)event);
				status->play			= FALSE;
				status->click_counter  = 0;
				window->milli			= 1;
				if (status->sync_in)
				{
					StopReceivingMtc(module);
				}
				if (status->sync_out)
				{
					StopSendingMtc(module);
				}
				break;
			case typeRTMCycleSet:
				status->leftloc 		= get_cycle_start((MidiSTPtr)event);
				status->rightloc		= get_cycle_end((MidiSTPtr)event);
				status->new	  = TRUE;
				window->milli			= 1;
				break;
			case typeRTMCycleOnOff:
				status->cycle 			= (BOOLEAN) get_cycle_on((MidiSTPtr)event);
				window->milli			= 1;
				break;
			case typeRTMRecordOnOff:
			/*
				status->record 		= (BOOLEAN) get_record_on((MidiSTPtr)event);
				window->milli			= 1;
				status->new	  = TRUE;
			*/
				break;
*/
			case typeQuarterFrame:
				receive_mtc (module, event, type, Pitch(event), Vel(event));
/*
				if (RefNum(event) == 0)	/* Nur externe Events */
					MidiSendIm(refNum, MidiCopyEv(event));
*/
				break;
			/* Thru-Funktion */
			case typeNote:
			case typeKeyOn:
			case typeKeyOff:
			case typeKeyPress:
			case typeCtrlChange:
			case typeProgChange:
			case typePitchWheel:
			default:
				if (RefNum(event) == 0)	/* Nur externe Events */
					 MidiSendIm(refNum, MidiCopyEv(event));
				break;
		} /* switch */
		MidiFreeEv (event);
	} /* for */
} /* receive_evts_tra */

PUBLIC VOID cdecl receive_alarm_tra (SHORT refNum, LONG code)
{
#ifdef RCV_MTC_BY_MIDI_SHARE
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	WINDOWP		window = module->window;
	TSyncInfo	info;

	switch ((WORD)code)
	{
		case MIDISyncStart:
			send_variable(VAR_SYNC_LOCKED, TRUE);
			send_cont(MidiGetExtTime());
			/* send_start_ev_delight (refNum, status->posit); */
			window->milli			= 1;
			status->new	 			= TRUE;
			break;
		case MIDISyncStop:
			send_variable(VAR_SYNC_LOCKED, FALSE);
			send_stop (MidiGetExtTime());
			/* send_stop_ev_delight (refNum, status->posit); */
			window->milli			= 1;
			status->new				= TRUE;
			break;
		case MIDIChangeSync:
			MidiGetSyncInfo (&info);
			status->new	  			= TRUE;
			status->sync			= info.syncMode & MIDISyncExternal;
			send_variable(VAR_SYNC_LOCKED, info.syncLocked);
			window->milli			= 1;
			break;
	} /* switch */
#endif
} /* receive_alarm_tra */
/****************************************************************************
* 							InstallFilter						 *
*---------------------------------------------------------------------------*
* Cette procÇdure dÇfinit les valeurs du filtre de l'application. Un filtre *
* est composÇ de trois parties, qui sont trois tableaux de boolÇens :		 * 
* 															 *
*		un tableau de 256 bits pour les ports Midi acceptÇs			 *
*		un tableau de 256 bits pour les types d'ÇvÇnements acceptÇs		 *
*		un tableau de  16 bits pour les canaux Midi acceptÇs			 *
* 															 *
* Dans le code ci dessous, le filtre est paramÇtrÇ pour accepter n'importe	 *
* quel type d'ÇvÇnement. 										 *
* 															 *
* Les paramätres de l'appel :										 *
* ---------------------------										 *
* 															 *
*		aucun												 *
* 															 *
*****************************************************************************/

PRIVATE VOID InstallFilter (WORD refNum)
{
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	TFilter		*filter = &status->filter;
	register		int i;

	for (i = 0; i<256; i++)
	{ 										
		AcceptBit(filter->evType,i);		/* accepte tous les types d'ÇvÇnements	*/
		AcceptBit(filter->port,i);		/* en provenance de tous les ports		*/
	} /* for */
											
	for (i = 0; i<16; i++)
		AcceptBit(filter->channel,i);	/* et sur tous les canaux Midi		*/
		
	MidiSetFilter( refNum, filter );   /* installe le filtre				*/
} /* InstallFilter */

PUBLIC VOID cdecl play_task_tra (LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3)
{
	/* Wird soundso oft aufgerufen, um neue Daten in
		das Fenster einzublenden */

	MidiEvPtr	myTask;
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	WINDOWP		window = module->window;
	REG LONG		left 	= status->leftloc;
	REG LONG		right	= status->rightloc;
	REG LONG		posit;
		
	window->milli = 1; /* Updaten */

	/* Zeit weiterzÑhlen */
	status->posit += QUANT;

	posit = status->posit;

	/* Cycle Restart */
	if ((status->cycle) && (posit >= right))
		if ((right > left)								/* Zw. Li und Re cyclen */
		 || ((left > right)	&& (posit < left)))	/* oder Åberspringen */
				send_pos(left);

	/* Punch In ? */
	if (status->punch_in && (posit >= left) && (posit <= right) && ~status->record) 
			send_record(TRUE);

	/* Punch Out ? */
	if (status->punch_out && (posit >= left) && (posit >= right) && status->record) 
			send_record(FALSE);

	/* Wenn weiterhin aufgenommen/gespielt werden soll, 
		muû der Task wieder eingeklinkt werden, ausser bei MTC-Sync */
	if (status->play)
	{
		if (!status->sync_in)
			myTask = MidiTask(play_task_tra, MidiGetTime() + QUANT, refNum, 0, 0, 0);

		if (status->click)
		{
			if ((status->posit % 1000) == 0)
				myTask = MidiTask(delayed_task_tra, MidiGetTime(), refNum, (LONG)TRADTaskBeepHi, 0, 0);
			else
				myTask = MidiTask(delayed_task_tra, MidiGetTime(), refNum, (LONG)TRADTaskBeepLo, 0, 0);
		} /* if click */
	} /* if play */

	send_variable(VAR_SMPTE, posit);
} /* play_task_tra */

PUBLIC VOID cdecl delayed_task_tra (LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3)
{
	/* Wird aufgerufen, um nicht EchtzeitfÑhige Funktionen auszufÅhren */
	RTMCLASSP	module 	= modulep[refNum];
	WORD action = (WORD)a1;

	switch (action)
	{
		case TRADTaskReset:
			module->reset (module);
			break;
		case TRADTaskBeepLo:
			Beep (module, FALSE);
			break;
		case TRADTaskBeepHi:
			Beep (module, TRUE);
			break;
	} /* switch */
} /* delayed_task_tra */

PRIVATE VOID	Beep (RTMCLASSP module, BOOL high)
{
	STAT_P		status = module->status;
	/* Piepser */
	vs_mute(phys_handle, MUTE_DISABLE);
	
	/* Tiefer Ton jede Sekunde */
	if (high)
	{
		/* Bconout (2, BEL); */
		v_sound( vdi_handle, status->click_freq*2, status->click_duration );
	} /* if */
	else
	{
		v_sound( vdi_handle, status->click_freq, status->click_duration );
	} /* else */
} /* Beep */

/*****************************************************************************/

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	WINDOWP		window = module->window;
	STAT_P		status = module->status;
	STAT_P		stat_alt = module->stat_alt;
   STRING		s;
	BOOLEAN		draw = (window > 0);		/* Zeichnen wenn neu? */
	BOOLEAN		force = status->new;				/* Auf jeden fall zeichnen? */
		
} /* set_dbox */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD		variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	LONG		posit 	= ((MSG_RTM_POSIT *)msg)->posit;
	STAT_P	status = module->status;
	WINDOWP	window = module->window;
	WORD		refNum = (WORD)module->special;
	MidiEvPtr	myTask;
	
	switch(type)
	{
		case RTM_POSIT:
			status->posit 			= posit;
			send_variable(VAR_SMPTE, status->posit);
			window->milli			= 1;
			break;
		case RTM_CONT:
			status->posit 			= posit;
			status->play			= TRUE;
			myTask = MidiTask(play_task_tra, MidiGetTime() + QUANT, refNum, 0, 0, 0);
			if (status->sync_in)
			{
			}
			if (status->sync_out)
			{
				StartSendingMtc(module, status->posit);
				myTask = MidiTask(sync_send_mtc, MidiGetTime(), refNum, module, status->posit, 1);
			}
			window->milli			= 1;
			break;
		case RTM_STOP:
			status->posit 			= posit;
			status->play			= FALSE;
			status->click_counter  = 0;
			window->milli			= 1;
			if (status->sync_in)
			{
				StopReceivingMtc(module);
			}
			if (status->sync_out)
			{
				StopSendingMtc(module);
			}
			break;
		case SET_VAR:			/* Record ein/ausschalten */
			switch (variable)
			{
				case VAR_PUF_PLAY:
					status->puf_sync	= (BOOLEAN)value;
					status->new	  = TRUE;
					window->milli = 1; /* Updaten */
					break;
				case VAR_BIG_PLAY:
					status->big_sync	= (BOOLEAN)value;
					status->new	  = TRUE;
					window->milli = 1; /* Updaten */
					break;
				case VAR_RECORD:
					if (value != status->record)
					{
						window->milli = 1; /* Updaten */
						/* nichts weiter nîtig, da VAR schon neuen Wert hat. */
						/* send_record(value); */
					}
					break;
				case VAR_TRA_PLAY:
					if (value)
						if (! status->play)
						{
							window->milli = 1; /* Updaten */
							send_variable(VAR_TRA_STOP, FALSE);
							send_cont(status->posit);
							/* send_start_ev_delight (refNum, status->posit); */
						}
					break;
				case VAR_TRA_STOP:
					if (value)
					{
						send_record(FALSE);
						send_variable(VAR_TRA_PLAY, FALSE);
						if (status->play)
						{
							/* Zuerst nur anhalten */
							send_stop(status->posit);
							/* send_stop_ev_delight (refNum, status->posit); */
						} /* if */
						else
						{
							if (status->posit > status->leftloc)
							/* Beim zweiten Mal zum linken Locator */				
								send_pos(status->leftloc);
							else
							/* Beim dritten Mal zum Anfang (Position 0) springen */
								send_pos(0);
						} /* else */
					} /* if */
					break;
				case VAR_TRA_FFWD:
					if (value)
					{
						send_pos(status->posit + QUANT);
						send_variable(VAR_TRA_FFWD, FALSE);
					}
					break;
				case VAR_TRA_REW:
					if (value)
					{
						send_pos(status->posit - QUANT);
						send_variable(VAR_TRA_REW, FALSE);
					}
					break;
				case VAR_SYNC_IN:
#ifdef RCV_MTC_BY_MIDI_SHARE
				{
					TSyncInfo	syncinfo;
		      	MidiGetSyncInfo (&syncinfo);

		      	if ((BOOLEAN) value)
			      	syncinfo.syncMode |= MIDISyncExternal;	/* Set Syncmode ext */
					else
			      	syncinfo.syncMode &= !MIDISyncExternal;	/* Set Syncmode ext */
						
		      	MidiSetSyncMode (syncinfo.syncMode);
		      	/*
		      	status->sync_in = !status->sync_in;
		      	if (status->sync_in)
			      	syncinfo.syncMode = MIDISyncExternal;	/* Syncmode ext */
			      else
			      	syncinfo.syncMode = 0;						/* Syncmode int */
		      	MidiSetSyncMode (syncinfo.syncMode | MIDISyncAnyPort);
		      	*/
		      }
#else
					status->sync_in = (BOOLEAN) value;
					if ((BOOLEAN) value) StartReceivingMtc (module);
#endif
					break;
				case VAR_SYNC_IN_LOCKED:
					status->locked = (BOOLEAN) value;
					/* Stop after unlock */
					/*
					if (status->play && !value)
						send_variable(VAR_TRA_STOP, TRUE);
					*/
					break;
				case VAR_SYNC_OUT:
					status->sync_out = (BOOLEAN) value;
					if (status->sync_out) StartSendingMtc(module, status->posit);
					else StopSendingMtc (module);
					break;
				case VAR_SYNC_OUT_FRAMES:
					if ((WORD)value != status->snd_mtc_frames)
					{
						status->snd_mtc_frames = (WORD) value;
						SetMtcFrames(module);
					} /* if */
					break;
				case VAR_SYNC_OUT_PORTS:
					status->snd_mtc_ports 	= (WORD)value;
					break;
/*
				case VAR_CYCLE:
					send_cycleonoff(value); 
					break;
				case VAR_CYCLE_START:
					send_cycleset (value, status->rightloc);
					break;
				case VAR_CYCLE_STOP:
					send_cycleset (status->leftloc, value);
					break;
*/
				case VAR_PUF_OFFSET:
					/* Receive-Offset = Send-Offset */
					status->rcv_mtc_offset = value;
					status->snd_mtc_offset = value;
					break;
				case VAR_SMPTE:
					/* Nur im Stop Modus, sonst gibts RÅckkoppelungen */
					if (!status->play && value != status->posit) 
						send_pos (value);
					break;
			} /* switch */
			break;
	} /* switch */

	window->milli = 1;
} /* message */

PRIVATE VOID send_pos	(LONG posit)
{
	MSG_RTM_POSIT msg;

	msg.posit = posit;
	
	if (ed4_module) ed4_module->message (ed4_module, RTM_POSIT, &msg);
	if (puf_module) puf_module->message (puf_module, RTM_POSIT, &msg);
	if (tra_module) tra_module->message (tra_module, RTM_POSIT, &msg);
}

PRIVATE VOID send_cont	(LONG posit)
{
	MSG_RTM_POSIT msg;

	msg.posit = posit;
	
	if (ed4_module) ed4_module->message (ed4_module, RTM_CONT, &msg);
	if (puf_module) puf_module->message (puf_module, RTM_CONT, &msg);
	if (tra_module) tra_module->message (tra_module, RTM_CONT, &msg);
	
}

PRIVATE VOID send_stop	(LONG posit)
{
	MSG_RTM_POSIT msg;

	msg.posit = posit;
	
	if (ed4_module) ed4_module->message (ed4_module, RTM_STOP, &msg);
	if (puf_module) puf_module->message (puf_module, RTM_STOP, &msg);
	if (tra_module) tra_module->message (tra_module, RTM_STOP, &msg);
}

PRIVATE VOID send_record	(BOOLEAN flag)
{
	send_variable(VAR_RECORD, flag);
} /* send_record */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
WINDOWP window;
MKINFO  *mk;

{
	LONG			x;
	RTMCLASSP	module = Module(window);
	WORD			refNum = (WORD)module->special;
	STAT_P		status = module->status;
	LONG			loc;
	
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */

	switch (window->exit_obj)
	{

		case TRABIGPUF:
			send_variable(VAR_PUF_PLAY, TRUE);
			send_variable(VAR_BIG_PLAY, TRUE);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, TRABIGPUF);
			/*
			draw_object (window, TRABIG);
			draw_object (window, TRAPUF);
			*/
			break;
	} /* switch */
} /* wi_click_mod */

/*****************************************************************************/
/* Taste fÅr Fenster                                                         */
/*****************************************************************************/

#define send_lfo_accel(value)\
{WORD lfo; for (lfo =0; lfo< MAXLFOS; lfo++) send_variable(VAR_LFA_ACC1 + lfo, 100 * value);}

#define send_mtr_accel(value)\
{WORD signal; for (signal =0; signal< MAXSIGNALS; signal++) send_variable(VAR_MTR_ACC0 + signal, 100 * value);}

PRIVATE BOOLEAN wi_key_mod (window, mk)
WINDOWP window;
MKINFO  *mk;

{
	RTMCLASSP	module = Module(window);
	WORD			refNum = (WORD)module->special;
	STAT_P		status = module->status;
	BOOLEAN		ok = FALSE;
	WORD			scan_code = mk->scan_code;
	WORD			ascii_code = mk->ascii_code;
	WORD			acceleration, signal = 0;
	BOOL 			flag;
	
	if (mk->ctrl)
	{
		switch (scan_code)
		{
			/* Zahlenreihe 1-0 */
			case 2: send_mtr_accel(1); ok = TRUE; break;
			case 3: send_mtr_accel(2); ok = TRUE; break;
			case 4: send_mtr_accel(3); ok = TRUE; break;
			case 5: send_mtr_accel(4); ok = TRUE; break;
			case 6: send_mtr_accel(5); ok = TRUE; break;
			case 7: send_mtr_accel(6); ok = TRUE; break;
			case 8: send_mtr_accel(7); ok = TRUE; break;
			case 9: send_mtr_accel(8); ok = TRUE; break;
			case 10: send_mtr_accel(9); ok = TRUE; break;
			case 11: send_mtr_accel(0); ok = TRUE; break;

			case F1: send_lfo_accel(1); ok = TRUE; break;
			case F2: send_lfo_accel(2); ok = TRUE; break;
			case F3: send_lfo_accel(3); ok = TRUE; break;
			case F4: send_lfo_accel(4); ok = TRUE; break;
			case F5: send_lfo_accel(5); ok = TRUE; break;
			case F6: send_lfo_accel(6); ok = TRUE; break;
			case F7: send_lfo_accel(7); ok = TRUE; break;
			case F8: send_lfo_accel(8); ok = TRUE; break;
			case F9: send_lfo_accel(9); ok = TRUE; break;
			case F10: send_lfo_accel(0); ok = TRUE; break;
		} /* switch scan_code */
		if (!ok)
		{
			switch (ascii_code)
			{
				case 'u':
					flag = var_get_value (var_module, VAR_MTR_UMK);
					send_variable (VAR_MTR_UMK, !flag);
					ok = TRUE;
					break;
				case 'U':
					flag = var_get_value (var_module, VAR_LFA_UMK);
					send_variable (VAR_LFA_UMK, !flag);
					ok = TRUE;
					break;
				case 'p':
					flag = var_get_value (var_module, VAR_MTR_PAUSE);
					send_variable (VAR_MTR_PAUSE, !flag);
					ok = TRUE;
					break;
				case 'P':
					flag = var_get_value (var_module, VAR_LFA_PAUSE);
					send_variable (VAR_LFA_PAUSE, !flag);
					ok = TRUE;
					break;
				default:
					ok = menu_key (window, mk);
			} /* switch */
		} /* if not ok */
	} /* if ctrl */

	/* Remote Steuerung fÅr PLAY/REC/MTR/LFO an/aus */
	switch (scan_code)
	{
		case LOC0: signal = 1; break;
		case LOC1: signal = 2; break;
		case LOC2: signal = 3; break;
		case LOC3: signal = 4; break;
		case LOC4: signal = 5; break;
		case LOC5: signal = 6; break;
		case LOC6: signal = 7; break;
		case LOC7: signal = 8; break;
	} /* switch scan_code */
	if (signal != 0)
	{
		if (mk->alt)
			send_variable (VAR_PUF_REC_SIG0 + signal, 	!var_get_value(var_module, VAR_PUF_REC_SIG0 + signal));
		else if (mk->shift)
			send_variable (VAR_MTR_ON0 + signal, 			!var_get_value(var_module, VAR_MTR_ON0 + signal));
		else if (mk->ctrl)
			send_variable (VAR_LFA_ON1 + signal - 1, 		!var_get_value(var_module, VAR_LFA_ON1 + signal - 1));
		else
			send_variable (VAR_PUF_PLAY_SIG0  + signal, 	!var_get_value(var_module, VAR_PUF_PLAY_SIG0 + signal));
	} /* if signal */	
	return (ok);
} /* wi_key_mod */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (window)
WINDOWP window;

{
	RTMCLASSP	module = Module(window);
	STAT_P		status = module->status;
	STAT_P		stat_alt = module->stat_alt;
	WORD			refNum = (WORD)module->special;
	BOOLEAN		new = status->new || (window->flags & WI_JUNK);
	WORD			msec;
	
	/* Sicherheitshalber */
	status->new = new;

	/* redraw_window(window, &window->scroll); */

	module->set_dbox (module);
	
	/* Aufruf von Hand */
	window->finished (window);
} /* wi_timer_mod */

/*****************************************************************************/
/* Nach zeichnen Status verÑndern                                            */
/*****************************************************************************/

PRIVATE VOID wi_finished_mod (window)
WINDOWP window;

{	
	RTMCLASSP	module = Module(window);
	STAT_P		status;
	STAT_P		stat_alt;

	window->milli = 0;
	status = module->status;
	stat_alt = module->stat_alt;
	status->new = FALSE;

	if (stat_alt->record != status->record)
	{	/* Status wurde per MidiShare-Interrupt-Message verÑndert,
			muû noch in VAR eingetragen werden */
		/* send_record (status->record); */
	} /* if */
	/* Neue Werte Åbernehmen */
	mem_move(stat_alt, status,(UWORD)sizeof(STATUS));
} /* wi_finished_mod */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;
{
	WINDOWP window;
	WORD    menu_height, inx;
	
	inx    = num_windows (CLASS_TRA, SRCH_ANY, NULL);
	window = create_window_obj (KIND, CLASS_TRA);
	
	if (window != NULL)
	{
		WINDOW_INITOBJ_OBJ

		window->flags     = FLAGS | WI_MODELESS;
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
		window->mousenum  = POINT_HAND;
		window->mouseform = NULL;
		window->milli     = MILLI;
		window->module   = 0;
		window->edit_obj  = 0;
		window->edit_inx  = 0;
		window->exit_obj  = 0;
		window->object    = obj;
		window->menu      = menu;
		window->click     = wi_click_mod;
		window->key       = wi_key_mod;
		window->timer     = wi_timer_mod;
		window->showinfo  = info_mod;
		window->finished	= wi_finished_mod;
			
		sprintf (window->name, (BYTE *)tra_text [FTRAN].ob_spec);
		sprintf (window->info, (BYTE *)tra_text [FTRAI].ob_spec, 0);
		
		create_displayobs (window);
		
	} /* if */
	
	return (window);                      /* Fenster zurÅckgeben */
} /* crt_mod */

PRIVATE VOID create_displayobs (WINDOWP window)
{	
	CrtObjectDOInsert (window, TRASMPTE,		VAR_SMPTE, ObjectTime, 0);
	CrtObjectDOInsert (window, TRALOCLEFT,		VAR_CYCLE_START, ObjectTime, CNTRL_LEFT);
	CrtObjectDOInsert (window, TRALOCRIGHT,	VAR_CYCLE_STOP, ObjectTime, CNTRL_RIGHT);
	CrtObjectDOInsert (window, TRAOFF,			VAR_PUF_OFFSET, ObjectTime, 0);

	CrtObjectDOInsert (window, TRAPUNCHIN,		VAR_PUF_PUNCH_IN, ObjectCheck, 0);
	CrtObjectDOInsert (window, TRAPUNCHOUT,	VAR_PUF_PUNCH_OUT, ObjectCheck, 0);

	CrtObjectDOInsert (window, TRAPLAY, 	VAR_TRA_PLAY, ObjectPush, ENTER);
	CrtObjectDOInsert (window, TRASTOP, 	VAR_TRA_STOP, ObjectPush, ZERO);
	CrtObjectDOInsert (window, TRAFFWD, 	VAR_TRA_FFWD, ObjectCheck, FW);
	CrtObjectDOInsert (window, TRAREW, 		VAR_TRA_REW, ObjectCheck, BW);
	CrtObjectDOInsert (window, TRAREC, 		VAR_RECORD, ObjectCheck, REC);

	CrtObjectDOInsert (window, TRACYCLE, 	VAR_CYCLE, ObjectCheck, CYCLEONOFF);
	CrtObjectDOInsert (window, TRACLICK, 	VAR_TRA_CLICK, ObjectCheck, CLICKONOFF);
	CrtObjectDOInsert (window, TRASYNCOUT,	VAR_SYNC_OUT, ObjectCheck, 0);
	CrtObjectDOInsert (window, TRASYNC, 	VAR_SYNC_IN, ObjectCheck, MASTERONOFF);
	CrtObjectDOInsert (window, TRASYNCLOCK, VAR_SYNC_IN_LOCKED, ObjectCheck, 0);
	CrtObjectDOInsert (window, TRAPUF, 		VAR_PUF_PLAY, ObjectCheck, 0);
	CrtObjectDOInsert (window, TRABIG, 		VAR_BIG_PLAY, ObjectCheck, 0);
} /* create_displayobs */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN		ok;
	WINDOWP 		window;
	RTMCLASSP	module;
	WORD    		ret;
	
	window = search_window (CLASS_TRA, SRCH_ANY, ITRA);
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{	
			module = Module(window);
			module->set_dbox (module);
			if (! open_window (window)) hndl_alert (ERR_NOOPEN);
		} /* if */
		else 
			top_window (window);
		window->opened = 1;
	} /* if */
	
	ok= window !=0;
	
	return (ok);
} /* open_tra */

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

	window = search_window (CLASS_DIALOG, SRCH_ANY, ITRA);
		
	if (window == NULL)
	{
		 form_center (tra_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (tra_info, NULL, ITRA, (BYTE *)tra_text [FTRAN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = tra_info;
		sprintf(s, "%-20s", TRADATE);
		set_ptext (tra_info, TRAIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (tra_info, TRACOMPILE, s);
		sprintf(s, "%-20s", TRAVERSION);
		set_ptext (tra_info, TRAIVERNR, s);
		/*
		sprintf(s, "%-20s",  MAXTRASETUPS);
		set_ptext (tra_info, TRAISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (tra_info, TRAIAKT, s);
		*/
		if (! open_dialog (ITRA)) hndl_alert (ERR_NOOPEN);
	}

	return (window != NULL);
} /* info_mod */

/*****************************************************************************/
/* Kreiere Modul                                                             */
/*****************************************************************************/
PRIVATE RTMCLASSP create ()
{
	WINDOWP		window;
	RTMCLASSP 	module;
	STRING		s;
	FILE			*fp;
	STAT_P		status;
	SHORT			refNum;
	
	module = create_module (module_name, instance_count);
		
	if (module != NULL)
	{
		module->class_number	= CLASS_TRA;
		module->icon			= &tra_desk[TRAICON];
		module->icon_position= ITRA;
		module->icon_number	= ITRA;	/* Soll bei Init vergeben werden */
		module->menu_title	= MOPTIONS;
		module->menu_position= MTRA;
		module->menu_item		= MTRA;	/* Soll bei Init vergeben werden */
		module->multiple		= FALSE;
		
		module->crt				= crt_mod;
		module->open			= open_mod;
		module->info			= info_mod;
		module->init			= init_tra;
		module->term			= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_CONTROL;
		module->apply				= 0;
		module->reset				= 0;
		module->precalc			= 0;
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
		module->max_setups		 	= MAXSETUPS;
		module->standard				= (SET_P)mem_alloc(sizeof(SETUP));
		module->actual->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->edited->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->status 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->stat_alt 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->load					= load_obj;
		module->save					= save_obj;
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
		mem_set(status, 0, (UWORD)sizeof(STATUS));

		/* Click Setup */
		status->click_freq 		= 400;
		status->click_duration 	= 2;
		status->click_counter  	= 0;

		/* Setup-Strukturen initialisieren */
		mem_set(module->standard, 0, (UWORD) sizeof(SETUP));
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (transport, NULL, ITRA);
		/* Modul-Struktur einbinden */
		window->module		= (VOID*) module;
		module->window		= window;
		refNum				= init_midishare();
		module->special	= (LONG) refNum;
		modulep[refNum]	= module;
		if (refNum>0)
			InstallFilter(refNum);									/* Midi-Input-Filter einbauen */	
		
		tra_module = module;
		init_mtc (module);

		add_rcv(VAR_SET_TRA,  module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_TRA, MAXSETUPS);
		add_rcv(VAR_SMPTE, module);		/* Message einklinken */
		add_rcv(VAR_PUF_OFFSET, module);		/* Message einklinken */
		add_rcv(VAR_SYNC_IN, module);			/* Message einklinken */
		add_rcv(VAR_SYNC_IN_LOCKED, module);	/* Message einklinken */
		add_rcv(VAR_SYNC_OUT, module);	/* Message einklinken */
		add_rcv(VAR_SYNC_OUT_FRAMES, module);	/* Message einklinken */
		add_rcv(VAR_SYNC_OUT_PORTS, module);	/* Message einklinken */
		add_rcv(VAR_TRA_PLAY, module);	/* Message einklinken */
		add_rcv(VAR_TRA_STOP, module);	/* Message einklinken */
		add_rcv(VAR_TRA_FFWD, module);	/* Message einklinken */
		add_rcv(VAR_TRA_REW, module);		/* Message einklinken */
		add_rcv(VAR_RECORD,   module);	/* Message einklinken */
		add_rcv(VAR_PUF_PLAY, module);	/* Message einklinken */
		add_rcv(VAR_BIG_PLAY, module);	/* Message einklinken */
		add_rcv(VAR_CYCLE, module);		/* Message einklinken */
		add_rcv(VAR_CYCLE_START, module);	/* Message einklinken */
		add_rcv(VAR_CYCLE_STOP, module);	/* Message einklinken */

		/* VAR initialisieren */
		send_variable(VAR_TRA_CLICK, FALSE);
		send_variable(VAR_TRA_STOP, TRUE);
		send_variable(VAR_PUF_PLAY, TRUE);
		send_variable(VAR_BIG_PLAY, TRUE);
		send_variable(VAR_RECORD, FALSE);
		send_variable(VAR_SYNC_OUT, TRUE);

		/* SMPTE auf Port 1 */
		send_variable(VAR_SYNC_OUT_PORTS, 0x0001);
		
		/* 25 fps SMPTE */
		send_variable(VAR_SYNC_OUT_FRAMES, 1);

	} /* if */
	return module;
} /* create */

/*****************************************************************************/
/* Lîsche Objekt                                                            */
/*****************************************************************************/
PUBLIC VOID destroy_mod (module)
RTMCLASSP module;
{
	INT	refNum;
	
	if (msh_available)
	{
		for (refNum = 0; module != modulep[refNum]; refNum++);
	
		if (refNum > 0)
		{
			refNums[instance_count--] = 0;
			MidiClose (refNum);				/* abmelden	*/
		} /* if */
	} /* if */
	
	destroy_obj (module);	/* weiter mit Standard-Routine */
} /* destroy_mod */

/*****************************************************************************/
/* MidiShare initialisieren                                                  */
/*****************************************************************************/

PRIVATE WORD init_midishare ()
{
	SHORT		ref, refNum = 0;			/* temporÑre Referenznummer */
	STRING	s;
	
	if (msh_available)
	{
		if (MidiShare())
		{
			if (instance_count <= max_instances)
			{
				if (instance_count == 0)
					sprintf (s, "TRA");
				else
					sprintf (s, "TRA %d", instance_count + 1);
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
			MidiSetRcvAlarm (refNum, receive_evts_tra);	/* Interrupt-Handler */		
			MidiSetApplAlarm (refNum, receive_alarm_tra);	/* Alarm-Handler */
			/* An alle anschlieûen */
			try_all_connect (refNum);
		} /* else */
	} /* if */
	
	return refNum;
	
} /* init_midishare */

/*****************************************************************************/
/* RSC îffnen                                                      		     */
/*****************************************************************************/

PRIVATE BOOLEAN init_rsc_tra ()

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
  tra_desk	= (OBJECT *)rs_trindex [TRA_DESK];	/* Adresse des TRA-Desktop */
  transport	= (OBJECT *)rs_trindex [TRANSPORT];	/* Adresse der Transportleiste */
  tra_text	= (OBJECT *)rs_trindex [TRA_TEXT];  /* Adresse der TRA-Texte */
  tra_info	= (OBJECT *)rs_trindex [TRA_INFO];	/* Adresse der TRA-Info-Anzeige */
  tra_help	= (OBJECT *)rs_trindex [TRA_HELP];	/* Adresse der TRA-Hilfe-Anzeige */
#else

  strcpy (rsc_name, TRA_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (tra_rsc_ptr, rsc_name))
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

  rs_gaddr (tra_rsc_ptr, R_TREE,  TRA_DESK,	&tran_desk);	/* Adresse des TRA-Desktop*/
  rs_gaddr (tra_rsc_ptr, R_TREE,  TRANSPORT,	&transport);	/* Adresse der Transportleiste */
  rs_gaddr (tra_rsc_ptr, R_TREE,  TRA_TEXT,	&tra_text);    /* Adresse der TRA-Texte */
  rs_gaddr (tra_rsc_ptr, R_TREE,  TRA_INFO,	&tra_info);    /* Adresse der TRA-Info-Anzeige */
  rs_gaddr (tra_rsc_ptr, R_TREE,  TRA_HELP,	&tra_help);    /* Adresse der TRA-Hilfe-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (tra_desk, TRUE);
	fix_objs (transport, TRUE);
	fix_objs (tra_text, TRUE);
	fix_objs (tra_info, TRUE);
	fix_objs (tra_help, TRUE);
	
	/*
	do_flags (tra_setup, TRACANCEL, UNDO_FLAG);
	do_flags (tra_setup, TRAHELP, HELP_FLAG);
	*/
	menu_enable(menu, MTRA, TRUE);

	return (TRUE);
} /* init_rsc_tra */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PUBLIC BOOLEAN term_rsc_tra ()
{
  BOOLEAN ok;

  ok = TRUE;

#if ((XRSC_CREATE|RSC_CREATE) == 0)
  ok = rs_free (tra_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc_tra */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_tra ()
{
	BOOLEAN ok = TRUE;

	ok &= init_rsc_tra ();
	instance_count = load_create_infos (create, module_name, max_instances);

	return (ok);
} /* init_tra */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN	ok = TRUE;

	ok &= term_rsc_tra ();
	return (ok);
} /* term_mod */
