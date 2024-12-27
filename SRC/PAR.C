/*****************************************************************************/
/*                                                                           */
/* Modul: PAR.C                                                           	  */
/*                                                                           */
/* Koordinaten-Anzeige                                                       */
/*                                                                           */
/*****************************************************************************/
#define PARVERSION "V 1.01"
#define PARDATE "02.02.95"

/*****************************************************************************
V 1.00
- MAE-Schaltungen entfernt, 02.02.95
- Bug in create_dispobj bei LFO beseitigt
V 0.11
- Umstellung auf dispobj
- load_create_infos und instance_count eingebaut
- window->module eingebaut
- Umbau auf create_window_obj
- Umbau info_mod
V 0.10 20.06.93
- Umstellung auf neue RTMCLASS Struktur
- Anzeige mit Datenreduktion
- Fehler in Behandlung von MTRPAUSE, PUFPAUSE und PUFZEITLUPE beseitigt
V 0.09, 17.04.93
- Fehler in Aufbau und Abfrage der Dialogbox beseitigt
- apply, reset, precalc entfernt
*****************************************************************************/

#ifndef XRSC_CREATE
/*#define XRSC_CREATE 1*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "par_mod.h"
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
#include "par.h"

#if XRSC_CREATE
#include "par_mod.rsh"
#include "par_mod.rh"
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

#define MOD_RSC_NAME "PAR_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 1l						/* Anzahl der PAR-Setups */
/****** TYPES ****************************************************************/
typedef struct setup *SET_P;

typedef struct setup
{
	/* Anzeige-Flags */
	UINT	lfa_on[12]			;	/* LFA an/aus  */
	UINT	lfa_umk		: 1	;	/* LFA Umkehrung  */
	UINT	lfa_pause	: 1	;	/* LFA gestoppt  */
	UINT	lfb_on[12]			;	/* LFB an/aus  */
	UINT	lfb_umk		: 1	;	/* LFB Umkehrung  */
	UINT	lfb_pause	: 1	;	/* LFB gestoppt  */
	UINT	mtr_on[9]			;	/* MTR an/aus  */
	UINT	mtr_umk		: 1	;	/* MTR Umkehrung  */
	UINT	mtr_pause	: 1	;	/* MTR gestoppt  */
	UINT	puf_rec[9]			;	/* PUF Record an/aus  */
	UINT	puf_play[9] 		;	/* PUF Play an/aus  */
	UINT	puf_pause	: 1	;	/* PUF gestoppt  */
	UINT	puf_zeitl	: 1	;	/* PUF Zeitlupe  */
	UINT	maa_spin		: 1	;	/* MAA Sperre innen  */
	UINT	maa_spout	: 1	;	/* MAA Sperre aussen  */
	UINT	mae_spin		: 1	;	/* MAE Sperre innen  */
	UINT	mae_spout	: 1	;	/* MAE Sperre aussen  */
} SETUP;		/* EnthÑlt alle Parameter einer kompletten PAR-Einstellung */

typedef struct status	
{
	BOOLEAN	new;				/* Hat sich etwas geÑndert ? */
} STATUS;

typedef struct status *STAT_P;	

/****** VARIABLES ************************************************************/
/* Resource */
PRIVATE WORD	par_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*par_rsc_ptr = &par_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *par_setup;
PRIVATE OBJECT *par_help;
PRIVATE OBJECT *par_desk;
PRIVATE OBJECT *par_text;
PRIVATE OBJECT *par_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "PAR";		/* Name, fÅr Extension etc. */

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

