/*****************************************************************************/
/*                                                                           */
/* Modul: MSH.H                                                         	  */
/*                                                                           */
/* MidiShare Deklarationen fÅr RTM                                           */
/*****************************************************************************/

/*****************************************************************************
- send_start_ev_delight und stop eingebaut, 18.03.95
- PUF-BIG Kommunikation eingebaut, 15.01.95
18.11.93
- TMidiEventVol und TMidiEvent3D eingebaut
- msh_available eingebaut
- try_name_connect und try_num_connect eingebaut
*****************************************************************************/
#ifndef __MSH__
#define __MSH__

/****** DEFINES **************************************************************/

#define MAXMSAPPLS 64					/* Max. Anzahl MidiShare Applikationen */

enum rtmprivate {
	typeRTMPosit = typePrivate,		/* Positionieren auf RTM internen Zeitpunkt */
	typeRTMCont,							/* Starten ab letztem POSIT-Zeitpunkt */
	typeRTMStop,							/* Stoppen */
	typeRTMCycleSet,						/* Cycle-Zeitpunkt setzen */
	typeRTMCycleOnOff,					/* Cycle-Modus an/aus */
	typeRTMRecordOnOff,					/* Record an/aus */
	typeRTMSetVar,							/* VAR Variable setzen */
	typeRTMEvent3D,						/* 3D Event */
	typeRTMEventVol						/* Volume Event fÅr Ausgabe */
};

#define Par1(e)	((LONG)((e)->ptr1))
#define Par2(e)	((LONG)((e)->ptr2))
#define Par3(e)	((LONG)((e)->ptr3))
#define Par4(e)	((LONG)((e)->ptr4))

#define get_posit(e) 				MidiGetField(e, 0L)
#define set_posit(e, posit) 		MidiSetField(e, 0L, (LONG)posit)
#define get_cycle_start(e)  		MidiGetField(e, 0L)
#define set_cycle_start(e, start)  MidiSetField(e, 0L, (LONG)start)
#define get_cycle_end(e)    		MidiGetField(e, 1L)
#define set_cycle_end(e, end) 	MidiSetField(e, 1L, (LONG)end)
#define get_cycle_on(e)				MidiGetField(e, 0L)
#define set_cycle_on(e, flag)		MidiSetField(e, 0L, (LONG)flag)
#define get_record_on(e)			MidiGetField(e, 0L)
#define set_record_on(e, flag)	MidiSetField(e, 0L, (LONG)flag)
#define get_var_number(e)				MidiGetField(e, 0L)
#define set_var_number(e, variable)	MidiSetField(e, 0L, (LONG)variable)
#define get_var_value(e)				MidiGetField(e, 1L)
#define set_var_value(e, value)		MidiSetField(e, 1L, (LONG)value)
#define SetContID(e, id)				MidiSetField (e, 0L, (LONG)id)
#define GetContID(e)				(WORD)MidiGetField (e, 0L)
#define SetContValue(e, value)		MidiSetField (e, 1L, (LONG)value)
#define GetContValue(e)			(WORD)MidiGetField (e, 1L)

/****** TYPES ****************************************************************/

typedef struct TMidiPosit *TMidiPositPtr;
typedef struct	TMidiPosit 
{
	LONG	posit;				/* RTM interner Zeitpunkt */
} TMidiPosit;

typedef struct TMidiCont *TMidiContPtr;
typedef struct	TMidiCont 
{
	LONG	posit;				/* RTM interner Zeitpunkt */
} TMidiCont;

typedef struct TMidiStop *TMidiStopPtr;
typedef struct	TMidiStop 
{
	LONG	posit;				/* RTM interner Zeitpunkt */
} TMidiStop;

typedef struct TMidiCycleSet *TMidiCycleSetPtr;
typedef struct	TMidiCycleSet 
{
	LONG	start;				/* Startzeit, RTM interner Zeitpunkt */
	LONG	stop;					/* Stopzeit, RTM interner Zeitpunkt */
} TMidiCycleSet;

typedef struct TMidiCycleOnOff *TMidiCycleOnOffPtr;
typedef struct	TMidiCycleOnOff 
{
	BOOLEAN flag;
} TMidiCycleOnOff;

typedef struct TMidiSetVar *TMidiSetVarPtr;
typedef struct	TMidiSetVar 
{
	/* FÅr Setzen einer Variablen im VAR-Modul */
	LONG	variable;
	LONG	value;
} TMidiSetVar;

typedef struct TMidiEvent3D *TMidiEvent3DPtr;
typedef struct	TMidiEvent3D 
{
	/* FÅr öbertragung eines 3D-Events */
	BYTE	track,
			x,
			y,
			z;
} TMidiEvent3D;

typedef struct TMidiEventVol *TMidiEventVolPtr;
typedef struct	TMidiEventVol 
{
	/* FÅr öbertragung eines Volume-Events */
	BYTE	track,
			vol;
} TMidiEventVol;

/****** VARIABLES ************************************************************/
GLOBAL BOOLEAN msh_available;	/* Flag, ob Midishare da */

/****** FUNCTIONS ************************************************************/
/* PUF-BIG Kommunikation */
VOID send_track_big (SHORT refnum, WORD signal, struct pufevent *start);
VOID send_part_big (SHORT refnum, struct pufevent *start);
BOOL receive_3d_ev_big (MidiEvPtr e);
BOOL send_signal_mute_ev_big (SHORT refnum, WORD signal);
BOOL send_all_mute_evs_big (SHORT refnum);
BOOL send_signal_demute_ev_big (SHORT refnum, WORD signal);
BOOL send_all_demute_evs_big (SHORT refnum);
VOID send_start_ev_delight (SHORT refnum,  LONG position );
VOID send_stop_ev_delight (SHORT refnum,  LONG position );
BOOL send_start_ev_big (SHORT refnum, LONG time);
BOOL send_stop_ev_big (SHORT refnum, LONG time);

/* RTM intern */
GLOBAL VOID timstr 				(LONG time, STRING timestr);
GLOBAL LONG strtim 				(STRING timestr);
GLOBAL VOID curtimstr			(STRING timestr);
GLOBAL VOID rtm_send 			(SHORT refNum, VOID *e);
GLOBAL VOID rtm_pos				(SHORT refNum, LONG time);
GLOBAL VOID rtm_cont				(SHORT refNum, LONG time);
GLOBAL VOID rtm_stop				(SHORT refNum, LONG time);
GLOBAL VOID rtm_record			(SHORT refNum, BOOLEAN flag);
GLOBAL VOID rtm_cycleset		(SHORT refNum, LONG start, LONG stop);
GLOBAL VOID rtm_cycleonoff 	(SHORT refNum, BOOLEAN flag);
GLOBAL SHORT try_name_connect	 (SHORT refnum, STRING name);
GLOBAL SHORT try_num_connect 	(SHORT refNum, SHORT ref);
GLOBAL BOOL	try_all_connect	(SHORT refNum);
extern BOOL	TCMidiRestore (VOID);

GLOBAL BOOL	init_msh  _((VOID));
GLOBAL BOOL	term_msh  _((VOID));

#endif /* __MSH__ */
