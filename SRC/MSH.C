/*****************************************************************************/
/*                                                                           */
/* Modul: MSH.C                                                          	  */
/*                                                                           */
/* MidiShare Funktionen fÅr RTM                                           	  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- send_start_ev_delight und stop eingebaut, 18.03.95
- CMO eingebaut, 24.08.95
23.07.94
- VAR_CYCLE eingebaut
- BIG Kommunikation eingebaut
26.07.93
- msh_available eingebaut
- unkonditionales Laden von MIDSHARE.PRG und MIDSAVE.PRG in init_msh
- send_ Fuktionen mit öberprÅfung auf (e) fÅr Betrieb ohne MS
- create gibt nun immer TRUE zurÅck, auch wenn MS nicht gefunden
- VAR in rtm_sendm und rtm_send eingebaut
- try_name_connect und try_num_connect eingebaut
20.05.93
- close_dakstat vor MidiShare Initialisierung wegen Textausgabe eigebaut
25.04.93
- Fehler in init beseitigt (ok-Status bei Aufruf von MS wenn MS bereits im Speicher)
22.04.93
- init und term incl. MS-Aufrufe eingefÅhrt
- MidiShare-Aufruf per Pexec()
- MidiFreeEvent in rtm_send Makro eingebaut
*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"
#include "dialog.h"
#include "resource.h"

#include "realtim4.h"
#include "objects.h"
#include "errors.h"
#include "realtspc.h"
#include "var.h"							/* wegen VAR_RECORD */
#include <msh_unit.h>					/* Deklarationen fÅr MidiShare */

#include "export.h"
#include "msh.h"							/* Deklarationen fÅr RTM-MidiShare Library */

/* VARIABLES *******************************************************************/
LOCAL SHORT	refpuf = 0,		/* MidiShare Referenz des Puffersequenzers */
				reftra = 0,
				refvar = 0,
				refcmo = 0,
				refbig = 0;
				
/********************************************************************/
/** Alle BigSeq Midi-Routinen
	
	
	Event-Struktur:
	
	field 0 - 1		Cybernetic-Arts Kennung (Manufacturer ID)
					Werte	<0x00>,<0x3d>
				
	field 2 		Event-Kennung  
					Werte	<0 - 7>
					0 =	3D
					1 =	Volume 
					2 = Part
					3 = Start
					4 = Stop
					5 = Offset
					6 = Track Mute
					7 = Track Demute
	
	field 3 		Spur Nummer 
					Werte 	<0 - 63>
	
	field 4 - n 	Data	
					
					Event-Typ				Daten-Art					
					
					0 =	3D					x, y, z	
					1 =	Volume 				vol
					2 = Part				x1, y1, z1 -
											xn, yn, zn
					3 = Start				time (in 4 Bytes)
					4 = Stop				time (in 4 Bytes)
					5 = Offset				time (in 4 Bytes)
					6 = Track Mute			track 1 - track n
					7 = Track Demute		track 1 - track n
						
					Werte
					x, y, z und vol 	<0 - 127>
					time				<0L - 0x7ffffff>
					track 				<0 - 64>
		

**/

/** DelightSyncCode	DSC (SysEx) **/

/** Fields **/
enum			DELIGHT_EV_FIELDS
	{
	DELIGHT_ID1_FIELD,
	DELIGHT_ID2_FIELD,
	DELIGHT_EV_TYPE_FIELD,
	DELIGHT_TRACK_NR_FIELD,
	DELIGHT_DATA_FIELD,
	
	NUM_DELIGHT_EV_FIELDS
	};

/** Delight ID **/
#define 		DELIGHT_ID1		0x00L
#define 		DELIGHT_ID2		0x3dL

/** Event types **/
enum			DELIGHT_EV_TYPES
	{
	DELIGHT_START_EV,
	DELIGHT_STOP_EV,
	
	NUM_DELIGHT_EV_TYPES
	};


/** Alle defines zu den BigSeq Events **/
/********************************************************************/
/********************************************************************/

/** Fields **/
enum			BIG_SEQ_EV_FIELDS
		{
 		BIG_SEQ_ID1_FIELD,
 		BIG_SEQ_ID2_FIELD,
 		BIG_SEQ_EV_TYPE_FIELD,
 		BIG_SEQ_TRACK_NR_FIELD,
 		BIG_SEQ_DATA_FIELD,
		NUM_BIG_SEQ_EV_FIELDS
 		};

