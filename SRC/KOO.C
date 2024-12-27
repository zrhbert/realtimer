/*****************************************************************************/
/*                                                                           */
/* Modul: KOO.C                                                           	  */
/*                                                                           */
/* Koordinaten-Anzeige                                                       */
/*                                                                           */
/*****************************************************************************/

#define KOOVERSION "V 1.00"
#define KOODATE "20.02.95"

/*****************************************************************************
V 1.00
- Var-Darstellung auf Bar ge„ndert, 20.02.95
- MILLI auf 100 ge„ndert, 09.02.95
- SysVAR eingebaut, 03.02.95
- panpan eingebaut, 04.01.95
V 0.14
- menu_title auf MCONTROLS ge„ndert, 27.11.94
V 0.13 19.05.94
- load_create_infos und instance_count eingebaut
- Umstellung auf DISPOBJ
V 0.12
- fehlende LFO-Anzeigen eingebaut
V 0.11
- Numerierung der Channels, Ports und Tracks intern ab 0 ..
- MTR und LFO-Pos eingebaut
- zoom entfernt
- window->module eingebaut
- wi_click_mod Funktion auskommentiert
- wi_finished_mod eingebaut
- bug in Datenreduktion in set_dbox beseitigt
- module->status Initialisierung in create
- Umbau auf create_window_obj
V 0.10
- Bug in create (module->window) beseitigt
V 0.09
- Umstellung auf neue RTMCLASS-Struktur
*****************************************************************************/
#ifndef XRSC_CREATE
/*#define XRSC_CREATE 1*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"
#include "time.h"

#include "realtim4.h"
#include "koo_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"
#include "dispobj.h"

#include "export.h"
#include "koo.h"

#if XRSC_CREATE
#include "koo_mod.rsh"
#include "koo_mod.rh"
#endif

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_RESIDENT | WI_NOSCROLL)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fr Scrolling */
#define INITX  (20 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 4 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (57 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  (13 * gl_hbox)           /* Anfangsh”he in Pixel */
#define MILLI  100                    	/* Millisekunden fr Zeitablauf */

#define MOD_RSC_NAME "KOO_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 1L						/* Anzahl der KOO-Setups */
#define NUMLINES 10						/* Anzahl Zeilen in der Anzeige */

/****** TYPES ****************************************************************/
typedef struct setup *SET_P;

typedef struct koo_signal_flags
{
	/* Anzeige-Flags */
	UINT	koorx	: 1	;	/* X-Koor anzeigen  */
	UINT	koory	: 1	;	/* Y-Koor anzeigen  */
	UINT	koorz	: 1	;	/* Z-Koor anzeigen  */
	UINT	vol	: 1	;	/* Volume anzeigen  */
	UINT	panbr	: 1	;	/* Panbreite anzeigen  */
	UINT	panpos: 1	;	/* Pan-Position anzeigen  */
	UINT	voruz	: 1	;	/* Vor-und-Zurck-Wert anzeigen  */
	UINT	dehn	: 1	;	/* Dehnung anzeigen  */
	UINT	mtrspd: 1	;	/* MTR-Geschw   */
	UINT	track : 1	;	/* CMI Signalzuweisung   */
	UINT  bigvol: 1	;	/* Volume aus BIG-Sequenzer */
} SFLAGS;				/* Enth„lt alle KOO-Parameter eines einzelnen Signals */

typedef struct koo_signal_val
{
	/* Anzeige-Werte */
	WORD	koorx			;	/* X-Koor */
	WORD	koory			;	/* Y-Koor */
	WORD	koorz			;	/* Z-Koor */
	WORD	vol			;	/* Volume */
	WORD	panbr			;	/* Panbreite */
	WORD	panpos		;	/* Pan-Position  */
	WORD	voruz			;	/* Vor-und-Zurck-Wert */
	WORD	dehn			;	/* Dehnung */
	WORD	mtrspd		;	/* MTR-Geschw */
	WORD	track			;	/* CMI Signalzuweisung   */
	WORD  bigvol		;	/* Volume aus BIG-Sequenzer */
} SVALUES;				/* Enth„lt alle aktuellen Parameter eines einzelnen Signals */

