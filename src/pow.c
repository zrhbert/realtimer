/*****************************************************************************/
/*                                                                           */
/* Modul: POW.C                                                              */
/*                                                                           */
/* Extern-Maus-Treiber                                                       */
/* fÅr Mouse-Systems MÑuse                                                   */
/*                                                                           */
/*****************************************************************************/
#define POWVERSION "V 1.02"
#define POWDATE "30.01.95"

/* Updates *******************************************************************
V 1.02
- ClickSetupField eingebaut, 30.01.95
- MAXPERCENT ausgebaut, 03.01.95
V 1.01 23.12.94
- Polling-Mode statt Auto-Modus
V 1.00 28.11.94
- Wertebereich auf MAXPERCENT umgestellt
- send_messages eingebaut
V 0.03 20.11.94
- default-Port jetzt 3 (Serial 1)
- RS232 Handling mit Port-Addressierung 1..4 aus POGLI
- Baudrate auf 19.200 umgestellt
V 0.02 19.05.94
- load_create_infos und instance_count eingebaut
- restliche VAR's in send_messages eingebaut
V 0.01
*****************************************************************************/

#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "pow_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"
#include "pogli\deglitch.h"
#include "pogli\pg_io.h"

#include "export.h"
#include "pow.h"

#if XRSC_CREATE
#include "pow_mod.rsh"
#include "pow_mod.rh"
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
#define POWPORT 3

#define MOD_RSC_NAME "POW_MOD.RSC"		/* Name der Resource-Datei */
#define MAXSETUPS 500l						/* Anzahl der POW-Setups */

#define MAXMOUSE MAXKOOR*4					/* Weiteste Position des Glove */

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

#define SGN(a) ((a)>0?1:-1)

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
} SETUP;	/* EnthÑlt alle Parameter einer kompletten POW-Einstellung */

typedef struct status *STAT_P;

typedef struct status
{	
	GLOVE_DATA	glove;
	POINT_3D	koor;					/* X-Y-Z-Koordinaten */
	BYTE	rot;
	BYTE	keys;
	BYTE	f;
	WORD	port;						/* Nummer des seriellen Anschlusses */
	WORD	actualaux;				/* TOS-Nummer des seriellen Ports */
	WORD	prop;						/* aktueller Proportional-Wert */
	BOOL	deglitch;				/* Bewegung beruhigen */
	BOOL	connected;				/* Glove angeschlossen und aktiv */
	BOOL	connect_aborted;		/* Glove nicht entdeckt */
} STATUS;

/****** VARIABLES ************************************************************/
PRIVATE WORD	pow_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*pow_rsc_ptr = &pow_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *pow_setup;
PRIVATE OBJECT *pow_help;
PRIVATE OBJECT *pow_desk;
PRIVATE OBJECT *pow_text;
PRIVATE OBJECT *pow_info;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "POW";		/* Name, fÅr Extension etc. */

