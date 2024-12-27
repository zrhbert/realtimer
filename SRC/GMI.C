/*****************************************************************************/
/*                                                                           */
/* Modul: GMI.C                                                              */
/*                                                                           */
/*****************************************************************************/
#define GMIVERSION "V 1.01"
#define GMIDATE "30.01.95"

/*****************************************************************************
V 1.01
- wi_click_mod ”ffnet nun die entsprechenden Editoren bei click auf die Namen, 24.01.95
V 1.00
- GEN-Mini aus GEN herausgel”st
*****************************************************************************/
#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "gmi_mod.h"
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
#include "gmi.h"

#if XRSC_CREATE
#include "gmi_mod.rsh"
#include "gmi_mod.rh"
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

#define MOD_RSC_NAME "GMI_MOD.RSC"		/* Name der Resource-Datei */
#define MAXSETUPS 1L							/* Anzahl der GMI-Setups */

/****** TYPES ****************************************************************/
typedef struct setup
{
	VOID *dummy;
} SETUP;	/* Datenstruktur zum merken eines einzelnen General-Setups */

typedef	struct setup *SET_P;

typedef	struct status *STAT_P;

typedef struct status
{
	BOOLEAN new;					/* neue Werte */
} STATUS;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "GMI";		/* Name, fr Extension etc. */

/****** VARIABLES ************************************************************/
PRIVATE WORD	gmi_rsc_hdr;					/* Zeigerstruktur fr RSC-Datei */
PRIVATE WORD	*gmi_rsc_ptr = &gmi_rsc_hdr;		/* Zeigerstruktur fr RSC-Datei */
PRIVATE OBJECT *gmi_setup;
PRIVATE OBJECT *gmi_help;
PRIVATE OBJECT *gmi_desk;
PRIVATE OBJECT *gmi_text;
PRIVATE OBJECT *gmi_info;

/****** FUNCTIONS ************************************************************/

/* Interne GMI-Funktionen */
PRIVATE VOID		get_setup		_((RTMCLASSP module));
PRIVATE VOID		set_setup		_((RTMCLASSP module));

PRIVATE BOOLEAN	init_standard 	_((RTMCLASSP module));
PRIVATE VOID 		create_displayobs	(WINDOWP window);

PRIVATE BOOLEAN	init_rsc			_((VOID));
PRIVATE BOOLEAN	term_rsc			_((VOID));

/*****************************************************************************/

PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
}

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
}

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD		variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	ED_P		actual	= module->actual;
	SET_P		akt		= actual->setup;
	STAT_P	status	= module->status;
	
	status->new			= TRUE;

/*
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch(variable)
			{
				case VAR_SET_GEN:
					if (value != actual->number)
						module->set_setnr(module, value); 
					break;
				case VAR_SET_LFA:
					if (value != akt->lfa_setup)
					{
						akt->lfa_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_LFB:
					if (value != akt->lfb_setup)
					{
						akt->lfb_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_MAA:
					if (value != akt->maa_setup)
					{
						akt->maa_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_MAE:
					if (value != akt->mae_setup)
					{
						akt->mae_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_MTR:
					if (value != akt->mtr_setup)
					{
						akt->mtr_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_SPG:
					if (value != akt->spg_setup)
					{
						akt->spg_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_SPO:
					if (value != akt->spo_setup)
					{
						akt->spo_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_SPS:
					if (value != akt->sps_setup)
					{
						akt->sps_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_GEP:		
					if (value != akt->gep_setup)
					{
						akt->gep_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_MAN:		
					if (value != akt->man_setup)
					{
						akt->man_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_ROT:		
					if (value != akt->rot_setup)
					{
						akt->rot_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
				case VAR_SET_VAR:		
					if (value != akt->var_setup)
					{
						akt->var_setup 	= value; 
						status->new			= TRUE;
						actual->modified	= TRUE;
					}
					break;
			} /* switch */
			break;
	} /* switch */
*/
	Window(module)->milli = 1;
} /* message */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
WINDOWP window;
MKINFO  *mk;

