/*****************************************************************************/
/*                                                                           */
/* Modul: SYN.C                                                              */
/*                                                                           */
/* SYN System-Synchronisation                                                */
/*****************************************************************************/
#define SYNVERSION "V 0.09"
#define SYNDATE "04.03.95"

/*****************************************************************************
V 0.09
- MTC Send Felder eingebaut, 4.3.95
- auf Getxxx, Setxxx umgestellt
V 0.08
- MTITLE auf MCONTROLS geÑndert, 27.11.94
19.05.94
- load_create_infos und instance_count eingebaut
- start/stop/offset/port parameter eingebaut
V 0.07
- window->module eingebaut
- Bug in create (module->window) beseitigt
V 0.06
- Umstellung auf neue RTMCLASS-Struktur
*****************************************************************************/

#ifndef XRSC_CREATE
/* #define XRSC_CREATE TRUE                    /* X-Resource-File im Code */ */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "syn_mod.h"
#include "realtspc.h"
#include "var.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"
#include <msh_unit.h>		/* Deklarationen fÅr MidiShare Library */
#include "msh.h"

#include "export.h"
#include "syn.h"

#if XRSC_CREATE
#include "syn_mod.rsh"
#include "syn_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 3 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangshîhe in Pixel */
#define MILLI  0                        /* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "SYN_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS		1L					/* Anzahl der SYN-Setups */
/****** TYPES ****************************************************************/
typedef struct setup
{
	BOOLEAN	sync_all;		/* Sync auf beliebigem Input */
	WORD		sync_port;			/* Port fÅr MTC */
} SETUP;

typedef struct setup *SET_P;

typedef struct status
{
	VOID *dummy;
} STATUS;

typedef struct setup *STAT_P;

/****** VARIABLES ************************************************************/
PRIVATE WORD	syn_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*syn_rsc_ptr = &syn_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *syn_setup;
PRIVATE OBJECT *syn_help;
PRIVATE OBJECT *syn_desk;
PRIVATE OBJECT *syn_text;
PRIVATE OBJECT *syn_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "SYN";		/* Name, fÅr Extension etc. */

/****** FUNCTIONS ************************************************************/

PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	SET_P		ed = module->edited->setup;
	LONG		tmp;
	WORD		frames_out, ports_out = 0;
	
	GetCheck (syn_setup, SYNSYNALLPORTS, &ed->sync_all); 

	GetPWord (syn_setup, 	SYNSMPTEINDEV, &ed->sync_port);

	
	send_variable (VAR_PUF_START, GetPTime (syn_setup, SYNSMPTESTART, NULL));
	send_variable (VAR_PUF_STOP, GetPTime (syn_setup, SYNSMPTESTOP, NULL));
	send_variable (VAR_PUF_OFFSET, GetPTime (syn_setup, SYNSMPTEOFFSET, NULL));

	if (GetCheck (syn_setup, SYNFRAMESO24,	NULL)) frames_out = 0;
	if (GetCheck (syn_setup, SYNFRAMESO25,	NULL)) frames_out = 1;
	if (GetCheck (syn_setup, SYNFRAMESO30D,	NULL)) frames_out = 2;
	if (GetCheck (syn_setup, SYNFRAMESO30,	NULL)) frames_out = 3;

	send_variable (VAR_SYNC_OUT_FRAMES, frames_out);

	if (GetCheck (syn_setup, SYNPORTSO1,	NULL)) ports_out |= 1;
	if (GetCheck (syn_setup, SYNPORTSO2,	NULL)) ports_out |= 2;
	if (GetCheck (syn_setup, SYNPORTSO3,	NULL)) ports_out |= 4;
	if (GetCheck (syn_setup, SYNPORTSO4,	NULL)) ports_out |= 8;

	send_variable (VAR_SYNC_OUT_PORTS, ports_out);
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	SET_P		ed = module->edited->setup;
	STRING	s;
	WORD		frames_in, frames_out, ports_out;

	frames_in = var_get_value(var_module, VAR_SYNC_IN_FRAMES);
	SetCheck (syn_setup, SYNFRAMESI24,	frames_in == 0); 
	SetCheck (syn_setup, SYNFRAMESI25,	frames_in == 1); 
	SetCheck (syn_setup, SYNFRAMESI30D, frames_in == 2); 
	SetCheck (syn_setup, SYNFRAMESI30,	frames_in == 3); 
