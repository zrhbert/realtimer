/*****************************************************************************/
/*                                                                           */
/* Modul: CMI.C                                                              */
/*                                                                           */
/* Cybermove-Input-Zuweisung                                                 */
/*                                                                           */
/*****************************************************************************/
#define CMIVERSION "V 1.06"
#define CMIDATE "05.02.95"

/*****************************************************************************
V 1.06
- auf GetPxxx/SetPxxx umgestellt, 05.02.95
- minmax fÅr ctrl-port und channel, 02.02.95
- ClickSetupField eingebaut, 30.01.95
V 1.05
- load_create_infos und instance_count eingebaut
- Ausgabe von VAR_CMI_SIGNAL1..8
V 1.04
- Numerierung der Channels, Ports und Tracks intern ab 0 .. 
- Import umgebaut
- Numerierung umgebaut auf 0 .. 63
- window->module eingebaut
- Umbau auf create_window_obj
V 1.03
- Bug in create (module->window) beseitigt
V 1.02
- Umstellung auf neue RTMCLASS-Struktur
V 1.01
- Dialogbox sortiert
- Standard-Port nun 0 und 1
- Port/Channel manipulation in wi_click rep.

V 1.00 18.04.93
- Einstellung fÅr CMI-Ports und Channels eingebaut

*****************************************************************************/
#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "cmi_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "msh_unit.h"
#include "msh.h"

#include "objects.h"

#include "export.h"
#include "cmi.h"

#if XRSC_CREATE
#include "cmi_mod.rsh"
#include "cmi_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangshîhe in Pixel */
#define MILLI  1000                     /* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "CMI_MOD.RSC"		/* Name der Resource-Datei */
#define MAXSETUPS  200l			/* Anzahl der CM-Input-Setups */
#define SELMUSTER 0x0A1				/* Muster fÅr belegte Inputs in Setup-Box */
											/* StÑrke 2, Text durchsichtig, Farbe schwarz */

#define Object(input) \
	(input<MAXINPUTS) ? CMICM1INP1 + input : CMICM2INP1 + input - MAXINPUTS
/****** TYPES ****************************************************************/
typedef	struct	setup
{
	WORD	input [MAXSIGNALS];	/* Nummer des CM-Inputs */
} SETUP;

typedef	struct	setup	*SET_P;	/* Zeiger auf CMI-Setup */

typedef	struct	status
{
	WORD	channel1,				/* Ausgabekanal fÅr erstes CMI-System */
			channel2,				/* Ausgabekanal fÅr zweites CMI-System */
			port1,					/* Ausgabeanschluû fÅr erstes CMI-System */
			port2;					/* Ausgabeanschluû fÅr zweites CMI-System */
} STATUS;

typedef	struct status	*STAT_P;	/* Zeiger auf CMI-STATUS */

/****** VARIABLES ************************************************************/
PRIVATE WORD	cmi_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*cmi_rsc_ptr = &cmi_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *cmi_setup;
PRIVATE OBJECT *cmi_help;
PRIVATE OBJECT *cmi_desk;
PRIVATE OBJECT *cmi_text;
PRIVATE OBJECT *cmi_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "CMI";		/* Name, fÅr Extension etc. */

/****** FUNCTIONS ************************************************************/

/* Interne CMI-Funktionen */
PRIVATE VOID	set_dbox_signal _((RTMCLASSP module, UWORD aktsignal));
PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STAT_P	status = module->status;
	UWORD		signal, in, aktsignal;
	BOOLEAN	found = FALSE;
	WORD		object;
	
	/* AngewÑhltes Signal ermitteln */
	for (signal = 1; signal < MAXSIGNALS; signal++)
		if(get_checkbox (cmi_setup, CMISIGNAL1  + signal-1))
			aktsignal = signal;

	/* Zugewiesenen Input auf Null (nicht zugewiesen) setzen */
	ed->input[aktsignal] = 0;
	
	/* Numerierung der Channels, Ports und Tracks intern ab 0 .. */
	for (in = 0; !found && in < MAXINPUTS*2; in++)
	{
		object = Object(in);
		if(get_checkbox (cmi_setup, object))
		{
			/* Falls Signal zugewiesen wurde */
			/* Entsprechenden Input eintragen */
			ed->input[aktsignal] = in;
			found = TRUE;
		} /* if */
	} /* for */
	status->port1 		= GetPWord (cmi_setup, CMICM1PORTNR, NULL) - 1;
	status->channel1 	= GetPWord (cmi_setup, CMICM1CHANNELNR, NULL) - 1;
	status->port1 		= minmax(status->port1, 0, 31);
	status->channel1 	= minmax(status->channel1, 0, 15);

	status->port2 		= GetPWord (cmi_setup, CMICM2PORTNR, NULL) - 1;
	status->channel2 	= GetPWord (cmi_setup, CMICM2CHANNELNR, NULL) - 1;
	status->port2 		= minmax(status->port2, 0, 31);
	status->channel2 	= minmax(status->channel2, 0, 15);

} /* get_dbox */