{
	RTMCLASSP	module = Module(window), rtmmodule;
	BOOL			ok = FALSE;
	WORD			i, icon;
	
	undo_state (window->object, window->exit_obj, SELECTED);
	switch (window->exit_obj)
	{
		case GMIGENT:	icon = IGEN; break;
		case GMILFAT:	icon = ILFO; break;
		case GMIMAAT:	icon = IMAA; break;
		case GMIMAET:	icon = IMAE; break;
		case GMIMTRT:	icon = IMTR; break;
		case GMISPGT:	icon = ISPG; break;
		case GMISPOT:	icon = ISPO; break;
		case GMISPST:	icon = ISPS; break;
		case GMIROTT:	icon = IROT; break;
		case GMIVART:	icon = IVAR; break;
	}

  	/* Untersuche alle Module */
	for (i = 0; i < rtmtop && !ok; i++)
	{
		rtmmodule = rtmmodules [i];

		/* Suche passendes Modul und ”ffne */
		if (rtmmodule->icon_position == icon)
			ok = rtmmodule->open (icon);
	} /* for i */
			
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */

} /* wi_click_mod */

/*****************************************************************************/
/* Zeitablauf fr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (window)
WINDOWP window;
{
	RTMCLASSP	module = Module(window);

	window->milli = 100;
} /* wi_timer_mod */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;
{
	/* create the GMI Box */
	
	WINDOWP window;
	WORD    menu_height;
	
	window = search_window (CLASS_GMI, SRCH_OPENED, GMI_SETUP);
	
	if (window == NULL)
	{
		window = create_window_obj (KIND, CLASS_GMI);
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
		window->click     = wi_click_mod;
		window->timer     = wi_timer_mod;
		window->showinfo  = info_mod;
		
		sprintf (window->name, (BYTE *)gmi_text [FGMIN].ob_spec);
		
		create_displayobs (window);
	} /* if */
	
  return (window);                      /* Fenster zurckgeben */
} /* crt_mod */

