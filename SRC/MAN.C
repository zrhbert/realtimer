/*****************************************************************************/
/*                                                                           */
/* Modul: MAN.C                                                              */
/*                                                                           */
/* Modul-Manager                                                             */
/*                                                                           */
/*****************************************************************************/
#define MANVERSION "V 0.14"
#define MANDATE "02.02.95"

/*****************************************************************************
V 0.14
- SETUPS_INTERN, 02.02.95
- MAXSETUPS auf 1, 02.02.95
- ClickSetupField eingebaut, 30.01.95
- precalc_modules und reset_modules eingebaut
V 0.13 15.07.94
- man_module eingebaut
- load_create_infos und instance_count eingebaut
- Verwaltung der Module im Setup Åber MOD_INFO Strukturen
V 0.12
- POW aus Standard-Setup herausgenommen
- window->module eingebaut
- Aufruf von control-Modulen in apply, precalc und reset
- module->window in create
- Umbau auf create_window_obj
V 0.11
- Umstellung auf neue RTMCLASS Struktur
V 0.10	12.05.93
- Module an/aus schaltbar gemacht
- select-Fehler in wi_click beseitigt
- mros.h entfernt
V 0.09	18.04.93
- LFO in Standard-Setup aufgenommen
*****************************************************************************/

#ifndef XRSC_CREATE
/*#define XRSC_CREATE 1 */                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "man_mod.h"
#include "realtspc.h"
#include "var.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"

#include "export.h"
#include "man.h"

#if XRSC_CREATE
#include "man_mod.rsh"
#include "man_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                 /* X-Faktor */
#define YFAC   gl_hbox                 /* Y-Faktor */
#define XUNITS 1                       /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                       /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)          /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)          /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)          /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)          /* Anfangshîhe in Pixel */
#define MILLI  0						/* Millisekunden fÅr Zeitablauf */

#define MAN_RSC_NAME "MAN_MOD.RSC"		/* Name der Resource-Datei */
#define MAXSETUPS 1l
#define MAXMODULES 10						/* Max. Anzahl Module pro Queue */
/****** TYPES ****************************************************************/

typedef struct mod_info *MOD_INFOP;
typedef struct mod_info
{
	RTMCLASSP	module;		/* Pointer to the Module */
	BOOLEAN		on;			/* Flag for Module on/off */
} MOD_INFO;

typedef struct setup *SET_P;
typedef struct setup
{
	/* Modul-Infos, sortiert nach Typen */
	MOD_INFO		mod_info[NUM_MODULE_TYPES][MAXMODULES];
} SETUP;

typedef struct status *STAT_P;

typedef struct status
{
	BOOLEAN	record;			/* Record oder Play */
} STATUS;

/****** VARIABLES ************************************************************/
PRIVATE WORD	man_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*man_rsc_ptr = &man_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *man_setup;
PRIVATE OBJECT *man_help;
PRIVATE OBJECT *man_desk;
PRIVATE OBJECT *man_menu;
PRIVATE OBJECT *man_text;
PRIVATE OBJECT *man_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "MAN";		/* Name, fÅr Extension etc. */

/****** FUNCTIONS ************************************************************/
/* Interne MAN-Funktionen */
PUBLIC PUF_INF *apply_modules	(MOD_INFOP mod_infos, PUF_INF *event);
PUBLIC VOID 	reset_modules	(MOD_INFOP mod_infos);
PUBLIC VOID 	precalc_modules	(MOD_INFOP mod_infos);

PRIVATE VOID	init_standard _((RTMCLASSP module));

PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	WORD		offsets[5] = {MANINPUT1-MANINPUT0, MANCALC1-MANCALC0,
									MANCONTROL1-MANCONTROL0, MANOUTPUT1-MANOUTPUT0, 1} ;
	WORD 		offset, slot, type;
	INT 		mod, x[5] = {0,0,0,0,0};
	MOD_INFOP	mod_info;
	RTMCLASSP 	rtmmodule;

	
	for (mod = 0; mod < max_rtmmodules; mod++)
	{
		rtmmodule = rtmmodules[mod];
		if (rtmmodule)
		{
			type = rtmmodule->object_type;
			slot = x[type];
			offset = offsets[type];
			mod_info = &ed->mod_info[type][slot];
			switch (type)
			{
				case MODULE_INPUT:
					mod_info->on = 
							get_checkbox (man_setup, MANINPUT0 + slot*offset);
					break;
				case MODULE_CALC:
					mod_info->on = 
							get_checkbox (man_setup, MANCALC0 + slot*offset);
					break;
				case MODULE_CONTROL:
					mod_info->on = 
							get_checkbox (man_setup, MANCONTROL0 + slot*offset);
					break;
				case MODULE_OUTPUT:
					mod_info->on = 
							get_checkbox (man_setup, MANOUTPUT0 + slot*offset);
					break;
				case MODULE_OTHER:
					break;
			} /* switch */
			x[type]++;
		} /* if */
	} /* for */
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P			edited = module->edited;
	SET_P			ed = module->edited->setup;
	STRING		s;
	WORD			offsets[5] = {MANINPUT1-MANINPUT0, MANCALC1-MANCALC0,
										MANCONTROL1-MANCONTROL0, MANOUTPUT1-MANOUTPUT0, 1} ;
	WORD			type, offset;
	INT 			mod, slot, x[5] = {0,0,0,0,0};
	MOD_INFOP	mod_info;
	RTMCLASSP 	rtmmodule;

	for (mod = 0; mod < max_rtmmodules; mod++)
	{
		rtmmodule = rtmmodules[mod];
		if (rtmmodule)
		{
			type = rtmmodule->object_type;
			slot = x[type];
			offset = offsets[type];
			mod_info = &ed->mod_info[type][slot];
			/* Evtl. neues Modul eintragen */
			if (mod_info->module == 0) mod_info->module = rtmmodule;
			switch (type)
			{
				case MODULE_INPUT:
					set_ptext (man_setup, MANINPUTTEXT0 + slot*offset, mod_info->module->object_name);
					set_checkbox (man_setup, MANINPUT0 + slot*offset, mod_info->on);
					break;
				case MODULE_CALC:
					set_ptext (man_setup, MANCALCTEXT0 + slot*offset, mod_info->module->object_name);
					set_checkbox (man_setup, MANCALC0 + slot*offset, mod_info->on);
					break;
				case MODULE_CONTROL:
					set_ptext (man_setup, MANCONTROLTEXT0 + slot*offset, mod_info->module->object_name);
					set_checkbox (man_setup, MANCONTROL0 + slot*offset, TRUE);
					break;
				case MODULE_OUTPUT:
					set_ptext (man_setup, MANOUTPUTTEXT0 + slot*offset, mod_info->module->object_name);
					set_checkbox (man_setup, MANOUTPUT0 + slot*offset,	mod_info->on);
					break;
				case MODULE_OTHER:
					break;
			} /* switch */
			x[type]++;
		} /* if */
	} /* for */
	offset = offsets[MODULE_INPUT];
	for (slot = x[MODULE_INPUT]; slot < MAXMODULES; slot++) /* Rest auffÅllen */
	{
		set_ptext (man_setup, MANINPUTTEXT0 + slot*offset, "");
		set_checkbox (man_setup, MANINPUT0 + slot*offset, FALSE);
	} /* for */
	offset = offsets[MODULE_CALC];
	for (slot = x[MODULE_CALC]; slot < MAXMODULES; slot++) /* Rest auffÅllen */
	{
		set_ptext (man_setup, MANCALCTEXT0 + slot*offset, "");
		set_checkbox (man_setup, MANCALC0 + slot*offset, FALSE);
	} /* for */
	offset = offsets[MODULE_CONTROL];
	for (slot = x[MODULE_CONTROL]; slot < MAXMODULES; slot++) /* Rest auffÅllen */
	{
		set_ptext (man_setup, MANCONTROLTEXT0 + slot*offset, "");
		set_checkbox (man_setup, MANCONTROL0 + slot*offset, FALSE);
	} /* for */
	offset = offsets[MODULE_OUTPUT];
	for (slot = x[MODULE_OUTPUT]; slot < MAXMODULES; slot++) /* Rest auffÅllen */
	{
		set_ptext (man_setup, MANOUTPUTTEXT0 + slot*offset, "");
		set_checkbox (man_setup, MANOUTPUT0 + slot*offset, FALSE);
	} /* for */
	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (man_setup, MANSETNR , s);
} /* set_dbox */

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	STATUS 		*status = module->status;
	SETUP 		*man = module->actual->setup;
	MOD_INFO		(*mod_infos)[MAXMODULES] = man->mod_info;

	/* Erst alle INPUT-, dann CALC-, dann OUTPUT-Module abfertigen */
	/* Jede Klasse wird in der durch man->xxxx[] festgelegten
		Reihenfolge bearbeitet */
		
	if (status->record) /* Record an? */
	{
		apply_modules (mod_infos[MODULE_INPUT], event);
		apply_modules (mod_infos[MODULE_CALC], event);
	} /* if */

	apply_modules (mod_infos[MODULE_CONTROL], event);
	apply_modules (mod_infos[MODULE_OUTPUT], event);

	return event;
} /* apply */

