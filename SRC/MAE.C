/*****************************************************************************/
/*                                                                           */
/* Modul: MAE.C                                                              */
/*                                                                           */
/* Extern-Maus-Treiber                                                       */
/* fÅr Mouse-Systems MÑuse                                                   */
/*                                                                           */
/*****************************************************************************/
#define MAEVERSION "V 1.05"
#define MAEDATE "05.02.95"

/* Updates *******************************************************************
V 1.05
- 0-UnterdrÅckung in set_dbox, 19.02.95
- Standard mit Sperre aussen an, 05.02.95
- ClickSetupField eingebaut, 30.01.95
- Get/Setxxx verwendet, 04.01.95
- Sperren() eingebaut, 03.01.95
V 1.04 30.11.94
- max/min Werte auf +/- 100 gesetzt
- default fÅr sperre_an_aussen ist nun FALSE
- RS232 Handling mit Port-Addressierung 1..4 aus POGLI
- load_create_infos und instance_count eingebaut
- restliche VAR's in send_messages eingebaut
- separate Mausports fÅr mit TEST und ohne
V 1.03
- Radius fÅr sperre aussen im standard auf 0 gesetzt
- Standard-Mausport auf 7 (Serial 2 am TT)
- Mausport-Umschaltung wieder eingebaut
- window->module eingebaut
- module->window in create
- Umbau auf create_window_obj
V 1.02
- Umstellung auf neue RTMCLASS-Struktur
V 1.01
- Fehler in prop-Behandlung beseitigt
- Port-Umschaltung verÑndert und ausgebaut
V 1.00	17.04.93
- Flags fÅr Sperren an/aus eingebaut
- Anzeigebreite fÅr Setup-Nr auf 5 gesetzt in RSC
- Ansteuerung fÅr TT-Schnittstellen eingebaut
- Prop und Sperre innen mÅssen noch eingebaut werden
*****************************************************************************/

#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "mae_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"
#include "math.h"			/* wg. sqrt */
#include "objects.h"
/* #include "pow.h" */

#include "export.h"
#include "mae.h"

#if XRSC_CREATE
#include "mae_mod.rsh"
#include "mae_mod.rh"
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
#define MILLI  1000                     /* Millisekunden fÅr Zeitablauf */

#define MAUSPORT 2

#define MOD_RSC_NAME "MAE_MOD.RSC"		/* Name der Resource-Datei */
#define MAXSETUPS 500l						/* Anzahl der MAE-Setups */

#define MAXMOUSE MAXKOOR*4					/* Weiteste Position der Maus */

#define CHECK_PORT(port) \
	if (port < 1) port = 4;\
	else if (port > 4) port = 1;

/* Alte Version, vor POGLI
/* Nur legale Zuweisungen erlauben */
#define CHECK_PORT(port) \
	if (port < 1) port = 9;\
	else if (port == 2) port = 6;\
	else if ((port < 6) || (port > 9)) port = 1;
*/
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
	UINT		sperre_an_innen : 1,
				sperre_an_aussen : 1;
	WORD		zoom;					/* Zoom-Faktor */
	WORD		speedy;				/* Beschleunigungs-Faktor */
	WORD		sperre_innen;		/* Radius fÅr innere Sperre */
	WORD		sperre_aussen;		/* Radius fÅr Ñussere Sperre */
} SETUP;	/* EnthÑlt alle Parameter einer kompletten MAE-Einstellung */

typedef struct status *STAT_P;

typedef struct status
{	
	WORD	mousex;					/* X-Position */
	WORD	mousey;					/* Z-Position */
	UINT	buttonl : 1,			/* Status linker Knopf */
			buttonm : 1,			/* Status mittlerer Knopf */
			buttonr : 1;			/* Status rechter Knopf */
	POINT_3D	koor;					/* X-Y-Z-Koordinaten */
	WORD	port;						/* Nummer der Seriellen Schnittstelle (1...4) */
	WORD	actualaux;				/* OS-Nummer der Seriellen Schnittstelle (1, 6...9) */
	WORD	prop;						/* aktueller Proportional-Wert */
} STATUS;