/*
	for(signal=0; signal<12; signal++)
		ed->lfa_on[signal] = get_checkbox (par_setup, PARLFA1 + signal);

	ed->lfa_umk = get_checkbox (par_setup, PARLFAUMKEHR);
	ed->lfa_pause = get_checkbox (par_setup, PARLFAPAUSE);

	for(signal=0; signal<12; signal++)
		ed->lfb_on[signal] = get_checkbox (par_setup, PARLFB1 + signal);

	ed->lfb_umk = get_checkbox (par_setup, PARLFBUMKEHR);
	ed->lfb_pause = get_checkbox (par_setup, PARLFBPAUSE);

	for(signal=0; signal<MAXSIGNALS; signal++)
		ed->mtr_on[signal] = get_checkbox (par_setup, PARMTR0 + signal);

	ed->mtr_umk   = get_checkbox (par_setup, PARMTRUMKEHR);
	ed->mtr_pause = get_checkbox (par_setup, PARMTRPAUSE);
	ed->puf_pause = get_checkbox (par_setup, PARPUFPAUSE);
	ed->puf_zeitl = get_checkbox (par_setup, PARPUFZEITLUPE);

	for(signal=0; signal<MAXSIGNALS; signal++)
		ed->puf_rec[signal] = get_checkbox (par_setup, PARREC0 + signal);

	for(signal=0; signal<MAXSIGNALS; signal++)
		ed->puf_play[signal] = get_checkbox (par_setup, PARPLAY0 + signal);

	ed->maa_spin	= get_checkbox (par_setup, PARMAASPIN);
	ed->maa_spout	= get_checkbox (par_setup, PARMAASPOUT);
	ed->mae_spin	= get_checkbox (par_setup, PARMAESPIN);
	ed->mae_spout	= get_checkbox (par_setup, PARMAESPOUT);
*/
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
	
/*
	for(signal=0; signal<12; signal++)
		update_checkbox (window, PARLFA1 + signal, akt->lfa_on[signal], ed->lfa_on[signal], draw);

	update_checkbox (window, PARLFAUMKEHR, akt->lfa_umk, ed->lfa_umk, draw);
	update_checkbox (window, PARLFAPAUSE, akt->lfa_pause, ed->lfa_pause, draw);

	for(signal=0; signal<12; signal++)
		update_checkbox (window, PARLFB1 + signal, akt->lfb_on[signal], ed->lfb_on[signal], draw);

	update_checkbox (window, PARLFBUMKEHR, akt->lfb_umk, ed->lfb_umk, draw);
	update_checkbox (window, PARLFBPAUSE, akt->lfb_pause, ed->lfb_pause, draw);

	for(signal=0; signal<MAXSIGNALS; signal++)
		update_checkbox (window, PARMTR0 + signal, akt->mtr_on[signal], ed->mtr_on[signal], draw);

	update_checkbox (window, PARMTRUMKEHR, akt->mtr_umk, ed->mtr_umk, draw);
	update_checkbox (window, PARMTRPAUSE, akt->mtr_pause, ed->mtr_pause, draw);
	update_checkbox (window, PARPUFPAUSE, akt->puf_pause, ed->puf_pause, draw);
	update_checkbox (window, PARPUFZEITLUPE, akt->puf_zeitl, ed->puf_zeitl, draw);

	for(signal=0; signal<MAXSIGNALS; signal++)
		update_checkbox (window, PARREC0 + signal, akt->puf_rec[signal], ed->puf_rec[signal], draw);

	for(signal=0; signal<MAXSIGNALS; signal++)
		update_checkbox (window, PARPLAY0 + signal, akt->puf_play[signal], ed->puf_play[signal], draw);

	update_checkbox (window, PARMAASPIN, akt->maa_spin, ed->maa_spin, draw);
	update_checkbox (window, PARMAASPOUT, akt->maa_spout, ed->maa_spout, draw);
	update_checkbox (window, PARMAESPOUT, akt->mae_spout, ed->mae_spout, draw);
	update_checkbox (window, PARMAESPIN, akt->mae_spin, ed->mae_spin, draw);

*/
	/* Neue Daten Åbernehmen */
	mem_move(ed, akt, (UWORD)module->setup_length);

} /* set_dbox */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	ED_P		actual = module->actual;
	SET_P		akt = actual->setup;
	UWORD 	signal;

	send_variable(VAR_SET_PAR, actual->number);

