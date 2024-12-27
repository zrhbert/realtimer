/*****************************************************************************/
/*                                                                           */
/* Modul: EC4.C                                                           	  */
/*                                                                           */
/* CUE-List-Editor                                                           */
/*                                                                           */
/*****************************************************************************/

#define EC4VERSION "V 0.01"
#define EC4DATE "03.01.95"

/*****************************************************************************
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
#include "ec4_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"
#include "dispobj.h"
#include "ed4.h"
#include "msh_unit.h"
#include "msh.h"

#include "export.h"
#include "ec4.h"

#if XRSC_CREATE
#include "ec4_mod.rsh"
#include "ec4_mod.rh"
#endif

/****** DEFINES **************************************************************/

#define KIND   (NAME|INFO|CLOSER|FULLER|MOVER|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_RESIDENT | WI_MOUSE | WI_NOSCROLL)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (57 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  (20 * gl_hbox)           /* Anfangsh”he in Pixel */
#define MILLI  100							/* Millisekunden fr Zeitablauf */

#define MOD_RSC_NAME "EC4_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 1L						/* Anzahl der EC4-Setups */
#define NUMLINES 10						/* Anzahl Zeilen in der Anzeige */

/****** TYPES ****************************************************************/
typedef struct setup *SET_P;
typedef struct setup
{
	VOID *dummy;
} SETUP;		/* Enth„lt alle Parameter einer kompletten EC4-Einstellung */

typedef struct status *STAT_P;
typedef struct status
{
	BOOL	new;
	SOUNDOBJ	event;
	WORD		num_events;					/* Anzahl der Events im Fenster */
	RTMCLASSP	refmodule;				/* Das Bezugsmodul */
} STATUS;

/****** VARIABLES ************************************************************/
/* Resource */
PRIVATE WORD	ec4_rsc_hdr;					/* Zeigerstruktur fr RSC-Datei */
PRIVATE WORD	*ec4_rsc_ptr = &ec4_rsc_hdr;		/* Zeigerstruktur fr RSC-Datei */
PRIVATE OBJECT *ec4_setup;
PRIVATE OBJECT *ec4_cue;
PRIVATE OBJECT *ec4_help;
PRIVATE OBJECT *ec4_desk;
PRIVATE OBJECT *ec4_text;
PRIVATE OBJECT *ec4_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 20;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "EC4";		/* Name, fr Extension etc. */

/****** FUNCTIONS ************************************************************/

/* Interne EC4-Funktionen */
PUBLIC LONG	GetNumEvents (RTMCLASSP module);
PUBLIC VOID	GetFirstEvent (RTMCLASSP module);
PUBLIC VOID	GetPrevEvent (RTMCLASSP module);
PUBLIC VOID	GetNextEvent (RTMCLASSP module);
PUBLIC VOID	ModifyEvent (RTMCLASSP module);
PUBLIC VOID	InsertEvent (RTMCLASSP module);
PUBLIC VOID	DeleteEvent (RTMCLASSP module);

PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

PRIVATE WORD ComputeNumDO (WINDOWP window);
PRIVATE VOID ComputeWorkDO (WINDOWP window, WORD obj_num, RECT *work);
PRIVATE VOID create_displayobs (WINDOWP window);

PRIVATE VOID    	get_dbox_editor	(RTMCLASSP module);
PRIVATE VOID    	set_dbox_editor	(RTMCLASSP module);
PRIVATE VOID 		wi_click_editor   (WINDOWP window, MKINFO *mk);
PRIVATE VOID 		deditor 				(WINDOWP refwindow);
PRIVATE RTMCLASSP define_editor (WINDOWP window, RTMCLASSP refmodule);

/*****************************************************************************/
/* Editor Fenster                                                            */
/*****************************************************************************/