/****** VARIABLES ************************************************************/
PRIVATE WORD	mae_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*mae_rsc_ptr = &mae_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *mae_setup;
PRIVATE OBJECT *mae_help;
PRIVATE OBJECT *mae_desk;
PRIVATE OBJECT *mae_text;
PRIVATE OBJECT *mae_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "MAE";		/* Name, fÅr Extension etc. */

/****** FUNCTIONS ************************************************************/
/* Interne MAE-Funktionen */
PRIVATE FLOAT DistanceXZ (POINT_3D *p);
PRIVATE VOID Sperren (RTMCLASSP module);

PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	STAT_P	status = module->status;
	SET_P		ed = module->edited->setup;
	STRING 	s, format = "%4d";

	GetPWord (mae_setup, MAEROTX, &ed->rotx);
	GetPWord (mae_setup, MAEROTY, &ed->roty);
	GetPWord (mae_setup, MAEROTZ, &ed->rotz);

	ed->prop_rotx = GetCheck (mae_setup, MAEPROPROTX, NULL);
	ed->prop_roty = GetCheck (mae_setup, MAEPROPROTY, NULL);
	ed->prop_rotz = GetCheck (mae_setup, MAEPROPROTZ, NULL);
	
	GetPWord (mae_setup, MAEWINKXY, &ed->winkxy);
	GetPWord (mae_setup, MAEWINKXZ, &ed->winkxz);
	GetPWord (mae_setup, MAEWINKYZ, &ed->winkyz);
	
	ed->prop_winkxy = GetCheck (mae_setup, MAEPROPWINKXY, NULL);
	ed->prop_winkxz = GetCheck (mae_setup, MAEPROPWINKXZ, NULL);
	ed->prop_winkyz = GetCheck (mae_setup, MAEPROPWINKYZ, NULL);

	GetPWord (mae_setup, MAEZOOM, &ed->zoom);
	GetPWord (mae_setup, MAESPEEDOU, &ed->speedy);
	
	GetPWord (mae_setup, MAESPERREAUSSEN, &ed->sperre_aussen);
	GetPWord (mae_setup, MAESPERREINNEN, &ed->sperre_innen);

	ed->sperre_an_aussen = GetCheck (mae_setup, MAESPERREAUSAN, NULL);
	ed->sperre_an_innen = GetCheck (mae_setup, MAESPERREINAN, NULL);

	GetPWord (mae_setup, MAEPORTNR, &status->port);

	CHECK_PORT (status->port)
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	STAT_P	status = module->status;
	SET_P		ed = edited->setup;
	STRING 	s, format = "%4d";

	SetPWordN (mae_setup, MAEROTX, ed->rotx);
	SetPWordN (mae_setup, MAEROTY, ed->roty);
	SetPWordN (mae_setup, MAEROTZ, ed->rotz);

	SetCheck (mae_setup, MAEPROPROTX, ed->prop_rotx);
	SetCheck (mae_setup, MAEPROPROTY, ed->prop_roty);
	SetCheck (mae_setup, MAEPROPROTZ, ed->prop_rotz);
	
	SetPWordN (mae_setup, MAEWINKXY, ed->winkxy);
	SetPWordN (mae_setup, MAEWINKXZ, ed->winkxz);
	SetPWordN (mae_setup, MAEWINKYZ, ed->winkyz);
	
	SetCheck (mae_setup, MAEPROPWINKXY, ed->prop_winkxy);
	SetCheck (mae_setup, MAEPROPWINKXZ, ed->prop_winkxz);
	SetCheck (mae_setup, MAEPROPWINKYZ, ed->prop_winkyz);

	SetPWordN (mae_setup, MAEZOOM, ed->zoom);
	SetPWordN (mae_setup, MAESPEEDOU, ed->speedy);
	
	SetPWordN (mae_setup, MAESPERREAUSSEN, ed->sperre_aussen);
	SetPWordN (mae_setup, MAESPERREINNEN, ed->sperre_innen);
	SetCheck (mae_setup, MAESPERREAUSAN, ed->sperre_an_aussen);
	SetCheck (mae_setup, MAESPERREINAN, ed->sperre_an_innen);

	SetPWord (mae_setup, MAEPORTNR, status->port);

	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (mae_setup, MAESETNR , s);
} /* set_dbox */