/** Cycernetic Arts ID **/
#define 		BIG_SEQ_CYB_ARTS_ID1	0x00
#define 		BIG_SEQ_CYB_ARTS_ID2	0x3d

/** Event types **/
enum			BIG_SEQ_EV_TYPES
		{
		BIG_SEQ_3D_EV,
		BIG_SEQ_VOLUME_EV,
		BIG_SEQ_PART_EV,
		BIG_SEQ_START_EV,
		BIG_SEQ_STOP_EV,
		BIG_SEQ_OFFSET_EV,
		BIG_SEQ_TRACK_MUTE_EV,
		BIG_SEQ_TRACK_DEMUTE_EV,
		NUM_BIG_SEQ_EV_TYPES
		};

/** Value maxima **/
#define			BIG_SEQ_MAX_VALUE	127
#define			BIG_SEQ_MAX_TRACKS	64

/*****************************************************************************/
#define rtm_sendm(refnum, e)\
	if (refpuf > 0) MidiSendIm(refpuf, MidiCopyEv(e));\
	if (refvar > 0) MidiSendIm(refvar, MidiCopyEv(e));\
	if (reftra > 0) MidiSendIm(reftra, MidiCopyEv(e));\
	MidiFreeEv(e);

/********************************************************************/
void set_time_ev_big( MidiEvPtr e, LONG value  )
{
LONG	 val[ 4 ];

val[ 0 ] = ( value & 255L );
value  >>= 8;
val[ 1 ] = ( value & 255L );
value  >>= 8;
val[ 2 ] = ( value & 255L );
value  >>= 8;
val[ 3 ] = ( value & 255L );

MidiAddField( e, val[ 3 ] );
MidiAddField( e, val[ 2 ] );
MidiAddField( e, val[ 1 ] );
MidiAddField( e, val[ 0 ] );

}

/********************************************************************/
LONG get_time_ev_big( MidiEvPtr e )
{
LONG value;

value   = ( MidiGetField( e, BIG_SEQ_DATA_FIELD ) & 255L );
value <<= 8;
value  += ( MidiGetField( e, BIG_SEQ_DATA_FIELD + 1 ) & 255L );
value <<= 8;
value  += ( MidiGetField( e, BIG_SEQ_DATA_FIELD + 2 ) & 255L );
value <<= 8;
value  += ( MidiGetField( e, BIG_SEQ_DATA_FIELD + 3 ) & 255L );

return value;
}
 
/********************************************************************/
/** 3D Event senden **/
/********************************************************************/
void send_3d_ev_big (SHORT refnum, INT track_nr, INT x, INT y, INT z )
{
MidiEvPtr	e;

e = MidiNewEv( typeStream );
if ( !e ) return;

MidiAddField( e, BIG_SEQ_CYB_ARTS_ID1 );
MidiAddField( e, BIG_SEQ_CYB_ARTS_ID2 );

MidiAddField( e, BIG_SEQ_3D_EV );
MidiAddField( e, (LONG)track_nr );

MidiAddField( e, (LONG)x );
MidiAddField( e, (LONG)y );
MidiAddField( e, (LONG)z );

MidiSendIm( refnum, e );

}

/********************************************************************/
/** 3D Track senden **/
/********************************************************************/
VOID send_track_big (SHORT refnum, WORD signal, PUFEVP start)
{
	MidiEvPtr	e;
	PUFEVP		evp;
	KOOR_SINGLE	*koor_s;
	PUF_INF		*info;
	POINT_3D		*point;
	
	e = MidiNewEv( typeStream );
	if ( !e ) return;
	
	MidiAddField (e, BIG_SEQ_CYB_ARTS_ID1);
	MidiAddField (e, BIG_SEQ_CYB_ARTS_ID2);
	
	MidiAddField (e, BIG_SEQ_PART_EV);
	MidiAddField (e, var_get_value (var_module, VAR_CMI_SIGNAL1 + signal - 1));
	
	evp = start;
	do
	{
		info = &evp->event;
		koor_s = &info->koors->koor[signal];
		point = &koor_s->koor;
		MidiAddField (e, (LONG)point->x);
		MidiAddField (e, (LONG)point->y);
		MidiAddField (e, (LONG)point->z);
		
		evp = evp->next;
		
	} while (evp != start);
	MidiSendIm (refbig, e);
} /* send_track_big */