typedef struct setup
{
	VOID *dummy;
} SETUP;		/* Enth„lt alle Parameter einer kompletten KOO-Einstellung */

typedef struct status *STAT_P;

typedef struct status
{
	SFLAGS	flags[MAXSIGNALS];
	SVALUES	values[MAXSIGNALS];
	BOOLEAN	new;		/* Flag fr komplett neuen Aufbau */
	LONG		var_values[MAXSETVARS];	/* Systemvariablen */
} STATUS;

/****** VARIABLES ************************************************************/
/* Resource */
PRIVATE WORD	koo_rsc_hdr;					/* Zeigerstruktur fr RSC-Datei */
PRIVATE WORD	*koo_rsc_ptr = &koo_rsc_hdr;		/* Zeigerstruktur fr RSC-Datei */
PRIVATE OBJECT *koo_setup;
PRIVATE OBJECT *koo_help;
PRIVATE OBJECT *koo_desk;
PRIVATE OBJECT *koo_text;
PRIVATE OBJECT *koo_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 20;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "KOO";		/* Name, fr Extension etc. */

/****** FUNCTIONS ************************************************************/

/* Interne KOO-Funktionen */
PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

PRIVATE VOID create_displayobs (WINDOWP window);
/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		akt = module->actual->setup;
	SET_P		ed = edited->setup;
	STRING 	s, format = "%5ld";
	
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	SET_P		akt = module->actual->setup;
	STAT_P	stat_akt = module->status;
	STAT_P	stat_alt = module->stat_alt;
	WORD		offset;
	REG SFLAGS *koo_f, *alt_f;
	REG SVALUES *koo_v, *alt_v;
	REG BOOLEAN force_draw = module->status->new;
	STRING 	s, format = "%4d";
	UWORD 	signal;
	WINDOWP	window = module->window;
	WORD		h = gl_hbox, w = gl_wbox;
	
	text_default (vdi_handle);
	/* 		    01234012340123401234012340123401234012340123401234 */
	sprintf (s, "   X    Y    Z   VOL  PBr  PPs  V+Z  Spd  Deh  CMI");
	v_text (vdi_handle, 1 * h, 2 * w, s, w);
	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		v_gtext (vdi_handle, signal, signal, "Test");
		koo_v	=&stat_akt->values[signal];
		
		sprintf (s, "%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d",
			koo_v->koorx, koo_v->koory, koo_v->koorz, koo_v->vol, 
			koo_v->panbr, koo_v->panpos, koo_v->voruz, koo_v->mtrspd, 
			koo_v->dehn, koo_v->track);
			v_text (vdi_handle, 1 + signal * h, 2 * w, s, w);
	} /* for */
/*
	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		offset=(KOO1-KOO0)*(signal);
		koo_f	=&(stat_akt->flags[signal]);
		alt_f	=&(stat_alt->flags[signal]);
		koo_v	=&(stat_akt->values[signal]);
		alt_v	=&(stat_alt->values[signal]);

		update_ptext (window, KOOX0 + offset, koo_v->koorx, alt_v->koorx, s, force_draw);
		update_ptext (window, KOOY0 + offset, koo_v->koory, alt_v->koory, s, force_draw);
		update_ptext (window, KOOZ0 + offset, koo_v->koorz, alt_v->koorz, s, force_draw);
		update_ptext (window, KOOVOL0 + offset, koo_v->vol, alt_v->vol, s, force_draw);
		update_ptext (window, KOOPBR0 + offset, koo_v->panbr, alt_v->panbr, s, force_draw);
		update_ptext (window, KOOPPOS0 + offset, koo_v->panpos, alt_v->panpos, s, force_draw);
		update_ptext (window, KOOVUZ0 + offset, koo_v->voruz, alt_v->voruz, s, force_draw);
		update_ptext (window, KOOSPD0 + offset, koo_v->mtrspd, alt_v->mtrspd, s, force_draw);
		update_ptext (window, KOODEHN0 + offset, koo_v->dehn, alt_v->dehn, s, force_draw);
		update_ptext (window, KOOCMI0 + offset, koo_v->track, alt_v->track, s, force_draw);
	} /* for */