PRIVATE VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;
	STAT_P	status = module->status;

	send_variable(VAR_SET_MAE, module->actual->number);
	send_variable(VAR_MAE_SPERRE_INNEN, akt->sperre_an_innen);
	send_variable(VAR_MAE_SPERRE_AUSSEN, akt->sperre_an_aussen);
} /* send_messages */

PRIVATE FLOAT DistanceXZ (POINT_3D *p)
{
	/* Distanz des XZ Teils des Vektors berechnen */
	return sqrt((p->x * p->x) + (p->z * p->z));
} /* DistanceXZ */

PRIVATE VOID Sperren (RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;
	STAT_P	status = module->status;
	POINT_3D *mkoor = &status->koor;
	FLOAT		dist, rel;
	WORD		sperre_aussen = akt->sperre_aussen;
	WORD		sperre_innen = akt->sperre_innen;
	
	if (akt->sperre_an_aussen)
	{
		if (sperre_aussen == 0)
		{
			mkoor->x = 0;
			mkoor->y = 0;
			mkoor->z = 0;
		} /* if 0 */
		else
		{
			/* Entfernung von X und Z messen */
			dist = DistanceXZ (mkoor);
			if (dist > sperre_aussen)
			{
				/* Korrekturfaktor berechnen */
				rel = dist/sperre_aussen;
				mkoor->x /= rel;
				mkoor->z /= rel;
			}
			/* Y wird getrennt berechnent */
			if (mkoor->y > sperre_aussen) mkoor->y = sperre_aussen;
			if (mkoor->y < -sperre_aussen) mkoor->y = -sperre_aussen;
		} /* else aussen == 0 */
	} /* if  sperre_aussen */
				
	if (akt->sperre_an_innen)
	{
		/* Entfernung von X und Z neu messen */
		dist = DistanceXZ (mkoor);
		if (dist < sperre_innen)
		{
			if (dist > 0)
			{
				/* Korrekturfaktor berechnen */
				rel = sperre_innen/dist;
				mkoor->x *= rel;
				mkoor->z *= rel;
			} /* if dist != 0*/
		} /* if */
	} /* if sperre_innen */
} /* Sperren */

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	STAT_P	status = module->status;
	POINT_3D *mkoor = &status->koor;
	SET_P		akt = module->actual->setup;
	WORD 		zoom = akt->zoom;
	WORD		signal, max = MAXPERCENT;
	KOOR_SINGLE *signals = event->koors->koor;
	register POINT_3D	*koor;
	
	/* Long-Multiplikation um öberlauf zu vermeiden */
	mkoor->x = (WORD)((LONG)zoom * (LONG)(status->mousex) / 100L);
	mkoor->z = (WORD)((LONG)zoom * (LONG)(status->mousey) / 100L);

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

	Sperren (module);
	
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

	send_variable(VAR_MAEX, status->koor.x);
	send_variable(VAR_MAEY, status->koor.y);
	send_variable(VAR_MAEZ, status->koor.z);

	return (event);
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	/* ZurÅcksetzen von Werten */
	STAT_P	status = module->status;
	WORD		aux = status->port;
	WORD		actualaux;
	BCONMAP *bcmap;
		
	/* Schnittstelle konfigurieren */
	/*
	 * Aus: POGLI Software aux_io.c
	 * Feststellen, ob die gewÅnschte Schnittstellen existiert und 
	 * Ablegen der Information in actualaux (modulglobal).
	 */
	if ((long)(bcmap = (BCONMAP*)Bconmap(-2)) == 44L)
		actualaux = 1;
	else
	{
		if (aux <= bcmap->maptabsize)
		{
			actualaux = aux + 5;
			Bconmap(actualaux);
		}
		else
			actualaux = 1;
	}
	status->actualaux = actualaux;
	
/* Alter Code
	if (Bconmap)							/* Nur ab TOS 030 */
	{
		if (status->port == 1)
		/* ST-Kompatible Schnittstelle */
			Bconmap(6);
		else
			Bconmap(status->port);	/* Nur ab TOS 030 */
	} /* if */
