/*****************************************************************************/
/*                                                                           */
/* Modul: A3D.C                                                              */
/*                                                                           */
/*****************************************************************************/
#define A3DVERSION "V 1.00"
#define A3DDATE "30.01.95"

#define A3D

#ifdef A3D
/*****************************************************************************
V 1.00
- ClickSetupField eingebaut, 30.01.95
- Setup-Class in dsetup auf CLASS_xxx umgestellt
- in set_dbox Perspektive eingebaut
V 0.16 15.06.94
- load_create_infos und instance_count eingebaut
- resized default workspace
- Restructured for new Dispobj structure, using KOOR's from the VAR's
V 0.15, 11.01.94
- VAR senden und empfangen ausgeschaltet, wg. Mehrdeutigkeit bei mehreren Fenstern
- load/save aktiviert
- Umstellung auf Display-Objekte
- Umstellung auf kleines RSC-Format
V 0.14 30.07.93
- wi_click_mod eingebaut
- INITW und INITH vergrîûert
- get_edit_setup und get_setnr_setup eingebaut
- wi_start_mod eingebaut, set_a3d in wi_draw_mod eingebaut
- Fehler in Tetraeder Innenraum beseitigt
- window->module eingebaut
- init_koor: set_dakstat fÅr XYK-Neuberechnung eingebaut
- wi_finished_mod eingebaut
- tetraeder und quadrophon eingebaut
- Umbau auf create_window_obj
V 0.12
- copy_icon umgebaut
V 0.11
- Umstellung auf neue RTMCLASS-Struktur
*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "a3d_mod.h"
#include "realtspc.h"
#include "var.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"
#include "dispobj.h"

#include "export.h"
#include "a3d.h"

#if XRSC_CREATE
#include "a3d_mod.rsh"
#include "a3d_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_RESIDENT | WI_MOUSE | WI_NOSCROLL)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (160)				         /* Anfangsbreite in Pixel */
#define INITH  (100)         				/* Anfangshîhe in Pixel */
#define MILLI  100                    	/* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "A3D_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 100l					/* Anzahl der A3D-Setups */

enum koor_states 
{USED, NEW, FREE};		/* Zustand der Koordinaten in den zwei Gruppen */

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 20;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "A3D";		/* Name, fÅr Extension etc. */

/****** TYPES ****************************************************************/

typedef struct status *STAT_P;

typedef struct status
{
	WORD		xoffset,		/* Mitte des Fensters in X-Richtung */
				yoffset,		/*               ... und Y-Richtung */
				zoom;			/* 	Grîûe des Fensters in Relation zu xykoor */
	KOOR_ALL	koor_alt;	/* Letzte gezeichnete Koordinaten */
	KOOR_ALL	koor_akt1;	/* Neue Koordinaten, Gruppe 1 */
	WORD		koor_stat1;	/* Momentaner Zustand dieser Gruppe */
	KOOR_ALL	koor_akt2;	/* Neue Koordinaten, Gruppe 2 */
	WORD		koor_stat2;	/* Momentaner Zustand dieser Gruppe */
	BOOLEAN	new;			/* Zwang zum Neuzeichnen */
	BOOLEAN	reset_flag;	/* Parameter wurden zurueckgesetzt */
	WINDOWP		refwindow;
	RTMCLASSP	refmodule;
} STATUS;

typedef struct setup *SET_P;
typedef struct setup
{
	BOOLEAN	anzeige[MAXSIGNALS],		/* Flags fÅr Anzeige an/aus */
				fadenkreuz[MAXSIGNALS]; /* Flags fÅr Fadenkreuz pro Signal */
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
} SETUP;	/* EnthÑlt alle Parameter einer kompletten A3D-Einstellung */

/****** VARIABLES ************************************************************/
PRIVATE WORD	a3d_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*a3d_rsc_ptr = &a3d_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *a3d_menu;
PRIVATE OBJECT *a3d_setup;
PRIVATE OBJECT *a3d_shelp;
PRIVATE OBJECT *a3d_help;
PRIVATE OBJECT *a3d_desk;
PRIVATE OBJECT *a3d_text;
PRIVATE OBJECT *a3d_info;
PRIVATE OBJECT *a3d_raum;

/****** FUNCTIONS ************************************************************/