/****** FUNCTIONS ************************************************************/
/* Interne POW-Funktionen */
PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	STAT_P	status = module->status;
	SET_P		ed = module->edited->setup;
	STRING 	s, format = "%4d";

	get_ptext (pow_setup, POWROTX, s);
	sscanf (s, format, &ed->rotx);
	get_ptext (pow_setup, POWROTY, s);
	sscanf (s, format, &ed->roty);
	get_ptext (pow_setup, POWROTZ, s);
	sscanf (s, format, &ed->rotz);
	
	ed->prop_rotx = get_checkbox (pow_setup, POWPROPROTX);
	ed->prop_roty = get_checkbox (pow_setup, POWPROPROTY);
	ed->prop_rotz = get_checkbox (pow_setup, POWPROPROTZ);
	get_ptext (pow_setup, POWWINKXY, s);
	sscanf (s, format, &ed->winkxy);
	get_ptext (pow_setup, POWWINKXZ, s);
	sscanf (s, format, &ed->winkxz);
	get_ptext (pow_setup, POWWINKYZ, s);
	sscanf (s, format, &ed->winkyz);

	ed->prop_winkxy = get_checkbox (pow_setup, POWPROPWINKXY);
	ed->prop_winkxz = get_checkbox (pow_setup, POWPROPWINKXZ);
	ed->prop_winkyz = get_checkbox (pow_setup, POWPROPWINKYZ);

	get_ptext (pow_setup, POWZOOM, s);
	sscanf (s, format, &ed->zoom);
	
	get_ptext (pow_setup, POWSPERREAUSSEN, s);
	sscanf (s, format, &ed->sperre_aussen);
	ed->sperre_an_aussen = get_checkbox (pow_setup, POWSPERREAUSAN);
	get_ptext (pow_setup, POWSPERREINNEN, s);
	sscanf (s, format, &ed->sperre_innen);
	ed->sperre_an_innen = get_checkbox (pow_setup, POWSPERREINAN);
	get_ptext (pow_setup, POWPORTNR, s);
	sscanf (s, format, &status->port);

	CHECK_PORT (status->port)
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	STAT_P	status = module->status;
	SET_P		ed = edited->setup;
	STRING 	s, format = "%4d";

	sprintf (s, format, ed->rotx);
	set_ptext (pow_setup, POWROTX, s);
	sprintf (s, format, ed->roty);
	set_ptext (pow_setup, POWROTY, s);
	sprintf (s, format, ed->rotz);
	set_ptext (pow_setup, POWROTZ, s);

	set_checkbox (pow_setup, POWPROPROTX, ed->prop_rotx);
	set_checkbox (pow_setup, POWPROPROTY, ed->prop_roty);
	set_checkbox (pow_setup, POWPROPROTZ, ed->prop_rotz);
	
	sprintf (s, format, ed->winkxy);
	set_ptext (pow_setup, POWWINKXY, s);
	sprintf (s, format, ed->winkxz);
	set_ptext (pow_setup, POWWINKXZ, s);
	sprintf (s, format, ed->winkyz);
	set_ptext (pow_setup, POWWINKYZ, s);
	
	set_checkbox (pow_setup, POWPROPWINKXY, ed->prop_winkxy);
	set_checkbox (pow_setup, POWPROPWINKXZ, ed->prop_winkxz);
	set_checkbox (pow_setup, POWPROPWINKYZ, ed->prop_winkyz);

	sprintf (s, format, ed->zoom);
	set_ptext (pow_setup, POWZOOM, s);
	
	sprintf (s, format, ed->sperre_aussen);
	set_ptext (pow_setup, POWSPERREAUSSEN, s);
	set_checkbox (pow_setup, POWSPERREAUSAN, ed->sperre_an_aussen);
	sprintf (s, format, ed->sperre_innen);
	set_ptext (pow_setup, POWSPERREINNEN, s);
	set_checkbox (pow_setup, POWSPERREINAN, ed->sperre_an_innen);

	sprintf (s, "%2d", status->port);
	set_ptext (pow_setup, POWPORTNR, s);

	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (pow_setup, POWSETNR , s);
} /* set_dbox */

PRIVATE VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;
	STAT_P	status = module->status;

	send_variable(VAR_SET_POW, module->actual->number);
	send_variable(VAR_POW_SPERRE_INNEN, akt->sperre_an_innen);
	send_variable(VAR_POW_SPERRE_AUSSEN, akt->sperre_an_aussen);
} /* send_messages */

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	STAT_P	status = module->status;
	POINT_3D *mkoor = &status->koor;
	SET_P		akt = module->actual->setup;
	WORD		signal, max;
	KOOR_SINGLE *signals = event->koors->koor;
	register POINT_3D	*koor;
	
	if (akt->sperre_an_aussen)
	 	max = min(MAXPERCENT-1, akt->sperre_aussen);
	else
	 	max = MAXPERCENT-1;

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
		koor = &signals[signal].koor;
		
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
	
	return (event);
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	/* ZurÅcksetzen von Werten */
	STAT_P	status = module->status;
	GLOVE_DATA *glove = &status->glove;

	memset(glove, 0, sizeof(GLOVE_DATA) );

	if (status->connected || status->connect_aborted)
	{
		/* Auto-Mode abschalten */
		pg_datamode (0);
		glove->gstat1  = 6;
	
		status->connected = FALSE;
		status->connect_aborted = FALSE;
	} /* if connected */
	
	status->koor.x 	= 0;	
	status->koor.y 	= 0;	
	status->koor.z 	= 0;	
} /* reset */

