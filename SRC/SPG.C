/*****************************************************************************/
/*                                                                           */
/* Modul: SPG.C                                                           	  */
/*                                                                           */
/*****************************************************************************/
#define SPGVERSION "V 0.13"
#define SPGDATE "19.02.95"

/*****************************************************************************
V 0.13
- apply FunkionalitÑt aus RTM 3 eingebaut, 19.02.95
V 0.12
- ClickSetupField eingebaut, 30.01.95
V 0.11
- load_create_infos und instance_count eingebaut
- import modernisiert
- window->module eingebaut
- import modifiziert
- Umbau auf create_window_obj
V 0.10
- Bug in create (module->window) beseitigt
V 0.09
- Umstellung auf neue RTMCLASS-Struktur
*****************************************************************************/

#ifndef XRSC_CREATE
/*#define XRSC_CREATE 1*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "spg_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"

#include "math.h"

#include "export.h"
#include "spg.h"

#if XRSC_CREATE
#include "spg_mod.rsh"
#include "spg_mod.rh"
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
#define MILLI  0                     	/* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "SPG_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 1000l					/* Anzahl der SPG-Setups */

/****** TYPES ****************************************************************/
typedef struct setup *SET_P;

typedef struct spg_single
{
	WORD	drehung_x,		/* Drehung um die X-Achse */
			drehung_y,
			drehung_z,
			winkel_xy,		/* Winkel gegen XY-FlÑche */
			winkel_xz,		/* Winkel gegen XZ-FlÑche */
			winkel_yz;		/* Winkel gegen YZ-FlÑche */
	UINT	prop_dx : 1,	/* Proportional fÅr Drehung X */
			prop_dy : 1,	/* usw. */
			prop_dz : 1,
			prop_wxy : 1,
			prop_wxz : 1,
			prop_wyz : 1;
} SPG_SINGLE;				/* EnthÑlt alle SPG-Parameter eines einzelnen Signals */

typedef struct setup
{
	SPG_SINGLE	spg_single [MAXSIGNALS];	/* EnthÑlt die SPG-Informationen fÅr die einzelnen KanÑle */
} SETUP;		/* EnthÑlt alle Parameter einer kompletten SPG-Einstellung */

typedef struct status
{
	WORD	prop;		/* Proportional-Faktor */
} STATUS;
/****** VARIABLES ************************************************************/
/* Resource */
PRIVATE WORD	spg_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*spg_rsc_ptr = &spg_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *spg_setup;
PRIVATE OBJECT *spg_help;
PRIVATE OBJECT *spg_desk;
PRIVATE OBJECT *spg_text;
PRIVATE OBJECT *spg_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 20;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "SPG";		/* Name, fÅr Extension etc. */