/*
	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (koo_setup, KOOSETNR , s);
*/
*/
} /* set_dbox */

/*****************************************************************************/

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	WINDOWP	window = module->window;

	window->milli = 1; 	/* Update so schnell wie m”glich */
	return event;
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	/* Zurcksetzen von Werten */
} /* reset */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	/* Vorausberechnung */
} /* precalc */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD 	variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	STAT_P	status	= module->status;
	SVALUES	*values	= status->values;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			if (variable == VAR_SET_KOO)
				module->set_setnr(module, value);
			else if ((variable >= VAR_LFA_PANBREITE1) && (variable < VAR_LFA_PANBREITE1 + 4 ))
				/* Panbreite fr diesen Kanal setzen */
				values[variable - VAR_LFA_PANBREITE1 ].panbr = (WORD)value;
			else if ((variable >= VAR_LFA_PANPOS1) && (variable < VAR_LFA_PANPOS1 + 4 ))
				/* Pan-Position fr diesen Kanal setzen */
				values[variable - VAR_LFA_PANPOS1 ].panpos = (WORD)value;
			else if ((variable >= VAR_LFA_VORZUR0) && (variable < VAR_LFA_VORZUR0 + 9 ))
				/* Vor-und-Zurck fr diesen Kanal setzen */
				values[variable - VAR_LFA_VORZUR0 ].voruz = (WORD)value;
			else if ((variable >= VAR_LFB_PANBREITE1) && (variable < VAR_LFB_PANBREITE1 + 4 ))
				/* Panbreite fr diesen Kanal setzen */
				values[variable - VAR_LFB_PANBREITE1 ].panbr = (WORD)value;
			else if ((variable >= VAR_LFB_PANPOS1) && (variable < VAR_LFB_PANPOS1 + 4 ))
				/* Pan-Position fr diesen Kanal setzen */
				values[variable - VAR_LFB_PANPOS1 ].panpos = (WORD)value;
			else if ((variable >= VAR_LFB_VORZUR0) && (variable < VAR_LFB_VORZUR0 + 9 ))
				/* Vor-und-Zurck fr diesen Kanal setzen */
				values[variable - VAR_LFB_VORZUR0 ].voruz = (WORD)value;
			else if ((variable >= VAR_MTR_ACC0) && (variable < VAR_MTR_ACC0 + 9 ))
				/* MTR-Beschleunigung fr diesen Kanal setzen */
				values[variable - VAR_MTR_ACC0 ].mtrspd = (WORD)value;
			else if ((variable >= VAR_CMI_SIGNAL1) && (variable < VAR_CMI_SIGNAL1 + 8 ))
				/* Pan-Position fr diesen Kanal setzen */
				values[variable - VAR_CMI_SIGNAL1 ].track = (WORD)value;
			else if (variable < VAR_VAR0 + MAXSETVARS )
			/* nicht n”tig, wird ber var_get_value abgefragt 
				status->var_values[variable - VAR_VAR0] = value;
			*/
			
			if (module->window) module->window->milli = 1;
			break;
	} /* switch */
} /* message */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;

	send_variable(VAR_SET_KOO, module->actual->number);
} /* send_messages */

/*****************************************************************************/
/* ™ffne Fenster                                                             */
/*****************************************************************************/

PRIVATE VOID wi_open_mod (window)
WINDOWP window;

{
	/* box (window, TRUE); */
	clr_scroll (window);
} /* wi_open_mod */

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
/* Vor zeichnen Status ver„ndern                                             */
/*****************************************************************************/

PRIVATE VOID wi_start_mod (window)
WINDOWP window;

{	
	RTMCLASSP	module = Module(window);
	STAT_P	status = Status(window);
	DISPOBJP	dispobj;
	LIST_P	header, element;
	BOOLEAN	new = status->new || (window->flags & WI_JUNK);
		
	status->new = new;

	/* Liste der Display-Objekte durchgehen und neue Koor eintragen */

	header = window->dispobjs;
	element = list_next(header);
	while (element != header) {
		dispobj = (DISPOBJP) element->key;
		if (new) dispobj->new	= TRUE;
		element = element->next;
	} /* while */
		
} /* wi_start_mod */