PRIVATE VOID    get_dbox_editor	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		akt = module->actual->setup;
	SET_P		ed = edited->setup;
	STAT_P	status = module->status;
	SO_P		event = &status->event;
	WORD		offset = EC4SIG3 - EC4SIG2, signal;
	
	/* Timing */
	GetPTime (ec4_cue, EC4CUETIME, &event->cue_time);
	GetPTime (ec4_cue, EC4ENTRYTIME, &event->entry_time);
	GetPTime (ec4_cue, EC4EXITTIME, &event->exit_time);
	
	/* Parameter */
	GetPFloat (ec4_cue, EC4SPEEDX, &event->speed.x );
	GetPFloat (ec4_cue, EC4SPEEDY, &event->speed.y);
	GetPFloat (ec4_cue, EC4SPEEDZ, &event->speed.z);
	GetPWord (ec4_cue, EC4VOLUME, &event->volume);

	/* Info fr 1. Signal */
	GetPWord (ec4_cue, EC4SIG1CHANNEL, &event->input_ch[0]);
	event->input_ch[0]--;
	GetPFloat (ec4_cue, EC4POSX, &event->position[0].x);
	GetPFloat (ec4_cue, EC4POSY, &event->position[0].y);

	for (signal = 1; signal < MAXSIGNALS-1; signal++)
	{
		GetPWord (ec4_cue, EC4SIG2CHANNEL+(signal-1)*offset, &event->input_ch[signal]+1);
		GetPFloat (ec4_cue, EC4SIG2OFFX+(signal-1)*offset, &event->position[signal].x);
		GetPFloat (ec4_cue, EC4SIG2OFFY+(signal-1)*offset, &event->position[signal].y);
		GetPFloat (ec4_cue, EC4SIG2OFFZ+(signal-1)*offset, &event->position[signal].z);
	} /* for signal */
	
	GetPWord (ec4_cue, EC4KEY, &event->ev_key);
	GetPWord (ec4_cue, EC4CHANNEL, &event->ev_ch);
	GetPWord (ec4_cue, EC4VELOCITY, &event->ev_vel);
	GetPWord (ec4_cue, EC4PORT, &event->ev_port);

} /* get_dbox_editor */

PRIVATE VOID    set_dbox_editor	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STAT_P	status = module->status;
	SO_P		event = &status->event;
	WORD		offset = EC4SIG3 - EC4SIG2, signal;
	
	/* Timing */
	SetPTime (ec4_cue, EC4CUETIME, event->cue_time);
	SetPTime (ec4_cue, EC4ENTRYTIME, event->entry_time);
	SetPTime (ec4_cue, EC4EXITTIME, event->exit_time);
	
	/* Parameter */
	SetPFloat (ec4_cue, EC4SPEEDX, event->speed.x );
	SetPFloat (ec4_cue, EC4SPEEDY, event->speed.y);
	SetPFloat (ec4_cue, EC4SPEEDZ, event->speed.z);
	SetPWord (ec4_cue, EC4VOLUME, event->volume);

	/* Info fr 1. Signal */
	SetPWord (ec4_cue, EC4SIG1CHANNEL, event->input_ch[0]+1);
	SetPFloat (ec4_cue, EC4POSX, event->position[0].x);
	SetPFloat (ec4_cue, EC4POSY, event->position[0].y);

	for (signal = 1; signal < MAXSIGNALS-1; signal++)
	{
		SetPWord (ec4_cue, EC4SIG2CHANNEL+(signal-1)*offset, event->input_ch[signal]+1);
		SetPFloat (ec4_cue, EC4SIG2OFFX+(signal-1)*offset, event->position[signal].x);
		SetPFloat (ec4_cue, EC4SIG2OFFY+(signal-1)*offset, event->position[signal].y);
		SetPFloat (ec4_cue, EC4SIG2OFFZ+(signal-1)*offset, event->position[signal].z);
	} /* for signal */
	
	SetPWord (ec4_cue, EC4KEY, event->ev_key);
	SetPWord (ec4_cue, EC4CHANNEL, event->ev_ch);
	SetPWord (ec4_cue, EC4VELOCITY, event->ev_vel);
	SetPWord (ec4_cue, EC4PORT, event->ev_port);
	
} /* set_dbox_editor */

