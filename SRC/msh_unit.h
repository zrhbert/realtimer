/**************************************************************************/
/**
  							MIDI SHARE
    
	Le pr�sent fichier d�crit les points d'entr�e de MidiShare, ainsi que 
	les structures de donn�es et les constantes utilis�es. Le code Midi-
	Share proprement dit est contenu dans le fichier MIDSHARE.PRG, qui 
	doit etre plac� dans un dossier Auto. Au d�marrage de la machine, ce 
	code est charg� en m�moire, il est ancr� aux vecteurs $94 et $98 
	(trap 5 et trap 6).	Toutes les proc�dures et fonctions d�finies ici 
	se servent de cette "ancre" pour acc�der au code.

**/	
/**************************************************************************/

#ifndef	_MidiShareUnit_
#define	_MidiShareUnit_


/**************************************************************************/
/** Typdeklarationen die die Kompatibilit�t des C-Codes gew�hrleisten.
    ( auch zum Apple )
**/    
/**************************************************************************/

typedef char 	*Ptr;
typedef char 	Byte;
typedef short 	Boolean;

/**************************************************************************/
/** Event-Type f�r die Funktion MidiNewEv()
**/
/**************************************************************************/
		
#define typeNote		0	/* note avec hauteur, v�locit� et dir�e en ms */
		
#define typeKeyOn		1	/* Note On avec hauteur et v�locit� 		 */
#define typeKeyOff		2	/* Note Off avec hauteur et v�locit� 		 */
#define typeKeyPress 	3	/* Poly KeyPress avec hauteur et pression  	 */
#define typePolyPress 	3	/* Poly KeyPress avec hauteur et pression  	 */
#define typeCtrlChange	4	/* Control Change avec controleur et valeur 	 */
#define typeProgChange	5	/* Program Change avec num�ro de programme 	 */
#define typeChanPress	6	/* Channel Pressure avec pression 		*/
#define typeAfterTouch	6	/* Channel Pressure avec pression 		*/
#define typePitchWheel	7	/* Pitch Wheel avec LSB et MSB 			*/
#define typePitchBend	7	/* Pitch Wheel avec LSB et MSB 			*/

#define typeSongPos		8	/* Song Position Pointer LSB und MSB	*/
#define typeSongSel		9	/* Song Select avec num�ro de song 		*/
#define typeClock		10	/* Timing Clock						 	*/
#define typeStart		11	/* Start								*/
#define typeContinue	12	/* Continue							 	*/
#define typeStop		13	/* Stop								 	*/

#define typeTune		14	/* Tune Request							*/
#define typeActiveSens	15	/* Active Sensing						*/
#define typeReset		16	/* System Reset						 	*/

#define typeSysEx		17	/* System Excl with variable length     */
#define typeStream		18	/* Stream with variable length          */

							/* Events 19...127 are free to use		*/
#define typePrivate		19	/* Delight: Privat-Events				*/
#define typeKey			20	/* Delight: Key Code					*/

#define typeUniversal	30  	/* Universal			*/

#define typeProcess		128	/* Privat event for MidiCall			*/
#define typeDProcess	129	/* Privat event for MidiDTasks	 		*/
#define typeQuarterFrame 130 /* Privat event for MTC synchonisation */

#define typeCtrl14b		131	/* */
#define typeNonRegParam 132	/* */
#define typeRegParam	133 /* */

#define typeSeqNum		134
#define typeText		135
#define typeCopyright	136
#define typeSeqName		137
#define typeInstName	138
#define typeLyric		139
#define typeMarker		140
#define typeCuePoint	141
#define typeChanPrefix	142
#define typeEndTrack	143
#define typeTempo		144
#define typeSMPTEOffset	145
#define typeTimeSign	146
#define typeKeySign		147
#define typeSpecific	148
#define typeReserved	149	/* 149...254  Reserved for futur usage  */
	
#define typeDead		255	/* Privat event "invalid" */
		
