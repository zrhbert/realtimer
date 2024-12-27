/*****************************************************************************/
/*                                                                           */
/* Modul: DISPOBJ.H                                                          */
/* Datum: 20.02.95                                                           */
/*                                                                           */
/* Header-File for Display-Objects                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- var und text fÅr Bar, 20.02.95
- ObjectPush eingebaut, 17.02.95
- DOUniBarOrientation eingebaut, 15.01.95
27.12.94
- set_uni auf LONG umgestellt
- MAXUNI eingebaut
- shared_persp in spacestatus eingebaut
- volume in spacestatus eingebaut
- POS_1/2/3D in realtspc.h ausgelagert
- lookup arrays eingebaut
14.07.94
- CrtCMOSpaceDOInsert um input_base erweitert
- CreateCMOSpaceDispobj eingebaut
16.06.94
- CreatexxxDOInsert eingebaut
12.02.94
- CreateDispObj mit area Parameter
- CreateTextDispobj eingebaut
10.01.94
- set_area eingebaut
*****************************************************************************/

#ifndef __DISPOBJ__
#define __DISPOBJ__

/****** DEFINES **************************************************************/
#define MAXPOS 10		/* Max. Number of Types, Vars, Ranges, Indices */
#define MAXUNI 20		/* Max. Number of Universal Parameter */
#define MAXDOKOOR 100 /* Max. des Koordinatenbereiches fÅr DO's */

/* AbkÅrzung fÅr Setzen von VAR-Nummer und Text in einem Aufruf */
#define SetVarTextDispobj(obj, id, var, text)\
 obj->set_var(obj, id, var);\
 obj->set_text(obj, id, text)

#define CrtObjectDOInsert(window, object, var, type, key)\
	list_insert(window->dispobjs, list_new_el (CreateObjectDispobj (window, object, var, type, key)));

#define CrtTextDOInsert(window, type, mode, area, var, text)\
	list_insert(window->dispobjs, list_new_el (CreateTextDispobj (window, type, mode, area, var, text)));

#define 	CrtBarDOInsert(window, type, mode, area, var, text) \
	list_insert(window->dispobjs, list_new_el (CreateBarDispobj (window, type, mode, area, var, text)));

#define 	CrtSpaceDOInsert(window, type, mode, area) \
	list_insert(window->dispobjs, list_new_el (CreateSpaceDispobj (window, type, mode, area)));

#define 	CrtCMOSpaceDOInsert(window, type, mode, area, input_base) \
	list_insert(window->dispobjs, list_new_el (CreateCMOSpaceDispobj (window, type, mode, area, input_base)));

#define 	CrtED4SpaceDOInsert(window, type, mode, area) \
	list_insert(window->dispobjs, list_new_el (CreateED4SpaceDispobj (window, type, mode, area)));

#define 	CopyCMOSpaceDOInsert(window, dispobj) \
	list_insert(window->dispobjs, list_new_el (dispobj->copy (dispobj)));

#define 	CrtSODOInsert(window, type, mode, area) \
	list_insert(window->dispobjs, list_new_el (CreateSODispobj (window, type, mode, area)));

enum DispObj 
{
	DOText,
	DOObject,
	DOBar,
	DOArea,
	DOSpace
};

enum DOParSize
{
	DOSizeWidth,
	DOSizeHeight,
	DOSizeFont0,
	DOSizeFont1,
	NumDOParSize
};

enum DOParType
{
	DOType0,
	DOType1,
	NumDOParType
};

enum DOParUni
{
	DOUniRotationX,
	DOUniRotationY,
	DOUniRotationZ,
	NumDOParUni
};

enum TypeText
{
	TextNormal,
	TextCMI
};

enum TypeObject
{
	ObjectValue,
	ObjectTime,
	ObjectSelect,
	ObjectCheck,
	ObjectPush
};

enum DispObjParTypeSpace
{
	DOTypeSpaceForm = NumDOParType,
	DOTypeSpaceMode
};

enum DispObjParUniSO
{
	DOUniSOEvent = NumDOParUni
/*
	DOUniSOCueTime = NumDOParUni,
	DOUniSOEntryTime,
	DOUniSOExitTime,
	DOUniSOActive,
	DOUniSOSpeed,
	DOUniSOVolume,
	DOUniSOEvKey,
	DOUniSOEvCh,
	DOUniSOEvPort,
	DOUniSOEvDelay,
	DOUniSOPosX,
	DOUniSOPosY,
	DOUniSOPosZ
*/
};

enum DispObjParUniSpace
{
	DOUniSpaceArrows = NumDOParUni,
	DOUniSpaceNumbers,
	DOUniSpaceInside,
	DOUniSpaceCrosshair,
	DOUniSpaceDisplay,
	DOUniSpaceVolume,
	DOUniSpacePerspective,
	DOUniSpaceInputBase
};

enum mode 
{
	MONO,
	STEREO,
	QUADRO,
	OKTO
};

enum raumform {
	TETRAEDER,
	SECHSKANAL,
	OKTAEDER,
	WUERFEL,
	WUERFELLANG,
	WUERFELHOCH,
	WUERFELMITTE,
	WUERFELDOPP,
	QUADROPHON,
	KREISFORM,
	NumSpaceForms
};

