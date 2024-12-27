/*****************************************************************************/
/*                                                                           */
/* Modul: EFF.C                                                              */
/*                                                                           */
/* Effekt-Ger„te Treiber                                                     */
/*                                                                           */
/*****************************************************************************/
#define EFFVERSION "V 0.08"
#define EFFDATE "30.01.96"

/*****************************************************************************
V 0.08
- ClickSetupField eingebaut, 30.01.95
- open_mod repariert, 24.07.94
V 0.07 19.05.94
- load_create_infos und instance_count eingebaut
- window->module eingebaut
- module->window in create
- Umbau auf create_window_obj
V 0.06
- Umstellung auf neue RTMCLASS-Struktur
*****************************************************************************/
#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE */                   /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "eff_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"

#include "export.h"
#include "eff.h"

#if XRSC_CREATE
#include "eff_mod.rsh"
#include "eff_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangsh”he in Pixel */
#define MILLI  1000                     /* Millisekunden fr Zeitablauf */

#define MOD_RSC_NAME "EFF_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 200l						/* Anzahl der EFF-Setups */
/****** TYPES ****************************************************************/
typedef	struct setup *SET_P;

typedef struct setup
{
	WORD		nummer;
} SETUP;	/* Enth„lt alle Parameter einer kompletten EFF-Einstellung */

typedef	struct	status
{
	VOID *dummy;
} STATUS;

typedef	struct status	*STAT_P;	/* Zeiger auf CMI-STATUS */
/****** VARIABLES ************************************************************/
PRIVATE WORD	eff_rsc_hdr;					/* Zeigerstruktur fr RSC-Datei */
PRIVATE WORD	*eff_rsc_ptr = &eff_rsc_hdr;		/* Zeigerstruktur fr RSC-Datei */
PRIVATE OBJECT *eff_setup;
PRIVATE OBJECT *eff_help;
PRIVATE OBJECT *eff_desk;
PRIVATE OBJECT *eff_text;
PRIVATE OBJECT *eff_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 20;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "EFF";		/* Name, fr Extension etc. */

/****** FUNCTIONS ************************************************************/

/* Interne EFF-Funktionen */
PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STRING	s;
	WORD		variable, offset;
	
	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (eff_setup, EFFSETNR , s);
} /* set_dbox */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD variable = ((MSG_SET_VAR *)msg)->variable;
	LONG	value		= ((MSG_SET_VAR *)msg)->value;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch (variable)
			{
				case VAR_SET_EFF:
					module->set_setnr(module, value);
					break;
			} /* switch */
			break;
	} /* switch */
} /* message */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	/* Vorausberechnung */
} /* precalc */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	send_variable(VAR_SET_EFF, module->actual->number);
} /* send_messages */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
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

	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case EFFSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case EFFSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case EFFSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case EFFSETSTORE:
			module->set_store (window, edited->number);
			break;
		case EFFSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case EFFOK   :
			module->set_ok (window);
			break;
		case EFFCANCEL:
			module->set_cancel (window);
		   break;
		case EFFHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case EFFSTANDARD:
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
				set_ptext (eff_setup, EFFSETNR, s);
				draw_object(window, EFFSETNR);
			} /* if */
			undo_state (window->object, window->exit_obj, SELECTED);
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

  inx    = num_windows (CLASS_EFF, SRCH_OPENED, NULL);
  window = create_window_obj (KIND, CLASS_EFF);

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
    window->click     = wi_click_mod;
    window->showinfo  = info_mod;
	
    sprintf (window->name, (BYTE *)eff_text [FEFFN].ob_spec);
    sprintf (window->info, (BYTE *)eff_text [FEFFI].ob_spec, 0);
    
  } /* if */

  return (window);                      /* Fenster zurckgeben */
} /* crt_mod */