/**************************************************************************/
/** Midi-Typen **/
/**************************************************************************/
	
#define NoteOff		0x80	
#define NoteOn		0x90
#define PolyTouch	0xa0
#define ControlChg	0xb0
#define ProgramChg	0xc0
#define AfterTouch 	0xd0
#define PitchBend 	0xe0
#define SysRealTime 0xf0
#define SysEx 		0xf0
#define QFrame  	0xf1
#define SongPos 	0xf2
#define SongSel 	0xf3
#define UnDef2 		0xf4
#define UnDef3 		0xf5
#define Tune 		0xf6
#define EndSysX 	0xf7
#define MClock 		0xf8
#define UnDef4 		0xf9
#define MStart 		0xfa
#define MCont 		0xfb
#define MStop 		0xfc
#define UnDef5 		0xfd
#define ActSense 	0xfe
#define MReset 		0xff	


/**************************************************************************/
/** Midi-Port Konstanten **/
/**************************************************************************/

#define ModemPort		0
#define PrinterPort		1

		
/**************************************************************************/
/** Midi-Error-Codes die von einigen Funktionen geliefert werden **/
/**************************************************************************/
		
#define MIDIerrSpace	-1	/** Kein Platz mehr 	**/
#define MIDIerrRefNum	-2	/** Falsche refnum 		**/
#define MIDIerrBadType	-3	/** Falscher Event-Typ	**/
#define MIDIerrIndex	-4	/** Falscher Index auf Event-Felder **/
		

/***********************************************************************
* 					SYNCHRONISATION CODES								
*-----------------------------------------------------------------------
* List of the error codes returned by some MidiShare functions.																	
************************************************************************/
		
#define MIDISyncExternal 0x8000	/* bit-15 for external synchronisation */
#define MIDISyncInternal 0x0000
#define MIDISyncAnyPort	 0x4000 /* bit-14 for synchronisation on any port */



/***********************************************************************
* 						  CHANGE CODES							
*-----------------------------------------------------------------------
When an application need to know about context modifications like opening 
and closing of applications, opening and closing of midi ports, changes 
in connections between applications, it can install an ApplAlarm (with 
MidiSetApplAlarm). This ApplAlarm is then called by MidiShare every time 
a context modification happens with a 32-bits code describing the 
modification. The hi 16-bits part of this code is the refNum of the 
application involved in the context modification, the low 16-bits part 
describe the type of change as listed here.
************************************************************************/
		
enum{	MIDIOpenAppl=1,
		MIDICloseAppl,
		MIDIChgName,
		MIDIChgConnect,
		MIDIOpenModem,
		MIDICloseModem,
		MIDIOpenPrinter,
		MIDIClosePrinter,
		MIDISyncStart=550,
		MIDISyncStop,
		MIDIChangeSync
};



/**************************************************************************/
/** Event-Struktur 
	Alle Events sind aus einer odr mehreren Zellen a 16 Bytes aufgebaut 
**/	
/**************************************************************************/

/**************************************************************************/
/** Zelle einer System-Exclusive Erweiterung **/
/**************************************************************************/
typedef struct TMidiSEX *MidiSEXPtr;
typedef struct TMidiSEX 
	{
	MidiSEXPtr	link;		/** Link auf die n�chste Zelle  **/
	Byte		data[12];	/** 12 Bytes f�r die Daten 		**/
	}TMidiSEX;


/**************************************************************************/
/** Zelle eines Private-Events **/
/**************************************************************************/
typedef struct TMidiST *MidiSTPtr;
typedef struct TMidiST 
	{
	Ptr ptr1;				/** 4 Pointer zur freien Verf�gung **/
	Ptr ptr2;			
	Ptr ptr3;
	Ptr ptr4;
	}TMidiST;


/**************************************************************************/
/** Zelle eines Universal-Events **/
/**************************************************************************/
typedef struct TMidiUni *MidiUniPtr;
typedef struct TMidiUni 
	{
	long	type_id;
	short	val[ 6 ];
	}TMidiUni;
	