PRIVATE VOID create_displayobs (WINDOWP window)
{	
	CrtObjectDOInsert (window, GMIGEN, VAR_SET_GEN, ObjectValue, 0);
	CrtObjectDOInsert (window, GMILFA, VAR_SET_LFA, ObjectValue, 0);
	CrtObjectDOInsert (window, GMILFB, VAR_SET_LFB, ObjectValue, 0);
	CrtObjectDOInsert (window, GMIMAA, VAR_SET_MAA, ObjectValue, 0);
	CrtObjectDOInsert (window, GMIMAE, VAR_SET_MAE, ObjectValue, 0);
	CrtObjectDOInsert (window, GMIMTR, VAR_SET_MTR, ObjectValue, 0);
	CrtObjectDOInsert (window, GMISPG, VAR_SET_SPG, ObjectValue, 0);
	CrtObjectDOInsert (window, GMISPO, VAR_SET_SPO, ObjectValue, 0);
	CrtObjectDOInsert (window, GMISPS, VAR_SET_SPS, ObjectValue, 0);
	CrtObjectDOInsert (window, GMIMAN, VAR_SET_MAN, ObjectValue, 0);
	CrtObjectDOInsert (window, GMIROT, VAR_SET_ROT, ObjectValue, 0);
	CrtObjectDOInsert (window, GMIVAR, VAR_SET_VAR, ObjectValue, 0);

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
	
	window = search_window (CLASS_GMI, SRCH_ANY, GMI_SETUP);
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (gmi_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			module = Module(window);
			
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
	WORD		ret;
	STRING	s;
	RTMCLASSP	module = Module(window);

	window = search_window (CLASS_DIALOG, SRCH_ANY, IGMI);
		
	if (window == NULL)
	{
		 form_center (gmi_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (gmi_info, NULL, IGMI, (BYTE *)gmi_text [FGMIN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = gmi_info;
		sprintf(s, "%-20s", GMIDATE);
		set_ptext (gmi_info, GMIIVERDA, s);
		sprintf(s, "%-20s",  __DATE__);
		set_ptext (gmi_info, GMICOMPILE, s);
		sprintf(s, "%-20s", GMIVERSION);
		set_ptext (gmi_info, GMIIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (gmi_info, GMIISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (gmi_info, GMIIAKT, s);

		if (! open_dialog (IGMI)) hndl_alert (ERR_NOOPEN);
	}

  return (window != NULL);
} /* info_mod */

/*****************************************************************************/
GLOBAL	RTMCLASSP create_gmi ()
{
	WINDOWP		window;
	RTMCLASSP 	module;
	FILE			*fp;

	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_GMI;
		module->icon				= &gmi_desk[GMIICON];
		module->icon_position 	= IGMI;
		module->icon_number		= IGMI;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MGMI;
		module->menu_item			= MGMI;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_gmi;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_CONTROL;
		module->message			= message;
		module->create				= create_gmi;
		module->destroy			= destroy_obj;
		module->test				= test_obj;
/*		
		module->file_pointer			= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->file_pointer, 0, (UWORD)sizeof(FILE));
		module->import_pointer		= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->import_pointer, 0, (UWORD)sizeof(FILE));
*/		
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

		init_standard (module);
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (gmi_setup, NULL, GMI_SETUP);
		/* Modul-Struktur einbinden */
		window->module		= (VOID*) module;
		module->window		= window;

		
/*	
		add_rcv(VAR_SET_GEN, module);	/* Message einklinken */
		add_rcv(VAR_SET_LFA, module);	/* Message einklinken */
		add_rcv(VAR_SET_LFB, module);	/* Message einklinken */
		add_rcv(VAR_SET_MAA, module);	/* Message einklinken */
		add_rcv(VAR_SET_MAE, module);	/* Message einklinken */
		add_rcv(VAR_SET_MTR, module);	/* Message einklinken */
		add_rcv(VAR_SET_SPG, module);	/* Message einklinken */
		add_rcv(VAR_SET_SPO, module);	/* Message einklinken */
		add_rcv(VAR_SET_SPS, module);	/* Message einklinken */
*/
	} /* if */
	
	return module;
} /* create_gmi */

PRIVATE BOOLEAN init_standard (RTMCLASSP module)
{
	return TRUE;
} /* init_standard */


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
  gmi_setup = (OBJECT *)rs_trindex [GMI_SETUP]; /* Adresse der GMI-Parameter-Box */
  gmi_help  = (OBJECT *)rs_trindex [GMI_HELP];	/* Adresse der GMI-Hilfe */
  gmi_desk  = (OBJECT *)rs_trindex [GMI_DESK];	/* Adresse des GMI-Desktops */
  gmi_text  = (OBJECT *)rs_trindex [GMI_TEXT];	/* Adresse der GMI-Texte */
  gmi_info 	= (OBJECT *)rs_trindex [GMI_INFO];	/* Adresse der GMI-Info-Anzeige */
#else

  rs_gaddr (gmi_rsc_ptr, R_TREE,  GMI_SETUP,	&gmi_setup);   /* Adresse der GMI-Parameter-Box */
  rs_gaddr (gmi_rsc_ptr, R_TREE,  GMI_HELP,	&gmi_help);    /* Adresse der GMI-Hilfe */
  rs_gaddr (gmi_rsc_ptr, R_TREE,  GMI_DESK,	&gmi_desk);    /* Adresse des GMI-Desktop */
  rs_gaddr (gmi_rsc_ptr, R_TREE,  GMI_TEXT,	&gmi_text);    /* Adresse der GMI-Texte */
  rs_gaddr (gmi_rsc_ptr, R_TREE,  GMI_INFO,	&gmi_info);    /* Adresse der GMI-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (gmi_setup, TRUE);
	fix_objs (gmi_help, TRUE);
	fix_objs (gmi_desk, TRUE);
	fix_objs (gmi_text, TRUE);
	fix_objs (gmi_info, TRUE);
	
	menu_enable(menu, MGMI, TRUE);

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
  ok = rs_free (gmi_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_gmi ()
{
	BOOLEAN				ok = TRUE;

	ok &= init_rsc ();
	instance_count = load_create_infos (create_gmi, module_name, max_instances);
	
	return (ok);
} /* init_gmi */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	ok &= term_rsc ();
	return (ok);
} /* term_mod */
