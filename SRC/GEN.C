/*****************************************************************************/
/*                                                                           */
/* Modul: GEN.C                                                              */
/*                                                                           */
/*****************************************************************************/
#define GENVERSION "V 1.04"
#define GENDATE "19.02.95"

/*****************************************************************************
V 1.04
- 0-Unterdrckung in set_dbox, 19.02.95
- ClickSetupField eingebaut, 30.01.95
- GetP auf 3. Parameter umgestellt
V 1.03
- auf SetPxxx Funktionen umgestellt
- dispobj.h ausgebaut
- gen_module eingebaut
- GEN-Mini ausgebaut
V 1.02 4.3.94
- load_create_infos und instance_count eingebaut
- window->module eingebaut
- module-type fr setup in define_setup nun other
- Umbau auf create_window_obj
- Bug in create (module->window) beseitigt
- import_status in import
V 1.01
- Umstellung auf neue RTMCLASS-Struktur
*****************************************************************************/
#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "gen_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"

#include "export.h"
#include "gen.h"

#if XRSC_CREATE
#include "gen_mod.rsh"
#include "gen_mod.rh"
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

#define MOD_RSC_NAME "GEN_MOD.RSC"		/* Name der Resource-Datei */
#define MAXSETUPS 50000L			/* Anzahl der GEN-Setups */

/****** TYPES ****************************************************************/
typedef struct setup
{
	LONG	lfa_setup,
			lfb_setup,
			maa_setup,
			mae_setup,
			mtr_setup,
			spg_setup,
			spo_setup,
			sps_setup,
			gep_setup,				/* General-Fhrungspunkt */
			rot_setup,
			man_setup,
			var_setup;
	WORD	lfa_prop,
			lfb_prop,
			maa_prop,	
			mae_prop,	
			mtr_prop,	
			spg_prop,
			spo_prop,
			sps_prop,
			rel_mtr_lfo,			/* Verh„ltnis LFO zu MTR-Speed */
			zoom_prop;				/* Zoom Proport-Faktor */
} SETUP;	/* Datenstruktur zum merken eines einzelnen General-Setups */

typedef	struct setup *SET_P;

typedef	struct status *STAT_P;

typedef struct status
{
	BOOLEAN new;					/* neue Werte */
} STATUS;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "GEN";		/* Name, fr Extension etc. */

/****** VARIABLES ************************************************************/
PRIVATE WORD	gen_rsc_hdr;					/* Zeigerstruktur fr RSC-Datei */
PRIVATE WORD	*gen_rsc_ptr = &gen_rsc_hdr;		/* Zeigerstruktur fr RSC-Datei */
PRIVATE OBJECT *gen_setup;
PRIVATE OBJECT *gen_help;
PRIVATE OBJECT *gen_desk;
PRIVATE OBJECT *gen_text;
PRIVATE OBJECT *gen_info;

/****** FUNCTIONS ************************************************************/

/* Interne GEN-Funktionen */
PRIVATE VOID		dsetup			_((WINDOWP window));
PRIVATE VOID		get_setup		_((RTMCLASSP module));
PRIVATE VOID		set_setup		_((RTMCLASSP module));
PRIVATE VOID		click_setup		_((WINDOWP window, MKINFO *mk));
PRIVATE RTMCLASSP define_setup	_((WINDOWP window, RTMCLASSP refmodule));

PRIVATE BOOLEAN	init_standard 	_((RTMCLASSP module));

PRIVATE BOOLEAN	init_rsc			_((VOID));
PRIVATE BOOLEAN	term_rsc			_((VOID));

/*****************************************************************************/

PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;

	GetPLong (gen_setup, GENSETNR, &edited->number);

	GetPLong (gen_setup, GENLFASET, &ed->lfa_setup);
	GetPLong (gen_setup, GENLFBSET, &ed->lfb_setup);
	GetPLong (gen_setup, GENMAASET, &ed->maa_setup);
	GetPLong (gen_setup, GENMAESET, &ed->mae_setup);
	GetPLong (gen_setup, GENMTRSET, &ed->mtr_setup);
	GetPLong (gen_setup, GENSPGSET, &ed->spg_setup);
	GetPLong (gen_setup, GENSPOSET, &ed->spo_setup);
	GetPLong (gen_setup, GENSPSSET, &ed->sps_setup);

	GetPLong (gen_setup, GENGEPSET, &ed->gep_setup);
	GetPLong (gen_setup, GENMANSET, &ed->man_setup);
	GetPLong (gen_setup, GENROTSET, &ed->rot_setup);
	GetPLong (gen_setup, GENVARSET, &ed->var_setup);

	GetPWord (gen_setup, GENLFAPROP, &ed->lfa_prop);
	GetPWord (gen_setup, GENLFBPROP, &ed->lfb_prop);
	GetPWord (gen_setup, GENMAAPROP, &ed->maa_prop);
	GetPWord (gen_setup, GENMAEPROP, &ed->mae_prop);
	GetPWord (gen_setup, GENMTRPROP, &ed->mtr_prop);
	GetPWord (gen_setup, GENSPGPROP, &ed->spg_prop);
	GetPWord (gen_setup, GENSPOPROP, &ed->spo_prop);
	GetPWord (gen_setup, GENSPSPROP, &ed->sps_prop);

	GetPWord (gen_setup, GENZOOMPROP, &ed->zoom_prop);
	GetPWord (gen_setup, GENRELMTRLFO, &ed->rel_mtr_lfo);
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STRING 	s;
	
	SetPLongN (gen_setup, GENLFASET, ed->lfa_setup);
	SetPLongN (gen_setup, GENLFBSET, ed->lfb_setup);
	SetPLongN (gen_setup, GENMAASET, ed->maa_setup);
	SetPLongN (gen_setup, GENMAESET, ed->mae_setup);
	SetPLongN (gen_setup, GENMTRSET, ed->mtr_setup);
	SetPLongN (gen_setup, GENSPGSET, ed->spg_setup);
	SetPLongN (gen_setup, GENSPOSET, ed->spo_setup);
	SetPLongN (gen_setup, GENSPSSET, ed->sps_setup);

	SetPLongN (gen_setup, GENGEPSET, ed->gep_setup);
	SetPLongN (gen_setup, GENMANSET, ed->man_setup);
	SetPLongN (gen_setup, GENROTSET, ed->rot_setup);
	SetPLongN (gen_setup, GENVARSET, ed->var_setup);

	SetPWord (gen_setup, GENLFAPROP, ed->lfa_prop);
	SetPWord (gen_setup, GENLFBPROP, ed->lfb_prop);
	SetPWord (gen_setup, GENMAAPROP, ed->maa_prop);
	SetPWord (gen_setup, GENMAEPROP, ed->mae_prop);
	SetPWord (gen_setup, GENMTRPROP, ed->mtr_prop);
	SetPWord (gen_setup, GENSPGPROP, ed->spg_prop);
	SetPWord (gen_setup, GENSPOPROP, ed->spo_prop);
	SetPWord (gen_setup, GENSPSPROP, ed->sps_prop);

	SetPWord (gen_setup, GENZOOMPROP, ed->zoom_prop);
	SetPWord (gen_setup, GENRELMTRLFO, ed->rel_mtr_lfo);

	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (gen_setup, GENSETNR , s);
} /* set_dbox */