/**************************************************************************/
/** Zelle eines normalen Events **/
/**************************************************************************/
typedef struct TMidiEv *MidiEvPtr;
typedef struct TMidiEv 
	{
	MidiEvPtr		link;	/** Pointer auf das n�chste Event 	  (0)  **/
	unsigned long	date;	/** Datum des Events in Millisekunden (4)  **/
	Byte			evType;	/** Event-Typ 						  (8)  **/
	Byte			refNum;	/** Nummer der Applikation 			  (9)  **/
	Byte			port;	/** Midi Port 						  (10) **/
	Byte			chan;	/** Midi Kanal  					  (11) **/	
	union 					/** Varianten der Event-Typen 		  (12) **/ 
	  {
	  struct				/** Noten-Infos 					  (12) **/
		{
		Byte  pitch;		/** Notenh�he 						  (12) **/
		Byte  vel;			/** Velocity    					  (13) **/
		short dur;			/** Dauer       					  (14) **/
		} note;
	  Byte		 data[4];	/** Infos anderer MidiEvents 		  (12) **/
	  unsigned short val[2];/** DoppelController etc. 			  (12) **/
	  MidiSEXPtr linkSE;	/** Pointer-Erweiterung SysEx 		  (12) **/
	  MidiSTPtr  linkST; 	/** Pointer-Erweiterung Privat Event  (12) **/
	  MidiUniPtr linkUni;	/** Pointer-Erweiterung Integr. Note  (12) **/
	  } info;
	} TMidiEv;


/**************************************************************************/
/** Sequenz-Kopf **/
/**************************************************************************/
typedef struct TMidiSeq *MidiSeqPtr;
typedef struct TMidiSeq 
	{
	MidiEvPtr	first;		/** Erstes Event der Sequenz  (0)  **/
	MidiEvPtr	last;		/** Letztes Event der Sequenz (4)  **/
	MidiEvPtr	undef1;		/** Reserviert 				  (8)  **/
	MidiEvPtr	undef2;		/**							  (12) **/
	} TMidiSeq;	


/**************************************************************************/
/** Midi-Filter **/
/**************************************************************************/
typedef struct TFilter *FilterPtr;
typedef struct TFilter 
	{
	Byte	port[32];		/** Port  0...255 : 256 bits **/
	Byte	evType[32];		/** Typen 0...255 : 256 bits **/
	Byte	channel[2];		/** Kanal 0...15  :  16 bits **/
	Byte	unused[2];
	} TFilter;


/**************************************************************************/
/** Name eine MidiShare Applikation **/
/**************************************************************************/
typedef Byte	MidiName[32];


/*------------------------ Synchronisation informations ---------------*/

typedef struct TSyncInfo *SyncInfoPtr;
typedef struct TSyncInfo
{
 	long		time;
 	long		reenter;
 	unsigned short	syncMode;
 	Byte		syncLocked; 
 	Byte		syncPort;
	long		syncStart;
	long		syncStop;
	long		syncOffset;
	long		syncSpeed;
	long		syncBreaks;
	short		syncFormat;
} TSyncInfo; 

typedef struct TSmpteLocation *SmpteLocPtr;
typedef struct TSmpteLocation
{
 	short		format;	/* (0:24f/s, 1:25f/s, 2:30DFf/s, 3:30f/s) */
 	short		hours;	/* 0..23							*/
 	short		minutes;	/* 0..59							*/
 	short		seconds;	/* 0..59							*/
 	short		frames;	/* 0..30 (according to format)		*/
 	short		fracs;	/* 0..99 (1/100 of frames)			*/
} TSmpteLocation; 

/**************************************************************************/
/** Strukturen f�r die statistischen Zust�nde **/
/**************************************************************************/
typedef struct MidiStat *MidiStatPtr;
typedef struct	MidiStat 
	{
	long rcvErrs;				/** Anzahl der Receive-Errors     **/
	long allocErrs;				/** Anzahl der Allokations-Errors **/
	long rcvEvs;				/** Anzahl der empfangenen Events **/
	long xmtEvs;				/** Anzahl der gesendeten Events  **/
	} MidiStat;