*/

	/* 1200 Baud, 0 Handshake */
	Rsconf(7,0,-1,-1,-1,-1);

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
	/* Vorausberechnung */
	STAT_P	status = module->status;
	BYTE		byte;
	WORD		port = status->actualaux;
		
	while (Bconstat(port))
	{
		byte = Bconin(port);
		if (((UBYTE)byte>=0x80) && ((UBYTE)byte<=0x87))
		{
			/* Tasten */

			status->buttonl	= !(byte & 0x4);
			status->buttonm	= !(byte & 0x2);
			status->buttonr	= !(byte & 0x1);
			
			/* Erste Messung */
			byte = Bconin(port); /* Umwandlung von */
			status->mousex	+= (WORD)byte;			/* Signed Char in Unsigned Word */
			byte = Bconin(port);
			status->mousey	-= (WORD)byte;
			
			/* Zweite Messung addieren */
			byte = Bconin(port);
			status->mousex	+= (WORD)byte;
			byte = Bconin(port);
			status->mousey	-= (WORD)byte;
			
			if (status->mousex > MAXMOUSE)
				status->mousex =  MAXMOUSE;
			else if(status->mousex<-MAXMOUSE)
				status->mousex = -MAXMOUSE;
				
			if (status->mousey > MAXMOUSE)
				status->mousey =  MAXMOUSE;
			else if (status->mousey < -MAXMOUSE)
				status->mousey = -MAXMOUSE;
		} /* if */
	} /* while */

} /* precalc */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD variable = ((MSG_SET_VAR *)msg)->variable;
	LONG	value		= ((MSG_SET_VAR *)msg)->value;
	SET_P	akt = module->actual->setup;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch (variable)
			{
				case VAR_SET_MAE:
					module->set_setnr(module, value);
					break;
				case VAR_PROP_MAE:
					module->status->prop = (WORD)value;
					break;
				case VAR_MAE_SPERRE_INNEN:
					akt->sperre_an_innen = (WORD)value;
					break;
				case VAR_MAE_SPERRE_AUSSEN:
					akt->sperre_an_aussen = (WORD)value;
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
	WORD			i, item, variable;
	STRING		s;
	UWORD			signal, offset;
	static		LONG x = 0;	
	WORD			event, ret;
	RTMCLASSP	module = Module(window);
	ED_P			edited = module->edited;
	SET_P			ed = edited->setup;
	STAT_P		status = module->status;
	
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case MAESETINC:
			module->set_nr (window, edited->number+1);
			break;
		case MAESETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case MAESETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case MAESETSTORE:
			module->set_store (window, edited->number);
			break;
		case MAESETRECALL:
			module->set_recall (window, edited->number);
			break;
		case MAEOK   :
			module->set_ok (window);
			break;
		case MAECANCEL:
			module->set_cancel (window);
		   break;
		case MAEHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case MAESTANDARD:
			module->set_standard (window);
		   break;
		case ROOT:
		case NIL:
			break;
		case MAEPORTINC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->port++;
			CHECK_PORT(status->port)
			module->set_dbox(module);
			draw_object(window, MAEPORTINC);
			draw_object(window, MAEPORTNR);
			break;
		case MAEPORTDEC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->port--;
			CHECK_PORT(status->port)
			module->set_dbox(module);
			draw_object(window, MAEPORTDEC);
			draw_object(window, MAEPORTNR);
			break;
		case MAEPORTNR:
			undo_state (window->object, window->exit_obj, SELECTED);
			module->get_dbox(module);
			draw_object(window, MAEPORTNR);
			break;
		default :	
			if(edited->modified == FALSE)
			{
				edited->modified = TRUE;
				sprintf(s, "%ld*", edited->number);
				set_ptext (mae_setup, MAESETNR, s);
				draw_object(window, MAESETNR);
			} /* if */
			module->get_dbox(module);
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
	
	inx    = num_windows (CLASS_MAE, SRCH_OPENED, NULL);
	window = create_window_obj (KIND, CLASS_MAE);
	
	
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
		
		sprintf (window->name, (BYTE *)mae_text [FMAEN].ob_spec);
		sprintf (window->info, (BYTE *)mae_text [FMAEI].ob_spec, 0);
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
	
	window = search_window (CLASS_MAE, SRCH_ANY, MAE_SETUP);
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (mae_setup, ROOT, EDITABLE);
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

	window = search_window (CLASS_DIALOG, SRCH_ANY, IMAE);
		
	if (window == NULL)
	{
		 form_center (mae_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (mae_info, NULL, IMAE, (BYTE *)mae_text [FMAEN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = mae_info;
		sprintf(s, MAEDATE);
		set_ptext (mae_info, MAEIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (mae_info, MAECOMPILE, s);
		sprintf(s, "%-20s", MAEVERSION);
		set_ptext (mae_info, MAEIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (mae_info, MAEISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (mae_info, MAEIAKT, s);

		if (! open_dialog (IMAE)) hndl_alert (ERR_NOOPEN);
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
		module->class_number		= CLASS_MAE;
		module->icon				= &mae_desk[MAEICON];
		module->icon_position	= IMAE;
		module->icon_number		= IMAE;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MMAE;
		module->menu_item			= MMAE;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_mae;
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
		/* PrÅfen, ob DEFAULT-Datei vorhanden */
		if((fp=fopen(module->file_name, "rb"))!=0)
		{
			/* Wenn vorhanden, laden */
			fclose(fp);
			module->load(module, module->file_name, FALSE);
		} /* if */

		/* Status-Strukturen initialisieren */
		mem_set(module->status, 0, (UWORD) sizeof(SETUP));
		module->status->port			=  MAUSPORT;
	
		/* Setup-Strukturen initialisieren */
		standard = module->standard;
		mem_set(module->standard, 0, (UWORD) sizeof(SETUP));
		standard->zoom					=  40;
		standard->speedy 				=  MAXPERCENT/10;
		standard->sperre_aussen 	=  0;
		standard->sperre_an_aussen	=  TRUE;
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (mae_setup, NULL, MAE_SETUP);
		/* Modul-Struktur einbinden */
		window->module = (VOID*) module;
		module->window = window;
		
		add_rcv(VAR_SET_MAE, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_MAE, MAXSETUPS);
		add_rcv(VAR_PROP_MAE, module);	/* Message einklinken */
		add_rcv(VAR_MAE_SPERRE_INNEN, module);	/* Message einklinken */
		add_rcv(VAR_MAE_SPERRE_AUSSEN, module);	/* Message einklinken */
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
  mae_setup = (OBJECT *)rs_trindex [MAE_SETUP]; /* Adresse der MAE-Parameter-Box */
  mae_help  = (OBJECT *)rs_trindex [MAE_HELP];	/* Adresse der MAE-Hilfe */
  mae_desk  = (OBJECT *)rs_trindex [MAE_DESK];	/* Adresse des MAE-Desktops */
  mae_text  = (OBJECT *)rs_trindex [MAE_TEXT];	/* Adresse der MAE-Texte */
  mae_info 	= (OBJECT *)rs_trindex [MAE_INFO];	/* Adresse der MAE-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (mae_rsc_ptr, rsc_name))
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

  rs_gaddr (mae_rsc_ptr, R_TREE,  MAE_SETUP,	&mae_setup);   /* Adresse der MAE-Parameter-Box */
  rs_gaddr (mae_rsc_ptr, R_TREE,  MAE_HELP,	&mae_help);    /* Adresse der MAE-Hilfe */
  rs_gaddr (mae_rsc_ptr, R_TREE,  MAE_DESK,	&mae_desk);    /* Adresse des MAE-Desktop */
  rs_gaddr (mae_rsc_ptr, R_TREE,  MAE_TEXT,	&mae_text);    /* Adresse der MAE-Texte */
  rs_gaddr (mae_rsc_ptr, R_TREE,  MAE_INFO,	&mae_info);    /* Adresse der MAE-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (mae_setup, TRUE);
	fix_objs (mae_help, TRUE);
	fix_objs (mae_desk, TRUE);
	fix_objs (mae_text, TRUE);
	fix_objs (mae_info, TRUE);
	
	
	do_flags (mae_setup, MAECANCEL, UNDO_FLAG);
	do_flags (mae_setup, MAEHELP, HELP_FLAG);
	
	menu_enable(menu, MMAE, TRUE);

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
  ok = rs_free (mae_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_mae ()
{
	BOOLEAN	ok = TRUE;
	
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);
	return (ok);
} /* init_mae */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = FALSE;
	ok &= term_rsc ();
	return (ok);
} /* term_mod */