/****** FUNCTIONS ************************************************************/
/* Interne SPG-Funktionen */
PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	ED_P			edited = module->edited;
	SET_P			ed = edited->setup;
	STRING 		s;
	WORD 			signal, offset;
	SPG_SINGLE *spg_s;
	
	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		offset=(SPG1-SPG0)*signal;
		spg_s=&ed->spg_single[signal];

		spg_s->prop_dx  = GetCheck (spg_setup, SPGPROPDX0 + offset, NULL);
		spg_s->prop_dy  = GetCheck (spg_setup, SPGPROPDY0 + offset, NULL);
		spg_s->prop_dz  = GetCheck (spg_setup, SPGPROPDZ0 + offset, NULL);
		
		spg_s->prop_wxy  = GetCheck (spg_setup, SPGPROPWXY0 + offset, NULL);
		spg_s->prop_wxz  = GetCheck (spg_setup, SPGPROPWXZ0 + offset, NULL);
		spg_s->prop_wyz  = GetCheck (spg_setup, SPGPROPWYZ0 + offset, NULL);

		GetPWord (spg_setup, SPGDREHX0 + offset, &spg_s->drehung_x);
		GetPWord (spg_setup, SPGDREHY0 + offset, &spg_s->drehung_y);
		GetPWord (spg_setup, SPGDREHZ0 + offset, &spg_s->drehung_z);

		GetPWord (spg_setup, SPGWINKXY0 + offset, &spg_s->winkel_xy);
		GetPWord (spg_setup, SPGWINKXZ0 + offset, &spg_s->winkel_xz);
		GetPWord (spg_setup, SPGWINKYZ0 + offset, &spg_s->winkel_yz);
	} /* for */
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STRING	s;
	WORD		signal, offset;
	SPG_SINGLE *spg_s;

	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		offset=(SPG1-SPG0)*signal;
		spg_s=&ed->spg_single[signal];

		SetCheck (spg_setup, SPGPROPDX0 + offset, spg_s->prop_dx);
		SetCheck (spg_setup, SPGPROPDY0 + offset, spg_s->prop_dy);
		SetCheck (spg_setup, SPGPROPDZ0 + offset, spg_s->prop_dz);
		
		SetCheck (spg_setup, SPGPROPWXY0 + offset, spg_s->prop_wxy);
		SetCheck (spg_setup, SPGPROPWXZ0 + offset, spg_s->prop_wxz);
		SetCheck (spg_setup, SPGPROPWYZ0 + offset, spg_s->prop_wyz);
	
		SetPWordN (spg_setup, SPGDREHX0 + offset, spg_s->drehung_x);
		SetPWordN (spg_setup, SPGDREHY0 + offset, spg_s->drehung_y);
		SetPWordN (spg_setup, SPGDREHZ0 + offset, spg_s->drehung_z);

		SetPWordN (spg_setup, SPGWINKXY0 + offset, spg_s->winkel_xy);
		SetPWordN (spg_setup, SPGWINKXZ0 + offset, spg_s->winkel_xz);
		SetPWordN (spg_setup, SPGWINKYZ0 + offset, spg_s->winkel_yz);
	}
	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (spg_setup, SPGSETNR , s);
} /* set_dbox */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	send_variable(VAR_SET_SPG, module->actual->number);
} /* send_messages */
/*****************************************************************************/

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	SPG_SINGLE	*spg_s;
	SET_P			akt = module->actual->setup;
	WORD			x, y, z, dx, dy, dz, dxz, dxyz;
	WORD			wx, wy, wz, wxy, wxz, wyz;
	POINT_3D		*point;
	KOOR_ALL		*koor = event->koors;
	UWORD			signal;
	LONG			prop = (LONG)module->status->prop;

	/* Zeiger auf Info des ersten Signals */
	spg_s = akt->spg_single;
	for (signal = 0; signal < MAXSIGNALS; signal++)
	{
		/* Nur aktiv werden, wenn etwas eingestellt ist */
		if (spg_s->winkel_xz || spg_s->drehung_y)
		{
			point = &koor->koor[signal].koor;
			x = point->x;
			y = point->y;
			z = point->z;
			
			dx=sqrt(y*y+z*z);
			dy=sqrt(x*x+z*z);
			dz=sqrt(x*x+y*y);
	
			/* Winkel in der XY-FlÑche berechnen fÅr Drehung um Z-Achse */
			if (x < 0)
				wz=180+Deg(atan((FLOAT)y/(FLOAT)x));	
			else if (x > 0)
				wz=Deg(atan((FLOAT)y/(FLOAT)x));
			else
				wz=Sign(y)*90;
	
			/* Winkel in der XZ-FlÑche berechnen fÅr Drehung um Y-Achse */
			if (x < 0)
				wy=180+Deg(atan((FLOAT)z/(FLOAT)x));	
			else if (x > 0)
				wy=Deg(atan((FLOAT)z/(FLOAT)x));
			else
				wy=Sign(z)*90;
	
			/* Winkel in der YZ-FlÑche berechnen fÅr Drehung um X-Achse */
			if (y< 0)
				wx=180+Deg(atan((FLOAT)z/(FLOAT)y));	
			else if (y > 0)
				wx=Deg(atan((FLOAT)z/(FLOAT)y));
			else
				wx=Sign(z)*90;
	
			dxyz = sqrt(x*x+y*y+z*z);
	
			/*  Winkel Y-Achse gegen X-Z-FlÑche */
	   	if (!dy)
		   	wxz=Sign(y)*90;
			else
				wxz=Deg(atan((FLOAT)y/(FLOAT)dy));
	
			if (spg_s->prop_wxz)
				wxz += spg_s->winkel_xz * prop/100;
			else
				wxz += spg_s->winkel_xz;

			if (spg_s->prop_dy)
				wy  -= spg_s->drehung_y * prop/100;
			else
				wy  -= spg_s->drehung_y;
	
			/* Koordinate=berechneter Wert */
/*
			point->x = dxyz * cos(Rad(wy));
			point->z = dxyz * sin(Rad(wy));
*/
			point->x = dxyz * Cosq(wy)*Cosq(wxz);
			point->y = dxyz * Sinq(wxz);
			point->z = dxyz * Sinq(wy)*Cosq(wxz);
		} /* if */
		spg_s++;
	} /* for signal */

	return event;
} /* apply */

