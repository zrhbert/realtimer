/*****************************************************************************/
/*                                                                           */
/* Modul: SPS.C                                                              */
/*                                                                           */
/*****************************************************************************/
#define SPSVERSION "V 1.04"
#define SPSDATE "30.01.95"

/*****************************************************************************
V 1.04
- ClickSetupField eingebaut, 30.01.95
V 1.03
- Bug in get_sps_flags (bei import) beseitigt, 14.01.94
V 1.02
- load_create_infos und instance_count eingebaut
- Fehler in get_sps_macro beseitigt
V 1.01
- import modernisiert
- window->module eingebaut
- import modifiziert
- Bug in create (module->window) beseitigt
V 1.00
- Umstellung auf neue RTMCLASS-Struktur
*****************************************************************************/

#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE */                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "sps_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"

#include "export.h"
#include "sps.h"

#if XRSC_CREATE
#include "sps_mod.rsh"
#include "sps_mod.rh"
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
#define MILLI  0                     /* Millisekunden fr Zeitablauf */

#define MOD_RSC_NAME "SPS_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 1000l					/* Anzahl der SPS-Setups */

#define SPIEGEL_X 0x0001					/* fr Makro-Berechnung */
#define SPIEGEL_Y 0x0002
#define SPIEGEL_Z 0x0004
#define TAUSCH_XY 0x0008
#define TAUSCH_XZ 0x0010
#define TAUSCH_YZ 0x0020
/****** TYPES ****************************************************************/

typedef struct sps_single
{
	UINT	spiegel_x : 1;	/* Bitfeld fr "Spiegelung der X-Koordinate" */
	UINT	spiegel_y : 1;	/* Bitfeld fr "Spiegelung der X-Koordinate" */
	UINT	spiegel_z : 1;	/* Bitfeld fr "Spiegelung der X-Koordinate" */
	UINT	tausch_xy : 1;	/* Bitfeld fr "Vertauschung X-Y LU/RO" */
	UINT	tausch_xz : 1;	/* Bitfeld fr "Vertauschung X-Z LH/RV" */
	UINT	tausch_yz : 1;	/* Bitfeld fr "Vertauschung Y-Z OV/UH" */
} SPS_SINGLE;				/* Enth„lt alle SPS-Parameter eines einzelne Signals */

typedef struct setup *SET_P;

typedef struct setup
{
	SPS_SINGLE	sps_single [MAXSIGNALS]; /* Enth„lt die SPS-Informationen fr die einzelnen Kan„le */
} SETUP;	/* Enth„lt alle Parameter einer kompletten SPS-Einstellung */

typedef struct status *STAT_P;

typedef struct status
{
	WORD	prop;		/* Proportional-Faktor */
} STATUS;

/****** VARIABLES ************************************************************/
PRIVATE WORD	sps_rsc_hdr;					/* Zeigerstruktur fr RSC-Datei */
PRIVATE WORD	*sps_rsc_ptr = &sps_rsc_hdr;		/* Zeigerstruktur fr RSC-Datei */
PRIVATE OBJECT *sps_setup;
PRIVATE OBJECT *sps_help;
PRIVATE OBJECT *sps_desk;
PRIVATE OBJECT *sps_text;
PRIVATE OBJECT *sps_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 20;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "SPS";		/* Name, fr Extension etc. */