PRIVATE VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;

	send_variable (VAR_SET_GEN, module->actual->number);
	send_variable (VAR_SET_LFA, akt->lfa_setup);
	send_variable (VAR_SET_LFB, akt->lfb_setup);
	send_variable (VAR_SET_MAA, akt->maa_setup);
	send_variable (VAR_SET_MAE, akt->mae_setup);
	send_variable (VAR_SET_MTR, akt->mtr_setup);
	send_variable (VAR_SET_SPG, akt->spg_setup);
	send_variable (VAR_SET_SPO, akt->spo_setup);
	send_variable (VAR_SET_SPS, akt->sps_setup);
	send_variable (VAR_SET_GEP, akt->gep_setup);
	send_variable (VAR_SET_MAN, akt->man_setup);
	send_variable (VAR_SET_ROT, akt->rot_setup);
	send_variable (VAR_SET_VAR, akt->var_setup);

	send_variable (VAR_PROP_LFA, (LONG)akt->lfa_prop);
	send_variable (VAR_PROP_LFB, (LONG)akt->lfb_prop);
	send_variable (VAR_PROP_MAA, (LONG)akt->maa_prop);
	send_variable (VAR_PROP_MAE, (LONG)akt->mae_prop);
	send_variable (VAR_PROP_MTR, (LONG)akt->mtr_prop);
	send_variable (VAR_PROP_SPG, (LONG)akt->spg_prop);
	send_variable (VAR_PROP_SPO, (LONG)akt->spo_prop);
	send_variable (VAR_PROP_SPS, (LONG)akt->sps_prop);

	module->status->new = TRUE;
} /* send_messages */

/*****************************************************************************/

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD		variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	ED_P		actual	= module->actual;
	SET_P		akt		= actual->setup;
	STAT_P	status	= module->status;
	
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
} /* message */