/*****************************************************************************/
/* Nach zeichnen Status ver„ndern                                            */
/*****************************************************************************/

PRIVATE VOID wi_finished_mod (window)
WINDOWP window;

{	
	STAT_P	status = Status(window);

	status->new = FALSE;
} /* wi_finished_mod */

/*****************************************************************************/
/* Zeitablauf fr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (window)
WINDOWP window;
{

	redraw_window(window, &window->scroll);
	/* Gelegentlich updaten */
	window->milli = MILLI;

} /* wi_timer_mod */

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
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
	WINDOWP	window;
	WORD		menu_height;
	
	window = create_window_obj (KIND, CLASS_KOO);
	
	if (window != NULL)
	{
		WINDOW_INITWIN_OBJ
				
		window->flags     = FLAGS;
		window->icon 	   = icon;
		window->doc.x     = 0;
		window->doc.y     = 0;
		window->doc.w     = 0;
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
		window->object    = NULL;
		window->menu      = menu;
		window->open    	= wi_open_mod;
		window->click     = wi_click_mod;
		window->timer     = wi_timer_mod;
		window->showinfo  = info_mod;
		window->snap 		= wi_snap_mod;
		window->draw    	= wi_draw_mod;
		window->start 		= wi_start_mod;
		window->finished  = wi_finished_mod;
		
		/* Display-Objekte einklinken */
		create_displayobs (window);

		sprintf (window->name, (BYTE *)koo_text [FKOON].ob_spec);
		sprintf (window->info, (BYTE *)koo_text [FKOOI].ob_spec, 0);
	} /* if */
	
	return (window);                      /* Fenster zurckgeben */
} /* crt_mod */