PUBLIC PUF_INF *apply_modules	(MOD_INFOP mod_infos, PUF_INF *event)
{
	/* Traverse all Modules of a kind and call apply */
	WORD			slot;
	RTMCLASSP 	mod;
	MOD_INFOP	mod_info = mod_infos;
	
	for (slot = 0; slot < MAXMODULES; slot++)
	{
		mod = mod_info->module;
		if(mod  && mod_info->on)
		{
			if (mod)
				if(mod->apply)
					event = mod->apply (mod, event);
		} /* if */
		mod_info++;
	} /* for */
	return event;
} /* apply_modules */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	STATUS 		*status = module->status;
	SETUP 		*man = module->actual->setup;
	MOD_INFO		(*mod_infos)[MAXMODULES] = man->mod_info;

	/* Erst alle INPUT-, dann CALC-, dann OUTPUT-Module abfertigen */
	/* Jede Klasse wird in der durch man->xxxx[] festgelegten
		Reihenfolge bearbeitet */
		
	reset_modules (mod_infos[MODULE_INPUT]);
	reset_modules (mod_infos[MODULE_CALC]);

	reset_modules (mod_infos[MODULE_CONTROL]);
	reset_modules (mod_infos[MODULE_OUTPUT]);

} /* reset */

PUBLIC VOID reset_modules	(MOD_INFOP mod_infos)
{
	/* Traverse all Modules of a kind and call apply */
	WORD			slot;
	RTMCLASSP 	mod;
	MOD_INFOP	mod_info = mod_infos;
	
	for (slot = 0; slot < MAXMODULES; slot++)
	{
		mod = mod_info->module;
		if(mod  && mod_info->on)
		{
			if (mod)
				if(mod->reset)
					mod->reset (mod);
		} /* if */
		mod_info++;
	} /* for */
} /* reset_modules */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	STATUS 		*status = module->status;
	SETUP 		*man = module->actual->setup;
	MOD_INFO		(*mod_infos)[MAXMODULES] = man->mod_info;

	/* Erst alle INPUT-, dann CALC-, dann OUTPUT-Module abfertigen */
	/* Jede Klasse wird in der durch man->xxxx[] festgelegten
		Reihenfolge bearbeitet */
		
	if (status->record) /* Record an? */
	{
		precalc_modules (mod_infos[MODULE_INPUT]);
		precalc_modules (mod_infos[MODULE_CALC]);
	} /* if */

	precalc_modules (mod_infos[MODULE_CONTROL]);
	precalc_modules (mod_infos[MODULE_OUTPUT]);

} /* precalc */