PRIVATE VOID wi_click_editor (window, mk)
WINDOWP window;
MKINFO  *mk;
{
	RTMCLASSP	module = Module(window);
	STAT_P		status = module->status;
	BOOLEAN		new = status->new || (window->flags & WI_JUNK);
	RTMCLASSP 	refmodule = module->status->refmodule;
	WORD			exit_obj = window->exit_obj;
	
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (exit_obj)
	{
		case EC4CUEOK   :
			module->get_dbox(module);
			ModifyEvent (module);
			module->set_dbox(module);
			draw_object(window, ROOT);
			refmodule->window->milli = 1;
			refmodule->status->new = TRUE;
			break;
		case EC4CUECANCEL:
			module->set_dbox(module);
			draw_object(window, ROOT);
			refmodule->window->milli = 1;
			refmodule->status->new = TRUE;
		   break;
		case EC4CUEINSERT:
			module->get_dbox(module);
			InsertEvent (module);
			module->set_dbox(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object(window, ROOT);
			refmodule->window->milli = 1;
			refmodule->status->new = TRUE;
			break;
		case EC4CUEDELETE:
			module->get_dbox(module);
			DeleteEvent (module);
			GetNextEvent (module);
			module->set_dbox(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object(window, ROOT);
			refmodule->window->milli = 1;
			refmodule->status->new = TRUE;
		   break;
/*
		case EC4CUEHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
*/
		case EC4CUESTANDARD:
			GetFirstEvent (module);
			module->set_dbox(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object(window, ROOT);
		   break;
		case ROOT:
		case NIL:
			break;
		case EC4CUEPREVIOUS:
			module->get_dbox(module);
			ModifyEvent (module);
			GetPrevEvent (module);
			module->set_dbox(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object(window, ROOT);
			break;
		case EC4CUENEXT:
			module->get_dbox(module);
			ModifyEvent (module);
			GetNextEvent (module);
			module->set_dbox(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object(window, ROOT);
			break;
		default :	
			if (exit_obj >= EC4CUETIME && exit_obj <= EC4CUETIME + 4)
				ClickTimeField (window, EC4CUETIME, mk, 0, 0, UpdateTimeField);
			else if (exit_obj >= EC4ENTRYTIME && exit_obj <= EC4ENTRYTIME + 4)
				ClickTimeField (window, EC4ENTRYTIME, mk, 0, 0, UpdateTimeField);
			else if (exit_obj >= EC4EXITTIME && exit_obj <= EC4EXITTIME + 4)
				ClickTimeField (window, EC4EXITTIME, mk, 0, 0, UpdateTimeField);
		} /* switch */

	window->milli = 1;
} /* wi_click_editor */

PRIVATE VOID deditor (WINDOWP refwindow)
{
	RTMCLASSP	module;
	WINDOWP		window;
	WORD			ret;
	
	window = search_window (CLASS_DIALOG, SRCH_ANY, CLASS_EC4);
	
	if (window == NULL)
	{
		form_center (ec4_cue, &ret, &ret, &ret, &ret);
		
		window = crt_dialog (ec4_cue, NULL, CLASS_EC4, (BYTE *)ec4_text [FEC4N].ob_spec, WI_MODELESS);
	} /* if */
		
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			module = define_editor(window, (RTMCLASSP)refwindow->module); 

			window->edit_obj = find_flags (ec4_cue, ROOT, EDITABLE);
			window->edit_inx = NIL;
			module->set_dbox (module);
			undo_state (window->object, EC4HELP, DISABLED);
		} /* if */
		else
			window->opened = 1;
			
		if (! open_window (window)) hndl_alert (ERR_NOOPEN);
	
	} /* if */
} /* deditor */

PRIVATE RTMCLASSP define_editor (WINDOWP window, RTMCLASSP refmodule)
{
	RTMCLASSP module;

	if (window != NULL)
	{
		window->click    = wi_click_editor;
		window->key      = wi_key_obj;
		window->showinfo = info_mod;
		window->showhelp = showhelp_obj;
		module = create_module(module_name, instance_count);
		/* Informationen kopieren */
		mem_move (module, refmodule, (UWORD)sizeof (RTMCLASS));
		module->object_type	= MODULE_OTHER;
		module->send_messages	= send_messages;
		module->window		= window;
		window->module		= (VOID*) module;
		
		module->status->refmodule = refmodule;
		module->get_dbox	= get_dbox_editor;
		module->set_dbox	= set_dbox_editor;
		sprintf(module->object_name, "%s", refmodule->object_name);
		sprintf(window->name, " %s Editor ", module->object_name);
	} /* if */
	return module;
} /* define_editor */

/*****************************************************************************/

PUBLIC LONG	GetNumEvents (RTMCLASSP module)
{
	ED4_MSG msg;

	ed4_module->message (ed4_module, GET_NUM_EVENTS, &msg);

	/* Minus Header-Event */
	return msg.long_out-1;
	
} /* GetNumEvents */

PUBLIC VOID	GetFirstEvent (RTMCLASSP module)
{
	ED4_MSG msg;
	STAT_P	status	= module->status;

	msg.in1 = &status->event;
	
	/* Zurcksetzen von Werten */
	ed4_module->message (ed4_module, GET_FIRST_EVENT, &msg);

	/* Copy Event Info */
	status->event = *msg.out1;

} /* GetFirstEvent */

PUBLIC VOID	GetPrevEvent (RTMCLASSP module)
{
	ED4_MSG msg;
	STAT_P	status	= module->status;

	msg.in1 = &status->event;
	
	/* Zurcksetzen von Werten */
	ed4_module->message (ed4_module, GET_PREV_EVENT, &msg);

	/* Copy Event Info */
	status->event = *msg.out1;

} /* GetPrevEvent */

PUBLIC VOID	GetNextEvent (RTMCLASSP module)
{
	ED4_MSG msg;
	STAT_P	status	= module->status;

	msg.in1 = &status->event;
	
	/* Zurcksetzen von Werten */
	ed4_module->message (ed4_module, GET_NEXT_EVENT, &msg);

	/* Copy Event Info */
	status->event = *msg.out1;

} /* GetNextEvent */

PUBLIC VOID	ModifyEvent (RTMCLASSP module)
{
	ED4_MSG msg;
	STAT_P	status	= module->status;

	msg.in1 = &status->event;
	
	/* Zurcksetzen von Werten */
	ed4_module->message (ed4_module, MODIFY_EVENT, &msg);

} /* ModifyEvent */

PUBLIC VOID	InsertEvent (RTMCLASSP module)
{
	ED4_MSG msg;
	STAT_P	status	= module->status;

	msg.in1 = &status->event;
	
	/* Zurcksetzen von Werten */
	ed4_module->message (ed4_module, INSERT_EVENT, &msg);

} /* InsertEvent */

PUBLIC VOID	DeleteEvent (RTMCLASSP module)
{
	ED4_MSG msg;
	STAT_P	status	= module->status;

	msg.in1 = &status->event;
	
	/* Zurcksetzen von Werten */
	ed4_module->message (ed4_module, DELETE_EVENT, &msg);

} /* DeleteEvent */

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	WINDOWP	window = Window(module);

/*
	if (window)
		window->milli = 1; 	/* Update so schnell wie m”glich */
*/		
	return event;
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	WINDOWP		window = Window(module);
	STAT_P		status = module->status;

	/* GetFirstEvent (module); */

	status->new = TRUE;
	if (window)
		window->milli = 1; 	/* Update so schnell wie m”glich */
		
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
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			break;
	} /* switch */
} /* message */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;

	send_variable(VAR_SET_EC4, module->actual->number);
} /* send_messages */

