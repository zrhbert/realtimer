/*****************************************************************************/
/*                                                                           */
/* Modul: MAA.C                                                              */
/*                                                                           */
/* Atari-Maus-Treiber                                                        */
/*                                                                           */
/*****************************************************************************/
#define MAAVERSION "V 1.01"
#define MAADATE "30.01.95"

/*****************************************************************************
- ClickSetupField eingebaut, 30.01.95
- Wertebereich auf MAXPERCENT umgestellt
V 1.00
- send_messages eingebaut
V 0.08 19.05.94
- load_create_infos und instance_count eingebaut
- window->module eingebaut
- module->window in create
- Umbau auf create_window_obj
V 0.07
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
#include "maa_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"

#include "export.h"
#include "maa.h"

#if XRSC_CREATE
#include "maa_mod.rsh"
#include "maa_mod.rh"
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

#define MOD_RSC_NAME "MAA_MOD.RSC"		/* Name der Resource-Datei */
#define MAXSETUPS 200L					/* Anzahl der MAA-Setups */

#define MAXPERCENT 100					/* Gr”žter Wert fr Prozent VARs */

/****** TYPES ****************************************************************/
typedef struct setup *SET_P;

typedef struct setup
{
	WORD		rotx,					/* Drehungs-Winkel */
				roty,
				rotz,
				winkxy,
				winkxz,
				winkyz;
	UINT		prop_rotx : 1,		/* Proportional an/aus */
				prop_roty : 1,
				prop_rotz : 1,
				prop_winkxy : 1,
				prop_winkxz : 1,
				prop_winkyz : 1;
	WORD		zoom;					/* Zoom-Faktor */
	WORD		speedy;				/* Beschleunigungs-Faktor */
	WORD		sperre_an_innen;		/* Radius fr innere Sperre */
	WORD		sperre_an_aussen;		/* Radius fr „ussere Sperre */
} SETUP;	/* Enth„lt alle Parameter einer kompletten MAA-Einstellung */

typedef struct status *STAT_P;

typedef struct status
{
	WORD	mousex;					/* X-Position */
	WORD	mousey;					/* Z-Position */
	UINT	buttonl : 1,			/* Status linker Knopf */
			buttonm : 1,			/* Status mittlerer Knopf */
			buttonr : 1,			/* Status rechter Knopf */
			flag_sp_aussen : 1,	/* Sperre aussen an/aus */
			flag_sp_innen	: 1;	/* Sperre innen an/aus */
	POINT_3D	koor;					/* X-Y-Z-Koordinaten */
	WORD	prop;						/* Proportional-Faktor */
} STATUS;

/****** VARIABLES ************************************************************/
PRIVATE WORD	maa_rsc_hdr;					/* Zeigerstruktur fr RSC-Datei */
PRIVATE WORD	*maa_rsc_ptr = &maa_rsc_hdr;		/* Zeigerstruktur fr RSC-Datei */
PRIVATE OBJECT *maa_setup;
PRIVATE OBJECT *maa_help;
PRIVATE OBJECT *maa_desk;
PRIVATE OBJECT *maa_text;
PRIVATE OBJECT *maa_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 20;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "MAA";		/* Name, fr Extension etc. */

/****** FUNCTIONS ************************************************************/