PUBLIC VOID precalc_modules	(MOD_INFOP mod_infos)
{
	/* Traverse all Modules of a kind and call apply */
	WORD			slot;
	RTMCLASSP 	mod;
	MOD_INFOP	mod_info = mod_infos;
	
	for (slot = 0; slot < MAXMODULES; slot++)
	{
		mod = mod_info->module;
		if(mod  && mod_info->on)
		{
			if (mod)
				if(mod->precalc)
					mod->precalc (mod);
		} /* if */
		mod_info++;
	} /* for */
} /* precalc_modules */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	STATUS	*status = module->status;
	UWORD variable = ((MSG_SET_VAR *)msg)->variable;
	LONG	value		= ((MSG_SET_VAR *)msg)->value;

	switch(type)
	{
		case SET_VAR:			/* Record ein/ausschalten */
			switch (variable)
			{
				case VAR_SET_MAN:
					module->set_setnr(module, value);
					break;
				case VAR_RECORD:
					status->record = (WORD)value;
					break;
			} /* switch */
			break;
	} /* switch */

	/* TRUE = RECORD */
	/* FALSE = PLAY */
} /* message */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	send_variable(VAR_SET_MAN, module->actual->number);
} /* send_messages */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
WINDOWP window;
MKINFO  *mk;
{
	STRING		s;
	static		LONG x = 0;	
	RTMCLASSP	module = Module(window);
	ED_P			edited = module->edited;

	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case MANSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case MANSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case MANSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case MANSETSTORE:
			module->set_store (window, edited->number);
			break;
		case MANSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case MANOK   :
			module->set_ok (window);
			break;
		case MANCANCEL:
			module->set_cancel (window);
		   break;
		case MANHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case MANSTANDARD:
			init_standard(module);
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
				set_ptext (man_setup, MANSETNR, s);
				draw_object(window, MANSETNR);
			} /* if */
			/* undo_state (window->object, window->exit_obj, SELECTED); */
	}	/* switch */
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
	
	inx    = num_windows (CLASS_MAN, SRCH_ANY, NULL);
	window = create_window_obj (KIND, CLASS_MAN);
	
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
		window->bg_color  = -1;
		window->mousenum  = ARROW;
		window->mouseform = NULL;
		window->milli     = MILLI;
		window->special   = 0;
		window->edit_obj  = 0;
		window->edit_inx  = 0;
		window->exit_obj  = 0;
		window->object    = obj;
		window->menu      = menu;
		window->click     = wi_click_mod;
		window->showinfo  = info_mod;

		sprintf (window->name, (BYTE *)man_text [FMANN].ob_spec);
		sprintf (window->info, (BYTE *)man_text [FMANI].ob_spec, 0);
		
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
	
	window = search_window (CLASS_MAN, SRCH_ANY, MAN_SETUP);
	if (window != NULL)
	{
		if (window->opened == 0)
		{	
			window->edit_obj = find_flags (man_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			module = Module(window);
			
			module->set_edit (module);
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

	window = search_window (CLASS_DIALOG, SRCH_ANY, IMAN);
		
	if (window == NULL)
	{
		 form_center (man_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (man_info, NULL, IMAN, (BYTE *)man_text [FMANN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = man_info;
		sprintf(s, "%-20s", MANDATE);
		set_ptext (man_info, MANIVERDA, s);
		sprintf(s, "%-20s", MANVERSION);
		set_ptext (man_info, MANIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (man_info, MANISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (man_info, MANIAKT, s);

		if (! open_dialog (IMAN)) hndl_alert (ERR_NOOPEN);
	}

	return (window != NULL);
} /* info_mod */

/*****************************************************************************/
/* Kreiere Modul                                                             */
/*****************************************************************************/
GLOBAL RTMCLASSP create_man ()
{
	WINDOWP		window;
	RTMCLASSP 	module;
	FILE			*fp;
	
	module = create_module (module_name, instance_count);
		
	if (module != NULL)
	{
		module->class_number	= CLASS_MAN;
		module->icon			= &man_desk[MANICON];
		module->icon_position= IMAN;
		module->icon_number	= IMAN;	/* Soll bei Init vergeben werden */
		module->menu_title	= MOPTIONS;
		module->menu_position= MMAN;
		module->menu_item		= MMAN;	/* Soll bei Init vergeben werden */
		module->multiple		= FALSE;
		
		module->crt				= crt_mod;
		module->open			= open_mod;
		module->info			= info_mod;
		module->init			= init_man;
		module->term			= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_OTHER;
		module->apply				= apply;
		module->reset				= reset;
		module->precalc			= precalc;
		module->message			= message;
		module->create				= create_man;
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
		module->status 				= (STAT_P)mem_alloc (sizeof(STATUS));
		module->stat_alt 				= (STAT_P)mem_alloc (sizeof(STATUS));
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

		/* Status-Strukturen initialisieren */
		mem_set(module->status, 0, (UWORD) sizeof(STATUS));
		mem_set(module->stat_alt, 0, (UWORD) sizeof(STATUS));

		/* Setup-Strukturen initialisieren */
		init_standard (module);		
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (man_setup, NULL, MAN_SETUP);
		/* Modul-Struktur einbinden */
		window->module = (VOID*) module;
		module->window = window;
		
		add_rcv(VAR_SET_MAN, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_MAN, MAXSETUPS);
		add_rcv(VAR_RECORD,  module);	/* Message einklinken */
	
		man_module = module;
	} /* if */
	
	return module;
} /* create_man */

PRIVATE VOID init_standard (RTMCLASSP module)
{
	RTMCLASSP	rtmmodule;
	WORD			x[5]={0,0,0,0,0}, m, slot, type;
	SET_P			standard = module->standard;
	
	mem_set(standard, 0, (UWORD) sizeof(SETUP));
	for (m = 0; m < max_rtmmodules; m++)
	{
		rtmmodule = rtmmodules[m];
		if (rtmmodule)
		{
			type = rtmmodule->object_type;
			switch (type)
			{
				case MODULE_CALC:
				case MODULE_CONTROL:
				case MODULE_OUTPUT:
					slot = x[type]++;
					standard->mod_info[type][slot].module	= rtmmodule;
					standard->mod_info[type][slot].on		= TRUE;
					break;
				case MODULE_INPUT:
					slot = x[type]++;
					standard->mod_info[type][slot].module	= rtmmodule;
					/* MAA und POW gehîren nicht in die Standards */
					if (  (rtmmodule->class_number != CLASS_MAA)
						&& (rtmmodule->class_number != CLASS_POW))
					{
						standard->mod_info[type][slot].on	= TRUE;
					} /* if */
					break;
			} /* switch */
		} /* if */
	} /* for */
} /* init_standard */

/*****************************************************************************/
/* RSC îffnen                                                      		     */
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
  man_setup  = (OBJECT *)rs_trindex [MAN_SETUP];	/* Adresse der Transportleiste */
  man_help  = (OBJECT *)rs_trindex [MAN_HELP];	/* Adresse der MAN-Hilfe */
  man_desk  = (OBJECT *)rs_trindex [MAN_DESK];	/* Adresse des MAN-Desktops */
  man_menu  = (OBJECT *)rs_trindex [MAN_MENU];  /* Adresse der MAN-MenÅzeile */
  man_text  = (OBJECT *)rs_trindex [MAN_TEXT];  /* Adresse der MAN-Texte */
  man_info 	= (OBJECT *)rs_trindex [MAN_INFO];	/* Adresse der MAN-Info-Anzeige */
#else

  strcpy (rsc_name, MAN_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (man_rsc_ptr, rsc_name))
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

  rs_gaddr (man_rsc_ptr, R_TREE,  MAN_MENU,	&man_menu);    /* Adresse des MAN-MenÅs */
  rs_gaddr (man_rsc_ptr, R_TREE,  MAN_SETUP,	&man_setup);   /* Adresse der MAN-Parameter-Box */
  rs_gaddr (man_rsc_ptr, R_TREE,  MAN_HELP,	&man_help);    /* Adresse der MAN-Hilfe */
  rs_gaddr (man_rsc_ptr, R_TREE,  MAN_DESK,	&man_desk);    /* Adresse der MAN-Desktop */
  rs_gaddr (man_rsc_ptr, R_TREE,  MAN_TEXT,	&man_text);    /* Adresse der MAN-Texte */
  rs_gaddr (man_rsc_ptr, R_TREE,  MAN_INFO,	&man_info);    /* Adresse der MAN-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (man_menu, TRUE);
	fix_objs (man_setup, TRUE);
	fix_objs (man_help, TRUE);
	fix_objs (man_desk, TRUE);
	fix_objs (man_text, TRUE);
	fix_objs (man_info, TRUE);
	
	do_flags (man_setup, MANCANCEL, UNDO_FLAG);
	do_flags (man_setup, MANHELP, HELP_FLAG);
	
	menu_enable(menu, MMAN, TRUE);

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
  ok = rs_free (man_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_man ()
{
	BOOLEAN	ok = TRUE;

	ok &= init_rsc ();
	instance_count = load_create_infos (create_man, module_name, max_instances);
	
	return (ok);
} /* init_man */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	
	ok &= term_rsc ();
	return (ok);
} /* term_mod */