/* Modi fÅr Space */
#define SpaceModeSharedPersp 0x0001
/****** TYPES ****************************************************************/
typedef struct dispobj *DISPOBJP;           /* Zeiger fÅr Parameter */

typedef struct point_2d *POINT_2DP;
typedef struct point_2d {
	WORD	x,		/* Bildschirmkoordinaten */
			y;
} POINT_2D;

/* Typ fÅr Projektionsfunktionen */
typedef VOID (ProjectFn)(DISPOBJP dispobj, POS_3DP point, POINT_2DP proj);

typedef struct textstatus *TEXTSTATP;
typedef struct textstatus{
	STRING	text[MAXPOS];		/* Merker fÅr die fertigen Strings */
} TEXTSTATUS;

typedef struct objectstatus *OBJECTSTATP;
typedef struct objectstatus{
	WORD		object;	/* Nummer des Objektes im Baum */
	STRING	text;		/* Merker fÅr den fertigen String */
	CHAR		key;		/* Taste fÅr Bedienung des Objekts */
} OBJECTSTATUS;

typedef struct barstatus *BARSTATP;
typedef struct barstatus {
	POS_1D	position;
	POS_1D	pos_alt;
	TEXTSTATUS text;				/* Ein Textobjekt */
	WORD		xoffset,				/* Mittelpunkt der Anzeige */
				yoffset;
} BARSTATUS;

typedef struct areastatus *AREASTATP;
typedef struct areastatus {
	POS_2D	position;
	POS_2D	pos_alt;
	TEXTSTATUS text;				/* Ein Textobjekt */
	WORD		xoffset,				/* Mittelpunkt der Anzeige */
				yoffset;
} AREASTATUS;

typedef struct soundobjstatus *SOSTATP;
typedef struct soundobjstatus {
	LONGSTR		text;				/* Der fertige String */
	WORD			fontsize;		/* Zeichengroesse fÅr Beschriftung */
	SOUNDOBJ		event;			/* Ein Sound-Event */
} SOUNDOBJSTATUS;

typedef struct spacestatus *SPACESTATP;
typedef struct spacestatus {
	POS_3D 	position[MAXPOS];	/* Current Position, taken from VAR's */
	POS_3D 	pos_alt[MAXPOS];	/* Last positions for comparison */
	WORD		volume[MAXPOS];	/* Current Volume Settings */
	WORD		vol_alt[MAXPOS];	/* Last Volume Settings */
	WORD		crosshair[MAXPOS];/* Flags for crosshair on/off */
	WORD		display[MAXPOS];	/* Flags for display on/off */
	WORD		vol_line[MAXPOS];	/* Flags for volume display on/off */
	INT		numpoints;			/* Number of signals to be displayed */
	FLOAT		perspective;		/* Perspektive */
	WORD		mode;					/* In diesem Objekt benutzte Darstellung */
	WORD		xoffset,				/* Mittelpunkt der Anzeige */
				yoffset;
	BOOL		shared_persp;		/* Persp. Åbernehmen durch Pointer */
	WORD		*xpersp,				/* X Verzerrung durch Z */
				*ypersp,				/* Y Verzerrung durch Z */
				*xzoff,				/* X-Offset durch Z */ 
				*yzoff,				/* Y-Offset durch Z */ 
				*xwin,				/* Projektion von XZ->X im Fenster */
				*ywin;				/* Projektion von YZ->Y im Fenster */
	BOOL		kneu;					/* New Koordinates -> Update */
	BOOLEAN	inside;				/* Nur Innenraum anzeigen */
	BOOLEAN	arrows;				/* XYZ Achsen mit Pfeilen */
	BOOLEAN	numbers;				/* Numerierung an den Ecken */
	WORD		line_width;			/* Strichbreite der Anzeige */
	WORD		base;					/* Nummer des ersten 'echte' Signals (fÅr stereo, quadro, okto) */
	WORD		form;					/* Raumform fÅr dieses Fenster */
	WORD		fontsize;			/* Zeichengroesse fÅr Beschriftung */
	TEXTSTATUS text;				/* Ein Textobjekt */
	BOOL		initializing;		/* Koor updates nur explizit durchfÅhren */
	POLY_P	spaceform;			/* Zeiger auf Polygon fÅr Raumform */
	/* VOID		(*form_draw) (DISPOBJP dispobj);	/* Raumform Zeichenroutine */ */
	VOID		(*line_draw) (DISPOBJP dispobj);	/* Linien Zeichenroutine */
	VOID		(*vol_draw)  (DISPOBJP dispobj);	/* Volume Zeichenroutine */
	ProjectFn *projection;		/* Projektions-Routine */
} SPACESTATUS;

typedef union {
	TEXTSTATUS		text;
	OBJECTSTATUS	object;
	BARSTATUS		bar;
	AREASTATUS		area;
	SPACESTATUS 	space;
	SOUNDOBJSTATUS	sound;
} DISPOBJSTATUS;

