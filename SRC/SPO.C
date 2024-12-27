/*****************************************************************************/
/*                                                                           */
/* Modul: SPO.C                                                              */
/*                                                                           */
/*****************************************************************************/
#define SPOVERSION "V 1.03"
#define SPODATE "19.02.95"

/*****************************************************************************
V 1.03
- auf Getxxx Setxxx umgestellt, 19.02.95
- ClickSetupField eingebaut, 30.01.95
- load_create_infos und instance_count eingebaut
- Fehler in apply fr y und z beseitigt
V 1.02
- import modernisiert
- (RTMCLASSP)window->module eingebaut
- import modifiziert
- Umbau auf create_window_obj
V 1.01
- Bug in create (module->window) beseitigt
V 1.00
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
#include "spo_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"

#include "export.h"
#include "spo.h"

#if XRSC_CREATE
#include "spo_mod.rsh"
#include "spo_mod.rh"
#endif

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 4 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangsh”he in Pixel */
#define MILLI  0  	                   /* Millisekunden fr Zeitablauf */

#define MOD_RSC_NAME "SPO_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 1000L					/* Anzahl der SPO-Setups */

/****** TYPES ****************************************************************/
typedef struct setup *SET_P;

typedef struct spo_single
{
	WORD	offset_x,		/* Verschiebung auf der X-Achse */
			offset_y,		/* Verschiebung auf der Y-Achse */
			offset_z;		/* Verschiebung auf der Z-Achse */
	UINT	prop_x	: 1,	/* Flags fr proportional an/aus */
			prop_y	: 1,
			prop_z	: 1;
} SPO_SINGLE;				/* Enth„lt alle SPO-Parameter eines einzelne Signals */

typedef struct setup
{
	SPO_SINGLE	spo_single [MAXSIGNALS];	/* Enth„lt die SPO-Informationen fr die einzelnen Kan„le */
} SETUP;				/* Enth„lt alle Parameter einer kompletten SPO-Einstellung */

typedef struct status
{
	WORD	prop;		/* Proportional-Faktor */
} STATUS;
/****** VARIABLES ************************************************************/

/* Resource */
PRIVATE OBJECT *spo_desk;
PRIVATE OBJECT *spo_setup;
PRIVATE OBJECT *spo_help;
PRIVATE OBJECT *spo_text;
PRIVATE OBJECT *spo_info;

PRIVATE WORD	spo_rsc_hdr;					/* Zeigerstruktur fr RSC-Datei */
PRIVATE WORD	*spo_rsc_ptr = &spo_rsc_hdr;		/* Zeigerstruktur fr RSC-Datei */

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 20;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "SPO";		/* Name, fr Extension etc. */

/****** FUNCTIONS ************************************************************/
/* Interne SPO-Funktionen */
PRIVATE BOOLEAN init_rsc		_((VOID));
PRIVATE BOOLEAN term_rsc		_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STRING 	s;
	WORD 		signal, offset;
	SPO_SINGLE *spo_s;
	
	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		offset=(SPO1-SPO0)*signal;
		spo_s=&ed->spo_single[signal];
		
		spo_s->prop_x = GetCheck (spo_setup, SPOPROPX0 + offset, NULL);
		spo_s->prop_y = GetCheck (spo_setup, SPOPROPY0 + offset, NULL);
		spo_s->prop_z = GetCheck (spo_setup, SPOPROPZ0 + offset, NULL);
		
		GetPWord (spo_setup, SPOX0 + offset, &spo_s->offset_x);
		GetPWord (spo_setup, SPOY0 + offset, &spo_s->offset_y);
		GetPWord (spo_setup, SPOZ0 + offset, &spo_s->offset_z);
	} /* if */
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STRING	s;
	WORD		offset, signal;
	SPO_SINGLE *spo_s;
	
	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		offset=(SPO1-SPO0)*signal;
		spo_s=&ed->spo_single[signal];
		
		SetCheck (spo_setup, SPOPROPX0 + offset, spo_s->prop_x);
		SetCheck (spo_setup, SPOPROPY0 + offset, spo_s->prop_y);
		SetCheck (spo_setup, SPOPROPZ0 + offset, spo_s->prop_z);
			
		SetPWordN (spo_setup, SPOX0 + offset, spo_s->offset_x);
		SetPWordN (spo_setup, SPOY0 + offset, spo_s->offset_y);
		SetPWordN (spo_setup, SPOZ0 + offset, spo_s->offset_z);
	} /* for */
	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (spo_setup, SPOSETNR , s);
} /* set_dbox */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	send_variable(VAR_SET_SPO, module->actual->number);
} /* send_messages */
/*****************************************************************************/
PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	SPO_SINGLE	*spo_s;
	SET_P			akt = module->actual->setup;
	REG WORD		k;		/* Temp. Variable wg. šberlauf */
	POINT_3D		*point;
	KOOR_ALL		*koor = event->koors;
	UWORD			signal;
	LONG			prop = (LONG)module->status->prop;

	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		spo_s = &akt->spo_single[signal];
		point = &koor->koor[signal].koor;
		
		k = point->x;
		
		if (spo_s->prop_x)
			k += (WORD)((LONG)spo_s->offset_x * prop / 100);
		else
			k += spo_s->offset_x;

		if (k > MAXKOOR)
			k =  MAXKOOR;
		else if (k < -MAXKOOR)
			k = -MAXKOOR;

		point->x = (BYTE) k;
		
		k = point->y;

		if (spo_s->prop_y)
			k += (WORD)((LONG)spo_s->offset_y * prop / 100);
		else
			k += spo_s->offset_y;
	
		if (k > MAXKOOR)
			k =  MAXKOOR;
		else if (k < -MAXKOOR)
			k = -MAXKOOR;
	
		point->y = (BYTE) k;

		k = point->z;

		if (spo_s->prop_z)
			k += (WORD)((LONG)spo_s->offset_z * prop / 100);
		else
			k += spo_s->offset_z;

		if (k > MAXKOOR)
			k =  MAXKOOR;
		else if (k < -MAXKOOR)
			k = -MAXKOOR;

		point->z = (BYTE) k;

	} /* for */

	return event;
} /* apply */