/*
	SetCheck (syn_setup, SYNSYNALLPORTS,	info.syncMode & MIDISyncAnyPort); 

	SetPWord (syn_setup, 	SYNSMPTEINDEV, info.syncMode & 0xFF);
*/
	SetPTime (syn_setup, 	SYNSMPTESTART, var_get_value (var_module, VAR_PUF_START));
	SetPTime (syn_setup, 	SYNSMPTESTOP, var_get_value (var_module, VAR_PUF_STOP));
	SetPTime (syn_setup, 	SYNSMPTEOFFSET, var_get_value (var_module, VAR_PUF_OFFSET));

	frames_out = var_get_value(var_module, VAR_SYNC_OUT_FRAMES);
	SetCheck (syn_setup, SYNFRAMESO24,	frames_out == 0); 
	SetCheck (syn_setup, SYNFRAMESO25,	frames_out == 1); 
	SetCheck (syn_setup, SYNFRAMESO30D, frames_out == 2); 
	SetCheck (syn_setup, SYNFRAMESO30,	frames_out == 3); 

	ports_out = var_get_value(var_module, VAR_SYNC_OUT_PORTS);
	SetCheck (syn_setup, SYNPORTSO1,	ports_out & 1); 
	SetCheck (syn_setup, SYNPORTSO2,	ports_out & 2); 
	SetCheck (syn_setup, SYNPORTSO3,	ports_out & 4); 
	SetCheck (syn_setup, SYNPORTSO4,	ports_out & 8); 
} /* set_dbox */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;
	TSyncInfo	syncinfo;

	send_variable(VAR_SET_SYN, module->actual->number);
	
	if (MidiShare())
	{
		/* MidiShare konfigurieren */
	  	/* MidiGetSyncInfo (&syncinfo); */
		syncinfo.syncMode = akt->sync_port;
		syncinfo.syncMode |= MIDISyncExternal;
	  	if (akt->sync_all)
	  		syncinfo.syncMode |= MIDISyncAnyPort;
	  	else
	  		syncinfo.syncMode = 0;
	  	MidiSetSyncMode (syncinfo.syncMode);
  	} /* if */
} /* send_messages */

/*****************************************************************************/

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD variable = ((MSG_SET_VAR *)msg)->variable;
	LONG	value		= ((MSG_SET_VAR *)msg)->value;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch (variable)
			{
				case VAR_SET_SYN:	
					module->set_setnr(module, value);
					break;
			} /* switch */
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
	SET_P			ed = edited->setup;
	WORD 			i, item;
	STRING		s;
	UWORD			signal, offset;
	LONG			x;	
	WORD			exit_obj = window->exit_obj;
	
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case SYNOK   :
			module->set_ok (window);
		   break;
		case SYNCANCEL:
			module->set_cancel (window);
		   break;
		case SYNSTANDARD:
			module->set_standard (window);
		   break;
		case SYNHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case ROOT:
		case NIL:
			break;
		default :	
			if (exit_obj >= SYNSMPTESTART && exit_obj <= SYNSMPTESTART + 4)
				ClickTimeField (window, SYNSMPTESTART, mk, 0, 0, UpdateTimeField);
			else if (exit_obj >= SYNSMPTESTOP && exit_obj <= SYNSMPTESTOP + 4)
				ClickTimeField (window, SYNSMPTESTOP, mk, 0, 0, UpdateTimeField);
			else if (exit_obj >= SYNSMPTEOFFSET && exit_obj <= SYNSMPTEOFFSET + 4)
				ClickTimeField (window, SYNSMPTEOFFSET, mk, 0, 0, UpdateTimeField);
/*
			if(syn_setup_nr!=0)
			{
				setups[0]=setups[syn_setup_nr];
				syn_setup_alt=syn_setup_nr;
				syn_setup_nr=0;
				syn=&setups[syn_setup_nr];
				
				sprintf (s, "%ld", syn_setup_nr);
				set_ptext (syn_setup, SYNSETNR, s);
				draw_object(window, SYNSETNR);
			} /* if */