PRIVATE VOID		dsetup			_((WINDOWP refwindow));
PRIVATE VOID		click_setup		_((WINDOWP window, MKINFO *mk));
PRIVATE RTMCLASSP define_setup	_((WINDOWP window, RTMCLASSP refmodule));
PUBLIC VOID    	get_edit_setup		_((RTMCLASSP module));
PUBLIC BOOLEAN		set_setnr_setup	_((RTMCLASSP module, LONG setupnr));

PRIVATE BOOLEAN	init_rsc			_((VOID));
PRIVATE BOOLEAN	term_rsc			_((VOID));

/*****************************************************************************/

PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	SET_P	ed = module->edited->setup;
	STRING 	s;
	WORD	signal, offset;

	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(A3DLINIE1-A3DLINIE0)*signal;
		ed->anzeige[signal] = get_checkbox (a3d_setup, A3DLINIE0 + offset);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(A3DFADEN1-A3DFADEN0)*signal;
		ed->fadenkreuz[signal] = get_checkbox (a3d_setup, A3DFADEN0 + offset);
	} /* for */

	get_ptext (a3d_setup, A3DGRAFIKX, s);
	sscanf (s, "%d", &ed->rot_x);
	get_ptext (a3d_setup, A3DGRAFIKY, s);
	sscanf (s, "%d", &ed->rot_y);
	get_ptext (a3d_setup, A3DGRAFIKZ, s);
	sscanf (s, "%d", &ed->rot_z);
	get_ptext (a3d_setup, A3DGRAFIKPERSP, s);
	sscanf (s, "%d", &ed->persp);

	ed->innenraum = get_checkbox (a3d_setup, A3DINNENRAUM);

	if (get_checkbox (a3d_setup, A3DMONO))		ed->modus = MONO;
	if (get_checkbox (a3d_setup, A3DSTEREO))	ed->modus = STEREO;
	if (get_checkbox (a3d_setup, A3DQUADRO))	ed->modus = QUADRO;
	if (get_checkbox (a3d_setup, A3DOKTO))		ed->modus = OKTO;
	
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited 	= module->edited;
	SET_P		ed = edited->setup;
	STRING 	s;
	WORD		signal, offset;

	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(A3DLINIE1-A3DLINIE0)*signal;
		set_checkbox (a3d_setup, A3DLINIE0 + offset, ed->anzeige[signal]);
	} /* for */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		offset=(A3DFADEN1-A3DFADEN0)*signal;
		set_checkbox (a3d_setup, A3DFADEN0 + offset, ed->fadenkreuz[signal]);
	} /* for */

	
	sprintf (s, "%d", ed->rot_x);
	set_ptext (a3d_setup, A3DGRAFIKX, s);
	sprintf (s, "%d", ed->rot_y);
	set_ptext (a3d_setup, A3DGRAFIKY, s);
	sprintf (s, "%d", ed->rot_z);
	set_ptext (a3d_setup, A3DGRAFIKZ, s);
	sprintf (s, "%d", ed->persp);
	set_ptext (a3d_setup, A3DGRAFIKPERSP, s);

	set_checkbox (a3d_setup, A3DINNENRAUM, ed->innenraum);
	set_checkbox (a3d_setup, A3DAUSSENWELT, !(ed->innenraum));

	set_checkbox (a3d_setup, A3DMONO,	ed->modus == MONO);
	set_checkbox (a3d_setup, A3DSTEREO,	ed->modus == STEREO);
	set_checkbox (a3d_setup, A3DQUADRO,	ed->modus == QUADRO);
	set_checkbox (a3d_setup, A3DOKTO,	ed->modus == OKTO);

	copy_icon (&a3d_setup[A3DRAUMFORM], &a3d_raum[ed->raumform+1]);  /* Holen der Iconstruktur aus MTR_EBENEN Popup */
	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (a3d_setup, A3DSETNR , s);
} /* set_dbox */

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	STAT_P	status = module->status;
	WINDOWP	window = module->window;
	
/*	This handling is not necessary, as DISPOBJ take their data
	directly from the VAR's
	
	/* Wenn Gruppe 1 neue Koor erhalten soll, dann Koordinaten in
		Gruppe eins schieben, sonst in Gruppe 2 */
	if (status->koor_stat1 != USED)
	{
		status->koor_stat1 = NEW;
		mem_movex(&status->koor_akt1, event->koors, (UWORD)sizeof(KOOR_ALL));
	} /* if */
	else
	{
		status->koor_stat2 = NEW;
		mem_movex(&status->koor_akt2, event->koors, (UWORD)sizeof(KOOR_ALL));
	} /* else */
	