PRIVATE VOID set_fillbox (OBJECT *tree, WORD object, BOOL selected)
{
	/* Set the fill pattern in a box to grey or white */
	if (selected)
		((TEDINFO *)tree[object].ob_spec)->te_color |= SELMUSTER;
	else
		((TEDINFO *)tree[object].ob_spec)->te_color &= ~SELMUSTER;
} /* set_fillbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	STAT_P	status = module->status;
	STRING 	s, format = "%2d";

	set_dbox_signal(module, 1);

	/* Numerierung der Channels, Ports und Tracks intern ab 0 .. */
	SetPWordN (cmi_setup, CMICM1PORTNR, status->port1+1);
	SetPWordN (cmi_setup, CMICM1CHANNELNR, status->channel1+1);
	SetPWordN (cmi_setup, CMICM2PORTNR, status->port2+1);
	SetPWordN (cmi_setup, CMICM2CHANNELNR, status->channel2+1);

	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (cmi_setup, CMISETNR , s);
} /* set_dbox */

PRIVATE VOID    set_dbox_signal	(RTMCLASSP module, UWORD aktsignal)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STRING	s;
	UWORD		signal, input, in;
	WORD		object;
	
	/* AngewÑhltes Signal selektieren, alle anderen deselektieren */
	for (signal = 1; signal < MAXSIGNALS; signal++)
		set_checkbox (cmi_setup, CMISIGNAL1  + signal -1, aktsignal == signal);
		
	/* Numerierung der Channels, Ports und Tracks intern ab 0 .. */

	/* Alle Muster abschalten */
	for (in = 0; in < MAXINPUTS*2; in++)
	{
		object = Object(in);
		set_fillbox (cmi_setup, object, FALSE);
		set_checkbox (cmi_setup, object, FALSE);
	} /* for */

	/* Alle Signale durchgehen */
	/* Jedes Signal markiert seine Zuweisung in schwarz oder grau */
	for (signal = 1; signal < MAXSIGNALS; signal++)
	{
		/* Offset des DBox-Objektes des zugewiesenen Inputs */
		input = ed->input[signal];
		if (input < MAXINPUTS*2)
		{
			object = Object(input);
			if (signal == aktsignal)
			{
				/* Box selektieren, weil sie zum aktuellen Signal gehîrt */
				set_checkbox (cmi_setup, object, TRUE);
			} /* if */
			else
			{
				/* Box grau machen, weil sie zu einem nicht akt. Sig. gehîrt */
				set_fillbox (cmi_setup, object, TRUE);
			} /* else */
		} /* if input */
	} /* for */

} /* set_dbox_signal */

PRIVATE VOID    send_messages	(RTMCLASSP module)
{
	STAT_P	status = module->status;
	WORD 		*input = module->actual->setup->input;
	WORD		signal;
			
	/* Numerierung der Channels, Ports und Tracks intern ab 0 .. */
	/* ZulÑssigkeit der Port-Anwahl ÅberprÅfen */
	if (MidiShare())
	{
		if (!MidiGetPortState(status->port1))
			status->port1 = 0;
		if (!MidiGetPortState(status->port2))
			status->port2 = 0;
	} /* if */

	send_variable(VAR_SET_CMI, module->actual->number);
	send_variable(VAR_CMI_CHANNEL1, status->channel1);
	send_variable(VAR_CMI_CHANNEL2, status->channel2);
	send_variable(VAR_CMI_PORT1, status->port1);
	send_variable(VAR_CMI_PORT2, status->port2);
	
	for (signal = 1; signal < MAXSIGNALS; signal++)
		send_variable(VAR_CMI_SIGNAL1 + signal - 1, input[signal]);
	
} /* send_messages */

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	/* Input-Zuordnung in Event hineinkopieren */
	WORD *track = event->tracks->track;
	WORD *input = module->actual->setup->input;
	WORD signal;
	
	if(event)					/* Event gÅltig? */
		if(track && input)	/* Track-Information gÅltig? */
			for (signal = 1; signal < MAXSIGNALS; signal++)
				track[signal] = input[signal];

	return (event);
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	/* ZurÅcksetzen von Werten */
} /* reset */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	/* Vorausberechnung */
} /* precalc */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD variable = ((MSG_SET_VAR *)msg)->variable;
	LONG	value		= ((MSG_SET_VAR *)msg)->value;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch (variable)
			{
				case VAR_SET_CMI:
					module->set_setnr(module, value);
					break;
			} /* switch */
			break;
	} /* switch */
} /* message */