/**************************************************************************/
/** Makros f�r die Feldzugriffe **/
/**************************************************************************/

#define Link(e)		( (e)->link )
#define Date(e)		( (e)->date )
#define EvType(e)	( (e)->evType )
#define RefNum(e)	( (e)->refNum )
#define Port(e)		( (e)->port )
#define Canal(e)	( (e)->chan )
#define Chan(e)		( (e)->chan )
#define Pitch(e)	( (e)->info.note.pitch )
#define Vel(e)		( (e)->info.note.vel )
#define Dur(e)		( (e)->info.note.dur )
#define Quan(e)		( (e)->info.note.dur )
#define Data(e)		( (e)->info.data )
#define LinkSE(e)	( (e)->info.linkSE )		/** SysEx 			**/
#define LinkST(e)	( (e)->info.linkST )		/** Privat 			**/
#define LinkSK(e)	( (e)->info.linkSK )		/** Integrierte Note **/
#define Val1(e)		( (e)->info.val[0] )
#define Val2(e)		( (e)->info.val[1] )
#define Data1(e)	( (e)->info.data[0] )
#define Data2(e)	( (e)->info.data[1] )
#define Data3(e)	( (e)->info.data[2] )
#define Data4(e)	( (e)->info.data[3] )

/** Sequenz **/
#define First(s)	( (s)->first )				/** Erstes Event 	**/
#define Last(s)		( (s)->last )				/** Letztes Event 	**/
#define Third(s)	( (s)->undef1 )				/** Reserviert 		**/
#define Fourth(s)	( (s)->undef2 )				/** Reserviert 		**/

/** Privat-Event **/
#define Priv_ptr1(e)	( (e)->info.linkST->ptr1 )
#define Priv_ptr2(e)	( (e)->info.linkST->ptr2 )
#define Priv_ptr3(e)	( (e)->info.linkST->ptr3 )
#define Priv_ptr4(e)	( (e)->info.linkST->ptr4 )


/**************************************************************************/
/** Makros f�r die Filterzugriffe **/
/**************************************************************************/

#define AcceptBit(a,n)		(((Byte *)(a))[(n)>>3] |= (1<<((n)&7)))
#define RejectBit(a,n)		(((Byte *)(a))[(n)>>3] &= ~(1<<((n)&7)))
#define InvertBit(a,n)		(((Byte *)(a))[(n)>>3] ^= (1<<((n)&7)))
#define IsAcceptedBit(a,n)	(((Byte *)(a))[(n)>>3] & (1<<((n)&7)))


/**************************************************************************/
/** Sonstige Typdefinitionen **/
/**************************************************************************/

typedef void ( *TaskPtr)( /* long date, short refNum, long a1, long a2, long a3 */ );
typedef void ( *ApplyProcPtr)( /* MidiEvPtr e */ );
typedef void ( *RcvAlarmPtr)( /* short refNum */ );
typedef void ( *ApplAlarmPtr)( /* short refNum, long code */ );


/**************************************************************************/
/** Definition aller Einstiegs-Punkte von MidiShare
	Der Aufruf der Funktionen ist gleich wie bei sonstigen System-Aufrufen
	( gemdos, bios, xbios ): Die Parameter werden von rechts nach links 
	auf den Stack gelegt. Zuletzt die Nummer der Funktion.
	die Routine deren Assembler Code in der Tabelle "_trapCode" aufgereiht
	ist wrid aufgerufen. Nun f�hrt das Programm einen Sprung an die Stelle 
	aus an die der Vektor $98 zeigt. Dieser Vektor von trap #6 (Ankerpunkt
	von MidiShare) zeigt auf die Dispatch-Routine die den effektiven 
	Aufruf der Funktion realisiert. 
 	Der Vektor $98 wird beim laden von MIDISHARE.PRG initialisiert.
	

	Vorsicht:
	Das Programm mu� bei Turbo C ohne die Option "ANSI keywords only"
	compiliert werden, da die Funktion "micro_rtx" eine cdecl Funktion
	ist, deshalb der Compiler alle Parameter �ber den Stack �bergeben muss.
	Das Programm muss ausserdem mit der Library libmidi.lib gelinkt werden.
**/
/**************************************************************************/

