/*****************************************************************************/
/*                                                                           */
/* Modul: REALTSPC.H                                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- GetPText als Funktion, 04.03.95
- SetPLongN eingebaut, 19.02.95
- MSG_RTM_POSIT eingebaut, 19.02.95
- Sinq und Cosq eingebaut, 19.02.95
- Deg, Rad und Sign eingebaut, 19.02.95
- SetPWordN eingebaut, 15.01.95
- SetPWord2 und SetPWord3 eingebaut, 09.01.95
- ClickTimeField, UpdateTimeFn und UpdateField eingebaut, 08.01.95
- dlogin eingebaut
- SetPText und GetPText als Makros, 4.1.95
- mkstate eingebaut, 4.1.95
- Get/SetCheck eingebaut, 4.1.95
- GetPxxx mit optionalem drittem Parameter, 03.01.95
- ClickValueField, UpdateFieldFn und UpdateField eingebaut
- MAXPERCENT eingebaut, 03.01.95
- SetPxxx und GetPxxx eingebaut, 23.12.94
- SOUNDOBJ auf Multichannel umgestellt, 23.12.94
- POLYGON und EDGE eingebaut, 21.12.94
28.11.94
- soundobj aus ED4 eingebaut
15.01.94
- PUFEVP hier eingebaut, wg. MSH und BIG öbertragung
12.11.93
- Module() etc. ausgebaut -> objects.h
- MAXLFOS nun global
- Volume in KOOR_SINGLE auf UBYTE umgestellt
- TRACK_SINGLE, VOL_SINGLE eingefÅhrt
- KOOR_SINGLE und KOOR_ALL Elemente umbenannt
- update_ptext definition modifiziert
20.06.93
- copy_icon modifiziert
- ED_P Definition in OBJECTS.H verlegt
- update_ptext und update_checkbox eingebaut
*****************************************************************************/

#ifndef __REALTSPC__
#define __REALTSPC__

/****** DEFINES **************************************************************/
#define MAXSIGNALS 9		/* Verwaltete Signale= FÅhrungspunkt(0) + 8 Signale */
#define MAXKOOR 63		/* Hîchster/niedrigster Wert einer Koordinate */
#define MAXPERCENT 100	/* Hîchster/niedrigster Wert fÅr Prozentwerte */
#define MAXINPUTS 32		/* Anzahl der Inputs pro CM-System */
#define MAXLFOS	 12	/* Anzahl der LFO */
#define MAXVERTEX 64		/* Max. Zahl der Eckpunkte fÅr Polygone */
#define MAXEDGE 128		/* Max. Zahl der Linien fÅr Polygone */

#define QUANT 125			/* Aufruf alle xxx Millisekunden fÅr Puffersequencer */

#define WAVES 6			/* Anzahl der Wellenformen fÅr LFO und MTR */
#define WAVESTEPS 360	/* Anzahl der Schritte pro Wellenform */
#define WAVEAMPL 16000L	/* Amplitude fÅr Wellenformen */
enum WAVENAMES {
	WSINUS,
	WTRIANGLE,
	WSAWUP,
	WSAWDOWN,
	WSQUARE,
	WKANTE
};
/* Scancodes 10-er Tastatur */

#define FW					0x64
#define BW					0x63
#define CYCLEONOFF		0x65 /* Zahlenblock '/' */
#define PLUS				0x4e
#define MINUS				0x4a
#define LOC0				0x6d
#define LOC1				0x6e
#define LOC2				0x6f
#define LOC3				0x6a
#define LOC4				0x6b
#define LOC5				0x6c
#define LOC6				0x67
#define LOC7				0x68
#define LOC8				0x69
#define ZERO				0x70
#define ENTER				0x72
#define REC					0x66

/* Scancodes normale Tastatur fÅr MROS*/
#define MASTERONOFF		0x32 /* 'm' */
#define CLICKONOFF		0x2e /* 'c' */

/* macros for the message pipe to PC_HELP.ACC */
#define AC_HELP     1025            /* message to DA         */
#define AC_VERSION  1027            /* Ask DA for version    */
#define AC_COPY		1028            /* Ask DA to copy screen */
#define AC_REPLY    1026            /* message from DA       */
#define AC_NAME     "PC_HELP "      /* DA name               */

/* Message-Types */
enum {
		SET_VAR,				/* Systemvariable auf neuen Wert setzen */
		GET_VAR,				/* Wert einer Systemvariablen abfragen */
		GET_VARNAME,		/* Name einer Systemvariablen holen */
		SET_VARNAME,		/* Name einer Systemvariablen geben */
		ADD_VAR_RCV,		/* Modulfunktion in VAR-Updateliste einfÅgen */
		DEL_VAR_RCV,		/* Modulfunktion aus VAR-Updateliste herausnehmen */
		RTM_POSIT,
		RTM_STOP,
		RTM_CONT,
};