*/				
			break;						
	} /* switch */
} /* wi_click_mod */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
  WINDOWP window;
  WORD    menu_height, inx;

  inx    = num_windows (CLASS_SYN, SRCH_ANY, NULL);
  window = create_window_obj (KIND, CLASS_SYN);

  if (window != NULL)
  {

		WINDOW_INITOBJ_OBJ

    window->flags     = FLAGS | WI_MODELESS;
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
    window->mousenum  = ARROW;
    window->mouseform = NULL;
    window->milli     = MILLI;
    window->module   = 0;
    window->object    = obj;
    window->menu      = menu;
    window->click     = wi_click_mod;
    window->showinfo  = info_mod;

    sprintf (window->name, (BYTE *)syn_text [FSYNN].ob_spec);
    sprintf (window->info, (BYTE *)syn_text [FSYNI].ob_spec, 0);
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
RTMCLASSP	module;

	window = search_window (CLASS_SYN, SRCH_ANY, SYN_SETUP);

	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (syn_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			module = Module(window);
			
			module->set_dbox (module);
			
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

	window = search_window (CLASS_DIALOG, SRCH_ANY, ISYN);
		
	if (window == NULL)
	{
		 form_center (syn_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (syn_info, NULL, ISYN, (BYTE *)syn_text [FSYNN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = syn_info;
		sprintf(s, "%-20s", SYNDATE);
		set_ptext (syn_info, SYNIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (syn_info, SYNICOMPILE, s);
		sprintf(s, "%-20s", SYNVERSION);
		set_ptext (syn_info, SYNIVERNR, s);
		sprintf(s, "%-20d", module->max_setups);
		set_ptext (syn_info, SYNISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (syn_info, SYNIAKT, s);

		if (! open_dialog (ISYN)) hndl_alert (ERR_NOOPEN);
	}

	return (window != NULL);
} /* info_mod */

/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	RTMCLASSP 	module;
	SET_P			standard;
	WORD 			x;
	WINDOWP		window;
	FILE			*fp;

	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_SYN;
		module->icon				= &syn_desk[SYNICON];
		module->icon_position	= ISYN;
		module->icon_number		= ISYN;	/* Soll bei Init vergeben werden */
		module->menu_title		= MCONTROLS;
		module->menu_position	= MSYN;
		module->menu_item			= MSYN;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;

		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_syn;
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
		module->location				= SETUPS_INTERN;
		module->ram_modified 		= FALSE;
		module->max_setups		 		= MAXSETUPS;
		module->standard				= (SET_P)mem_alloc(sizeof(SETUP));
		module->actual->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->edited->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->status 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->stat_alt 				= 0;
		module->load					= load_obj;
		module->save					= save_obj;
		module->import					= 0;
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

		/* Initialisierung des Standard-Setups */
		standard = module->standard;
		mem_set(standard, 0, (UWORD) sizeof(SETUP));

		/* Fenster generieren */
		window = crt_mod (syn_setup, NULL, SYN_SETUP);
		/* Modul-Struktur einbinden */
		window->module		= (VOID*) module;
		module->window		= window;
		
		add_rcv(VAR_SET_SYN, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_SYN, MAXSETUPS);
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
  alesynsg = &rs_strings [FREESTR];             /* Adresse der Fehlermeldungen */
*/
/*
	syn_menu  = (OBJECT *)rs_trindex [SYN_SETUP]; /* Adresse des SYN-MenÅs */
*/
	syn_setup = (OBJECT *)rs_trindex [SYN_SETUP]; /* Adresse der SYN-Parameter-Box */
	syn_help  = (OBJECT *)rs_trindex [SYN_HELP];	/* Adresse der SYN-Hilfe */
	syn_desk  = (OBJECT *)rs_trindex [SYN_DESK];	/* Adresse des SYN-Desktops */
	syn_text  = (OBJECT *)rs_trindex [SYN_TEXT];	/* Adresse der SYN-Texte */
	syn_info 	= (OBJECT *)rs_trindex [SYN_INFO];	/* Adresse der SYN-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (syn_rsc_ptr, rsc_name))
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
/*
	rs_gaddr (syn_rsc_ptr, R_TREE,  SYN_SETUP,	&syn_menu);    /* Adresse des SYN-MenÅs */
*/
	rs_gaddr (syn_rsc_ptr, R_TREE,  SYN_SETUP,	&syn_setup);   /* Adresse der SYN-Parameter-Box */
	rs_gaddr (syn_rsc_ptr, R_TREE,  SYN_HELP,	&syn_help);    /* Adresse der SYN-Hilfe */
	rs_gaddr (syn_rsc_ptr, R_TREE,  SYN_DESK,	&syn_desk);    /* Adresse des SYN-Desktop */
	rs_gaddr (syn_rsc_ptr, R_TREE,  SYN_TEXT,	&syn_text);    /* Adresse der SYN-Texte */
	rs_gaddr (syn_rsc_ptr, R_TREE,  SYN_INFO,	&syn_info);    /* Adresse der SYN-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	/*fix_objs (syn_menu, TRUE);*/
	fix_objs (syn_setup, TRUE);
	fix_objs (syn_help, TRUE);
	fix_objs (syn_desk, TRUE);
	fix_objs (syn_text, TRUE);
	fix_objs (syn_info, TRUE);
	
	/*
	do_flags (syn_setup, SYNCANCEL, UNDO_FLAG);
	do_flags (syn_setup, SYNHELP, HELP_FLAG);
	*/
	menu_enable(menu, MSYN, TRUE);

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
  ok = rs_free (syn_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_syn ()

{
	BOOLEAN	ok = TRUE;
	
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);
	
 	return (ok);
} /* init_syn */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	ok &= term_rsc ();
	return (ok);
} /* term_mod */