extern int _trapCode_[];
#ifdef __PUREC__ /* BD */
	extern int _trapCode_[];
	extern unsigned long cdecl (* micro_rtx)(int number, ...);
#else
	extern unsigned long micro_rtx();
#endif

/**************************************************************************/
/** Zugriff auf die MidiShare-Umgebung **/
/**************************************************************************/

#define MidiGetVersion() 	(short)( *micro_rtx)(0)
#define MidiCountAppls() 	(short)( *micro_rtx)(1)

/* MidiGetIndAppl( short index) => short */
#define MidiGetIndAppl( a) 	(short)( *micro_rtx)(2,a)	

/* MidiGetNamedAppl( MidiName name) => short */
#define MidiGetNamedAppl( a)	(short)( *micro_rtx)(3,a)	 

 
/**************************************************************************/
/** SMPTE synchronization **/
/**************************************************************************/

/* void MidiGetSyncInfo(SyncInfoPtr p) */
#define MidiGetSyncInfo(p)			(void)micro_rtx(0x39,p)

/* void MidiSetSyncMode(unsigned short mode) */
#define MidiSetSyncMode(mode)			(void)micro_rtx(0x3A,mode)

/* MidiGetExtTime(void) => long */
#define MidiGetExtTime()				(long)micro_rtx(0x3E)

/* MidiInt2ExtTime(long) => long */
#define MidiInt2ExtTime(time)			(long)micro_rtx(0x3F,time)

/* MidiExt2IntTime(long) => long */
#define MidiExt2IntTime(time)			(long)micro_rtx(0x40,time)

/* void MidiTime2Smpte(long time, short format, SmpteLocPtr loc) */
#define MidiTime2Smpte(time, format, loc)	(void)micro_rtx(0x41,time,format,loc)

/* MidiSmpte2Time(SmpteLocPtr loc) => long */
#define MidiSmpte2Time(loc)			(long)micro_rtx(0x42,loc)



/**************************************************************************/
/** MidiShare f�r Anwendung �ffnen/schliessen **/
/**************************************************************************/

/* MidiOpen( MidiName applName) => short */
#define MidiOpen( a) 		(short)( *micro_rtx)(4,a)	 

/* MidiClose( short refNum) */
#define MidiClose( a)	 	(void)( *micro_rtx)(5,a) 


/*---------------------Configuration de l'application---------------------*/

/* MidiGetName( short refNum) => MidiName */
#define MidiGetName(a) 		(char *)( *micro_rtx)(6,a) 

/* MidiSetName( short refNum, MidiName applName) */
#define MidiSetName(a,b) 	(void)( *micro_rtx)(7,a,b) 

/* MidiGetInfo( short refNum) => Ptr */
#define MidiGetInfo(a) 		(Ptr)( *micro_rtx)(8,a) 

/* MidiSetInfo( short refNum, Ptr infoZone) */
#define MidiSetInfo(a,b) 	(void)( *micro_rtx)(9,a,b) 

/* MidiGetFilter( short refNum) => FilterPtr */
#define MidiGetFilter(a) 	(FilterPtr)( *micro_rtx)(0xA,a)	

/* MidiSetFilter( short refNum, FilterPtr filter) */
#define MidiSetFilter(a,b) 	(void)( *micro_rtx)(0xB,a,b) 

/* MidiGetRcvAlarm( short refNum) => RcvAlarmPtr */
#define MidiGetRcvAlarm(a) 	(RcvAlarmPtr)( *micro_rtx)(0xC,a)	