typedef struct{
	LONG		posit;		/* Das ist die Position */
}	MSG_RTM_POSIT;			/* Message fÅr Start/Cont/Stop */

/****** MACROS ****************************************************************/
#define SET_NR(a)\
	undo_state (window->object, window->exit_obj, SELECTED);\
	set_setnr(a);\
	set_edit();\
	set_dbox();\
	draw_object(window, ROOT);

#define SET_STORE(a, title)\
	undo_state (window->object, window->exit_obj, SELECTED);\
	dzahl(&a, window->work.x + window->object[window->exit_obj].ob_x, window->work.y + window->object[window->exit_obj].ob_y, title);\
	get_dbox();\
	get_edit();\
	get_setnr(a);\
	set_setnr(a);\
	set_edit();\
	set_dbox();\
	draw_object(window, ROOT);

#define SET_RECALL(a, title)\
	undo_state (window->object, window->exit_obj, SELECTED);\
	dzahl(&a, window->work.x + window->object[window->exit_obj].ob_x, window->work.y + window->object[window->exit_obj].ob_y, title);\
	set_setnr(a);\
	set_edit();\
	set_dbox();\
	draw_object(window, ROOT);


#define SET_OK\
	undo_state (window->object, window->exit_obj, SELECTED);\
	get_dbox();\
	get_edit();\
	/* get_setnr(edited->number); */ \
	/* set_setnr(edited->number); */ \
	set_edit();\
	set_dbox();\
	draw_object(window, ROOT);

#define SET_CANCEL\
	undo_state (window->object, window->exit_obj, SELECTED);\
	set_edit();\
	set_dbox();\
	/* draw_object(window, ROOT); nicht nîtig, wird von WINDOWS Åbernommen */

#define SET_STANDARD\
	undo_state (window->object, window->exit_obj, SELECTED);\
	set_setnr(0);\
	set_edit();\
	set_dbox();\
	draw_object(window, ROOT);

#define SetPoint(p,px,py,pz)\
	p.x = px;\
	p.y = py;\
	p.z = pz;

/* Winkel aus Bogenmaû berechen */
#define Deg(rad) 180*rad/M_PI

/* Bogenmaû aus Winkel berechen */
#define Rad(deg) M_PI*deg/180

/* Sign = Vorzeichenwert: 0, wenn x = 0, 1 wenn x > 0, -1, wenn x < 0 */
#define Sign(x) x > 0 ? 1 : x < 0 ? -1 : 0

/* Sinus und Cosinus Quick-Funktionen fÅr Winkel > -720 */
#define Sinq(deg) sinq[(deg+720)%360]
#define Cosq(deg) cosq[(deg+720)%360]

/****** TYPES ****************************************************************/

/*
	Datenstrukturen im Zusammenhang

 	PUFEVENT		PUF_INF												
	event ---->	koors    .  KOOR_ALL		koor[]  	.  KOOR_SINGLE	vol, koor	.  POINT_3D x,y,z
	next			tracks	. 	TRACK_ALL	track[] 		TRACK_SINGLE (WORD)
	prev			volumes	.	VOL_ALL		volume[]		VOL_SINGLE	(WORD)

*/

typedef struct point_3d
{
	BYTE 	x,y,z;		/* Koordinate eines Punktes im 3D-System */
} POINT_3D;

typedef struct koor_single
{
	POINT_3D	koor;			/* Koordinaten des Signals */
	UBYTE		volume;		/* Volume-Wert des Signals */
} KOOR_SINGLE;

typedef struct koor_all
{
	KOOR_SINGLE koor [MAXSIGNALS];	/* Zeiger-Array auf eine Gruppe
										von Signal-Koordinaten */
} KOOR_ALL;

typedef WORD TRACK_SINGLE;

typedef struct track_all
{
	/* Spur, auf die das Signal geroutet wird */
	TRACK_SINGLE	track [MAXSIGNALS];
} TRACK_ALL;

typedef WORD VOL_SINGLE;

typedef struct vol_all
{
	/* Volume-Wert, des Signals */
	VOL_SINGLE	volume [MAXSIGNALS];
} VOL_ALL;