/*
	for(signal=0; signal<12; signal++)
		send_variable (VAR_LFA_ON1 + signal, akt->lfa_on[signal]);

	send_variable (VAR_LFA_UMK, akt->lfa_umk);
	send_variable (VAR_LFA_PAUSE, akt->lfa_pause);
	for(signal=0; signal<12; signal++)
		send_variable (VAR_LFB_ON1 + signal, akt->lfb_on[signal]);

	send_variable (VAR_LFB_UMK, akt->lfb_umk);
	send_variable (VAR_LFB_PAUSE, akt->lfb_pause);

	for(signal=0; signal<MAXSIGNALS; signal++)
		send_variable (VAR_MTR_ON0 + signal, akt->mtr_on[signal]);

	send_variable (VAR_MTR_UMK, akt->mtr_umk);

	send_variable (VAR_MTR_PAUSE, akt->mtr_pause);

	send_variable (VAR_PUF_PAUSE, akt->puf_pause);

	send_variable (VAR_PUF_ZEITL, akt->puf_zeitl);

	for(signal=0; signal<MAXSIGNALS; signal++)
		send_variable (VAR_PUF_REC_SIG0 + signal, akt->puf_rec[signal]);

	for(signal=0; signal<MAXSIGNALS; signal++)
		send_variable (VAR_PUF_PLAY_SIG0 + signal, akt->puf_play[signal]);

	send_variable (VAR_MAA_SPERRE_INNEN, akt->maa_spin);
	send_variable (VAR_MAA_SPERRE_AUSSEN, akt->maa_spout);

	send_variable (VAR_MAE_SPERRE_INNEN, akt->mae_spin);
	send_variable (VAR_MAE_SPERRE_AUSSEN, akt->mae_spout);
*/

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
				case VAR_SET_PAR :		module->set_setnr(module, value);	break;
				case VAR_LFA_UMK :		akt->lfa_umk 		= (WORD) value; break;
				case VAR_LFA_PAUSE :		akt->lfa_pause 	= (WORD) value; break;
				case VAR_LFB_UMK :		akt->lfb_umk 		= (WORD) value; break;
				case VAR_LFB_PAUSE :		akt->lfb_pause 	= (WORD) value; break;
				case VAR_MTR_UMK :		akt->mtr_umk 		= (WORD) value; break;
				case VAR_MTR_PAUSE :		akt->mtr_pause 	= (WORD) value; break;
				case VAR_PUF_PAUSE :		akt->puf_pause 	= (WORD) value; break;
				case VAR_PUF_ZEITL :		akt->puf_zeitl 	= (WORD) value; break;
				case VAR_MAA_SPERRE_INNEN :	akt->maa_spin	= (WORD) value; break;
				case VAR_MAA_SPERRE_AUSSEN :	akt->maa_spout	= (WORD) value; break;
				case VAR_MAE_SPERRE_INNEN :	akt->mae_spin  = (WORD) value; break;
				case VAR_MAE_SPERRE_AUSSEN :	akt->mae_spout	= (WORD) value; break;
				default:
				if ((variable >= VAR_LFA_ON1) && (variable < VAR_LFA_ON1 + 12 ))
					/* Panbreite fÅr diesen Kanal setzen */
					akt->lfa_on[variable - VAR_LFA_ON1 ] = (WORD)value;

				else if ((variable >= VAR_LFB_ON1) && (variable < VAR_LFB_ON1 + 12 ))
					/* Panbreite fÅr diesen Kanal setzen */
					akt->lfb_on[variable - VAR_LFB_ON1 ] = (WORD)value;
					
				else if ((variable >= VAR_MTR_ON0) && (variable < VAR_MTR_ON0 + MAXSIGNALS ))
					/* Pan-Position fÅr diesen Kanal setzen */
					akt->mtr_on[variable - VAR_MTR_ON0 ] = (WORD)value;

				else if ((variable >= VAR_PUF_REC_SIG0) && (variable < VAR_PUF_REC_SIG0 + MAXSIGNALS ))
					/* Pan-Position fÅr diesen Kanal setzen */
					akt->puf_rec[variable - VAR_PUF_REC_SIG0 ] = (WORD)value;

				else if ((variable >= VAR_PUF_PLAY_SIG0) && (variable < VAR_PUF_PLAY_SIG0 + MAXSIGNALS ))
					/* Pan-Position fÅr diesen Kanal setzen */
					akt->puf_play[variable - VAR_PUF_PLAY_SIG0 ] = (WORD)value;

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
	
	window = create_window_obj (KIND, CLASS_PAR);
	
	
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
		
		sprintf (window->name, (BYTE *)par_text [FPARN].ob_spec);
		/* sprintf (window->info, (BYTE *)freetext [FPARI].ob_spec, 0); */
	
		create_displayobs (window);

	} /* if */
	
	return (window);                      /* Fenster zurÅckgeben */
} /* crt_par */