VOID send_part_big (SHORT refnum, PUFEVP start)
{
	WORD 		signal;
	STRING	title;

	if ( !refbig )
	/* Noch einmal probieren ob BIG inzwischen geladen ist */
		refbig = try_name_connect (refnum, "BIG");

	if (refbig > 0)
	{
		sprintf (title, "öbertragung PUF->BIG");
		daktstatus (title, "Daten werden Åbertragen...");
		for (signal = 1; signal < MAXSIGNALS; signal++)
		{
			send_track_big (refnum, signal, start);
			/* Nach jeder Spur die Anzeige auffrischen */
			set_daktstat(100 * signal/(MAXSIGNALS-1));
		} /* for */
		close_daktstat();
	} /* if */
	else
		hndl_alert (ERR_NOBIG);
		
} /* send_part_big */

/********************************************************************/
BOOL receive_3d_ev_big( MidiEvPtr e )
{

	return TRUE;
}

/********************************************************************/
BOOL send_signal_mute_ev_big (SHORT refnum, WORD signal)
{
	MidiEvPtr	e;
	
	e = MidiNewEv (typeStream);
	if (!e) return FALSE;
	
	MidiAddField (e, BIG_SEQ_CYB_ARTS_ID1);
	MidiAddField (e, BIG_SEQ_CYB_ARTS_ID2);
	
	MidiAddField (e, BIG_SEQ_TRACK_MUTE_EV);
	
	/* Zugehîrigen Ausgabekanal muten */
	MidiAddField (e, var_get_value (var_module, VAR_CMI_SIGNAL1 +signal -1));
	
	if ( !refbig )
	/* Noch einmal probieren ob BIG inzwischen geladen ist */
		refbig = try_name_connect (refnum, "BIG");

	MidiSendIm (refbig, e);
	return TRUE;
} /* send_signal_mute_ev_big */

/********************************************************************/
BOOL send_all_mute_evs_big (SHORT refnum)
{
	MidiEvPtr	e;
	WORD			signal;
	
	e = MidiNewEv( typeStream );
	if ( !e ) return FALSE;
	
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID1 );
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID2 );
	
	MidiAddField( e, BIG_SEQ_TRACK_MUTE_EV );
	
	for ( signal = 1; signal < MAXSIGNALS; signal++ )
	{
		if (var_get_value (var_module, VAR_PUF_PLAY_SIG0 + signal))
		{
			/* Zugehîrigen Ausgabekanal muten */
			MidiAddField (e, var_get_value (var_module, VAR_CMI_SIGNAL1 +signal -1));
		} /* if */
	} /* for */
	
	if ( !refbig )
	/* Noch einmal probieren ob BIG inzwischen geladen ist */
		refbig = try_name_connect (refnum, "BIG");

	MidiSendIm( refbig, e );
	return TRUE;
} /* send_all_mute_evs_big */

/********************************************************************/
BOOL send_signal_demute_ev_big (SHORT refnum, WORD signal)
{
	MidiEvPtr	e;
	
	e = MidiNewEv( typeStream );
	if ( !e ) return FALSE;
	
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID1 );
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID2 );
	
	MidiAddField( e, BIG_SEQ_TRACK_DEMUTE_EV );
	
	/* Zugehîrigen Ausgabekanal muten */
	MidiAddField (e, var_get_value (var_module, VAR_CMI_SIGNAL1 + signal - 1));
	
	if ( !refbig )
	/* Noch einmal probieren ob BIG inzwischen geladen ist */
		refbig = try_name_connect (refnum, "BIG");

	MidiSendIm( refbig, e );
	return TRUE;
} /* send_signal_demute_ev_big */

/********************************************************************/
BOOL send_all_demute_evs_big (SHORT refnum)
{
	MidiEvPtr	e;
	WORD			signal;
	
	e = MidiNewEv( typeStream );
	if ( !e ) return FALSE;
	
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID1 );
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID2 );
	
	MidiAddField( e, BIG_SEQ_TRACK_DEMUTE_EV );
	
	for ( signal = 1; signal < MAXSIGNALS; signal++ )
	{
		/* Zugehîrigen Ausgabekanal demuten */
		MidiAddField (e, var_get_value (var_module, 
			VAR_CMI_SIGNAL1 + signal - 1));
	} /* for */
	
	if ( !refbig )
	/* Noch einmal probieren ob BIG inzwischen geladen ist */
		refbig = try_name_connect (refnum, "BIG");

	MidiSendIm( refbig, e );
	return TRUE;
} /* send_all_demute_evs_big */