typedef struct puf_inf
{
	/* Diese Struktur wird zum Åbermitteln von Events zwischen
		PUF und anderen Modulen mit apply benutzt. Sie trÑgt die
		Informationen Åber bis zu MAXSIGNALS Spuren. */
	KOOR_ALL		*koors;				/* Zeiger auf die Koordinaten-Struktur */
	TRACK_ALL	*tracks;				/* Zeiger auf die Spur-Zuweisung */
	VOL_ALL		*volumes;			/* Zeiger auf General-Volume aus BIG */
} PUF_INF;

typedef struct pufevent *PUFEVP;

typedef struct pufevent
{
	/* Dieser Typ wird zum speichern der Daten im PUF benutzt */
	PUF_INF	event;	/* Koordinaten, Tracks, General-Vol fÅr alle Signale */
	PUFEVP next;			/* Zeiger auf nÑchste Koordinaten */
	PUFEVP prev;			/* Zeiger auf vorherige Koordinaten */
} PUFEVENT;

typedef FLOAT POS_1D;
typedef FLOAT *POS_1DP;

typedef struct pos_2d *POS_2DP;
typedef struct pos_2d {
	FLOAT x,
			y;
} POS_2D;

typedef struct pos_3d *POS_3DP;
typedef struct pos_3d {
	FLOAT x,
			y,
			z;
} POS_3D;

typedef struct edge *EDGE_P;
typedef struct edge {
	WORD	from,				/* Ausgangspunkt der Kante */
			to,				/* Endpunkt */
			line_style,		/* Striche, punktiert etc. */
			begin_style,	/* Pfeile, rund, etc. */
			end_style,		/* s.o. */
			color,			/* Farbe */
			width;			/* Breite */
} EDGE;

typedef struct polygon *POLY_P;
typedef struct polygon {
	POS_3D	vertex[MAXVERTEX];	/* Eckpunkte */
	WORD		num_vertices;			/* Anzahl Eckpunkte */
	WORD		num_numbers;			/* Anzahl zu numerierender Eckpunkte */
	EDGE		edge[MAXEDGE];			/* Linien-Angaben */
	WORD		num_edges;				/* Anzahl Linien */
} POLYGON;

typedef struct soundobj *SO_P;
typedef struct soundobj
{
	/* Dieser Typ wird zum speichern der Sound-Objekte im ED4 benutzt */
	LONG		cue_time;					/* SMPTE Zeit des Z=0 Durchgangs */
	LONG		entry_time;					/* SMPTE Zeit des Eintritts in 3D Raum */
	LONG		exit_time;					/* SMPTE Zeit des Austritts aus 3D Raum */
	BOOL		active;						/* Ausgabe an/aus */
	POS_3D	speed;						/* Geschw. des Objektes in Koor/Quant */
	WORD		input_ch[MAXSIGNALS];	/*	Channel am CM */
	WORD		volume;						/*	Channel-LautstÑrke am CM */
	WORD		ev_key;						/* Key-Event, Midi Key */
	WORD		ev_vel;						/* Key-Event, Midi Velocity */
	WORD		ev_ch;						/* Key-Event, Midi Kanal */
	WORD		ev_port;						/* Key-Event, Midi Port */
	WORD		ev_delay;					/* Key-Event, Distance */
	POS_3D	position[MAXSIGNALS];	/* Location / Offset */
	WORD		graf_mod;					/* Mono/Stereo/Quadro/Okto */
	SO_P		next;							/* Zeiger auf nÑchsten Event */
	SO_P		prev;							/* Zeiger auf vorigen Event */
	VOID		set_speed (SO_P so, FLOAT speed);
} SOUNDOBJ;	/* EnthÑlt alle Parameter eines ED4-Sound Objektes */

typedef VOID (UpdateFieldFn) (struct window* window, WORD obj, LONG value);

/**********************************************************************/

/****** VARIABLES ************************************************************/
GLOBAL LONG wave [WAVESTEPS][WAVES];
GLOBAL FLOAT	sinq[360];
GLOBAL FLOAT	cosq[360];

/****** FUNCTIONS ************************************************************/

GLOBAL BOOL GetCheck (OBJECT *tree, WORD obj, BOOL *val);
GLOBAL WORD GetPWord (OBJECT *tree, WORD obj, WORD *val);
GLOBAL LONG GetPTime (OBJECT *tree, WORD obj, LONG *val);
GLOBAL LONG GetPLong (OBJECT *tree, WORD obj, LONG *val);
GLOBAL FLOAT GetPFloat (OBJECT *tree, WORD obj, FLOAT *val);
GLOBAL CHAR *GetPText (OBJECT *tree, WORD obj, CHAR *val);