PRIVATE VOID get_glove_keys(RTMCLASSP module)
{
	STAT_P	status = module->status;
	GLOVE_DATA *glove = &status->glove;

	/* PG-Key-Abfrage aus POGLI bgihand.c */

	switch(glove->keys) 
	{
		case PGK_START: 
			break;
		case PGK_1:          /* PowerGlove Daten aufbereiten  */
			status->deglitch = 1;
			break;
		case PGK_2:
			status->deglitch = 0;
			break;
		case PGK_3:          
			break;
		case PGK_4:
			break;
		case PGK_9:           /* Hand um Y-Achse rotieren       */
			glove->gstat1 -= 1;
			if (glove->gstat1<0)
				glove->gstat1 += 12;
			break;
		/* 
		case PGK_LEFT:       /* Sichtpunkt verschieben */
			Draw_Center.y += 5;
			if (Draw_Center.y > 50)
				Draw_Center.y = 50;
			Draw_Redraw = 1;
			break;
		case PGK_RIGHT:
			Draw_Center.y -= 5;
			if (Draw_Center.y < -50)
				Draw_Center.y = -50;
			Draw_Redraw = 1;
			break;
		case PGK_UP:
			Draw_Center.x += 5;
			if (Draw_Center.x > 50)
				Draw_Center.x = 50;
			Draw_Redraw = 1;
			break;
		case PGK_DOWN:
			Draw_Center.x -= 5;
			if (Draw_Center.x < -50)
				Draw_Center.x = -50;
			Draw_Redraw = 1;
			break;
		case PGK_5:
			Draw_Center.z -= 5;
			if (Draw_Center.z < -300)
				Draw_Center.z = -300;
			Draw_Redraw = 1;
			break;
		case PGK_6:
			Draw_Center.z += 5;
			if (Draw_Center.z > -175)
				Draw_Center.z = -175;
			Draw_Redraw = 1;
			break;
		case PGK_B:
			Vec_Set(&Draw_Center, 30, 30, -200);
			Glove_Init(glove);
			Cube_Init(cube);
			Draw_Redraw = 1;
			break;
		default:
			break;
		*/
	} /* switch key */
} /* get_glove_keys */

PRIVATE BOOL		get_glove_data	(RTMCLASSP module)
{
	STAT_P		status = module->status;
	GLOVE_DATA 	tmp, *new = &status->glove;
	WORD			port = status->actualaux;
	BOOL			success = FALSE, read_ok = FALSE;
	INT 	 		z;
	BYTE			byte;
	
	while (Bconstat(port))
	{
		byte = Bconin(port);
		if ((UBYTE)byte == 0xa0)
		{
			tmp.x 		= Bconin(port);
			tmp.y 		= Bconin(port);
			tmp.z 		= Bconin(port);
			tmp.rot 		= Bconin(port);
			tmp.fingers	= Bconin(port);
			tmp.keys		= Bconin(port);
		} /* if */

		read_ok = (tmp.rot != -1);

		if (read_ok)
		{
			*new = tmp;
			if (status->deglitch) 
			{
				deglitch(new);
				dehyst(new);
			} /* if deglitch */

			/* Tastatur-Abfrage */
			get_glove_keys (module);
			
			/* Die Z-Achse des PowerGlove wird neu skaliert, da der */
			/* Wertebereich -127..+127 mehreren Metern entspricht.  */
			z = 3*(int)new->z;
			if ( abs(z) > 127)
				z = SGN(z) * 127;
			new->z = z;
		} /* if read_ok */
		
		/* Success ist TRUE wenn Åberhaupt eine Abfrage
			funktioniert hat. */
		success |= read_ok;
	} /* while */
	
	return success;
} /* get_glove_data */