PUBLIC BOOLEAN	import	(RTMCLASSP module, STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	FILE		*in;
	STRING	s;
	WORD		signal, x;
	SET_P		akt = module->actual->setup;
	SPO_SINGLE	*single;
	LONG		setnr, max_setups = module->max_setups;

	if (filename == NULL)
	{
		filename = s;
		strcpy (filename, module->import_name);
	} /* if */
	
	if (fileselect)
	{
		file_split (filename, NULL, NULL, filename, NULL);
		ok = select_file (filename, import_path, "*.EXP", "IMPORT von SPO-Dateien", module->import_name);
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
			daktstatus(" SPO-Datei wird importiert ... ", module->import_name);
			setnr = 1;
			module->flags |= FLAG_IMPORTING;
			while (ok != EOF && setnr < max_setups)
			{
				/* Zeiger auf Info des ersten Signals */
				single = akt->spo_single;
				for (signal = 0; signal < MAXSIGNALS; signal++)
				{
					ok = fscanf(in, "%d", &single->offset_x);
					ok = fscanf(in, "%d", &single->offset_y);
					ok = fscanf(in, "%d", &single->offset_z);
					single++;	/* Auf Info fr n„chstes Signal zeigen */
				} /* for */
				/* ok = fscanf(in, "%s", s);	/* Leerzeile */ */

				/* Setup speichern und n„chstes Setup anw„hlen */
				module->get_setnr(module, setnr);
				if (setnr % 20 == 0)
					set_daktstat((WORD)(100L*setnr/max_setups));
				setnr++;
			} /* while */
			module->actual->modified = TRUE;
			module->flags &= ~FLAG_IMPORTING;
			fclose(in);
			set_daktstat(100);
			close_daktstat();
		} /* else */
	} /* if */

	return (ok);
} /* import */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	/* Zurcksetzen von Werten */
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
				case VAR_SET_SPO:
					module->set_setnr(module, value);	break;
				case VAR_PROP_SPO:
					module->status->prop = (WORD)value;
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
	STRING		s;
	static		LONG x = 0;	
	RTMCLASSP	module = Module(window);
	ED_P			edited = module->edited;

	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case SPOSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case SPOSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case SPOSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case SPOSETSTORE:
			module->set_store (window, edited->number);
			break;
		case SPOSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case SPOOK   :
			module->set_ok (window);
			break;
		case SPOCANCEL:
			module->set_cancel (window);
		   break;
		case SPOHELP :
			module->help (module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case SPOLEARN:
			break;
		case SPOSTANDARD:
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
				set_ptext (spo_setup, SPOSETNR, s);
				draw_object(window, SPOSETNR);
			} /* if */
			module->get_dbox(module);
			undo_state (window->object, window->exit_obj, SELECTED);
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
  WORD    menu_height;

  window = create_window_obj (KIND, CLASS_SPO);

  if (window != NULL)
  {
    menu_height = (menu != NULL) ? gl_hattr : 0;

    window->flags     = FLAGS | WI_MODELESS;
    window->icon 	    = icon;
    window->doc.x     = 0;
    window->doc.y     = 0;
    window->doc.w     = 0;
    window->doc.h     = 0;
    window->xfac      = XFAC;
    window->yfac      = YFAC;
    window->xunits    = XUNITS;
    window->yunits    = YUNITS;
    window->scroll.x  = INITX;
    window->scroll.y  = INITY;
    window->scroll.w  = obj->ob_width;
    window->scroll.h  = obj->ob_height;
    window->work.x    = window->scroll.x;
    window->work.y    = window->scroll.y - menu_height;
    window->work.w    = window->scroll.w;
    window->work.h    = window->scroll.h + menu_height;
    window->bg_color  = -1;
    window->mousenum  = ARROW;
    window->mouseform = NULL;
    window->milli     = MILLI;
    window->module   = 0;
    window->edit_obj  = 0;
    window->edit_inx  = 0;
    window->exit_obj  = 0;
    window->object    = obj;
    window->menu      = menu;
    window->click     = wi_click_mod;
    window->showinfo  = info_mod;

    sprintf (window->name, (BYTE *)spo_text [FSPON].ob_spec);
    sprintf (window->info, (BYTE *)spo_text [FSPOI].ob_spec, 0);
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
	
	window = search_window (CLASS_SPO, SRCH_ANY, SPO_SETUP);
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (spo_setup, ROOT, EDITABLE);
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
	WORD		ret;
	STRING	s;
	RTMCLASSP	module = Module(window);

	window = search_window (CLASS_DIALOG, SRCH_ANY, ISPO);
		
	if (window == NULL)
	{
		 form_center (spo_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (spo_info, NULL, ISPO, (BYTE *)spo_text [FSPON].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = spo_info;
		sprintf(s, "%-20s", SPODATE);
		set_ptext (spo_info, SPOIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (spo_info, SPOCOMPILE, s);
		sprintf(s, "%-20s", SPOVERSION);
		set_ptext (spo_info, SPOIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (spo_info, SPOISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (spo_info, SPOIAKT, s);

		if (! open_dialog (ISPO)) hndl_alert (ERR_NOOPEN);
	}

  return (window != NULL);
} /* info_mod */

/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	WINDOWP		window;
	RTMCLASSP 	module;
	FILE			*fp;

	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_SPO;
		module->icon				= &spo_desk[SPOICON];
		module->icon_position 	= ISPO;
		module->icon_number		= ISPO;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MSPO;
		module->menu_item			= MSPO;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;

		module->open				= open_mod;
		module->info				= info_mod;

		module->init				= init_spo;
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

		/* Setup-Strukturen initialisieren */
		mem_set(module->standard, 0, (UWORD) sizeof(SETUP));
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (spo_setup, NULL, SPO_SETUP);
		/* Modul-Struktur einbinden */
		window->module 	= (VOID*) module;
		module->window		= window;
	
		add_rcv(VAR_SET_SPO,  module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_SPO, MAXSETUPS);
		add_rcv(VAR_PROP_SPO, module);	/* Message einklinken */
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
  spo_setup = (OBJECT *)rs_trindex [SPO_SETUP]; /* Adresse der SPO-Parameter-Box */
  spo_help  = (OBJECT *)rs_trindex [SPO_HELP];	/* Adresse der SPO-Hilfe */
  spo_desk  = (OBJECT *)rs_trindex [SPO_DESK]; 	/* Adresse des SPO-Desktop */
  spo_text  = (OBJECT *)rs_trindex [SPO_TEXT];	/* Adresse der SPO-Texte */
  spo_info 	= (OBJECT *)rs_trindex [SPO_INFO];	/* Adresse der SPO-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (spo_rsc_ptr, rsc_name))
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

  rs_gaddr (spo_rsc_ptr, R_TREE,  SPO_SETUP,	&spo_setup);   /* Adresse der SPO-Parameter-Box */
  rs_gaddr (spo_rsc_ptr, R_TREE,  SPO_HELP,	&spo_help);    /* Adresse der SPO-Hilfe */
  rs_gaddr (spo_rsc_ptr, R_TREE,  SPO_DESK,	&spo_desk);   	/* Adresse des SPO-Desktop */
  rs_gaddr (spo_rsc_ptr, R_TREE,  SPO_TEXT,	&spo_text);    /* Adresse der SPO-Texte */
  rs_gaddr (spo_rsc_ptr, R_TREE,  SPO_INFO,	&spo_info);    /* Adresse der SPO-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (spo_desk, TRUE);
	fix_objs (spo_setup, TRUE);
	fix_objs (spo_help, TRUE);
	fix_objs (spo_text, TRUE);
	fix_objs (spo_info, TRUE);
	
	
	do_flags (spo_setup, SPOCANCEL, UNDO_FLAG);
	do_flags (spo_setup, SPOHELP, HELP_FLAG);
	
	menu_enable(menu, MSPO, TRUE);

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
  ok = rs_free (spo_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_spo ()
{
	BOOLEAN	ok = TRUE;

	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

	return (ok);
} /* init_spo */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;

	ok &= term_rsc ();
	return (ok);
} /* term_mod */