/*****************************************************************************/
/* ™ffne Fenster                                                             */
/*****************************************************************************/

GLOBAL VOID wi_open_mod (window)
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
	LIST_P	header, element;
	WORD		event_num, num_obj;
	SOSTATP	soundstatus;
	DISPOBJP	dispobj;
	BOOL 		found = FALSE;
	
	if (mk->breturn > 1)
	{
		event_num = max(0,(mk->moy - window->work.y) / gl_hbox);
		event_num = min(event_num, GetNumEvents(module));

		/* Entsprechendes Objekt finden */
		header = window->dispobjs;
		element = list_next(header);
		while (!found && element != header)
		{
			dispobj = (DISPOBJP) element->key;
			found = inside (mk->mox, mk->moy, &dispobj->work);
			element = list_next (element);
		} /* while not found */
		
		if (found)
		{
			soundstatus = (SOSTATP)dispobj->status;
			status->event = soundstatus->event;
			window->milli = 1;
			deditor(window);
		} /* if found */
	} /* if doppelclick */
	
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
	SET_P		akt = Akt(window);
	DISPOBJP	dispobj;
	LIST_P	header, element;
	BOOLEAN	new = status->new || (window->flags & WI_JUNK);
	WORD		num_objs, rows, cols, obj_number = 0;
	WORD		signal, obj_nr = 0;
	SOSTATP	soundstatus;
	RECT		work, a;
	SO_P		event;

	status->new = new;
	
	/* Anzahl der Objekte berechnen */
	num_objs = ComputeNumDO (window);
	if (num_objs != status->num_events)
		create_displayobs (window);

	/* Liste der Display-Objekte durchgehen und Events eintragen */
	header = window->dispobjs;
	element = list_next(header);
	while (element != header) {
		dispobj = (DISPOBJP) element->key;

		if (obj_number == 0)	GetFirstEvent (module);

		GetNextEvent(module);
			
		if (new) {
			dispobj->new	= TRUE;
			dispobj->set_uni (dispobj, DOUniSOEvent, (LONG)&status->event);
			ComputeWorkDO (window, obj_number, &work);
			dispobj->set_work (dispobj, &work);
		} /* if new */
		obj_number++;
		element = list_next(element);
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
	
	window = create_window_obj (KIND, CLASS_EC4);
	
	if (window != NULL)
	{
		WINDOW_INITWIN_OBJ
				
		window->flags     = FLAGS;
		window->icon 	   = icon;
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
		window->mousenum  = ARROW;
		window->mouseform = NULL;
		window->milli     = MILLI;
		window->special   = 0;
		window->edit_obj  = 0;
		window->edit_inx  = 0;
		window->exit_obj  = 0;
		window->object    = 0;
		window->menu      = menu;
		window->click     = wi_click_mod;
		window->draw		= wi_draw_mod;
		window->snap      = wi_snap_mod;
		window->timer     = wi_timer_mod;
		window->showinfo  = info_mod;
		window->click		= wi_click_mod;
		window->start		= wi_start_mod;
		window->finished  = wi_finished_mod;
		
		sprintf (window->name, (BYTE *)ec4_text [FEC4N].ob_spec);
		sprintf (window->info, "   Cue-Time     Entry-Time    Exit-Time   Spd  Ch Vol   X    Y  ");
	} /* if */
	
	return (window);                      /* Fenster zurckgeben */
} /* crt_mod */