PUBLIC BOOLEAN	import	(RTMCLASSP module, STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	STRING	ext, title, filter;
	FILE		*in;
	STR128	s;
	SET_P		akt = module->actual->setup; /* Zeiger auf erstes Setup */
	WORD		signal, setnr = 0, *input = akt->input;
	LONG		max_setups = module->max_setups;
		
	if (filename == NULL)
	{
		filename = s;
		strcpy(filename, module->import_name);
	} /* if */

	if (fileselect)
	{
		file_split (filename, NULL, import_path, filename, NULL);
		strcpy(ext, "EXP");
		sprintf (filter, "*.%s", ext);
		sprintf (title, "%s-Datei importieren", module->object_name);
		ok = select_file (filename, import_path, filter, title, module->import_name);
	} /* if */
	
	if (ok)
	{
		in = fopen(module->import_name, "rb");
		if(in == 0)
		{	
	      hndl_alert_obj (module, ERR_FIMPORTOPEN);
	      ok = FALSE;
		} /* if */
		else
		{
			module->import_status |= FILE_OPENED;
			akt = module->actual->setup; /* Zeiger auf erstes Setup nochmal holen, wegen Supervisor-MÅll in file_split */
			
			daktstatus(" CMI-Datei wird importiert ... ", module->import_name);
			module->flags |= FLAG_IMPORTING;
			while (ok != EOF && setnr < max_setups)
			{
				input = akt->input;
				/* Zeiger auf Info des ersten Signals */
				for (signal = 0; signal < MAXSIGNALS; signal++)
				{
					ok = fscanf(in, "%d", &input[signal]);
					if (ok) input[signal] -= 1;
				} /* for */
				/* ok = fscanf(in, "%s", s);	/* Leerzeile */ */
			} /* while */
			module->actual->modified = TRUE;
			module->flags &= ~FLAG_IMPORTING;
			fclose(in);
			module->import_status &= ~FILE_OPENED;
			set_daktstat(100);
			close_daktstat();
		} /* else */
	} /* if */

	return (ok);
} /* import */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click (window, mk)
WINDOWP window;
MKINFO  *mk;
{
	WORD			i, item, variable;
	STRING		s;
	UWORD			signal, offset;
	static		LONG x = 0;	
	WORD			event, ret;
	RTMCLASSP	module = Module(window);
	ED_P			edited = module->edited;
	SET_P			ed = edited->setup;
	STAT_P		status = module->status;
	BOOLEAN		found;
	
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case CMISETINC:
			module->set_nr (window, edited->number+1);
			break;
		case CMISETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case CMISETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case CMISETSTORE:
			module->set_store (window, edited->number);
			break;
		case CMISETRECALL:
			module->set_recall (window, edited->number);
			break;
		case CMIOK   :
			module->set_ok (window);
			break;
		case CMICANCEL:
			module->set_cancel (window);
		   break;
		case CMIHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case CMISTANDARD:
			module->set_standard (window);
		   break;
		case ROOT:
		case NIL:
			break;
		case CMICM1PORTINC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->port1++;
			max(status->port1, 0);
			module->set_dbox(module);
			draw_object(window, CMICM1PORTINC);
			draw_object(window, CMICM1PORTNR);
			break;
		case CMICM1PORTDEC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->port1--;
			max(status->port1, 0);
			module->set_dbox(module);
			draw_object(window, CMICM1PORTDEC);
			draw_object(window, CMICM1PORTNR);
			break;
		case CMICM1PORTNR:
			undo_state (window->object, window->exit_obj, SELECTED);
			module->get_dbox(module);
			draw_object(window, CMICM1PORTNR);
			break;
		case CMICM2PORTINC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->port2++;
			max(status->port2, 0);
			module->set_dbox(module);
			draw_object(window, CMICM2PORTINC);
			draw_object(window, CMICM2PORTNR);
			break;
		case CMICM2PORTDEC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->port2--;
			max(status->port2, 0);
			module->set_dbox(module);
			draw_object(window, CMICM2PORTDEC);
			draw_object(window, CMICM2PORTNR);
			break;
		case CMICM2PORTNR:
			undo_state (window->object, window->exit_obj, SELECTED);
			module->get_dbox(module);
			draw_object(window, CMICM2PORTNR);
			break;
		case CMICM1CHANNELINC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->channel1++;
			max(status->channel1, 0);
			module->set_dbox(module);
			draw_object(window, CMICM1CHANNELINC);
			draw_object(window, CMICM1CHANNELNR);
			break;
		case CMICM1CHANNELDEC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->channel1--;
			max(status->channel1, 0);
			module->set_dbox(module);
			draw_object(window, CMICM1CHANNELDEC);
			draw_object(window, CMICM1CHANNELNR);
			break;
		case CMICM1CHANNELNR:
			undo_state (window->object, window->exit_obj, SELECTED);
			module->get_dbox(module);
			draw_object(window, CMICM1CHANNELNR);
			break;
		case CMICM2CHANNELINC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->channel2++;
			max(status->channel2, 0);
			module->set_dbox(module);
			draw_object(window, CMICM2CHANNELINC);
			draw_object(window, CMICM2CHANNELNR);
			break;
		case CMICM2CHANNELDEC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->channel2--;
			max(status->channel2, 0);
			module->set_dbox(module);
			draw_object(window, CMICM2CHANNELDEC);
			draw_object(window, CMICM2CHANNELNR);
			break;
		case CMICM2CHANNELNR:
			undo_state (window->object, window->exit_obj, SELECTED);
			module->get_dbox(module);
			draw_object(window, CMICM2CHANNELNR);
			break;
		default :	
			if(edited->modified == FALSE)
			{
				edited->modified = TRUE;
				sprintf(s, "%ld*", edited->number);
				set_ptext (cmi_setup, CMISETNR, s);
				draw_object(window, CMISETNR);
			} /* if */
			found = FALSE;
			for (signal = 1; !found && signal < MAXSIGNALS; signal++)
				if(window->exit_obj == CMISIGNAL1 + signal-1) 
				{
					set_dbox_signal(module, signal);
					found = TRUE;
				} /* if */	
			draw_object(window, ROOT);
			/* undo_state (window->object, window->exit_obj, SELECTED); */
			module->get_dbox(module);	/* Aktuelle Signal-Zuweisung Åbernehmen */
	} /* switch */
} /* wi_click */