/* MidiSetRcvAlarm( short refNum, RcvAlarmPtr alarm) */
#define MidiSetRcvAlarm(a,b) 	(void)( *micro_rtx)(0xD,a,b)	

/* MidiGetApplAlarm( short refNum) => ApplAlarmPtr */
#define MidiGetApplAlarm(a) 	(ApplAlarmPtr)( *micro_rtx)(0xE,a)	

/* MidiSetApplAlarm( short refNum, ApplAlarmPtr alarm) */
#define MidiSetApplAlarm(a,b)	(void)( *micro_rtx)(0xF,a,b)	


/*---------------------Connexions internes--------------------------------*/

/* MidiConnect( shotr src, short dest, Boolean state) */
#define MidiConnect(a,b,c) 	(void)( *micro_rtx)(0x10,a,b,c)

/* MidiIsConnected( short src, short dest) => Boolean */
#define MidiIsConnected(a,b) 	(Boolean)( *micro_rtx)(0x11,a,b)	


/*---------------------Gestion des ports midi-----------------------------*/

/* MidiGetPortState( short port) => Boolean */
#define MidiGetPortState(a) 	(Boolean)( *micro_rtx)(0x12,a)	

/* MidiSetPortState( short port, Boolean state) */
#define MidiSetPortState(a,b)	(void)( *micro_rtx)(0x13,a,b)


/*---------------------Gestion des �v�nements-----------------------------*/

#define MidiFreeSpace() 		(long)( *micro_rtx)(0x14)

/* MidiNewCell() => MidiEvPtr */
#define MidiNewCell()		(MidiEvPtr)( *micro_rtx)(0x33)

/* MidiFreeCell( MidiEvPtr ev) */
#define MidiFreeCell(a)		(void)( *micro_rtx)(0x34,a)

/* MidiNewEv( short typeNum) => MidiEvPtr */
#define MidiNewEv(a) 		(MidiEvPtr)( *micro_rtx)(0x15,a)	

/* MidiCopyEv( MidiEvPtr ev) => MidiEvPtr */
#define MidiCopyEv(a) 		(MidiEvPtr)( *micro_rtx)(0x16,a)	

/* MidiFreeEv( MidiEvPtr ev) */
#define MidiFreeEv(a) 		(void)( *micro_rtx)(0x17,a)	

/* MidiSetField( MidiEvPtr ev, short f, long v) */
#define MidiSetField(a,b,c) 	(void)( *micro_rtx)(0x18,a,b,c)

/* MidiGetField( MidiEvPtr ev, short f) => long */
#define MidiGetField(a,b)	(long)( *micro_rtx)(0x19,a,b)

/* MidiAddField( MidiEvPtr ev, long v) */
#define MidiAddField(a,b) 	(void)( *micro_rtx)(0x1A,a,b)

/* MidiCountFields( MidiEvPtr ev) => short */
#define MidiCountFields(a) 	(short)( *micro_rtx)(0x1B,a)	


/*---------------------Gestion des s�quences------------------------------*/

/* MidiNewSeq() => MidiSeqPtr */
#define MidiNewSeq() 		(MidiSeqPtr)( *micro_rtx)(0x1D)	

/* MidiAddSeq( MidiSeqPtr s, MidiEvPtr ev) */
#define MidiAddSeq(a,b) 		(void)( *micro_rtx)(0x1E,a,b)

/* MidiFreeSeq( MidiSeqPtr s) */
#define MidiFreeSeq(a) 		(void)( *micro_rtx)(0x1F,a)	

/* MidiClearSeq( MidiSeqPtr s) */
#define MidiClearSeq(a) 		(void)( *micro_rtx)(0x20,a)	

/* MidiApplySeq( MidiSeqPtr s, ApplyProcPtr ProcPtr) */
#define MidiApplySeq(a,b) 	(void)( *micro_rtx)(0x21,a,b)