typedef struct dispobj
{
	/* message ist erste Funktion, fÅr KompatibilitÑt mit dispobj */
	VOID		(* message)		(DISPOBJP module, WORD type, VOID *message);	/* Empfang einer Nachricht */
	WORD		handle;							/* Handle fÅr Objektes */
	WORD		opened;							/* Wie oft wurde Objektes geîffnet ? */
	UWORD		flags;							/* Flags des Objektes */
	UWORD		obj_type;						/* Art des Objektes */
	RECT		work;								/* Arbeitsbereich */
	WORD		bg_color;						/* Hintergrund-Farbe */
	WORD		mousenum;						/* Nummer der Mausform */
	MFORM		*mouseform;						/* Mausform, falls mousenum = 255 */
	POS_3D	rotation;						/* Rotation der Anzeige */
	INT		size[MAXPOS];					/* Universelle Grîûenmerker */
	INT		range[MAXPOS];					/* Universelle Bereichsmerker */
	INT		type[MAXPOS];					/* Universelle Typenmerker */
	CHAR		*text[MAXPOS];					/* Universelle Textmerker */
	UINT		var[MAXPOS];					/* Zu Åberwachenden VAR's*/
	LONG		var_value[MAXPOS];			/* Gespeicherter VAR-Wert */
	LONG		uni[MAXUNI];					/* Universelle Merker */
	BOOLEAN	new;								/* Neuzeichnen nîtig */
	DISPOBJSTATUS *status;          		/* Status der Anzeige */
	struct window *parent;					/* zugehîriges Fenster */
	VOID    (*start)  	(DISPOBJP dispobj);             /* Wird vor redraw aufgerufen */
	VOID    (*draw)      (DISPOBJP dispobj);             /* Zeichnen-Aktion */
	VOID    (*finished)	(DISPOBJP dispobj);             /* Wird nach redraw aufgerufen */
	VOID    (*click)     (DISPOBJP dispobj, MKINFO *mk); /* Klick-Aktion (selektieren) */
	BOOLEAN (*key)       (DISPOBJP dispobj, MKINFO *mk); /* Tastatur-Aktion */
	VOID    (*timer)     (DISPOBJP dispobj);             /* Aktion nach Ablaufen einer Zeitspanne */
	BOOLEAN (*set)			(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer); /* Setzen von Werten */
	VOID    (*reset)     (DISPOBJP dispobj);             /* Reset aller Status-Werte */
	VOID    (*text_default)(DISPOBJP dispobj);           /* Setzen der Text-Standard-Werte im VDI */
	VOID    (*moved)		(DISPOBJP dispobj);           	/* Aufruf nach Bewegung des Objektes */
	BOOLEAN (*set_work)  (DISPOBJP dispobj, RECT *work); /* Setzen der Objectgroesse */
	BOOLEAN (*set_rot)	(DISPOBJP dispobj, POS_3DP rotation); /* Setzen der Objekt-Rotation */
	BOOLEAN (*set_size)  (DISPOBJP dispobj, INT id, INT size); /* Setzen der Darstellungs-Groesse */
	BOOLEAN (*set_text)  (DISPOBJP dispobj, INT id, CHAR *text); /* Setzen der Texte */
	BOOLEAN (*set_range) (DISPOBJP dispobj, INT id, INT range); /* Setzen der Darstellungs-Bereiches */
	BOOLEAN (*set_type)  (DISPOBJP dispobj, INT id, INT type); /* Setzen des Darstellungs-Types */
	BOOLEAN (*set_var)  	(DISPOBJP dispobj, INT id, UINT var, BOOL reactive); /* Setzen der zu Åberwachenden VAR's */
	BOOLEAN (*set_uni)  	(DISPOBJP dispobj, INT id, LONG uni); /* Setzen der universellen Marker */
	VOID    (*delete)    (DISPOBJP dispobj);        /* Aktion nach dem Lîschen */
	DISPOBJP (*copy)   (DISPOBJP dispobj);        /* Kreieren durch Kopieren*/
} DISPOBJ;

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/
GLOBAL DISPOBJP CreateTextDispobj (struct window *parent, WORD type, WORD mode, RECT *area, UWORD var, STRING text);
GLOBAL DISPOBJP CreateObjectDispobj (struct window *parent, WORD object, UINT variable, WORD type, CHAR key);
GLOBAL DISPOBJP CreateBarDispobj (struct window *parent, WORD type, WORD mode, RECT *area, UWORD var, STRING text);
GLOBAL DISPOBJP CreateSpaceDispobj (struct window *parent, WORD type, WORD mode, RECT *area);
GLOBAL DISPOBJP CreateCMOSpaceDispobj (struct window *parent, WORD type, WORD mode, RECT *area, WORD input_base);
GLOBAL DISPOBJP CreateED4SpaceDispobj (struct window *parent, WORD type, WORD mode, RECT *area);
GLOBAL DISPOBJP CreateSODispobj (struct window *parent, WORD type, WORD mode, RECT *area);

GLOBAL BOOLEAN init_dispobj (VOID);
GLOBAL BOOLEAN term_dispobj (VOID);
#endif /* __DISPOBJ__ */