/* Interne MAA-Funktionen */
PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STRING 	s, format = "%4d";

	get_ptext (maa_setup, MAAROTX, s);
	sscanf (s, format, &ed->rotx);
	get_ptext (maa_setup, MAAROTY, s);
	sscanf (s, format, &ed->roty);
	get_ptext (maa_setup, MAAROTZ, s);
	sscanf (s, format, &ed->rotz);
	
	get_ptext (maa_setup, MAAWINKXY, s);
	sscanf (s, format, &ed->winkxy);
	get_ptext (maa_setup, MAAWINKXZ, s);
	sscanf (s, format, &ed->winkxz);
	get_ptext (maa_setup, MAAWINKYZ, s);
	sscanf (s, format, &ed->winkyz);

	get_ptext (maa_setup, MAAZOOM, s);
	sscanf (s, format, &ed->zoom);
	get_ptext (maa_setup, MAASPEEDOU, s);
	sscanf (s, format, &ed->speedy);
	
	get_ptext (maa_setup, MAASPERREAUSSEN, s);
	sscanf (s, format, &ed->sperre_an_aussen);
	get_ptext (maa_setup, MAASPERREINNEN, s);
	sscanf (s, format, &ed->sperre_an_innen);
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STRING 	s, format = "%4d";

	sprintf (s, format, ed->rotx);
	set_ptext (maa_setup, MAAROTX, s);
	sprintf (s, format, ed->roty);
	set_ptext (maa_setup, MAAROTY, s);
	sprintf (s, format, ed->rotz);
	set_ptext (maa_setup, MAAROTZ, s);
	
	sprintf (s, format, ed->winkxy);
	set_ptext (maa_setup, MAAWINKXY, s);
	sprintf (s, format, ed->winkxz);
	set_ptext (maa_setup, MAAWINKXZ, s);
	sprintf (s, format, ed->winkyz);
	set_ptext (maa_setup, MAAWINKYZ, s);

	sprintf (s, format, ed->zoom);
	set_ptext (maa_setup, MAAZOOM, s);
	sprintf (s, format, ed->speedy);
	set_ptext (maa_setup, MAASPEEDOU, s);
	
	sprintf (s, format, ed->sperre_an_aussen);
	set_ptext (maa_setup, MAASPERREAUSSEN, s);
	sprintf (s, format, ed->sperre_an_innen);
	set_ptext (maa_setup, MAASPERREINNEN, s);

	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (maa_setup, MAASETNR , s);
} /* set_dbox */