PRIVATE WORD ComputeNumDO (WINDOWP window)
{
	RTMCLASSP	module = Module(window);
	SET_P			akt = Akt(window);
	WORD			num;

	return GetNumEvents(module);
} /* ComputeNumDO */

PRIVATE VOID ComputeWorkDO (WINDOWP window, WORD obj_num, RECT *work)
{
	/* setzen des Work-Bereiches eines Display-Objektes */
	SET_P		akt = Akt(window);
	RECT		*scroll = &window->scroll;
	WORD		h = gl_hbox;
	
	work->x = scroll->x ;
	work->y = scroll->y + h*obj_num;
	work->w = scroll->w;
	work->h = h;
} /* ComputeWorkDO */

PRIVATE VOID create_displayobs (WINDOWP window)
{	
	RTMCLASSP	module = Module(window);
	STAT_P	status = Status(window);
	WORD		obj_number = 0, num_signals, num_displayobs;
	WORD		signal, h = gl_hbox, w = gl_wbox, x0, y0;
	LONGSTR	s;
	LIST_P	header, element;
	DISPOBJP	dispobj;
	SOSTATP	soundstatus;
	RECT		work, a;
	SO_P		event;

	if (window->work.h > 400)
		h = 16;
	else
		h = 8;
	
	/* Liste der Display-Objekte durchgehen und l”schen */
	header = window->dispobjs;
	element = list_next(header);
	while (element != header) {
		dispobj = (DISPOBJP) element->key;
		dispobj->delete (dispobj);
		element = list_next(element);
	} /* while */

	/* Alle objekte l”schen */
	list_empty (window->dispobjs);
	
	num_displayobs = ComputeNumDO (window);

	work.x = 0;
	work.y = 0;
	work.w = window->work.w;
	work.h = window->work.h;
	
	for (obj_number = 0; obj_number < num_displayobs; obj_number++)
	{
		ComputeWorkDO (window, obj_number, &work);
		dispobj = CreateSODispobj(window, 0, 0, &work);
		soundstatus = (SOSTATP)dispobj->status;
		/* Element einsetzen */
		list_insert(window->dispobjs, list_new_el (dispobj));
	} /* for */

	status->num_events = num_displayobs;

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
	
	window = search_window (CLASS_EC4, SRCH_ANY, icon);
	
	/* Wenn nicht gefunden */
	if (window == NULL)
	{
		if (create()>0);	/* Neue Instanz */
			window = search_window (CLASS_EC4, SRCH_CLOSED, icon);
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
	WORD		ret;
	STRING	s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, IEC4);
		
	if (window == NULL)
	{
		 form_center (ec4_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (ec4_info, NULL, IEC4, (BYTE *)ec4_text [FEC4N].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = ec4_info;
		sprintf(s, "%-20s", EC4DATE);
		set_ptext (ec4_info, EC4IVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (ec4_info, EC4COMPILE, s);
		sprintf(s, "%-20s", EC4VERSION);
		set_ptext (ec4_info, EC4IVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (ec4_info, EC4ISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (ec4_info, EC4IAKT, s);

		if (! open_dialog (IEC4)) hndl_alert (ERR_NOOPEN);
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
		module->class_number		= CLASS_EC4;
		module->icon				= &ec4_desk[EC4ICON];
		module->icon_position	= IEC4;
		module->icon_number		= IEC4;	/* Soll bei Init vergeben werden */
		module->menu_title		= MED4S;
		module->menu_position	= MEC4;
		module->menu_item			= MEC4;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_ec4;
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
		window = crt_mod (ec4_cue, NULL, IEC4);
		/* Modul-Struktur einbinden */
		window->module 	= (VOID*) module;
		module->window		= window;
		
		/* Display-Objekte einklinken */
		create_displayobs (window);
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
  ec4_setup 	= (OBJECT *)rs_trindex [EC4_SETUP]; 	/* Adresse der EC4-Cue-Setup-Box */
  ec4_cue 	= (OBJECT *)rs_trindex [EC4_CUE]; 	/* Adresse der EC4-Cue-Editor-Box */
  ec4_help  = (OBJECT *)rs_trindex [EC4_HELP];	/* Adresse der EC4-Hilfe */
  ec4_desk  = (OBJECT *)rs_trindex [EC4_DESK];	/* Adresse des EC4-Desktops */
  ec4_text  = (OBJECT *)rs_trindex [EC4_TEXT];	/* Adresse der EC4-Texte */
  ec4_info 	= (OBJECT *)rs_trindex [EC4_INFO];	/* Adresse der EC4-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (ec4_rsc_ptr, rsc_name))
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

  rs_gaddr (ec4_rsc_ptr, R_TREE,  EC4_SETUP,	&ec4_setup);  	/* Adresse der EC4-Cue-Setup-Box */
  rs_gaddr (ec4_rsc_ptr, R_TREE,  EC4_CUE,	&ec4_cue);   	/* Adresse der EC4-Cue-Editor-Box */
  rs_gaddr (ec4_rsc_ptr, R_TREE,  EC4_HELP,	&ec4_help);    /* Adresse der EC4-Hilfe */
  rs_gaddr (ec4_rsc_ptr, R_TREE,  EC4_DESK,	&ec4_desk);    /* Adresse der EC4-Desktop */
  rs_gaddr (ec4_rsc_ptr, R_TREE,  EC4_TEXT,	&ec4_text);    /* Adresse der EC4-Texte */
  rs_gaddr (ec4_rsc_ptr, R_TREE,  EC4_INFO,	&ec4_info);    /* Adresse der EC4-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (ec4_setup, TRUE);
	fix_objs (ec4_cue,  TRUE);
	fix_objs (ec4_help, TRUE);
	fix_objs (ec4_desk, TRUE);
	fix_objs (ec4_text, TRUE);
	fix_objs (ec4_info, TRUE);
	
	do_flags (ec4_setup, EC4CANCEL, UNDO_FLAG);
	do_flags (ec4_setup, EC4HELP, HELP_FLAG);

	do_flags (ec4_cue, EC4CUECANCEL, UNDO_FLAG);
	/* do_flags (ec4_cue, EC4CUEHELP, HELP_FLAG); */

	menu_enable(menu, MEC4, TRUE);

	return (TRUE);
} /* init_rsc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PRIVATE BOOLEAN term_rsc ()

{
	BOOLEAN ok = TRUE;

#if ((XRSC_CREATE||RSC_CREATE) == 0)
	ok = rs_free (ec4_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

	return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_ec4 ()
{
	WORD					x;
	BOOLEAN				ok = TRUE;
	 
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

	undo_flags (desktop, IEC4, DISABLED);
	return (ok);
} /* init_ec4 */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	
	ok &= term_rsc ();
	return (ok);
} /* term_mod */