/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
  WINDOWP window;
  WORD    menu_height, inx;

  window = search_window (CLASS_CMI, SRCH_ANY, icon);
  
  if (window == NULL)
  {
	 window = create_window_obj (KIND, CLASS_CMI);
  }

  if (window != NULL)
  {
		WINDOW_INITOBJ_OBJ

    window->flags     = FLAGS | WI_MODELESS;
    window->icon      = icon;
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
    window->mousenum  = ARROW;
    window->mouseform = NULL;
    window->milli     = MILLI;
    window->special   = 0;
    window->object    = obj;
    window->menu      = menu;
    window->click     = wi_click;
    window->showinfo  = info_mod;
	
    sprintf (window->name, (BYTE *)cmi_text [FCMIN].ob_spec);
    sprintf (window->info, (BYTE *)cmi_text [FCMII].ob_spec, 0);
    
  } /* if */

  return (window);                      /* Fenster zurÅckgeben */
} /* crt_mod */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN ok;
	WINDOWP window;
	WORD    ret;
	RTMCLASSP	module;
	
	window = search_window (CLASS_CMI, SRCH_ANY, CMI_SETUP);
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (cmi_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			module = Module(window);
			
			module->set_edit (module);
			module->set_dbox (module);
		} /* if */
		
		if (! open_window (window)) hndl_alert (ERR_NOOPEN);
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
	WORD			ret;
	STRING		s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, ICMI);
		
	if (window == NULL)
	{
		 form_center (cmi_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (cmi_info, NULL, ICMI, (BYTE *)cmi_text [FCMIN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = cmi_info;
		sprintf(s, "%-20s", CMIDATE);
		set_ptext (cmi_info, CMIIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (cmi_info, CMICOMPILE, s);
		sprintf(s, "%-20s", CMIVERSION);
		set_ptext (cmi_info, CMIIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (cmi_info, CMIISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (cmi_info, CMIIAKT, s);

		if (! open_dialog (ICMI)) hndl_alert (ERR_NOOPEN);
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
	SET_P			standard;
	STAT_P		status;
	WORD			signal;
		
	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_CMI;
		module->icon				= &cmi_desk[CMIICON];
		module->icon_position 	= ICMI;
		module->icon_number		= ICMI;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MCMI;
		module->menu_item			= MCMI;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_cmi;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_CALC;
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
		module->max_setups		 	= MAXSETUPS;
		module->standard				= (SET_P)mem_alloc(sizeof(SETUP));
		module->actual->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->edited->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->status 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->stat_alt 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->load					= load_obj;
		module->save					= save_obj;
		module->import					= import;
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

		/* Status initialisieren */
		status = module->status;
		mem_set(module->status, 0, (UWORD) sizeof(STATUS));
		/* Numerierung der Channels, Ports und Tracks intern ab 0 .. */
		status->channel1	= 15;
		status->channel2	= 14;
		status->port1		= 0;
		status->port2		= 1;
		
		/* Setup-Strukturen initialisieren */
		standard = module->standard;
		mem_set(standard, 0, (UWORD) sizeof(SETUP));
		for (signal = 1; signal< MAXSIGNALS; signal++)
			standard->input[signal] = signal - 1;
	
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (cmi_setup, NULL, CMI_SETUP);
		/* Modul-Struktur einbinden */
		window->module 	= (VOID*) module;
		module->window		= window;
		
		add_rcv(VAR_CMI_CHANNEL1, module);	/* Message einklinken */
		add_rcv(VAR_CMI_CHANNEL2, module);	/* Message einklinken */
		add_rcv(VAR_CMI_PORT1, module);	/* Message einklinken */
		add_rcv(VAR_CMI_PORT2, module);	/* Message einklinken */
		add_rcv(VAR_SET_CMI, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_CMI, MAXSETUPS);
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
  cmi_setup = (OBJECT *)rs_trindex [CMI_SETUP]; /* Adresse der CMI-Parameter-Box */
  cmi_help  = (OBJECT *)rs_trindex [CMI_HELP];	/* Adresse der CMI-Hilfe */
  cmi_desk  = (OBJECT *)rs_trindex [CMI_DESK];	/* Adresse des CMI-Desktops */
  cmi_text  = (OBJECT *)rs_trindex [CMI_TEXT];	/* Adresse der CMI-Texte */
  cmi_info 	= (OBJECT *)rs_trindex [CMI_INFO];	/* Adresse der CMI-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (cmi_rsc_ptr, rsc_name))
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

  rs_gaddr (cmi_rsc_ptr, R_TREE,  CMI_SETUP,	&cmi_setup);   /* Adresse der CMI-Parameter-Box */
  rs_gaddr (cmi_rsc_ptr, R_TREE,  CMI_HELP,	&cmi_help);    /* Adresse der CMI-Hilfe */
  rs_gaddr (cmi_rsc_ptr, R_TREE,  CMI_DESK,	&cmi_desk);    /* Adresse des CMI-Desktop */
  rs_gaddr (cmi_rsc_ptr, R_TREE,  CMI_TEXT,	&cmi_text);    /* Adresse der CMI-Texte */
  rs_gaddr (cmi_rsc_ptr, R_TREE,  CMI_INFO,	&cmi_info);    /* Adresse der CMI-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (cmi_setup, TRUE);
	fix_objs (cmi_help, TRUE);
	fix_objs (cmi_desk, TRUE);
	fix_objs (cmi_text, TRUE);
	fix_objs (cmi_info, TRUE);
	
	
	do_flags (cmi_setup, CMICANCEL, UNDO_FLAG);
	do_flags (cmi_setup, CMIHELP, HELP_FLAG);
	
	menu_enable(menu, MCMI, TRUE);

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
  ok = rs_free (cmi_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_cmi ()
{
	STR128 	s;
	FILE		*test;
	BOOLEAN	ok = TRUE;
	WORD		signal;
	
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);
	return (ok);
} /* init_cmi */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	ok &= term_rsc ();
	return (ok);
} /* term_mod */