*/
	window->milli = 1; 	/* Update so schnell wie mîglich */
	return event;
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	/* ZurÅcksetzen von Werten */
	STAT_P	status	= module->status;
	WINDOWP	window = module->window;

	status->reset_flag = TRUE;
	status->new = TRUE;
	if (window) window->milli = 1;
} /* reset */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	/* Vorausberechnung */
} /* precalc */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD 	variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;

	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch (variable)
			{
				case VAR_SET_A3D:
					/* module->set_setnr(module, value); */
					break;
			} /* switch */
			break;
	} /* switch */
} /* message */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;

	/* send_variable(VAR_SET_A3D, module->actual->number); */

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
		case A3DSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case A3DSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case A3DSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case A3DSETSTORE:
			module->set_store (window, edited->number);
			break;
		case A3DSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case A3DOK   :
			module->set_ok (window);
			break;
		case A3DCANCEL:
			module->set_cancel (window);
			break;
		case A3DHELP :
			module->help (module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case A3DSTANDARD:
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
				set_ptext (a3d_setup, A3DSETNR, s);
				draw_object(window, A3DSETNR);
			} /* if */
			switch (window->exit_obj)
			{
				case A3DLINIEALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(A3DLINIE1-A3DLINIE0)*signal;
						ed->anzeige[signal] = !ed->anzeige[signal];
						set_checkbox (a3d_setup, A3DLINIE0 + offset, ed->anzeige[signal]);
						draw_object(window, A3DLINIE0 + offset);
					} /* for */
					undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case A3DFADENALL:
					for (signal = 0; signal<MAXSIGNALS; signal++)
					{
						offset=(A3DFADEN1-A3DFADEN0)*signal;
						ed->fadenkreuz[signal] = !ed->fadenkreuz[signal];
						set_checkbox (a3d_setup, A3DFADEN0 + offset, ed->fadenkreuz[signal]);
						draw_object(window, A3DFADEN0 + offset);
					} /* for */
					undo_state (window->object, window->exit_obj, SELECTED);
					draw_object(window, window->exit_obj);
					break;
				case A3DRAUMFORM:	
					i = ed->raumform +1;
					item = popup_menu (a3d_raum, ROOT, 0, 0, i, TRUE, mk->momask);
					if ((item != NIL) && (item != i))
					{
						ed->raumform = item-1;
						/* Holen der Iconstruktur aus A3D_RAUMFORM Popup */
						copy_icon (&a3d_setup[A3DRAUMFORM], &a3d_raum[ed->raumform+1]);
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
	
	window = search_window (CLASS_DIALOG, SRCH_CLOSED, CLASS_A3D);
	
	if (window == NULL)
	{
		form_center (a3d_setup, &ret, &ret, &ret, &ret);
		
		window = crt_dialog (a3d_setup, NULL, A3D_SETUP, (BYTE *)a3d_text [FA3DSN].ob_spec, WI_MODELESS);
	} /* if */
		
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			module = define_setup(window, (RTMCLASSP)refwindow->module); 
			window->edit_obj = find_flags (a3d_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			
			module->set_edit (module);
			module->set_dbox (module);
			undo_state (window->object, A3DHELP, DISABLED);
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

	menu_check(a3d_menu, MA3DMONO, akt->modus == MONO);
	menu_check(a3d_menu, MA3DSTEREO, akt->modus == STEREO);
	menu_check(a3d_menu, MA3DQUADRO, akt->modus == QUADRO);
	menu_check(a3d_menu, MA3DOKTO, akt->modus == OKTO);
	menu_check(a3d_menu, MA3DINNENRAUM, akt->innenraum);

	menu_check(a3d_menu, MA3DTETRAEDER, akt->raumform == TETRAEDER);
	menu_check(a3d_menu, MA3DSECHSKANAL, akt->raumform == SECHSKANAL);
	menu_check(a3d_menu, MA3DOKTAEDER, akt->raumform == OKTAEDER);
	menu_check(a3d_menu, MA3DWUERFEL, akt->raumform == WUERFEL);
	menu_check(a3d_menu, MA3DWUERFELLANG, akt->raumform == WUERFELLANG);
	menu_check(a3d_menu, MA3DWUERFELHOCH, akt->raumform == WUERFELHOCH);
	menu_check(a3d_menu, MA3DWUERFELMITTE, akt->raumform == WUERFELMITTE);
	menu_check(a3d_menu, MA3DWUERFELDOPP, akt->raumform == WUERFELDOPP);
	menu_check(a3d_menu, MA3DKREISFORM, akt->raumform == KREISFORM);
	menu_check(a3d_menu, MA3DQUADROPHON, akt->raumform == QUADROPHON);
} /* update_menu_mod */

/*****************************************************************************/

PRIVATE VOID handle_menu_mod (window, title, item)
WINDOWP window;
WORD    title, item;

{
	SET_P		akt = Akt(window);
	STAT_P	status = Status(window);
		
	if (window != NULL)
		menu_normal (window, title, FALSE);         /* Titel invers darstellen */

	switch (title)
	{
		case MA3DINFO: switch (item)
		{
			case MA3DINFOANZEIG: info_mod(window, NIL);
										break;
		} /* switch */
		break;
		case MA3DMODES: switch (item)
		{
			case MA3DMONO:		akt->modus = MONO; 	break;
			case MA3DSTEREO: 	akt->modus = STEREO;	break;
			case MA3DQUADRO: 	akt->modus = QUADRO;	break;
			case MA3DOKTO: 	akt->modus = OKTO;	break;
		} /* switch */
		status->new = TRUE;
		window->milli = 1;
		break;
		case MA3DFORMS: switch (item)
		{
			case MA3DTETRAEDER:		akt->raumform = TETRAEDER;		break;
			case MA3DSECHSKANAL:		akt->raumform = SECHSKANAL;	break;
			case MA3DOKTAEDER:		akt->raumform = OKTAEDER;		break;
			case MA3DWUERFEL:			akt->raumform = WUERFEL;		break;
			case MA3DWUERFELLANG:	akt->raumform = WUERFELLANG;	break;
			case MA3DWUERFELHOCH:	akt->raumform = WUERFELHOCH;	break;
			case MA3DWUERFELMITTE:	akt->raumform = WUERFELMITTE;	break;
			case MA3DWUERFELDOPP:	akt->raumform = WUERFELDOPP;	break;
			case MA3DQUADROPHON:		akt->raumform = QUADROPHON;	break;
			case MA3DKREISFORM:		akt->raumform = KREISFORM;		break;
		} /* switch */
		status->new = TRUE;
		window->milli = 1;
		break;
		case MA3DOPTIONS: switch (item)
		{
			case MA3DSETUPS	:
				dsetup(window);
				break;
			case MA3DINNENRAUM		:
				akt->innenraum = !akt->innenraum;
				window->milli = 1;
				break;
			case MA3DPERSPEKTIVE 	:
				break;
		} /* switch */
		status->new = TRUE;
	} /* switch */
		
	if (window != NULL)
		menu_normal (window, title, TRUE);          /* Titel wieder normal darstellen */
} /* handle_menu_mod */

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
		/* clr_scroll (window);
			not needed because DO's do it themselves */
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
	WORD		index, x;
		
	status->new = new;
	
	/* Wenn neue Koordinate vorhanden, diese verwenden 
		und die andere Gruppe freigeben. */
	if (status->koor_stat1 == NEW)
	{
		status->koor_stat1 = USED;
		status->koor_stat2 = FREE;
		newkoor = TRUE;
	} /* if */
	else if (status->koor_stat2 == NEW)
	{
		status->koor_stat1 = FREE;
		status->koor_stat2 = USED;
		newkoor = TRUE;
	} /* else */

	if (status->koor_stat1 == USED)
		koor = &status->koor_akt1;
	else
		koor = &status->koor_akt2;

	/* Liste der Display-Objekte durchgehen und neue Koor eintragen */
	header = window->dispobjs;
	element = list_next(header);
	while (element != header) {
		dispobj = (DISPOBJP) element->key;
		if (new) {
			dispobj->new	= TRUE;
			dispobj->set_uni  (dispobj, DOUniSpaceInside, akt->innenraum);
			dispobj->set_uni  (dispobj, DOUniSpaceArrows, akt->pfeile);
			dispobj->set_type (dispobj, DOTypeSpaceForm, akt->raumform);
			dispobj->set_type (dispobj, DOTypeSpaceMode, akt->modus);
			dispobj->set_work (dispobj, &window->scroll);

			dispobj->set_uni  (dispobj, DOUniRotationX, akt->rot_x);
			dispobj->set_uni  (dispobj, DOUniRotationY, akt->rot_y);
			if (akt->rot_z !=0)
				dispobj->set_uni  (dispobj, DOUniRotationZ, akt->rot_z);
			dispobj->set_uni  (dispobj, DOUniSpacePerspective, akt->persp);

			/* Compute and send bitflags for display and crosshair */
			x = 0;
			for (index = 0; index < MAXSIGNALS; index ++)
				x = x | (akt->anzeige[index] << index);
			dispobj->set_uni (dispobj, DOUniSpaceDisplay, x);
			x = 0;
			for (index = 0; index < MAXSIGNALS; index ++)
				x = x | (akt->fadenkreuz[index] << index);
			dispobj->set_uni (dispobj, DOUniSpaceCrosshair, x);
		} /* if new */

		/*	This handling is not necessary, as DISPOBJ take their data
			directly from the VAR's
		if (new || newkoor) {
			for (index = 0; index < MAXPOS; index ++)
			{
				/* Die Daten muessen von absoluten Koordinaten in Prozent
					umgerechnet werden */
					
				point = &koor->koor[index].koor;
				position.x = point->x * 100 / MAXKOOR;
				position.y = point->y * 100 / MAXKOOR;
				position.z = point->z * 100 / MAXKOOR;

				(*dispobj->set) (dispobj, DOParPosition, index, (VOID*)&position);
			} /* for index */
		} /* if new || newkoor */
		*/

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
	
	inx    = num_windows (CLASS_A3D, SRCH_ANY, NULL);
	window = create_window_obj (KIND, CLASS_A3D);
	
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
		window->draw		= wi_draw_mod;
		window->snap      = wi_snap_mod;
		window->timer     = wi_timer_mod;
		window->showinfo  = info_mod;
		window->click		= wi_click_mod;
		window->start		= wi_start_mod;
		window->finished  = wi_finished_mod;

		/* Erstes Display-Objekt einklinken */
		dispobj = CreateSpaceDispobj (window, WUERFEL, 0, &window->scroll);
		if (dispobj)
			list_insert (window->dispobjs, list_new_el ((VOID*) dispobj));

		sprintf (window->name, (BYTE *)a3d_text [FA3DN].ob_spec, 0);
		sprintf (window->info, (BYTE *)a3d_text [FA3DI].ob_spec, 0);
	} /* if */
	
	return (window);                      /* Fenster zurÅckgeben */
} /* crt_mod */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN 	ok;
	WINDOWP 	window;
	
	/* Fenster suchen */
	window = search_window (CLASS_A3D, SRCH_CLOSED, icon);
	/* Wenn nicht gefunden */
	if (window == NULL)
	{
		if (create()>0);	/* Neue Instanz */
			window = search_window (CLASS_A3D, SRCH_CLOSED, icon);
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

	window = search_window (CLASS_DIALOG, SRCH_ANY, IA3D);
		
	if (window == NULL)
	{
		 form_center (a3d_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (a3d_info, NULL, IA3D, refwindow->name, WI_MODELESS);
	} /* if */
		
	if (window != NULL)
	{
		window->object = a3d_info;
		sprintf(s, "%-20s", A3DDATE);
		set_ptext (a3d_info, A3DIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (a3d_info, A3DCOMPILE, s);
		sprintf(s, "%-20s",  A3DVERSION);
		set_ptext (a3d_info, A3DIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (a3d_info, A3DISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (a3d_info, A3DIAKT, s);

		if (! open_dialog (IA3D)) hndl_alert (ERR_NOOPEN);
	} /* if */

  return (window != NULL);
} /* info_a3d */

/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	RTMCLASSP 	module;
	WINDOWP		window;
	STAT_P		status;
	SET_P			standard;
	FILE			*fp;
	WORD			signal;
	
	module = create_module (module_name, instance_count);
	
	if (module != NULL && instance_count < max_instances)
	{
		module->class_number		= CLASS_A3D;
		module->icon				= &a3d_desk[A3DICON];
		module->icon_position 	= IA3D;
		module->icon_number		= IA3D;	/* Soll bei Init vergeben werden */
		module->menu_title		= MOUTPUTS;
		module->menu_position	= MA3D;
		module->menu_item			= MA3D;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_a3d;
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
		status->koor_stat1 = USED;	/* Gruppe 1 wird benutzt */
		status->koor_stat2 = FREE;	/* Gruppe 2 freigegeben */
		
		/* Setup-Strukturen initialisieren */
		standard = module->standard;
		mem_set(standard, 0, (UWORD) sizeof(SETUP));
		standard->fadenkreuz[0] = TRUE;
		standard->modus			= STEREO;
		standard->raumform		= WUERFEL;
		standard->innenraum		= FALSE;

		/* Fluchtpunkt */
		standard->rot_x			= 60;
		standard->rot_y			= 50;
		standard->rot_z			= 100;
		
		for (signal = 0; signal<MAXSIGNALS; signal++)
		{
			standard->anzeige[signal] = TRUE;
		} /* for */
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (a3d_setup, a3d_menu, IA3D);
		/* Modul-Struktur einbinden */
		window->module	= (VOID*) module;
		sprintf(window->name, " %s ", module->object_name);
		module->window		= window;
		module->status->refmodule = module;	
		module->status->refwindow = window;	
		add_rcv(VAR_SET_A3D,  module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_A3D, MAXSETUPS);

		instance_count++;
	} /* if */
	return module;
} /* create */

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
  a3d_menu  = (OBJECT *)rs_trindex [A3D_MENU];  /* Adresse der A3D-MenÅzeile */
  a3d_setup = (OBJECT *)rs_trindex [A3D_SETUP]; /* Adresse der A3D-Parameter-Box */
  a3d_shelp = (OBJECT *)rs_trindex [A3D_SHELP];	/* Adresse der A3D-Parameter-Hilfe */
  a3d_help  = (OBJECT *)rs_trindex [A3D_HELP];	/* Adresse der A3D-Hilfe */
  a3d_desk  = (OBJECT *)rs_trindex [A3D_DESK];	/* Adresse des A3D-Desktops */
  a3d_text  = (OBJECT *)rs_trindex [A3D_TEXT];	/* Adresse der A3D-Texte */
  a3d_info 	= (OBJECT *)rs_trindex [A3D_INFO];	/* Adresse der A3D-Info-Anzeige */
  a3d_raum	= (OBJECT *)rs_trindex [A3D_RAUM];	/* Adresse der A3D-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (a3d_rsc_ptr, rsc_name))
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

  rs_gaddr (a3d_rsc_ptr, R_TREE,  A3D_MENU,	&a3d_menu);    /* Adresse des A3D-MenÅs */
  rs_gaddr (a3d_rsc_ptr, R_TREE,  A3D_SETUP,	&a3d_setup);   /* Adresse der A3D-Parameter-Box */
  rs_gaddr (a3d_rsc_ptr, R_TREE,  A3D_SHELP,	&a3d_shelp);   /* Adresse der A3D-Parameter-Hilfe */
  rs_gaddr (a3d_rsc_ptr, R_TREE,  A3D_HELP,	&a3d_help);    /* Adresse der A3D-Hilfe */
  rs_gaddr (a3d_rsc_ptr, R_TREE,  A3D_DESK,	&a3d_desk);    /* Adresse des A3D-Desktop */
  rs_gaddr (a3d_rsc_ptr, R_TREE,  A3D_TEXT,	&a3d_text);    /* Adresse der A3D-Texte */
  rs_gaddr (a3d_rsc_ptr, R_TREE,  A3D_INFO,	&a3d_info);    /* Adresse der A3D-Info-Anzeige */
  rs_gaddr (a3d_rsc_ptr, R_TREE,  A3D_RAUM,	&a3d_raum);    /* Adresse der A3D-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (a3d_menu, TRUE);
	fix_objs (a3d_setup, TRUE); 
	fix_objs (a3d_shelp, TRUE);
	fix_objs (a3d_help, TRUE);
	fix_objs (a3d_desk, TRUE);
	fix_objs (a3d_text, TRUE);
	fix_objs (a3d_info, TRUE);
	fix_objs (a3d_raum, TRUE);
	
	
	do_flags (a3d_setup, A3DCANCEL, UNDO_FLAG);
	do_flags (a3d_setup, A3DHELP, HELP_FLAG);
	
	menu_enable(menu, MA3D, TRUE);

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
  ok = rs_free (a3d_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	return (term_rsc ());
} /* term_mod */

#endif /* A3D */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_a3d ()
{
	BOOLEAN	ok = TRUE;

#ifdef A3D
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);
#endif /* A3D */

	return (ok);
} /* init_a3d */