PRIVATE VOID create_displayobs (WINDOWP window)
{	
	WORD		signal, lfo, h = gl_hbox, w = gl_wbox, x0, y0;
	LONGSTR	s;
	RECT		a;

	for(lfo=0; lfo < MAXLFOS; lfo++)
	{
		CrtObjectDOInsert (window, PARLFA1 + lfo, VAR_LFA_ON1 + lfo, ObjectCheck, 0);
		CrtObjectDOInsert (window, PARLFB1 + lfo, VAR_LFB_ON1 + lfo, ObjectCheck, 0);
	} /* for lfo */
	CrtObjectDOInsert (window, PARLFAUMKEHR, VAR_LFA_UMK, ObjectCheck, 0);
	CrtObjectDOInsert (window, PARLFAPAUSE, VAR_LFA_PAUSE, ObjectCheck, 0);
	CrtObjectDOInsert (window, PARLFBUMKEHR, VAR_LFB_UMK, ObjectCheck, 0);
	CrtObjectDOInsert (window, PARLFBPAUSE, VAR_LFB_PAUSE, ObjectCheck, 0);

	for(signal=0; signal < MAXSIGNALS; signal++)
	{
		CrtObjectDOInsert (window, PARMTR0 + signal, VAR_MTR_ON0 + signal, ObjectCheck, 0);
		CrtObjectDOInsert (window, PARPLAY0 + signal, VAR_PUF_PLAY_SIG0 + signal, ObjectCheck, 0);
		CrtObjectDOInsert (window, PARREC0 + signal, VAR_PUF_REC_SIG0 + signal, ObjectCheck, 0);
	} /* for */
	CrtObjectDOInsert (window, PARMTRPAUSE, VAR_MTR_PAUSE, ObjectCheck, 0);
	CrtObjectDOInsert (window, PARMTRUMKEHR, VAR_MTR_UMK, ObjectCheck, 0);
	CrtObjectDOInsert (window, PARPUFPAUSE, VAR_PUF_PAUSE, ObjectCheck, 0);
	CrtObjectDOInsert (window, PARPUFZEITLUPE, VAR_PUF_ZEITLUPE, ObjectCheck, 0);

/*
	CrtObjectDOInsert (window, PARMAASPIN, VAR_MAA_SPERRE_INNEN, ObjectCheck, 0);
	CrtObjectDOInsert (window, PARMAASPOUT, VAR_MAA_SPERRE_AUSSEN, ObjectCheck, 0);
*/
	CrtObjectDOInsert (window, PARMAESPIN, VAR_MAE_SPERRE_INNEN, ObjectCheck, 0);
	CrtObjectDOInsert (window, PARMAESPOUT, VAR_MAE_SPERRE_AUSSEN, ObjectCheck, 0);
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
	
	window = search_window (CLASS_PAR, SRCH_ANY, PAR_SETUP);
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (par_setup, ROOT, EDITABLE);
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

	window = search_window (CLASS_DIALOG, SRCH_ANY, IPAR);
		
	if (window == NULL)
	{
		 form_center (par_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (par_info, NULL, IPAR, (BYTE *)par_text [FPARN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = par_info;
		sprintf(s, "%-20s", PARDATE);
		set_ptext (par_info, PARIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (par_info, PARCOMPILE, s);
		sprintf(s, "%-20s", PARVERSION);
		set_ptext (par_info, PARIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (par_info, PARISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (par_info, PARIAKT, s);

		if (! open_dialog (IPAR)) hndl_alert (ERR_NOOPEN);
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
		module->class_number		= CLASS_PAR;
		module->icon				= &par_desk[PARICON];
		module->icon_position 	= IPAR;
		module->icon_number		= IPAR;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MPAR;
		module->menu_item			= MPAR;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_par;
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
		window = crt_mod (par_setup, NULL, PAR_SETUP);
		/* Modul-Struktur einbinden */
		window->module = (VOID*) module;
		module->window = window;
				
/*
		/* Messages einklinken */
		for (x = 0; x < 12; x++)
			add_rcv(VAR_LFA_ON1 + x, module);
		add_rcv(VAR_LFA_UMK, module);
		add_rcv(VAR_LFA_PAUSE, module);
	
		for (x = 0; x < 12; x++)
			add_rcv(VAR_LFB_ON1 + x, module);	
		add_rcv(VAR_LFB_UMK, module);
		add_rcv(VAR_LFB_PAUSE, module);	

		for (x = 0; x < MAXSIGNALS; x++)
			add_rcv(VAR_MTR_ON0 + x, module);	
		add_rcv(VAR_MTR_UMK, module);	
		add_rcv(VAR_MTR_PAUSE, module);
	
		for (x = 0; x < MAXSIGNALS; x++)
		{
			add_rcv(VAR_PUF_REC_SIG0 + x, module);
			add_rcv(VAR_PUF_PLAY_SIG0 + x, module);	
		} /* for */
		add_rcv(VAR_PUF_PAUSE, module);	
		add_rcv(VAR_PUF_ZEITL, module);	
		add_rcv(VAR_MAA_SPERRE_INNEN, module);
		add_rcv(VAR_MAA_SPERRE_AUSSEN, module);	
		add_rcv(VAR_MAE_SPERRE_INNEN, module);	
		add_rcv(VAR_MAE_SPERRE_AUSSEN, module);
*/	
	
		add_rcv(VAR_SET_PAR, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_PAR, MAXSETUPS);
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
  par_setup = (OBJECT *)rs_trindex [PAR_SETUP]; /* Adresse der PAR-Parameter-Box */
  par_help  = (OBJECT *)rs_trindex [PAR_HELP];	/* Adresse der PAR-Hilfe */
  par_desk  = (OBJECT *)rs_trindex [PAR_DESK];	/* Adresse des PAR-Desktops */
  par_text  = (OBJECT *)rs_trindex [PAR_TEXT];	/* Adresse der PAR-Texte */
  par_info 	= (OBJECT *)rs_trindex [PAR_INFO];	/* Adresse der PAR-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (par_rsc_ptr, rsc_name))
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

  rs_gaddr (par_rsc_ptr, R_TREE,  PAR_SETUP,	&par_setup);   /* Adresse der PAR-Parameter-Box */
  rs_gaddr (par_rsc_ptr, R_TREE,  PAR_HELP,	&par_help);    /* Adresse der PAR-Hilfe */
  rs_gaddr (par_rsc_ptr, R_TREE,  PAR_DESK,	&par_desk);    /* Adresse der PAR-Desktop */
  rs_gaddr (par_rsc_ptr, R_TREE,  PAR_TEXT,	&par_text);    /* Adresse der PAR-Texte */
  rs_gaddr (par_rsc_ptr, R_TREE,  PAR_INFO,	&par_info);    /* Adresse der PAR-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (par_setup, TRUE);
	fix_objs (par_help, TRUE);
	fix_objs (par_desk, TRUE);
	fix_objs (par_text, TRUE);
	fix_objs (par_info, TRUE);
	
	/*
	do_flags (par_setup, PARCANCEL, UNDO_FLAG);
	do_flags (par_setup, PARHELP, HELP_FLAG);
	*/
	menu_enable(menu, MPAR, TRUE);

	return (TRUE);
} /* init_rsc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PRIVATE BOOLEAN term_rsc ()

{
	BOOLEAN ok = TRUE;

#if ((XRSC_CREATE||RSC_CREATE) == 0)
	ok = rs_free (par_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

	return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_par ()
{
	BOOLEAN				ok = TRUE;
	 
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

	return (ok);
} /* init_par */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	
	ok &= term_rsc ();
	return (ok);
} /* term_mod */
