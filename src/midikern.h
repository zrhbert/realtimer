/*****************************************************************************/
/*                                                                           */
/* Modul: MIDIKERN.H                                                         */
/*                                                                           */
/* Internal replacement for the resident MidiShare kernel (MIDSHARE.PRG).   */
/* Declares the subset of the MidiShare API that RTM uses, implemented in   */
/* midikern.c as plain C functions inside the program itself. Include this  */
/* only through msh_unit.h, which defines the event types and structures.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __MIDIKERN__
#define __MIDIKERN__

/* Properly prototyped callback types. The K&R TaskPtr/RcvAlarmPtr types in
	msh_unit.h carry no argument info; calls through them would promote the
	short refNum to int and no longer match the callback definitions. */
typedef void (*MkTaskPtr)      (long date, short refNum, long a1, long a2, long a3);
typedef void (*MkRcvAlarmPtr)  (short refNum);
typedef void (*MkApplAlarmPtr) (short refNum, long code);

/* Kernel lifecycle (called from init_msh/term_msh) */
short MidiInternalStart (void);
void  MidiInternalStop  (void);

/* Environment */
Boolean MidiShare        (void);
short   MidiGetVersion   (void);
short   MidiOpen         (const char *name);
void    MidiClose        (short refNum);
short   MidiGetNamedAppl (const char *name);
void    MidiConnect      (short src, short dest, Boolean state);
void    MidiSetRcvAlarm  (short refNum, MkRcvAlarmPtr alarm);
void    MidiSetApplAlarm (short refNum, MkApplAlarmPtr alarm);
void    MidiSetFilter    (short refNum, FilterPtr filter);
Boolean MidiGetPortState (short port);
long    MidiFreeSpace    (void);
long    MidiTotalSpace   (void);

/* Events. The event arguments are untyped like the original macro API:
	the old code calls these with MidiSTPtr and other cell views. */
MidiEvPtr MidiNewEv      (short type);
MidiEvPtr MidiCopyEv     (void *e);
void      MidiFreeEv     (void *e);
void      MidiSetField   (void *e, long f, long v);
long      MidiGetField   (void *e, long f);
void      MidiAddField   (void *e, long v);
long      MidiCountFields (void *e);

/* Time */
long MidiGetTime (void);

/* Sending / receiving */
void      MidiSendIm   (short refNum, void *e);
long      MidiCountEvs (short refNum);
MidiEvPtr MidiGetEv    (short refNum);
void      MidiFlushEvs (short refNum);

/* Tasks */
MidiEvPtr MidiTask        (MkTaskPtr proc, long date, short refNum, long a1, long a2, long a3);
MidiEvPtr MidiDTask       (MkTaskPtr proc, long date, short refNum, long a1, long a2, long a3);
long      MidiCountDTasks (short refNum);
void      MidiFlushDTasks (short refNum);
void      MidiExec1DTask  (short refNum);

/* Synchronisation: external MIDI sync was dropped with the resident kernel.
	These keep the call sites alive and report permanent internal sync. */
void MidiGetSyncInfo (SyncInfoPtr p);
void MidiSetSyncMode (unsigned short mode);
long MidiGetExtTime  (void);

#endif /* __MIDIKERN__ */