PRIVATE VOID create_displayobs (WINDOWP window)
{	
	WORD		signal, h = gl_hbox, w = gl_wbox, x0, y0, line = 0;
	LONGSTR	s;
	RECT		a;
	
	if (desk.h > 400)
		h = 16;
	else
		h = 8;
		
	/* 	      0123401234012340123401234012340123401234012340123401234 */
	strcpy (s, "Sig   X    Y    Z   Zoom VOL  Spd  Pos  CMI");
	a.x = 0 * w;
	a.y = line++ * h;
	a.w = w * (WORD)strlen(s);
	a.h = h;
	CrtTextDOInsert (window, TextNormal, 0, &a, 0, s);

	for(signal=0; signal < MAXSIGNALS; signal++)
	{
		sprintf (s, " %d", signal);
		a.y = line++ * h;
		a.x = 0 * w;
		a.w = w * (WORD)strlen(s);
		a.h = h;
		sprintf (s, "%d", signal);
		CrtTextDOInsert (window, 0, 0, &a, 0, s);
		strcpy (s, " %4ld");
		a.x = a.x + a.w;
		a.w = w * 5;
		CrtTextDOInsert (window, 0, 0, &a, VAR_PUF_KOORX0 + signal, s);
		a.x = a.x + a.w;
		CrtTextDOInsert (window, 0, 0, &a, VAR_PUF_KOORY0 + signal, s);
		a.x = a.x + a.w;
		CrtTextDOInsert (window, 0, 0, &a, VAR_PUF_KOORZ0 + signal, s);
		a.x = a.x + a.w;
		CrtTextDOInsert (window, 0, 0, &a, VAR_LFA_ZOOM0 + signal, s);
		a.x = a.x + a.w;
		CrtTextDOInsert (window, 0, 0, &a, VAR_PUF_VOL0 + signal, s);
		a.x = a.x + a.w;
		CrtTextDOInsert (window, 0, 0, &a, VAR_MTR_ACC0 + signal, s);
		a.x = a.x + a.w;
		CrtTextDOInsert (window, 0, 0, &a, VAR_MTR0 + signal, s);
		if (signal > 0)
		{
			a.x = a.x + a.w;
			CrtTextDOInsert (window, TextCMI, 0, &a, VAR_CMI_SIGNAL1 + signal -1, s);
		} /* if */
	} /* for signal */

	strcpy (s, "        1-2  3-4  5-6  7-8");
	a.x = 0 * w;
	a.y = line++ * h;
	a.w = w * (WORD)strlen(s);
	a.h = h;
	CrtTextDOInsert (window, 0, 0, &a, 0, s);
	sprintf (s, "PanBr ");
	a.y = line++ * h;
	a.x = 0 * w;
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, 0, s);
	strcpy (s, " %4ld");
	for(signal=0; signal < 4; signal++)
	{
		a.x += a.w;
		a.w = w * 5;
		a.h = h;
		CrtTextDOInsert (window, 0, 0, &a, VAR_LFA_PANBREITE1 + signal, s);
	} /* for signal */

	sprintf (s, "PanPos");
	a.y = line++ * h;
	a.x = 0 * w;
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, 0, s);
	strcpy (s, " %4ld");
	for(signal=0; signal < 4; signal++)
	{
		a.x += a.w;
		a.w = w * 5;
		a.h = h;
		CrtTextDOInsert (window, 0, 0, &a, VAR_LFA_PANPOS1 + signal, s);
	} /* for signal */

	strcpy (s, "        1-2-3-4  5-6-7-8");
	a.x = 0 * w;
	a.y = line++ * h;
	a.w = w * (WORD)strlen(s);
	a.h = h;
	CrtTextDOInsert (window, 0, 0, &a, 0, s);
	sprintf (s, "QuadBr ");
	a.y = line++ * h;
	a.x = 0 * w;
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, 0, s);
	strcpy (s, " %4ld");
	for(signal=0; signal < 2; signal++)
	{
		a.x += a.w;
		a.w = w * 5;
		a.h = h;
		CrtTextDOInsert (window, 0, 0, &a, VAR_LFA_QUADBREITE1 + signal, s);
	} /* for signal */

	sprintf (s, "QuadPos");
	a.y = line++ * h;
	a.x = 0 * w;
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, 0, s);
	strcpy (s, " %4ld");
	for(signal=0; signal < 2; signal++)
	{
		a.x += a.w;
		a.w = w * 5;
		a.h = h;
		CrtTextDOInsert (window, 0, 0, &a, VAR_LFA_QUADPOS1 + signal, s);
	} /* for signal */

	sprintf (s, "SysVAR");
	a.y = line++ * h;
	a.x = 0 * w;
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, 0, s);
	strcpy (s, " %4ld");
	for(signal=0; signal < MAXSETVARS; signal++)
	{
		a.x += a.w;
		a.w = w * 5;
		a.h = h;
		CrtTextDOInsert (window, 0, 0, &a, VAR_VAR0 + signal, s);
	} /* for signal */
} /* create_displayobs */