/****** FUNCTIONS ************************************************************/
/* Interne SPS-Funktionen */
PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));
PRIVATE VOID set_sps_flags		_((SPS_SINGLE *single, WORD x));
PRIVATE WORD get_sps_macro		_((SPS_SINGLE *single));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	WORD 		signal, offset;
	SPS_SINGLE *sps_s;

	sps_s = ed->sps_single;
	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		offset=(SPS1-SPS0)*signal;
		sps_s->spiegel_x = get_checkbox (sps_setup, SPSSPIEGELX0 + offset);
		sps_s->spiegel_y = get_checkbox (sps_setup, SPSSPIEGELY0 + offset);
		sps_s->spiegel_z = get_checkbox (sps_setup, SPSSPIEGELZ0 + offset);
		sps_s->tausch_xy = get_checkbox (sps_setup, SPSTAUSCHXY0 + offset);
		sps_s->tausch_xz = get_checkbox (sps_setup, SPSTAUSCHXZ0 + offset);
		sps_s->tausch_yz = get_checkbox (sps_setup, SPSTAUSCHYZ0 + offset);
		sps_s++;
	} /* for */
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STRING	s;
	WORD		signal, offset;
	SPS_SINGLE *sps_s;

	sps_s = ed->sps_single;
	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		offset=(SPS1-SPS0)*signal;
		set_checkbox (sps_setup, SPSSPIEGELX0 + offset, sps_s->spiegel_x);
		set_checkbox (sps_setup, SPSSPIEGELY0 + offset, sps_s->spiegel_y);
		set_checkbox (sps_setup, SPSSPIEGELZ0 + offset, sps_s->spiegel_z);
		set_checkbox (sps_setup, SPSTAUSCHXY0 + offset, sps_s->tausch_xy);
		set_checkbox (sps_setup, SPSTAUSCHXZ0 + offset, sps_s->tausch_xz);
		set_checkbox (sps_setup, SPSTAUSCHYZ0 + offset, sps_s->tausch_yz);
		sprintf(s, "%2d", get_sps_macro(sps_s));
		set_ptext (sps_setup, SPSSINGLENR + offset, s);
		sps_s++;
	} /* for */

	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (sps_setup, SPSSETNR , s);
} /* set_dbox */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	send_variable(VAR_SET_SPS, module->actual->number);
} /* send_messages */

/*****************************************************************************/

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	SPS_SINGLE	*sps_s;
	SET_P			akt = module->actual->setup;
	POINT_3D		*point;
	KOOR_ALL		*koor = event->koors;
	UWORD			signal;

	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		sps_s=&(akt->sps_single[signal]);
		point = &(koor->koor[signal].koor);
		if (sps_s->spiegel_x) point->x *= -1;
		if (sps_s->spiegel_y) point->y *= -1;
		if (sps_s->spiegel_z) point->z *= -1;
		if (sps_s->tausch_xy) swap_byte(&(point->x), &(point->y));
		if (sps_s->tausch_xz) swap_byte(&(point->x), &(point->z));
		if (sps_s->tausch_yz) swap_byte(&(point->y), &(point->z));
	} /* for */

	return event;
} /* apply */

PUBLIC BOOLEAN	import	(RTMCLASSP module, STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	STRING	ext, title, filter;
	FILE		*in;
	STR128	s;
	WORD		signal, x;
	SET_P		akt = module->actual->setup;
	SPS_SINGLE	*single;
	LONG		setnr = 1;
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
			/* Zeiger auf erstes Setup nochmal holen, wegen Supervisor-Mll in file_split */
			akt = module->actual->setup;
			daktstatus(" SPS-Datei wird importiert ... ", module->import_name);
			module->flags |= FLAG_IMPORTING;
			while (ok != EOF && setnr < max_setups)
			{
				/* Zeiger auf Info des ersten Signals */
				single = akt->sps_single;
				for (signal = 0; signal < MAXSIGNALS; signal++)
				{
					ok = fscanf(in, "%d", &x);

					/* Erste Info war Mausmodus */
					if (signal > 0)
						set_sps_flags(single, x);
					else
						set_sps_flags(single, 0);
					
					single++;	/* Auf Info fr n„chste Signal zeigen */
				} /* for */
				/* ok = fscanf(in, "%s", s);	/* Leerzeile */ */

				/* Setup speichern und n„chstes Setup anw„hlen */
				if (! module->get_setnr(module, setnr))
					ok = EOF;	/* Import beenden */
				if (setnr % 20 == 0)
					set_daktstat((WORD)(100L*setnr/max_setups));
				setnr++;
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
				case VAR_SET_SPS:
					module->set_setnr(module, value);
					break;
				case VAR_PROP_SPS:
					module->status->prop = (WORD)value;
					break;
			} /* switch */
			break;
	} /* switch */
} /* message */