/********************************************************************/
/** Start Event senden **/
/********************************************************************/
GLOBAL void send_start_ev_delight(SHORT refnum,  LONG position )
{
MidiEvPtr	e;

e = MidiNewEv( typeSysEx );
if ( !e ) return;

MidiAddField( e, DELIGHT_ID1 );
MidiAddField( e, DELIGHT_ID2 );

MidiAddField( e, (LONG)DELIGHT_START_EV );
MidiAddField( e, (LONG)0 );

set_time_ev_big( e, position );

MidiSendIm(refnum, e );
}

/********************************************************************/
/** Stop Event senden **/
/********************************************************************/
GLOBAL void send_stop_ev_delight (SHORT refnum,  LONG position )
{
MidiEvPtr	e;

e = MidiNewEv( typeSysEx );
if ( !e ) return;

MidiAddField( e, DELIGHT_ID1 );
MidiAddField( e, DELIGHT_ID2 );

MidiAddField( e, (LONG)DELIGHT_STOP_EV );
MidiAddField( e, (LONG)0 );

set_time_ev_big( e, position );

MidiSendIm(refnum, e );

}

/********************************************************************/
BOOL send_start_ev_big (SHORT refnum, LONG time)
{
	MidiEvPtr	e;
	
	e = MidiNewEv( typeStream );
	if ( !e ) return FALSE;
	
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID1 );
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID2 );
	
	MidiAddField( e, BIG_SEQ_START_EV );
	
	set_time_ev_big (e, time);

	if ( !refbig )
	/* Noch einmal probieren ob BIG inzwischen geladen ist */
		refbig = try_name_connect (refnum, "BIG");

	MidiSendIm (refbig, e );
		
	return TRUE;
} /* send_start_ev_big */

/********************************************************************/
BOOL send_stop_ev_big (SHORT refnum, LONG time)
{
	MidiEvPtr	e;
	
	e = MidiNewEv( typeStream );
	if ( !e ) return FALSE;
	
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID1 );
	MidiAddField( e, BIG_SEQ_CYB_ARTS_ID2 );
	
	MidiAddField( e, BIG_SEQ_STOP_EV );
	
	set_time_ev_big (e, time);
		
	if ( !refbig )
	/* Noch einmal probieren ob BIG inzwischen geladen ist */
		refbig = try_name_connect (refnum, "BIG");

	MidiSendIm (refbig, e );
	return TRUE;
} /* send_stop_ev_big */

/********************************************************************/

GLOBAL VOID timstr (LONG time, STRING timestr)
{
	/* Wandelt einen Long-Wert fÅr die Anzahl der Millisekunden */
	/* in eine Kette von Zahlen der Form hh:mm:ss:ll um */
	REG LONG temp;
	REG WORD hh, mm, ss, ll;
	
	temp = time /1000;						/* Millisekunden abschneiden */
	ll = (WORD) (time - temp*1000);			/* ms merken */
	time /= 1000;
	temp /= 60;
	ss = (WORD) (time - temp*60);
	time /= 60;
	temp /= 60;
	mm = (WORD) (time - temp*60);
	time /= 60;
	temp /= 24;
	hh = (WORD) (time - temp*24);
	time /= 24;
	/*
	hh = time / 3600000L;
	mm = (time - hh * 3600000L) / 60000L;
	ss = (time - hh * 3600000L - mm * 60000L) / 1000;
	*/
	sprintf(timestr, "%2d:%2d:%2d:%3d", hh, mm, ss, ll);
} /* timstr */

