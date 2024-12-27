/*****************************************************************************/
/*                                                                           */
/* Modul: SPD.C                                                           	  */
/*                                                                           */
/* Koordinaten-Anzeige                                                       */
/*                                                                           */
/*****************************************************************************/
#define SPDVERSION "V 0.01"
#define SPDDATE "15.01.95"

/*****************************************************************************
V 0.01
- Ersterstellung
*****************************************************************************/

#ifndef XRSC_CREATE
/*#define XRSC_CREATE 1*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "spd_mod.h"
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
#include "spd.h"

#if XRSC_CREATE
#include "spd_mod.rsh"
#include "spd_mod.rh"
#endif

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT | WI_MOUSE)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangshîhe in Pixel */
#define MILLI  0                     	/* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "SPD_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 1l						/* Anzahl der SPD-Setups */
/****** TYPES ****************************************************************/
typedef struct setup *SET_P;

typedef struct setup
{
	/* Anzeige-Flags */
	UINT	mtr_spd	: 1	;	/* MTR Speed an/aus  */
	UINT	lfa_spd	: 1	;	/* LFB Speed an/aus  */
	UINT	lfb_spd	: 1	;	/* LFA Speed an/aus  */
} SETUP;		/* EnthÑlt alle Parameter einer kompletten SPD-Einstellung */

typedef struct status	
{
	BOOLEAN	new;				/* Hat sich etwas geÑndert ? */
} STATUS;

typedef struct status *STAT_P;	

/****** VARIABLES ************************************************************/
/* Resource */
PRIVATE WORD	spd_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*spd_rsc_ptr = &spd_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *spd_setup;
PRIVATE OBJECT *spd_help;
PRIVATE OBJECT *spd_desk;
PRIVATE OBJECT *spd_text;
PRIVATE OBJECT *spd_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "SPD";		/* Name, fÅr Extension etc. */

/****** FUNCTIONS ************************************************************/
PRIVATE VOID create_displayobs (WINDOWP window);

PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID get_dbox (RTMCLASSP module)
{	
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	UWORD 	signal;

} /* get_dbox */

/*****************************************************************************/

PRIVATE VOID set_dbox (RTMCLASSP module)
{	
			ED_P	actual = module->actual;
	REG 	SET_P	akt = actual->setup;
			ED_P	edited = module->edited;
	REG 	SET_P	ed = edited->setup;
	REG 	UWORD signal;
			BOOLEAN draw = FALSE;
	REG	WINDOWP	window = module->window;
	
	/* Neue Daten Åbernehmen */
	mem_move(ed, akt, (UWORD)module->setup_length);

} /* set_dbox */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	ED_P		actual = module->actual;
	SET_P		akt = actual->setup;
	UWORD 	signal;

/*
	send_variable(VAR_SET_PAR, actual->number);

} /* send_messages */
/*****************************************************************************/

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD 	variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	SET_P		akt = module->actual->setup;
	STAT_P	status = module->status;

	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch(variable)
			{
				case VAR_SET_SPD :		module->set_setnr(module, value);	break;
			} /* switch */
			module->set_dbox(module);
			break;
	} /* switch */
} /* message */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
WINDOWP window;
MKINFO  *mk;
{
	RTMCLASSP	module = Module(window);
	ED_P			edited = module->edited;

	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */

	module->get_dbox(module);
	module->get_edit(module);

} /* wi_click_mod */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (window)
WINDOWP window;
{
} /* wi_timer_mod */

/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;
{
	WINDOWP	window;
	WORD		menu_height;
	
	window = create_window_obj (KIND, CLASS_SPD);
	
	
	if (window != NULL)
	{

		WINDOW_INITOBJ_OBJ
				
		window->flags     = FLAGS | WI_MODELESS;
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
		window->mousenum  = POINT_HAND;
		window->mouseform = NULL;
		window->milli     = MILLI;
		window->special   = 0;
		window->edit_obj  = 0;
		window->edit_inx  = 0;
		window->exit_obj  = 0;
		window->object    = obj;
		window->menu      = menu;
		window->click     = wi_click_mod;
		window->timer     = wi_timer_mod;
		window->showinfo  = info_mod;
		
		sprintf (window->name, (BYTE *)spd_text [FSPDN].ob_spec);
		/* sprintf (window->info, (BYTE *)freetext [FSPDI].ob_spec, 0); */
	
		create_displayobs (window);

	} /* if */
	
	return (window);                      /* Fenster zurÅckgeben */
} /* crt_spd */