PUBLIC BOOLEAN	import	(RTMCLASSP module, STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	STRING	ext, title, filter;
	FILE		*in;
	STR128	s;
	WORD		signal;
	SET_P		akt = module->actual->setup; 
	SPG_SINGLE	*single;
	LONG		setnr = 1;
	LONG		max_setups = module->max_setups;
		
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
			/* Zeiger auf erstes Setup nochmal holen, wegen Supervisor-MÅll in file_split */
			akt = module->actual->setup;
			daktstatus(" SPG-Datei wird importiert ... ", module->import_name);
			module->flags |= FLAG_IMPORTING;
			while (ok != EOF && setnr < max_setups)
			{
				/* Zeiger auf Info des ersten Signals */
				single = akt->spg_single;
				for (signal = 0; signal < MAXSIGNALS; signal++)
				{
					ok = fscanf(in, "%d", &single->drehung_x);
					ok = fscanf(in, "%d", &single->winkel_xz);
					ok = fscanf(in, "%d", &single->drehung_z);
					single++;	/* Auf Info fÅr nÑchste Signal zeigen */
				} /* for */
				/* ok = fscanf(in, "%s", s);	/* Leerzeile */ */
				
				/* Setup speichern und nÑchstes Setup anwÑhlen */
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
			fclose(in);
			close_daktstat();
		} /* else */
	} /* if */

	return (ok);
} /* import */

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
				case VAR_SET_SPG:
					module->set_setnr(module, value);
					break;
				case VAR_PROP_SPG:
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
		case SPGSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case SPGSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case SPGSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case SPGSETSTORE:
			module->set_store (window, edited->number);
			break;
		case SPGSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case SPGOK   :
			module->set_ok (window);
			break;
		case SPGCANCEL:
			module->set_cancel (window);
		   break;
		case SPGHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case SPGSTANDARD:
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
				set_ptext (spg_setup, SPGSETNR, s);
				draw_object(window, SPGSETNR);
			} /* if */
			module->get_dbox(module);
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

  window = create_window_obj (KIND, CLASS_SPG);

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
    window->special   = 0;
    window->edit_obj  = 0;
    window->edit_inx  = 0;
    window->exit_obj  = 0;
    window->object    = obj;
    window->menu      = menu;
    window->click     = wi_click_mod;
    window->showinfo  = info_mod;

    sprintf (window->name, (BYTE *)spg_text [FSPGN].ob_spec);
    sprintf (window->info, (BYTE *)spg_text [FSPGI].ob_spec, 0);
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
	
	window = search_window (CLASS_SPG, SRCH_ANY, SPG_SETUP);
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (spg_setup, ROOT, EDITABLE);
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

	window = search_window (CLASS_DIALOG, SRCH_ANY, ISPG);
		
	if (window == NULL)
	{
		 form_center (spg_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (spg_info, NULL, ISPG, (BYTE *)spg_text [FSPGN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = spg_info;
		sprintf(s, "%-20s", SPGDATE);
		set_ptext (spg_info, SPGIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (spg_info, SPGCOMPILE, s);
		sprintf(s, "%-20s", SPGVERSION);
		set_ptext (spg_info, SPGIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (spg_info, SPGISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (spg_info, SPGIAKT, s);

		if (! open_dialog (ISPG)) hndl_alert (ERR_NOOPEN);
	}

  return (window != NULL);
} /* info_mod */

/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	WINDOWP		window;
	RTMCLASSP 	module;
	BOOLEAN		ok;
	FILE			*fp;

	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number	= CLASS_SPG;
		module->icon				= &spg_desk[SPGICON];
		module->icon_position = ISPG;
		module->icon_number	= ISPG;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MSPG;
		module->menu_item		= MSPG;	/* Soll bei Init vergeben werden */
		module->multiple		= FALSE;
		
		module->crt				= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_spg;
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
		/* PrÅfen, ob DEFAULT-Datei vorhanden */
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
		window = crt_mod (spg_setup, NULL, SPG_SETUP);
		/* Modul-Struktur einbinden */
		window->module 	= (VOID*) module;
		module->window		= window;
	
		add_rcv(VAR_SET_SPG,  module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_SPG, MAXSETUPS);
		add_rcv(VAR_PROP_SPG, module);	/* Message einklinken */
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
  spg_setup = (OBJECT *)rs_trindex [SPG_SETUP]; /* Adresse der SPG-Parameter-Box */
  spg_help  = (OBJECT *)rs_trindex [SPG_HELP];	/* Adresse der SPG-Hilfe */
  spg_desk  = (OBJECT *)rs_trindex [SPG_DESK];	/* Adresse des SPG-Desktops */
  spg_text  = (OBJECT *)rs_trindex [SPG_TEXT];	/* Adresse der SPG-Texte */
  spg_info 	= (OBJECT *)rs_trindex [SPG_INFO];	/* Adresse der SPG-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (spg_rsc_ptr, rsc_name))
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

  rs_gaddr (spg_rsc_ptr, R_TREE,  SPG_SETUP,	&spg_setup);   /* Adresse der SPG-Parameter-Box */
  rs_gaddr (spg_rsc_ptr, R_TREE,  SPG_HELP,	&spg_help);    /* Adresse der SPG-Hilfe */
  rs_gaddr (spg_rsc_ptr, R_TREE,  SPG_DESK,	&spg_desk);    /* Adresse der SPG-Desktop */
  rs_gaddr (spg_rsc_ptr, R_TREE,  SPG_TEXT,	&spg_text);    /* Adresse der SPG-Texte */
  rs_gaddr (spg_rsc_ptr, R_TREE,  SPG_INFO,	&spg_info);    /* Adresse der SPG-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (spg_setup, TRUE);
	fix_objs (spg_help, TRUE);
	fix_objs (spg_desk, TRUE);
	fix_objs (spg_text, TRUE);
	fix_objs (spg_info, TRUE);
	
	
	do_flags (spg_setup, SPGCANCEL, UNDO_FLAG);
	do_flags (spg_setup, SPGHELP, HELP_FLAG);
	
	menu_enable(menu, MSPG, TRUE);

	return (TRUE);
} /* init_rsc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PRIVATE BOOLEAN term_rsc ()

{
	BOOLEAN ok = TRUE;

#if ((XRSC_CREATE||RSC_CREATE) == 0)
	ok = rs_free (spg_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

	return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_spg ()
{
	BOOLEAN	ok = TRUE;

	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

	return (ok);
} /* init_spg */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	
	ok &= term_rsc ();
	return (ok);
} /* term_mod */