GLOBAL LONG strtim (STRING timestr)
{
	/* Wandelt eine Kette von Zahlen der Form hhmmssll */
	/* in einen Long-Wert fÅr die Anzahl der Millisekunden um */
	REG LONG time;
	LONG hh = 0L, mm = 0L, ss = 0L, ll = 0L;
	STRING s, trenn = ":. ";
	char *ts;
	LONG t[4], count = 0L;
	
	strcpy(s, timestr);
	
	ts = strtok(s, trenn);
	while (ts)
	{
		sscanf(ts, "%ld", &t[count]);
		count++;
		ts = strtok(NULL, trenn);
	} /* while */

	if (count>0) ll = t[--count];
	if (count>0) ss = t[--count];
	if (count>0) mm = t[--count];
	if (count>0) hh = t[--count];
	
	time = hh;
	time *= 60;
	time += mm;
	time *= 60;
	time += ss;
	time *= 1000;
	time += ll;

	return (time);
} /* strtim */

GLOBAL VOID curtimstr (STRING timestr)
{
	/* Gibt fÅllt den String mit der aktuellen Zeit */
	timstr (MidiGetTime(), timestr);
} /* curtimstr */

GLOBAL VOID rtm_send (SHORT refnum, VOID *e)
{
	/* Sendet einen MidiShare-Event an alle 
		RTM-Standard-Anwendungen */

	if (e)
	{
		/*
		if (refbig > 0) MidiSendIm (refbig, MidiCopyEv(e));
		*/
		if (refpuf > 0) MidiSendIm (refpuf, MidiCopyEv(e));
		if (refvar > 0) MidiSendIm (refvar, MidiCopyEv(e));
		if (reftra > 0) MidiSendIm (reftra, MidiCopyEv(e));
	/*	
		MidiFreeEv(e);	/* Original freigeben */
	*/
	} /* if */
} /* rtm_send */

GLOBAL VOID rtm_pos (SHORT refnum, LONG time)
{
	/* Positioniert alle angeschlossenen RTM-Module auf angegebene Zeit */
	MidiSTPtr e = (MidiSTPtr) MidiNewEv(typeRTMPosit);
	
	if (time<0) time = 0;
	
	if (e)
	{
		set_posit(e, time);
		rtm_sendm (refnum, e);
	} /* if */
} /* rtm_pos */

GLOBAL VOID rtm_cont (SHORT refnum, LONG time)
{
	/* Startet alle angeschlossenen RTM-Module ab angegebener Zeit */
/*
	TMidiContPtr e = (TMidiContPtr) MidiNewEv(typeRTMCont);
*/
	MidiSTPtr e = (MidiSTPtr) MidiNewEv(typeRTMCont);
	
	if (time<0) time = 0;

	if (e)
	{
		set_posit(e, time);
		rtm_sendm (refnum, e);
		send_start_ev_big (refnum, time);
	} /* if */
} /* rtm_cont */

GLOBAL VOID rtm_stop (SHORT refnum, LONG time)
{
	/* Stoppt alle angeschlossenen RTM-Module an angegebene Zeit */
/*
	TMidiStopPtr e = (TMidiStopPtr) MidiNewEv(typeRTMStop);
*/
	MidiSTPtr e = (MidiSTPtr) MidiNewEv(typeRTMStop);
	
	if (time<0) time = 0;

	if (e)
	{
		set_posit(e, time);
		rtm_sendm (refnum, e);
		send_stop_ev_big (refnum, time);
	} /* if */
} /* rtm_stop */

GLOBAL VOID rtm_cycleset (SHORT refnum, LONG start, LONG stop)
{
	/* Gibt allen angeschlossenen RTM-Modulen die
		aktuellen Cycle-Endwerte durch */
/*
	TMidiCycleSetPtr e = (TMidiCycleSetPtr) MidiNewEv(typeRTMCycleSet);
*/
	MidiSTPtr e = (MidiSTPtr) MidiNewEv(typeRTMCycleSet);
	
	if (e)
	{
		if (start<0) start = 0;
		set_cycle_start(e, start);
		if (stop<0) stop = 0;
		set_cycle_end(e, stop);
		rtm_sendm (refnum, e);
	} /* if */
} /* rtm_cycleset */

GLOBAL VOID rtm_cycleonoff (SHORT refnum, BOOLEAN flag)
{
	/* Gibt allen angeschlossenen RTM-Module an,
		ob Cycle-Modus aktiv ist oder nicht */
/*
	TMidiCycleOnOffPtr e = (TMidiCycleOnOffPtr) MidiNewEv(typeRTMCycleOnOff);
*/
	MidiSTPtr e = (MidiSTPtr) MidiNewEv(typeRTMCycleOnOff);
	
	if (e)
	{
		set_cycle_on(e, flag);
		rtm_sendm (refnum, e);
	} /* if */
	else
		send_variable(VAR_CYCLE, flag);
} /* rtm_cycleonoff */