PRIVATE VOID set_sps_flags(SPS_SINGLE *single, WORD x)
{
	single->spiegel_x = (x & SPIEGEL_X) ? TRUE : FALSE;
	single->spiegel_y = (x & SPIEGEL_Y) ? TRUE : FALSE;
	single->spiegel_z = (x & SPIEGEL_Z) ? TRUE : FALSE;
	single->tausch_xy = (x & TAUSCH_XY) ? TRUE : FALSE;
	single->tausch_xz = (x & TAUSCH_XZ) ? TRUE : FALSE;
	single->tausch_yz = (x & TAUSCH_YZ) ? TRUE : FALSE;
} /* set_sps_flags */

PRIVATE WORD get_sps_macro (SPS_SINGLE *single)
{
	/* Summierung aller Flags gibt Rckgabewerte */
	return  SPIEGEL_X * single->spiegel_x
			+ SPIEGEL_Y * single->spiegel_y
			+ SPIEGEL_Z * single->spiegel_z
			+ TAUSCH_XY * single->tausch_xy
			+ TAUSCH_XZ * single->tausch_xz
			+ TAUSCH_YZ * single->tausch_yz;
} /* get_sps_macro */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
WINDOWP window;
MKINFO  *mk;
{
	STRING	s;
	UWORD		signal, offset;
	static	LONG x = 0;	
	RTMCLASSP	module = Module(window);
	ED_P			edited = module->edited;
	SET_P		ed = edited->setup;
	SPS_SINGLE *sps_s;
	WORD		macro;
	BOOLEAN	found = FALSE;
	
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case SPSSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case SPSSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case SPSSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case SPSSETSTORE:
			module->set_store (window, edited->number);
			break;
		case SPSSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case SPSOK   :
			module->set_ok (window);
			break;
		case SPSCANCEL:
			module->set_cancel (window);
			break;
		case SPSHELP :
			module->help (module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case SPSSTANDARD:
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
				set_ptext (sps_setup, SPSSETNR, s);
				draw_object(window, SPSSETNR);
			} /* if */
			for(signal=0; signal<MAXSIGNALS && ! found; signal++)
			{
				offset = (SPS1-SPS0)*signal;
				sps_s = &ed->sps_single[signal];
				switch (window->exit_obj-offset)
				{
					case SPSSINGLENR:
						get_ptext (sps_setup, SPSSINGLENR, s);
						sscanf (s, "%d", &macro);
						set_sps_flags(sps_s, macro);
						module->set_dbox(module);
	               undo_state (window->object, window->exit_obj, SELECTED);
						window->edit_obj = SPSSINGLENR + offset;
						draw_object(window, SPS0 + offset);
						found = TRUE;
						break;									
					case SPSSINGLEMINUS:
						macro = get_sps_macro(sps_s);
						macro = (macro+64-1)%64; /* DEC */
						set_sps_flags(sps_s, macro);
						module->set_dbox(module);
						undo_state (window->object, window->exit_obj, SELECTED);
						draw_object(window, SPS0 + offset);
						found = TRUE;
						break;									
					case SPSSINGLEPLUS:
						macro = get_sps_macro(sps_s);
						macro = (macro+64+1)%64; /* INC */
						set_sps_flags(sps_s, macro);
						module->set_dbox(module);
						undo_state (window->object, window->exit_obj, SELECTED);
						draw_object(window, SPS0 + offset);
						found = TRUE;
						break;
					case SPSSPIEGELX0 :
					case SPSSPIEGELY0 :
					case SPSSPIEGELZ0 :
					case SPSTAUSCHXY0 :
					case SPSTAUSCHXZ0 :
					case SPSTAUSCHYZ0 :
						module->get_dbox(module);
						module->set_dbox(module);
						draw_object(window, SPS0 + offset);
						found = TRUE;
						break;									
				} /* switch */
			} /* for */
			if (!found)
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

  window = create_window_obj (KIND, CLASS_SPS);

  if (window != NULL)
  {
    menu_height = (menu != NULL) ? gl_hattr : 0;

    window->flags     = FLAGS | WI_MODELESS;
    window->icon      = icon;
    window->doc.x     = 0;
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

    sprintf (window->name, (BYTE *)sps_text [FSPSN].ob_spec);
    sprintf (window->info, (BYTE *)sps_text [FSPSI].ob_spec, 0);
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
	
	window = search_window (CLASS_SPS, SRCH_ANY, SPS_SETUP);
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (sps_setup, ROOT, EDITABLE);
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
	ED_P			actual = module->actual;

	window = search_window (CLASS_DIALOG, SRCH_ANY, ISPS);
		
	if (window == NULL)
	{
		 form_center (sps_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (sps_info, NULL, ISPS, (BYTE *)sps_text [FSPSN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = sps_info;
		sprintf(s, "%-20s", SPSDATE);
		set_ptext (sps_info, SPSIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (sps_info, SPSCOMPILE, s);
		sprintf(s, "%-20s", SPSVERSION);
		set_ptext (sps_info, SPSIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (sps_info, SPSISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (sps_info, SPSIAKT, s);

		if (! open_dialog (ISPS)) hndl_alert (ERR_NOOPEN);
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
		module->class_number		= CLASS_SPS;
		module->icon				= &sps_desk[SPSICON];
		module->icon_position	= ISPS;
		module->icon_number		= ISPS;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MSPS;
		module->menu_item			= MSPS;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_sps;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_CALC;
		module->apply				= apply;
		module->reset				= reset;
		module->precalc			= precalc;
		module->message			= message;
		module->create				= create;
	
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
		module->import					= import;
		module->get_dbox				= get_dbox;
		module->set_dbox				= set_dbox;
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
		window = crt_mod (sps_setup, NULL, SPS_SETUP);
		/* Modul-Struktur einbinden */
		window->module		= (VOID*) module;
		module->window		= window;
	
		add_rcv(VAR_SET_SPS,  module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_SPS, MAXSETUPS);
		add_rcv(VAR_PROP_SPS, module);	/* Message einklinken */
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
  sps_setup = (OBJECT *)rs_trindex [SPS_SETUP]; /* Adresse der SPS-Parameter-Box */
  sps_help  = (OBJECT *)rs_trindex [SPS_HELP];	/* Adresse der SPS-Hilfe */
  sps_desk  = (OBJECT *)rs_trindex [SPS_DESK];	/* Adresse des SPS-Desktops */
  sps_text  = (OBJECT *)rs_trindex [SPS_TEXT];	/* Adresse der SPS-Texte */
  sps_info 	= (OBJECT *)rs_trindex [SPS_INFO];	/* Adresse der SPS-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (sps_rsc_ptr, rsc_name))
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

  rs_gaddr (sps_rsc_ptr, R_TREE,  SPS_SETUP,	&sps_setup);   /* Adresse der SPS-Parameter-Box */
  rs_gaddr (sps_rsc_ptr, R_TREE,  SPS_HELP,	&sps_help);    /* Adresse der SPS-Hilfe */
  rs_gaddr (sps_rsc_ptr, R_TREE,  SPS_DESK,	&sps_desk);    /* Adresse des SPS-Desktop */
  rs_gaddr (sps_rsc_ptr, R_TREE,  SPS_TEXT,	&sps_text);    /* Adresse der SPS-Texte */
  rs_gaddr (sps_rsc_ptr, R_TREE,  SPS_INFO,	&sps_info);    /* Adresse der SPS-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (sps_setup, TRUE);
	fix_objs (sps_help, TRUE);
	fix_objs (sps_desk, TRUE);
	fix_objs (sps_text, TRUE);
	fix_objs (sps_info, TRUE);
	
	
	do_flags (sps_setup, SPSCANCEL, UNDO_FLAG);
	do_flags (sps_setup, SPSHELP, HELP_FLAG);
	
	menu_enable(menu, MSPS, TRUE);

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
  ok = rs_free (sps_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_sps ()
{
	BOOLEAN ok = TRUE;

	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

 	return (ok);
} /* init_sps */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;

	ok &= term_rsc ();
	return (ok);
} /* term_mod */