PUBLIC BOOLEAN	import	(RTMCLASSP module, STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	STRING	ext, title, filter;
	FILE		*in;
	STR128	s;
	WORD		signal, setnr = 0;
	SET_P		akt = module->actual->setup; /* Zeiger auf erstes Setup */
	LONG		dummy, temp, max_setups = module->max_setups;
	STRING	name;
	LONG		*lfa_setup	= &akt->lfa_setup, 
				*lfb_setup 	= &akt->lfb_setup, 
				*maa_setup 	= &akt->maa_setup, 
				*mae_setup 	= &akt->mae_setup, 
				*mtr_setup 	= &akt->mtr_setup, 
				*spg_setup 	= &akt->spg_setup, 
				*spo_setup 	= &akt->spo_setup, 
				*sps_setup 	= &akt->sps_setup,
				*gep_setup 	= &akt->gep_setup,
				*man_setup 	= &akt->man_setup,
				*rot_setup 	= &akt->rot_setup,
				*var_setup 	= &akt->var_setup;
	WORD		*lfa_prop 	= &akt->lfa_prop,
				*lfb_prop 	= &akt->lfb_prop,
				*maa_prop 	= &akt->maa_prop,	
				*mae_prop 	= &akt->mae_prop,	
				*mtr_prop 	= &akt->mtr_prop,	
				*spg_prop 	= &akt->spg_prop,
				*spo_prop 	= &akt->spo_prop,
				*sps_prop 	= &akt->sps_prop,
				*rel_mtr_lfo = &akt->rel_mtr_lfo,
				*zoom_prop 	= &akt->zoom_prop;

	if (filename == NULL)
	{
		filename = s;
		strcpy (filename, module->import_name);
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
			akt = module->actual->setup; /* Zeiger auf erstes Setup nochmal holen, wegen Supervisor-Mll in file_split */
			
			daktstatus(" GEN-Datei wird importiert ... ", module->import_name);
			module->flags |= FLAG_IMPORTING;
			while (ok != EOF && setnr < max_setups)
			{
				setnr++;
				ok = fscanf(in, "%s", name);					/* GEN Nummer */
				ok = fscanf(in, "%ld", mtr_setup);			/* 0 MTR-Setup */
				if (*mtr_setup >= 1999) *mtr_setup = 0;	/* Standard */
				ok = fscanf(in, "%ld", lfa_setup);			/* 1 LFO-Setup */
				ok = fscanf(in, "%ld", &dummy);				/* 2 Input-Setup */
				if (*lfa_setup >= 1999 ) *lfa_setup = 0;	/* Standard */
				ok = fscanf(in, "%ld", &temp);				/* 3 SPI-Setup */
				if (temp >= 999 ) temp = 0;					/* Standard */
				if (temp < 400)
				{
					/* SPI->SPS */
					*spg_setup = 0;
					*spo_setup = 0;
					*sps_setup = temp;
				} /* if */
				else if (temp >= 400 && temp < 500)
				{
					/* SPI->SPG */
					*spg_setup = temp;
					*spo_setup = 0;
					*sps_setup = 0;
				} /* else if */
				else if (temp >= 500 && temp < 800)
				{
					/* SPI->SPO */
					*spg_setup = 0;
					*spo_setup = temp;
					*sps_setup = 0;
				} /* else if */
				
				ok = fscanf(in, "%d", &dummy);				/* 4 Signal-Setup */
				ok = fscanf(in, "%d", rel_mtr_lfo);			/* 5 Rel. MTR-LFO */
				
				/* Rest */
				*lfa_prop	= 100;
				*lfb_prop	= 100;
				*maa_setup	= 0;
				*maa_prop	= 100;	
				*mae_setup	= 0;
				*mae_prop	= 100;	
				*mtr_prop	= 100;
				*spg_prop	= 100;
				*spo_prop	= 100;
				*sps_prop	= 100;
				*gep_setup	= 0;		/* General-Fhrungspunkt */
				*man_setup	= 0;
				*rot_setup	= 0;
				*var_setup	= 0;
				*zoom_prop	= 100;	/* Zoom Proport-Faktor */
				if (! module->get_setnr(module, setnr))
					ok = EOF;	/* Import beenden */
				if (setnr % 20 == 0)
					set_daktstat((WORD)(100L*setnr/max_setups));
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
		case GENSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case GENSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case GENSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case GENSETSTORE:
			module->set_store (window, edited->number);
			break;
		case GENSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case GENOK   :
			module->set_ok (window);
			break;
		case GENCANCEL:
			module->set_cancel (window);
		   break;
		case GENHELP :
			module->help (module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case GENSTANDARD:
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
				set_ptext (gen_setup, GENSETNR, s);
				draw_object(window, GENSETNR);
			} /* if */
		} /* switch */
} /* wi_click_mod */

/*****************************************************************************/
/* Zeitablauf fr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (window)
WINDOWP window;
{

} /* wi_timer_mod */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;
{
	WINDOWP window;
	WORD    menu_height;
	
	window = search_window (CLASS_GEN, SRCH_OPENED, GEN_SETUP);
	
	if (window == NULL)
	{
		window = create_window_obj (KIND, CLASS_GEN);
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
		
		sprintf (window->name, (BYTE *)gen_text [FGENN].ob_spec);
		
	} /* if */
	
  return (window);                      /* Fenster zurckgeben */
} /* crt_mod */

/*****************************************************************************/
/* ™ffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN ok;
	WINDOWP window;
	RTMCLASSP	module;
	
	window = search_window (CLASS_GEN, SRCH_ANY, GEN_SETUP);
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (gen_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			module = Module(window);
			
			module->set_dbox (module);
			if (! open_window (window)) hndl_alert (ERR_NOOPEN);
		} /* if */
		else 
			top_window (window);
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
	WORD		ret;
	STRING	s;
	RTMCLASSP	module = Module(window);

	window = search_window (CLASS_DIALOG, SRCH_ANY, IGEN);
		
	if (window == NULL)
	{
		 form_center (gen_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (gen_info, NULL, IGEN, (BYTE *)gen_text [FGENN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = gen_info;
		sprintf(s, "%-20s", GENDATE);
		set_ptext (gen_info, GENIVERDA, s);
		sprintf(s, "%-20s",  __DATE__);
		set_ptext (gen_info, GENCOMPILE, s);
		sprintf(s, "%-20s", GENVERSION);
		set_ptext (gen_info, GENIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (gen_info, GENISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (gen_info, GENIAKT, s);

		if (! open_dialog (IGEN)) hndl_alert (ERR_NOOPEN);
	}

  return (window != NULL);
} /* info_mod */

/*****************************************************************************/
GLOBAL	RTMCLASSP create_gen ()
{
	WINDOWP		window;
	RTMCLASSP 	module;
	FILE			*fp;

	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_GEN;
		module->icon				= &gen_desk[GENICON];
		module->icon_position 	= IGEN;
		module->icon_number		= IGEN;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MGEN;
		module->menu_item			= MGEN;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_gen;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_CONTROL;
		module->message			= message;
		module->create				= create_gen;
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
		module->max_setups		 		= MAXSETUPS;
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
		window = crt_mod (gen_setup, NULL, GEN_SETUP);
		/* Modul-Struktur einbinden */
		window->module		= (VOID*) module;
		module->window		= window;
	
		add_rcv(VAR_SET_GEN, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_GEN, MAXSETUPS);
		add_rcv(VAR_SET_LFA, module);	/* Message einklinken */
		add_rcv(VAR_SET_LFB, module);	/* Message einklinken */
		add_rcv(VAR_SET_MAA, module);	/* Message einklinken */
		add_rcv(VAR_SET_MAE, module);	/* Message einklinken */
		add_rcv(VAR_SET_MTR, module);	/* Message einklinken */
		add_rcv(VAR_SET_SPG, module);	/* Message einklinken */
		add_rcv(VAR_SET_SPO, module);	/* Message einklinken */
		add_rcv(VAR_SET_SPS, module);	/* Message einklinken */
	
	} /* if */
	
	return module;
} /* create_gen */

PRIVATE BOOLEAN init_standard (RTMCLASSP module)
{
	SET_P	standard = module->standard;
	
	/* Setup-Strukturen initialisieren */
	mem_set(standard, 0, (UWORD) sizeof(SETUP));
	standard->lfa_prop = 100;
	standard->lfb_prop = 100;
	standard->maa_prop = 100;
	standard->mae_prop = 100;
	standard->mtr_prop = 100;
	standard->spg_prop = 100;
	standard->spo_prop = 100;
	standard->sps_prop = 100;
	standard->rel_mtr_lfo = 0;			/* Verh„ltnis LFO zu MTR-Speed */
	standard->zoom_prop = 100;				/* Zoom Proport-Faktor */
	
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
  gen_setup = (OBJECT *)rs_trindex [GEN_SETUP]; /* Adresse der GEN-Parameter-Box */
  gen_help  = (OBJECT *)rs_trindex [GEN_HELP];	/* Adresse der GEN-Hilfe */
  gen_desk  = (OBJECT *)rs_trindex [GEN_DESK];	/* Adresse des GEN-Desktops */
  gen_text  = (OBJECT *)rs_trindex [GEN_TEXT];	/* Adresse der GEN-Texte */
  gen_info 	= (OBJECT *)rs_trindex [GEN_INFO];	/* Adresse der GEN-Info-Anzeige */
#else

  rs_gaddr (gen_rsc_ptr, R_TREE,  GEN_SETUP,	&gen_setup);   /* Adresse der GEN-Parameter-Box */
  rs_gaddr (gen_rsc_ptr, R_TREE,  GEN_HELP,	&gen_help);    /* Adresse der GEN-Hilfe */
  rs_gaddr (gen_rsc_ptr, R_TREE,  GEN_DESK,	&gen_desk);    /* Adresse des GEN-Desktop */
  rs_gaddr (gen_rsc_ptr, R_TREE,  GEN_TEXT,	&gen_text);    /* Adresse der GEN-Texte */
  rs_gaddr (gen_rsc_ptr, R_TREE,  GEN_INFO,	&gen_info);    /* Adresse der GEN-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (gen_setup, TRUE);
	fix_objs (gen_help, TRUE);
	fix_objs (gen_desk, TRUE);
	fix_objs (gen_text, TRUE);
	fix_objs (gen_info, TRUE);
	
	do_flags (gen_setup, GENCANCEL, UNDO_FLAG);
	do_flags (gen_setup, GENHELP, HELP_FLAG);
	
	menu_enable(menu, MGEN, TRUE);

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
  ok = rs_free (gen_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_gen ()
{
	BOOLEAN				ok = TRUE;

	ok &= init_rsc ();
	instance_count = load_create_infos (create_gen, module_name, max_instances);
	
	return (ok);
} /* init_gen */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	ok &= term_rsc ();
	return (ok);
} /* term_mod */