/*****************************************************************************/
/* ™ffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN ok;
	WINDOWP window;
	RTMCLASSP	module;
	
	window = search_window (CLASS_KOO, SRCH_ANY, icon);
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			module = Module(window);
			module->status->new	 = TRUE;

			/*
			window->edit_obj = find_flags (koo_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			
			module->set_edit (module);
			module->set_dbox (module);
			*/
			if (! open_window (window)) hndl_alert (ERR_NOOPEN);
		} /* if */
		else top_window (window);
		window->opened = 1;
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
	WORD		ret;
	STRING	s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, IKOO);
		
	if (window == NULL)
	{
		 form_center (koo_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (koo_info, NULL, IKOO, (BYTE *)koo_text [FKOON].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = koo_info;
		sprintf(s, "%-20s", KOODATE);
		set_ptext (koo_info, KOOIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (koo_info, KOOCOMPILE, s);
		sprintf(s, "%-20s", KOOVERSION);
		set_ptext (koo_info, KOOIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (koo_info, KOOISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (koo_info, KOOIAKT, s);

		if (! open_dialog (IKOO)) hndl_alert (ERR_NOOPEN);
	}

  return (window != NULL);
} /* info_mod */

/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	WINDOWP		window;
	RTMCLASSP 	module;
	STRING		s;
	FILE			*fp;
	WORD			x;
	
	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_KOO;
		module->icon				= &koo_desk[KOOICON];
		module->icon_position	= IKOO;
		module->icon_number		= IKOO;	/* Soll bei Init vergeben werden */
		module->menu_title		= MOUTPUTS;
		module->menu_position	= MKOO;
		module->menu_item			= MKOO;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_koo;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_OUTPUT;
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
		module->location				= SETUPS_INTERN;
		module->ram_modified 		= FALSE;
		module->max_setups		 		= MAXSETUPS;
		module->standard				= (SET_P)mem_alloc(sizeof(SETUP));
		module->actual->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->edited->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->status 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->stat_alt 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->get_dbox				= get_dbox;
		module->set_dbox				= set_dbox;
		module->send_messages		= send_messages;
		if (module->location == SETUPS_INTERN)
		{
			module->setups	= mem_alloc (sizeof (SETUP) * MAXSETUPS);
			mem_lset (module->setups, 0, sizeof (SETUP) * MAXSETUPS);
		} /* if */
		else
		{
		} /* else */
		/* Prfen, ob DEFAULT-Datei vorhanden */
		if((fp=fopen(module->file_name, "rb"))!=0)
		{
			/* Wenn vorhanden, laden */
			fclose(fp);
			module->load(module, module->file_name, FALSE);
		} /* if */

		/* Setup-Strukturen initialisieren */
		mem_set(module->standard, 0, (UWORD) sizeof(SETUP));
		
		/* Status-Strukturen initialisieren */
		mem_set(module->status, 0, (UWORD) sizeof(STATUS));
		mem_set(module->stat_alt, 0, (UWORD) sizeof(STATUS));
		module->status->new = TRUE;

		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (koo_setup, NULL, IKOO);
		/* Modul-Struktur einbinden */
		window->module 	= (VOID*) module;
		module->window		= window;
		
	} /* if */
	
	return module;
} /* create */

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
  koo_setup = (OBJECT *)rs_trindex [KOO_SETUP]; /* Adresse der KOO-Parameter-Box */
  koo_help  = (OBJECT *)rs_trindex [KOO_HELP];	/* Adresse der KOO-Hilfe */
  koo_desk  = (OBJECT *)rs_trindex [KOO_DESK];	/* Adresse des KOO-Desktops */
  koo_text  = (OBJECT *)rs_trindex [KOO_TEXT];	/* Adresse der KOO-Texte */
  koo_info 	= (OBJECT *)rs_trindex [KOO_INFO];	/* Adresse der KOO-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (koo_rsc_ptr, rsc_name))
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

  rs_gaddr (koo_rsc_ptr, R_TREE,  KOO_SETUP,	&koo_setup);   /* Adresse der KOO-Parameter-Box */
  rs_gaddr (koo_rsc_ptr, R_TREE,  KOO_HELP,	&koo_help);    /* Adresse der KOO-Hilfe */
  rs_gaddr (koo_rsc_ptr, R_TREE,  KOO_DESK,	&koo_desk);    /* Adresse der KOO-Desktop */
  rs_gaddr (koo_rsc_ptr, R_TREE,  KOO_TEXT,	&koo_text);    /* Adresse der KOO-Texte */
  rs_gaddr (koo_rsc_ptr, R_TREE,  KOO_INFO,	&koo_info);    /* Adresse der KOO-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (koo_setup, TRUE);
	fix_objs (koo_help, TRUE);
	fix_objs (koo_desk, TRUE);
	fix_objs (koo_text, TRUE);
	fix_objs (koo_info, TRUE);
	
	/*
	do_flags (koo_setup, KOOCANCEL, UNDO_FLAG);
	do_flags (koo_setup, KOOHELP, HELP_FLAG);
	*/
	menu_enable(menu, MKOO, TRUE);

	return (TRUE);
} /* init_rsc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PRIVATE BOOLEAN term_rsc ()

{
	BOOLEAN ok = TRUE;

#if ((XRSC_CREATE||RSC_CREATE) == 0)
	ok = rs_free (koo_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

	return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_koo ()
{
	WORD					x;
	BOOLEAN				ok = TRUE;
	 
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

	return (ok);
} /* init_koo */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	
	ok &= term_rsc ();
	return (ok);
} /* term_mod */