/*---------------------Date courante--------------------------------------*/

#define MidiGetTime()	 	(long)( *micro_rtx)(0x22)



/*---------------------Emissions Midi-------------------------------------*/

/* MidiSendIm( short refNum, MidiEvPtr ev) */
#define MidiSendIm(a,b)	 			(void)( *micro_rtx)(0x23,a,b)

/* MidiSend( short refNum, MidiEvPtr ev) */
#define MidiSend(a,b) 				(void)( *micro_rtx)(0x24,a,b)

/* MidiSendAt( short refNum, MidiEvPtr ev, long d) */
#define MidiSendAt(a,b,c) 			(void)( *micro_rtx)(0x25,a,b,c)


/*---------------------Receptions Midi------------------------------------*/

/* MidiCountEvs( short refNum) =>long */
#define MidiCountEvs(a)	 			(long)( *micro_rtx)(0x26,a)	

/* MidiGetEv( short refNum) => MidiEvPtr */
#define MidiGetEv(a) 				(MidiEvPtr)( *micro_rtx)(0x27,a)	

/* MidiAvailEv( short refNum) => MidiEvPtr */
#define MidiAvailEv(a) 				(MidiEvPtr)( *micro_rtx)(0x28,a)	

/* MidiFlushEvs( short refNum) */
#define MidiFlushEvs(a)				(void)( *micro_rtx)(0x29,a)	


/*---------------------Boites aux lettres---------------------------------*/

/* MidiReadSync( Ptr adrMem) => Ptr */
#define MidiReadSync(a)	 			(Ptr)( *micro_rtx)(0x2A,a)	

/* MidiWriteSync( Ptr adrMem, Ptr val) => Ptr */
#define MidiWriteSync(a,b) 			(Ptr)( *micro_rtx)(0x2B,a,b)


/*---------------------Lancement des taches-------------------------------*/

/* MidiCall( TaskPtr proc, long date, short refNum, long a1, long a2, long a3) */
#define MidiCall(a,b,c,d,e,f)		(void)( *micro_rtx)(0x2C,a,b,c,d,e,f)

/* extensions taches diff�r�es */

/* MidiTask( TaskPtr proc, long date, short refNum, long a1, long a2, long a3) => MidiEvPtr */
#define MidiTask(a,b,c,d,e,f)		(MidiEvPtr)( *micro_rtx)(0x2D,a,b,c,d,e,f)

/* MidiDTask( TaskPtr proc, long date, short refNum, long a1, long a2, long a3) => MidiEvPtr */
#define MidiDTask(a,b,c,d,e,f)		(MidiEvPtr)( *micro_rtx)(0x2E,a,b,c,d,e,f)

/* MidiForgetTask( MidiEvPtr *ev) */
#define MidiForgetTask(a)			(void)( *micro_rtx)(0x2F,a)

/* MidiCountDTasks( short refNum) => long */
#define MidiCountDTasks(a)			(long)( *micro_rtx)(0x30,a)

/* MidiFlushDTasks( short refNum) */
#define MidiFlushDTasks(a)			(void)( *micro_rtx)(0x31,a)

/* MidiExec1DTask( short refNum) */
#define MidiExec1DTask(a)			(void)( *micro_rtx)(0x32,a)

/* MidiTotalSpace() => long */
#define MidiTotalSpace()			(long)( *micro_rtx)(0x35)

/* MidiGetStatPtr() => MidiStatPtr */
#define MidiGetStatPtr()			(MidiStatPtr)( *micro_rtx)(0x36)

/* MidiGrowSpace( long space) => long */
#define MidiGrowSpace(a)			(long)( *micro_rtx)(0x37, a)
/* ATTENTION : MidiGrowSpace ne peut etre appel� que par un 
   accessoire de bureau ! */

/*---------------------Controle MidiShare---------------------------------*/
extern Boolean MidiShare(void);	/* BD: void eingesetzt f�r PureC */

#endif

/********************************* FIN ************************************/