GLOBAL VOID rtm_record	 (SHORT refnum, BOOLEAN flag)
{
	/* Gibt allen angeschlossenen RTM-Modulen an,
		ob Record-Modus aktiv ist oder nicht */

/*		
	MidiSTPtr e = (MidiSTPtr) MidiNewEv(typeRTMRecordOnOff);
	
	if (e)
	{
		set_record_on(e, flag);
		rtm_sendm (refnum, e);
	} /* if */
	else
*/
	send_variable(VAR_RECORD, flag);
} /* rtm_record */

GLOBAL SHORT try_name_connect	 (SHORT refnum, STRING name)
{
	SHORT 	ref;
	
	ref = MidiGetNamedAppl(name);			/* Referenznummer holen */
	if (ref>0)
		return try_num_connect (refnum, ref);

	return 0;
} /* try_name_connect */

GLOBAL SHORT try_num_connect (SHORT refnum, SHORT ref)
{
	if (ref>=0 && refnum >=0) {
		MidiConnect(ref,refnum,TRUE);				/* Input der Appl. anschliessen */
		MidiConnect(refnum,ref,TRUE);				/* Output der Appl. anschliessen */
		return ref;
	} /* if */
	return 0;
} /* try_num_connect */

GLOBAL BOOLEAN try_all_connect	 (SHORT refnum)
{
	BOOL	ret;
		
	/* Modul an alle Applikationen anschliessen,
		Referenznummer setzen, bzw. ungÅltig machen */
	ret = try_num_connect (0, refnum);
	if (refpuf > 0)
		refpuf = try_num_connect (refnum, refpuf);
	if ( !refpuf )
		refpuf = try_name_connect (refnum, "PUF");
	if (reftra > 0)
		reftra = try_num_connect (refnum, reftra);
	if ( !reftra )
		reftra = try_name_connect (refnum, "TRA");
	if (refvar > 0)
		refvar = try_num_connect (refnum, refvar);
  /* BD 2012_01_22 disable VAR
  	if ( !refvar )
		refvar = try_name_connect (refnum, "VAR");
 */
	if (refbig > 0)
		refbig = try_num_connect (refnum, refbig);
	if ( !refbig )
		refbig = try_name_connect (refnum, "BIG");
	if (refcmo > 0)
		refcmo = try_num_connect (refnum, refcmo);
	if ( !refcmo )
		refbig = try_name_connect (refnum, "CMO");
	return ret;
} /* try_all_connect */


/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_msh ()
{
	STRING	s;
	BOOLEAN	ok;
	VOID		*orig, *temp;

	strcpy (s, midishare_path);
	strcat (s, "MIDSHARE.PRG");

	close_daktstat();					/* Sicherheitshalber */

	
#ifdef MIDISHARE_SCREEN_CLEAN
	/* Bildschirm zeitweise umschalten */
	orig = Logbase ();
	temp = malloc (1024*1024/8);
	if (temp)
		Setscreen (temp, (void*)-1, -1);
#endif
		
	/* Ist MidiShare da? */
	ok = (Pexec(0, s, "","") >=0 );



	if (ok)
	{
		strcpy (s, midishare_path);
		strcat (s, "MIDISAVE.PRG");

		printf ("MIDISAVE.PRG wird gestartet ...");
		Pexec(0, s, "","");
		TCMidiRestore();
	} /* if */


	ok = MidiShare();
#ifdef MIDISHARE_SCREEN_CLEAN
	if (temp)
		Setscreen (orig, (void*)-1, -1);
#endif
	
	if (!ok)							/* Ist MidiShare da? */
	{
		hndl_alert (ERR_NOMIDISHARE);
		ok = FALSE;
		msh_available  = FALSE;
	} /* if */
	else
		msh_available  = TRUE;

	return (TRUE); /* immer TRUE zurÅckgeben, auch wenn MS nicht da */
} /* init_msh */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_msh ()
{
	BOOLEAN ok = TRUE;
	return (ok);
} /* term_msh */