/*****************************************************************************/
/* ™ffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN 	ok;
	WINDOWP 	window;
	
	/* Fenster suchen */
	window = search_window (CLASS_EFF, SRCH_CLOSED, icon);
	/* Wenn nicht gefunden */
	if (window == NULL)
	{
		if (create()>0);	/* Neue Instanz */
			window = search_window (CLASS_EFF, SRCH_CLOSED, icon);
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
	RTMCLASSP	module = (RTMCLASSP)window->module;
	WORD		ret;
	STRING	s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, IEFF);
		
	if (window == NULL)
	{
		 form_center (eff_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (eff_info, NULL, IEFF, (BYTE *)eff_text [FEFFN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = eff_info;
		sprintf(s, "%-20s", EFFDATE);
		set_ptext (eff_info, EFFIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (eff_info, EFFCOMPILE, s);
		sprintf(s, "%-20s", EFFVERSION);
		set_ptext (eff_info, EFFIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (eff_info, EFFISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (eff_info, EFFIAKT, s);

		if (! open_dialog (IEFF)) hndl_alert (ERR_NOOPEN);
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

	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_EFF;
		module->icon				= &eff_desk[EFFICON];
		module->icon_position 	= IEFF;
		module->icon_number		= IEFF;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MEFF;
		module->menu_item			= MEFF;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_eff;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_OUTPUT;
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
		module->max_setups		 		= MAXSETUPS;
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
		/* Prfen, ob DEFAULT-Datei vorhanden */
		if((fp=fopen(module->file_name, "rb"))!=0)
		{
			/* Wenn vorhanden, laden */
			fclose(fp);
			module->load(module, module->file_name, FALSE);
		} /* if */

		/* Setup-Strukturen initialisieren */
		mem_set(module->standard, 0, (UWORD) sizeof(SETUP));
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (eff_setup, NULL, EFF_SETUP);
		/* Modul-Struktur einbinden */
		window->module = (VOID*) module;
		module->window = window;
		
		add_rcv(VAR_SET_EFF, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_EFF, MAXSETUPS);
		/* add_rcv(VAR_PROP_EFF, module);	/* Message einklinken */ */
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
  eff_setup = (OBJECT *)rs_trindex [EFF_SETUP]; /* Adresse der EFF-Parameter-Box */
  eff_help  = (OBJECT *)rs_trindex [EFF_HELP];	/* Adresse der EFF-Hilfe */
  eff_desk  = (OBJECT *)rs_trindex [EFF_DESK];	/* Adresse des EFF-Desktops */
  eff_text  = (OBJECT *)rs_trindex [EFF_TEXT];	/* Adresse der EFF-Texte */
  eff_info 	= (OBJECT *)rs_trindex [EFF_INFO];	/* Adresse der EFF-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (eff_rsc_ptr, rsc_name))
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

  rs_gaddr (eff_rsc_ptr, R_TREE,  EFF_SETUP,	&eff_setup);   /* Adresse der EFF-Parameter-Box */
  rs_gaddr (eff_rsc_ptr, R_TREE,  EFF_HELP,	&eff_help);    /* Adresse der EFF-Hilfe */
  rs_gaddr (eff_rsc_ptr, R_TREE,  EFF_DESK,	&eff_desk);    /* Adresse des EFF-Desktop */
  rs_gaddr (eff_rsc_ptr, R_TREE,  EFF_TEXT,	&eff_text);    /* Adresse der EFF-Texte */
  rs_gaddr (eff_rsc_ptr, R_TREE,  EFF_INFO,	&eff_info);    /* Adresse der EFF-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (eff_setup, TRUE);
	fix_objs (eff_help, TRUE);
	fix_objs (eff_desk, TRUE);
	fix_objs (eff_text, TRUE);
	fix_objs (eff_info, TRUE);
	
	
	do_flags (eff_setup, EFFCANCEL, UNDO_FLAG);
	do_flags (eff_setup, EFFHELP, HELP_FLAG);
	
	menu_enable(menu, MEFF, TRUE);

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
  ok = rs_free (eff_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_eff ()
{
	BOOLEAN ok = TRUE;
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);
	
	return (ok);
} /* init_eff */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
  BOOLEAN ok = TRUE;
  ok &= term_rsc ();
  return (ok);
} /* term_mod */