PRIVATE BOOL	init_glove	(RTMCLASSP module)
{
	STAT_P	status = module->status;
	GLOVE_DATA tmp, *new = &status->glove;
	BOOL 		ok = FALSE;
	
	status->actualaux = pg_auxinit(status->port, 19200L, -1);
	
	/* Auf automatischen Datenmodus umschalten */
	pg_datamode (1);
	/* Daten abholen */
	ok = pg_getdata (&tmp.x, &tmp.y, &tmp.z, &tmp.rot, &tmp.fingers, &tmp.keys);
	if (!ok)
	{
		/* Vielleicht waren wir schon im CMD Mode */
		pg_sendcmd('d', '1');
		/* 2. Versuch */
		ok = pg_getdata (&tmp.x, &tmp.y, &tmp.z, &tmp.rot, &tmp.fingers, &tmp.keys);
	} /* if not ok */
	
	return ok;
} /* init_glove */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	/* PG-Abfrage aus POGLI bgihand.c */
	STAT_P	status = module->status;
	POINT_3D	*koor = &status->koor;
	WORD		port = status->port;
	GLOVE_DATA tmp, *new = &status->glove;
	BOOL		success = FALSE;

	/* öbertragung initialisieren wenn nîtig */
	if (!status->connected && ! status->connect_aborted)
	{
		status->connected = init_glove (module);
		if (!status->connected) status->connect_aborted = TRUE;
	} /* if not connected */
	
	if (status->connected)
	{
		success = get_glove_data (module);
		if (success) 
		{
			/* Koordinaten Åbernehmen */
			status->koor.x = new->x;
			status->koor.y = new->y;
			status->koor.z = new->z;
			
			/* Hoher Ton bei Erfolg */
			vs_mute(phys_handle, MUTE_DISABLE);
			v_sound( vdi_handle, 800, 1);

			send_variable(VAR_POWX, status->koor.x);
			send_variable(VAR_POWY, status->koor.y);
			send_variable(VAR_POWZ, status->koor.z);

		} /* if success */
	
	} /* if connected */

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
				case VAR_SET_POW:
					module->set_setnr(module, value);
					break;
				case VAR_PROP_POW:
					module->status->prop = (WORD)value;
					break;
				case VAR_POW_SPERRE_INNEN:
					akt->sperre_an_innen = (WORD)value;
					break;
				case VAR_POW_SPERRE_AUSSEN:
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
		case POWSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case POWSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case POWSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case POWSETSTORE:
			module->set_store (window, edited->number);
			break;
		case POWSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case POWOK   :
			module->set_ok (window);
			break;
		case POWCANCEL:
			module->set_cancel (window);
		   break;
		case POWHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case POWSTANDARD:
			module->set_standard (window);
		   break;
		case ROOT:
		case NIL:
			break;
		case POWPORTINC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->port++;
			CHECK_PORT(status->port)
			module->set_dbox(module);
			draw_object(window, POWPORTINC);
			draw_object(window, POWPORTNR);
			break;
		case POWPORTDEC:
			undo_state (window->object, window->exit_obj, SELECTED);
			status->port--;
			CHECK_PORT(status->port)
			module->set_dbox(module);
			draw_object(window, POWPORTDEC);
			draw_object(window, POWPORTNR);
			break;
		case POWPORTNR:
			undo_state (window->object, window->exit_obj, SELECTED);
			module->get_dbox(module);
			draw_object(window, POWPORTNR);
			break;
		default :	
			if(edited->modified == FALSE)
			{
				edited->modified = TRUE;
				sprintf(s, "%ld*", edited->number);
				set_ptext (pow_setup, POWSETNR, s);
				draw_object(window, POWSETNR);
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
	
	inx    = num_windows (CLASS_POW, SRCH_OPENED, NULL);
	window = create_window_obj (KIND, CLASS_POW);
	
	
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
		
		sprintf (window->name, (BYTE *)pow_text [FPOWN].ob_spec);
		sprintf (window->info, (BYTE *)pow_text [FPOWI].ob_spec, 0);
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
	
	window = search_window (CLASS_POW, SRCH_ANY, POW_SETUP);
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (pow_setup, ROOT, EDITABLE);
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

	window = search_window (CLASS_DIALOG, SRCH_ANY, IPOW);
		
	if (window == NULL)
	{
		 form_center (pow_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (pow_info, NULL, IPOW, (BYTE *)pow_text [FPOWN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = pow_info;
		sprintf(s, POWDATE);
		set_ptext (pow_info, POWIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (pow_info, POWCOMPILE, s);
		sprintf(s, "%-20s", POWVERSION);
		set_ptext (pow_info, POWIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (pow_info, POWISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (pow_info, POWIAKT, s);

		if (! open_dialog (IPOW)) hndl_alert (ERR_NOOPEN);
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
		module->class_number		= CLASS_POW;
		module->icon				= &pow_desk[POWICON];
		module->icon_position 	= IPOW;
		module->icon_number		= IPOW;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MPOW;
		module->menu_item			= MPOW;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_pow;
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
		module->status->port			=  POWPORT;
		module->status->deglitch	=  0;
	
		/* Setup-Strukturen initialisieren */
		standard = module->standard;
		mem_set(module->standard, 0, (UWORD) sizeof(SETUP));
		standard->zoom					=  40;
		standard->speedy 				=  MAXPERCENT/10;
		standard->sperre_aussen 	=  MAXPERCENT / 3;
		standard->sperre_an_aussen	=  TRUE;
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (pow_setup, NULL, POW_SETUP);
		/* Modul-Struktur einbinden */
		window->module = (VOID*) module;
		module->window = window;
		
		add_rcv(VAR_SET_POW, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_POW, MAXSETUPS);
		add_rcv(VAR_PROP_POW, module);	/* Message einklinken */
		add_rcv(VAR_POW_SPERRE_INNEN, module);	/* Message einklinken */
		add_rcv(VAR_POW_SPERRE_AUSSEN, module);	/* Message einklinken */
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
  pow_setup = (OBJECT *)rs_trindex [POW_SETUP]; /* Adresse der POW-Parameter-Box */
  pow_help  = (OBJECT *)rs_trindex [POW_HELP];	/* Adresse der POW-Hilfe */
  pow_desk  = (OBJECT *)rs_trindex [POW_DESK];	/* Adresse des POW-Desktops */
  pow_text  = (OBJECT *)rs_trindex [POW_TEXT];	/* Adresse der POW-Texte */
  pow_info 	= (OBJECT *)rs_trindex [POW_INFO];	/* Adresse der POW-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (pow_rsc_ptr, rsc_name))
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

  rs_gaddr (pow_rsc_ptr, R_TREE,  POW_SETUP,	&pow_setup);   /* Adresse der POW-Parameter-Box */
  rs_gaddr (pow_rsc_ptr, R_TREE,  POW_HELP,	&pow_help);    /* Adresse der POW-Hilfe */
  rs_gaddr (pow_rsc_ptr, R_TREE,  POW_DESK,	&pow_desk);    /* Adresse des POW-Desktop */
  rs_gaddr (pow_rsc_ptr, R_TREE,  POW_TEXT,	&pow_text);    /* Adresse der POW-Texte */
  rs_gaddr (pow_rsc_ptr, R_TREE,  POW_INFO,	&pow_info);    /* Adresse der POW-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (pow_setup, TRUE);
	fix_objs (pow_help, TRUE);
	fix_objs (pow_desk, TRUE);
	fix_objs (pow_text, TRUE);
	fix_objs (pow_info, TRUE);
	
	
	do_flags (pow_setup, POWCANCEL, UNDO_FLAG);
	do_flags (pow_setup, POWHELP, HELP_FLAG);
	
	menu_enable(menu, MPOW, TRUE);

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
  ok = rs_free (pow_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_pow ()
{
	BOOLEAN	ok = TRUE;
	
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);
	return (ok);
} /* init_pow */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = FALSE;
	
	pg_auxexit ();
	ok &= term_rsc ();
	return (ok);
} /* term_mod */