GLOBAL VOID SetCheck (OBJECT *tree, WORD obj, CONST BOOL val);
GLOBAL VOID SetPWord (OBJECT *tree, WORD obj, CONST WORD value);
GLOBAL VOID SetPWordN (OBJECT *tree, WORD obj, CONST WORD value);
GLOBAL VOID SetPWord2 (OBJECT *tree, WORD obj, CONST WORD value);
GLOBAL VOID SetPWord2N (OBJECT *tree, WORD obj, CONST WORD value);
GLOBAL VOID SetPWord3 (OBJECT *tree, WORD obj, CONST WORD value);
GLOBAL VOID SetPWord3N (OBJECT *tree, WORD obj, CONST WORD value);
GLOBAL VOID SetPLong (OBJECT *tree, WORD obj, CONST LONG value);
GLOBAL VOID SetPLongN (OBJECT *tree, WORD obj, CONST LONG value);
GLOBAL VOID SetPTime (OBJECT *tree, WORD obj, CONST LONG value);
GLOBAL VOID SetPFloat (OBJECT *tree, WORD obj, CONST FLOAT value);
#define SetPText(tree, obj, text) set_ptext(tree, obj, text)

GLOBAL VOID SetVertex (POS_3DP v, FLOAT x, FLOAT y, FLOAT z);
GLOBAL VOID AddVertex (POLY_P poly, FLOAT x, FLOAT y, FLOAT z);
GLOBAL VOID SetEdge (EDGE_P e, WORD from, WORD to, WORD style, WORD begin, WORD end, WORD color, WORD width);
GLOBAL VOID AddEdge (POLY_P poly, WORD from, WORD to, WORD style, WORD begin, WORD end, WORD color, WORD width);

GLOBAL VOID daktstatus 		_((STRING title, STRING text));
GLOBAL VOID close_daktstat _((VOID));
GLOBAL VOID set_daktstat	_((WORD percent));

GLOBAL VOID	dzahl 			_((LONG *number, INT posx, INT posy, STRING title));
GLOBAL VOID	dzeit	 			_((LONG *time, INT posx, INT posy, STRING title));
GLOBAL VOID dsave 			_((STR128 filename, struct rtmclass *module));

GLOBAL BOOL dlogin (VOID);

GLOBAL VOID mkstate( MKINFO *mk);

/* Verwaltung von Zahlen-Feldern */
GLOBAL LONG ClickValueField (struct window* window, WORD object, MKINFO *mk, LONG min, LONG max, UpdateFieldFn update);
GLOBAL UpdateFieldFn UpdateValueField;

/* Verwaltung von Zeit-Feldern */
GLOBAL LONG ClickTimeField (struct window* window, WORD object, MKINFO *mk, LONG min, LONG max, UpdateFieldFn update);
GLOBAL UpdateFieldFn UpdateTimeField;

/* Verwaltung von Setup-Nummer-Feldern */
GLOBAL LONG ClickSetupField (struct window* window, WORD object, MKINFO *mk);

GLOBAL VOID swap_int 		_((INT *i1, INT *i2));
GLOBAL VOID swap_word 		_((WORD *w1, WORD *w2));
GLOBAL VOID swap_byte 		_((BYTE *w1, BYTE *w2));
GLOBAL LONG	minmaxsetup 	_((LONG *value, LONG maxvalue));
GLOBAL VOID	copy_icon 		_((OBJECT *dobj, OBJECT *sobj));
GLOBAL VOID	getpath			_((char *s));	/* holt den ganzen Pfad incl. Laufwerksbuchstaben */
GLOBAL VOID	cutpath			_((char *s));

GLOBAL BOOLEAN help_rtm			_((char *KeyWord ));
GLOBAL VOID update_ptext		_((struct window* window, WORD object, INT newval, INT oldval, STRING format, BOOLEAN froce_draw));
GLOBAL VOID update_checkbox 	_((struct window* window, WORD object, BOOLEAN new, BOOLEAN old, BOOLEAN force_draw));

GLOBAL BOOLEAN controller_out (SHORT refnum, WORD port, WORD channel, WORD controller, WORD value);

GLOBAL VOID *mem_setx 		_((VOID *dest, WORD val, UWORD len));
GLOBAL VOID *mem_movex		_((VOID *dest, CONST VOID *src, UWORD len));
GLOBAL VOID *mem_lsetx		_((VOID *dest, WORD val, ULONG len));
GLOBAL VOID *mem_lmovex		_((VOID *dest, CONST VOID *src, ULONG len));

GLOBAL BOOLEAN init_realtspc  _((VOID));
GLOBAL BOOLEAN term_realtspc  _((VOID));

#endif /* __REALTSPC__ */