PRIVATE VOID create_displayobs (WINDOWP window)
{	
	WORD		signal, lfo, h = gl_hbox, w = gl_wbox, x0, y0;
	LONGSTR	s;
	RECT		a;

	CrtBarDOInsert (window, SPDLFAUMKEHR, VAR_LFA_UMK, ObjectCheck, 0);
	CrtObjectDOInsert (window, SPDLFAPAUSE, VAR_LFA_PAUSE, ObjectCheck, 0);
	CrtObjectDOInsert (window, SPDLFBUMKEHR, VAR_LFB_UMK, ObjectCheck, 0);
	CrtObjectDOInsert (window, SPDLFBPAUSE, VAR_LFB_PAUSE, ObjectCheck, 0);

	for(signal=0; signal < MAXSIGNALS; signal++)
	{
		CrtObjectDOInsert (window, SPDMTR0 + signal, VAR_MTR_ON0 + signal, ObjectCheck, 0);
		CrtObjectDOInsert (window, SPDPLAY0 + signal, VAR_PUF_PLAY_SIG0 + signal, ObjectCheck, 0);
		CrtObjectDOInsert (window, SPDREC0 + signal, VAR_PUF_REC_SIG0 + signal, ObjectCheck, 0);
	} /* for */
	CrtObjectDOInsert (window, SPDMTRPAUSE, VAR_MTR_PAUSE, ObjectCheck, 0);
	CrtObjectDOInsert (window, SPDMTRUMKEHR, VAR_MTR_UMK, ObjectCheck, 0);
	CrtObjectDOInsert (window, SPDPUFPAUSE, VAR_PUF_PAUSE, ObjectCheck, 0);
	CrtObjectDOInsert (window, SPDPUFZEITLUPE, VAR_PUF_ZEITLUPE, ObjectCheck, 0);

	CrtObjectDOInsert (window, SPDMAASPIN, VAR_MAA_SPERRE_INNEN, ObjectCheck, 0);
	CrtObjectDOInsert (window, SPDMAASPOUT, VAR_MAA_SPERRE_AUSSEN, ObjectCheck, 0);
	CrtObjectDOInsert (window, SPDMAESPIN, VAR_MAE_SPERRE_INNEN, ObjectCheck, 0);
	CrtObjectDOInsert (window, SPDMAESPOUT, VAR_MAE_SPERRE_AUSSEN, ObjectCheck, 0);
} /* create_displayobs */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN ok;
	WINDOWP window;
	RTMCLASSP	module;
	
	window = search_window (CLASS_SPD, SRCH_ANY, SPD_SETUP);
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (spd_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			module = Module(window);
			
			module->set_edit (module);
			module->set_dbox (module);
			if (! open_window (window)) 
				hndl_alert (ERR_NOOPEN);
			else
				window->opened = 1;
		} /* if */
		else
			top_window (window);
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

	window = search_window (CLASS_DIALOG, SRCH_ANY, ISPD);
		
	if (window == NULL)
	{
		 form_center (spd_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (spd_info, NULL, ISPD, (BYTE *)spd_text [FSPDN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = spd_info;
		sprintf(s, "%-20s", SPDDATE);
		set_ptext (spd_info, SPDIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (spd_info, SPDCOMPILE, s);
		sprintf(s, "%-20s", SPDVERSION);
		set_ptext (spd_info, SPDIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (spd_info, SPDISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (spd_info, SPDIAKT, s);

		if (! open_dialog (ISPD)) hndl_alert (ERR_NOOPEN);
	}

  return (window != NULL);
} /* info_mod */

/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	WINDOWP		window;
	RTMCLASSP 	module;
	FILE			*fp;
	WORD			x;
	
	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_SPD;
		module->icon				= &spd_desk[SPDICON];
		module->icon_position 	= ISPD;
		module->icon_number		= ISPD;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MSPD;
		module->menu_item			= MSPD;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_spd;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_CONTROL;
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
		/* PrÅfen, ob DEFAULT-Datei vorhanden */
		if((fp=fopen(module->file_name, "rb"))!=0)
		{
			/* Wenn vorhanden, laden */
			fclose(fp);
			module->load(module, module->file_name, FALSE);
		} /* if */

		/* Setup-Strukturen initialisieren */
		mem_set(module->standard, 0, (UWORD) sizeof(SETUP));
		mem_set(module->actual->setup, 0, (UWORD) sizeof(SETUP));
		mem_set(module->edited->setup, -1, (UWORD) sizeof(SETUP));
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (spd_setup, NULL, SPD_SETUP);
		/* Modul-Struktur einbinden */
		window->module = (VOID*) module;
		module->window = window;
				
		add_rcv(VAR_SET_SPD, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_SPD, MAXSETUPS);
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
  spd_setup = (OBJECT *)rs_trindex [SPD_SETUP]; /* Adresse der SPD-Parameter-Box */
  spd_help  = (OBJECT *)rs_trindex [SPD_HELP];	/* Adresse der SPD-Hilfe */
  spd_desk  = (OBJECT *)rs_trindex [SPD_DESK];	/* Adresse des SPD-Desktops */
  spd_text  = (OBJECT *)rs_trindex [SPD_TEXT];	/* Adresse der SPD-Texte */
  spd_info 	= (OBJECT *)rs_trindex [SPD_INFO];	/* Adresse der SPD-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (spd_rsc_ptr, rsc_name))
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

  rs_gaddr (spd_rsc_ptr, R_TREE,  SPD_SETUP,	&spd_setup);   /* Adresse der SPD-Parameter-Box */
  rs_gaddr (spd_rsc_ptr, R_TREE,  SPD_HELP,	&spd_help);    /* Adresse der SPD-Hilfe */
  rs_gaddr (spd_rsc_ptr, R_TREE,  SPD_DESK,	&spd_desk);    /* Adresse der SPD-Desktop */
  rs_gaddr (spd_rsc_ptr, R_TREE,  SPD_TEXT,	&spd_text);    /* Adresse der SPD-Texte */
  rs_gaddr (spd_rsc_ptr, R_TREE,  SPD_INFO,	&spd_info);    /* Adresse der SPD-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (spd_setup, TRUE);
	fix_objs (spd_help, TRUE);
	fix_objs (spd_desk, TRUE);
	fix_objs (spd_text, TRUE);
	fix_objs (spd_info, TRUE);
	
	/*
	do_flags (spd_setup, SPDCANCEL, UNDO_FLAG);
	do_flags (spd_setup, SPDHELP, HELP_FLAG);
	*/
	menu_enable(menu, MSPD, TRUE);

	return (TRUE);
} /* init_rsc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PRIVATE BOOLEAN term_rsc ()

{
	BOOLEAN ok = TRUE;

#if ((XRSC_CREATE||RSC_CREATE) == 0)
	ok = rs_free (spd_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

	return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_spd ()
{
	BOOLEAN				ok = TRUE;
	 
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

	return (ok);
} /* init_spd */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	
	ok &= term_rsc ();
	return (ok);
} /* term_mod */