PUBLIC PUF_INF *apply (RTMCLASSP module, PUF_INF *event)
{
	STAT_P	status = module->status;
	POINT_3D *mkoor = &(module->status->koor);
	ED_P		actual = module->actual;
	SET_P		akt = actual->setup;
	WORD 		zoom = akt->zoom;
	WORD		signal, max;
	KOOR_SINGLE *signals = event->koors->koor;
	register POINT_3D	*koor;
	
	/* Long-Multiplikation um šberlauf zu vermeiden */
	mkoor->x = (WORD)((LONG)zoom * (LONG)(status->mousex) / 100l);
	mkoor->z = (WORD)((LONG)zoom * (LONG)(status->mousey) / 100l);

	if (status->flag_sp_aussen)
	 	max = min(MAXPERCENT, akt->sperre_an_aussen);
	else
	 	max = MAXPERCENT;

	if(status->buttonr)
		if(status->buttonm)
			mkoor->y += akt->speedy * 2;
		else
			mkoor->y += akt->speedy;
	else if(status->buttonl)
		if(status->buttonm)
			mkoor->y -= akt->speedy * 2;
		else
			mkoor->y -= akt->speedy;
			
	/* Internes Clipping */
	if      (mkoor->x >  max)	mkoor->x =  max;
	else if (mkoor->x < -max)	mkoor->x = -max;

	if      (mkoor->y >  max)	mkoor->y =  max;
	else if (mkoor->y < -max)	mkoor->y = -max;

	if      (mkoor->z >  max)	mkoor->z =  max;
	else if (mkoor->z < -max)	mkoor->z = -max;

	/* Und nun alles in die Koordinaten _aller_ Signale kopieren */
	for (signal = 0; signal<MAXSIGNALS; signal++)
	{
		koor = &(signals[signal].koor);
		
		/* jeweils Koordinate verschieben und clippen */
		koor->x += mkoor->x;
		if     (koor->x >  MAXPERCENT) koor->x =  MAXPERCENT;
		else if(koor->x < -MAXPERCENT) koor->x = -MAXPERCENT;  

		koor->y += mkoor->y;
		if     (koor->y >  MAXPERCENT) koor->y =  MAXPERCENT;  
		else if(koor->y < -MAXPERCENT) koor->y = -MAXPERCENT;  

		koor->z += mkoor->z;
		if     (koor->z >  MAXPERCENT) koor->z =  MAXPERCENT;  
		else if(koor->z < -MAXPERCENT) koor->z = -MAXPERCENT;  
	} /* for */
	
	send_variable(VAR_MAAX, status->koor.x);
	send_variable(VAR_MAAY, status->koor.y);
	send_variable(VAR_MAAZ, status->koor.z);

	return event;
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	STAT_P		status = module->status;
	
	/* Zurcksetzen von Werten */
	status->mousex 	= 0;	
	status->mousey 	= 0;	
	status->buttonl 	= FALSE;	
	status->buttonm 	= FALSE;	
	status->buttonr 	= FALSE;	
	status->koor.x 	= 0;	
	status->koor.y 	= 0;	
	status->koor.z 	= 0;	
} /* reset */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	STAT_P	status = module->status;
	MKINFO	mkinfo, *mk = &mkinfo;
	WORD 		max_x = desk.w /2;
	WORD 		max_y = desk.h /2;

	graf_mkstate(&mk->mox, &mk->moy, &mk->momask, &mk->kstate);
	
	/* Vorausberechnung */
	status->buttonl	= mk->momask & 0x1;
	status->buttonr	= mk->momask & 0x2;
	
	/* Erste Messung */
	status->mousex	= mk->mox;			/* Signed Char in Unsigned Word */
	status->mousey	= mk->moy;
	
	if (status->mousex > max_x)
		status->mousex =  max_x;
	else if(status->mousex<-max_x)
		status->mousex = -max_x;
		
	if (status->mousey > max_y)
		status->mousey =  max_y;
	else if (status->mousey < -max_y)
		status->mousey = -max_y;
} /* precalc */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD 	variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch (variable)
			{
				case VAR_SET_MAA:
					module->set_setnr(module, value);
					break;
				case VAR_PROP_MAA:
					module->status->prop = (WORD)value;
					break;
				case VAR_MAA_SPERRE_INNEN:
					module->status->flag_sp_innen = (WORD)value;
					break;
				case VAR_MAA_SPERRE_AUSSEN:
					module->status->flag_sp_aussen = (WORD)value;
					break;
			} /* switch */
			break;
	} /* switch */
} /* message */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;
	STAT_P	status = module->status;

	send_variable(VAR_SET_MAA, module->actual->number);
	send_variable(VAR_MAA_SPERRE_INNEN, akt->sperre_an_innen);
	send_variable(VAR_MAA_SPERRE_AUSSEN, akt->sperre_an_aussen);
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
		case MAASETINC:
			module->set_nr (window, edited->number+1);
			break;
		case MAASETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case MAASETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case MAASETSTORE:
			module->set_store (window, edited->number);
			break;
		case MAASETRECALL:
			module->set_recall (window, edited->number);
			break;
		case MAAOK   :
			module->set_ok (window);
			break;
		case MAACANCEL:
			module->set_cancel (window);
		   break;
		case MAAHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case MAASTANDARD:
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
				set_ptext (maa_setup, MAASETNR, s);
				draw_object(window, MAASETNR);
			} /* if */
			module->get_dbox(module);
			undo_state (window->object, window->exit_obj, SELECTED);
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

  inx    = num_windows (CLASS_MAA, SRCH_OPENED, NULL);
  window = create_window_obj (KIND, CLASS_MAA);


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

    sprintf (window->name, (BYTE *)maa_text [FMAAN].ob_spec);
    sprintf (window->info, (BYTE *)maa_text [FMAAI].ob_spec, 0);
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
	
	window = search_window (CLASS_MAA, SRCH_ANY, MAA_SETUP);
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->opened = 1;
			window->edit_obj = find_flags (maa_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			module = Module(window);
			
			module->set_edit (module);
			module->set_dbox (module);
			if (! open_window (window)) hndl_alert (ERR_NOOPEN);
		} /* if */
		else top_window (window);
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

	window = search_window (CLASS_DIALOG, SRCH_ANY, IMAA);
		
	if (window == NULL)
	{
		 form_center (maa_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (maa_info, NULL, IMAA, (BYTE *)maa_text [FMAAN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = maa_info;
		sprintf(s, "%-20s", MAADATE);
		set_ptext (maa_info, MAAIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (maa_info, MAACOMPILE, s);
		sprintf(s, "%-20s", MAAVERSION);
		set_ptext (maa_info, MAAIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (maa_info, MAAISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (maa_info, MAAIAKT, s);

		if (! open_dialog (IMAA)) hndl_alert (ERR_NOOPEN);
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
	SET_P			standard;

	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_MAA;
		module->icon				= &maa_desk[MAAICON];
		module->icon_position 	= IMAA;
		module->icon_number		= IMAA;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MMAA;
		module->menu_item			= MMAA;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_maa;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_INPUT;
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
		standard = module->standard;
		mem_set(standard, 0, (UWORD) sizeof(SETUP));
		standard->zoom					=  40;
		standard->speedy 				=  desk.h /10;
		standard->sperre_an_aussen	=  desk.w /2;
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (maa_setup, NULL, MAA_SETUP);
		/* Modul-Struktur einbinden */
		window->module = (VOID*) module;
		module->window = window;
		
		add_rcv(VAR_SET_MAA, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_MAA, MAXSETUPS);
		add_rcv(VAR_PROP_MAA, module);	/* Message einklinken */
		add_rcv(VAR_MAA_SPERRE_INNEN, module);	/* Message einklinken */
		add_rcv(VAR_MAA_SPERRE_AUSSEN, module);	/* Message einklinken */
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
  maa_setup = (OBJECT *)rs_trindex [MAA_SETUP]; /* Adresse der MAA-Parameter-Box */
  maa_help  = (OBJECT *)rs_trindex [MAA_HELP];	/* Adresse der MAA-Hilfe */
  maa_desk  = (OBJECT *)rs_trindex [MAA_DESK];	/* Adresse des MAA-Desktops */
  maa_text  = (OBJECT *)rs_trindex [MAA_TEXT];	/* Adresse der MAA-Texte */
  maa_info 	= (OBJECT *)rs_trindex [MAA_INFO];	/* Adresse der MAA-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (maa_rsc_ptr, rsc_name))
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

  rs_gaddr (maa_rsc_ptr, R_TREE,  MAA_SETUP,	&maa_setup);   /* Adresse der MAA-Parameter-Box */
  rs_gaddr (maa_rsc_ptr, R_TREE,  MAA_HELP,	&maa_help);    /* Adresse der MAA-Hilfe */
  rs_gaddr (maa_rsc_ptr, R_TREE,  MAA_DESK,	&maa_desk);    /* Adresse des MAA-Desktop */
  rs_gaddr (maa_rsc_ptr, R_TREE,  MAA_TEXT,	&maa_text);    /* Adresse der MAA-Texte */
  rs_gaddr (maa_rsc_ptr, R_TREE,  MAA_INFO,	&maa_info);    /* Adresse der MAA-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (maa_setup, TRUE);
	fix_objs (maa_help, TRUE);
	fix_objs (maa_desk, TRUE);
	fix_objs (maa_text, TRUE);
	fix_objs (maa_info, TRUE);
	
	
	do_flags (maa_setup, MAACANCEL, UNDO_FLAG);
	do_flags (maa_setup, MAAHELP, HELP_FLAG);
	
	menu_enable(menu, MMAA, TRUE);

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
  ok = rs_free (maa_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_maa ()
{
	STR128	s;
	FILE		*test;
	BOOLEAN	ok = TRUE;
	
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);
	
	return (ok);
} /* init_maa */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = FALSE;
	ok &= term_rsc ();
	return (ok);
} /* term_mod */

