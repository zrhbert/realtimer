/*****************************************************************************/
/*                                                                           */
/* Modul: CMO.C                                                              */
/*                                                                           */
/* Cybermove-Output                                                          */
/*                                                                           */
/*****************************************************************************/

#define CMOVERSION "V 0.05"
#define CMODATE "30.01.95"

/*****************************************************************************
V 0.05
- ClickSetupField eingebaut, 30.01.95
- MAXPERCENT ausgebaut, 03.01.95
V 0.04, 28.12.94
- auf MODULE_OTHER umgestellt
- Menü-Abfrage für Kreisen eingebaut
- Vol-Anzeige eingebaut
V 0.03
- Bug in Anzahl der angezeigten Signale pro Objekt repariert
- Bug in Umrechnung Pitch->Koor beseitigt
V 0.02 28.08.94
- load_create_infos und instance_count eingebaut
- List-Funktionen ausgegliedert
01.08.93
- erstellt
*****************************************************************************/

#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim5.h"
#include "cmo_mod.h"
#include "realtspc.h"
#include "dispobj.h"
#include "var.h"
#include "puf.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"
#include <msh_unit.h>		/* Deklarationen für MidiShare Library */
#include "msh.h"

#include "export.h"
#include "cmo.h"

#if XRSC_CREATE
#include "cmo_mod.rsh"
#include "cmo_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_RESIDENT | WI_MOUSE | WI_NOSCROLL)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten für Scrolling */
#define YUNITS 1                        /* Y-Einheiten für Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (50 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  (20 * gl_hbox)           /* Anfangshöhe in Pixel */
#define MILLI  100                     /* Millisekunden für Zeitablauf */

#define MOD_RSC_NAME "CMO_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 100l					/* Anzahl der CMO-Setups */
#define CMO_CHANNELS 2

/* Umwandlung von MIDI-Format auf interne Koordinatendarstellung */
#define MIDI_TO_INTERN(koor) (koor - 64)

/****** TYPES ****************************************************************/

typedef struct status *STAT_P;
typedef struct status
{
	BOOLEAN	new;							/* Zwang zum Neuzeichnen */
	BOOLEAN	reset_flag;					/* Parameter wurden zurueckgesetzt */
	KOOR_SINGLE	koor[CMO_CHANNELS][MAXINPUTS];
	BOOL		kneu[CMO_CHANNELS][MAXINPUTS];
	WINDOWP		refwindow;
	RTMCLASSP	refmodule;				/* Das Bezugsmodul */
	TFilter	filter;						/* Midi-In-Filter	*/
	WORD		num_spaces;					/* Anzahl der Würfel */
} STATUS;

typedef struct setup *SET_P;
typedef struct setup
{
	WORD		grafikmodus;				/* In diesem F. benutzte Darstellung */
	WORD		raumform;					/* Raumform für dieses Fenster */
	WORD		rot_x,						/* Grafik-Parameter: Rotationen */
				rot_y,
				rot_z,
				distanz,						/* Distanz */
				persp;						/* Perspektive */
	WORD		master;						/* Master-Level */
	WORD		midi_channel;				/* 16: system 1, 15: system 2 (legacy setup encoding) */
	BOOLEAN	innenraum;					/* Nur Innenraum anzeigen */
	BOOLEAN	pfeile;						/* Pfeile anzeigen */
} SETUP;	/* Enthält alle Parameter einer kompletten CMO-Einstellung */

/****** VARIABLES ************************************************************/
PRIVATE WORD	cmo_rsc_hdr;					/* Zeigerstruktur für RSC-Datei */
PRIVATE WORD	*cmo_rsc_ptr = &cmo_rsc_hdr;		/* Zeigerstruktur für RSC-Datei */
PRIVATE OBJECT *cmo_menu;
PRIVATE OBJECT *cmo_setup;
PRIVATE OBJECT *cmo_shelp;
PRIVATE OBJECT *cmo_help;
PRIVATE OBJECT *cmo_desk;
PRIVATE OBJECT *cmo_text;
PRIVATE OBJECT *cmo_info;
PRIVATE OBJECT *cmo_raum;

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 2;			/* Max Anzahl Instanzen */
PRIVATE CHAR module_name[] = "CMO";			/* Name, für Extension etc. */
PRIVATE BOOLEAN sending_setup = FALSE;

PRIVATE RTMCLASSP	modulep[MAXMSAPPLS];		/* Zeiger auf Modul-Strukturen */
/****** FUNCTIONS ************************************************************/

PRIVATE VOID		dsetup			_((WINDOWP refwindow));
PRIVATE VOID		click_setup		_((WINDOWP window, MKINFO *mk));
PRIVATE RTMCLASSP define_setup	_((WINDOWP window, RTMCLASSP refmodule));
PUBLIC VOID    	get_edit_setup		_((RTMCLASSP module));
PUBLIC BOOLEAN		set_setnr_setup	_((RTMCLASSP module, LONG setupnr));

PRIVATE BOOLEAN	init_rsc			_((VOID));
PRIVATE BOOLEAN	term_rsc			_((VOID));

PRIVATE WORD ComputeNumDO (WINDOWP window);
PRIVATE VOID ComputeWorkDO (WINDOWP window, WORD obj_num, RECT *work);
PRIVATE VOID create_displayobs (WINDOWP window);
PRIVATE VOID init_standard (RTMCLASSP module, WORD instance);

/* MidiShare Funktionen */
MSH_RCVALARM_PROTO (receive_evts_cmo);
MSH_APPLALARM_PROTO (receive_alarm_cmo);
PRIVATE VOID		InstallFilter				_((SHORT refNum));
PRIVATE WORD		init_midishare 			_((RTMCLASSP module));

/*****************************************************************************/

PRIVATE VOID schedule_update (WINDOWP window)
{
	/* Coalesce MIDI updates without delaying an already pending redraw. */
	if (window && window->milli == 0) window->milli = MILLI;
}

MSH_RCVALARM_CALLBACK (receive_evts_cmo, refNum)
{
	MidiEvPtr	event;
	RTMCLASSP	module = modulep[refNum];
	SET_P			akt = module->actual->setup;
	STAT_P		status = module->status;
	KOOR_SINGLE	*koor;
	KOOR_SINGLE	previous;
	WINDOWP		window = module->window;
	WORD			signal, channel, port, type, bank;
	UBYTE 		pitch, value;
	
	/* Bis die Warteschlange leer ist, nicht nach vorher gezaehlter Anzahl: der Alarm kann
	   waehrend MidiSendIm() erneut aufgerufen werden, dann stimmt der Zaehler nicht mehr
	   und MidiGetEv() liefert NULL. */
	while ((event = MidiGetEv (refNum)) != NULL)
	{
		switch (EvType(event))
		{
			case typeKeyOn:
			case typeNote:
				channel = (WORD) Chan(event);
				port = (UBYTE) Port(event);
				pitch = (UBYTE) Pitch(event);
				value = (UBYTE) Vel(event);
				if (pitch < 128 && value < 128)
				{
					/* Signal = bits 2 - 7 */
					signal = pitch>>2;
					/* X,Y,Z,Vol = bits 0 und 1 */
					type = pitch & 3;
					for (bank = 0; bank < CMO_CHANNELS; bank++)
					{
						UWORD channel_var = bank ? VAR_CMI_CHANNEL1 : VAR_CMI_CHANNEL2;
						UWORD port_var = bank ? VAR_CMI_PORT1 : VAR_CMI_PORT2;
						if (channel != var_get_value(var_module, channel_var) ||
						    port != var_get_value(var_module, port_var))
							continue;

						koor = &status->koor[bank][signal];
						previous = *koor;
						switch (type)
						{
							case 0: /* X */
								koor->koor.x = MIDI_TO_INTERN(value);
								break;
							case 1: /* Y */
								koor->koor.y = MIDI_TO_INTERN(value);
								break;
							case 2: /* Z */
								koor->koor.z = MIDI_TO_INTERN(value);
								break;
							case 3: /* Vol */
								koor->volume = value;
								break;
						} /* switch koor */
						if (memcmp (&previous, koor, sizeof (previous)) != 0)
						{
							status->kneu[bank][signal] = TRUE;
							if (bank == (akt->midi_channel == 15 ? 0 : 1))
								schedule_update (window);
						}
					} /* for bank */
				} /* if valid CMO event */
				break;
			case typeStop:
				module->reset(module);
				break;
		} /* switch */
		MidiFreeEv (event);
	} /* for */
} /* receive_evts_cmo */

MSH_APPLALARM_CALLBACK (receive_alarm_cmo, refNum, code)
{
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	WINDOWP		window = module->window;
	TSyncInfo	info;

	switch ((WORD)code)
	{
		case MIDISyncStart:
			break;
		case MIDISyncStop:
			module->reset(module);
			break;
	} /* switch */
} /* receive_alarm_cmo */

/****************************************************************************
* 							InstallFilter						 *
*---------------------------------------------------------------------------*
* Cette procédure définit les valeurs du filtre de l'application. Un filtre *
* est composé de trois parties, qui sont trois tableaux de booléens :		 * 
* 															 *
*		un tableau de 256 bits pour les ports Midi acceptés			 *
*		un tableau de 256 bits pour les types d'événements acceptés		 *
*		un tableau de  16 bits pour les canaux Midi acceptés			 *
* 															 *
* Dans le code ci dessous, le filtre est paramétré pour accepter n'importe	 *
* quel type d'événement. 										 *
* 															 *
* Les paramètres de l'appel :										 *
* ---------------------------										 *
* 															 *
*		aucun												 *
* 															 *
*****************************************************************************/

PRIVATE VOID InstallFilter (WORD refNum)
{
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	TFilter		*filter = &status->filter;
	register		int i;

	for (i = 0; i<256; i++)
		AcceptBit(filter->port,i);		/* en provenance de tous les ports		*/
	AcceptBit(filter->evType, typeKeyOn);
	AcceptBit(filter->evType, typeNote);
	AcceptBit(filter->evType, typeStop);
	for (i = 0; i < 16; i++)
		AcceptBit(filter->channel,i);
		
	MidiSetFilter( refNum, filter );   /* installe le filtre				*/
} /* InstallFilter */

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	SET_P	ed = module->edited->setup;
	STRING 	s;
	WORD	signal, offset;

	get_ptext (cmo_setup, CMOGRAFIKX, s);
	sscanf (s, "%hd", &ed->rot_x);
	get_ptext (cmo_setup, CMOGRAFIKY, s);
	sscanf (s, "%hd", &ed->rot_y);
	get_ptext (cmo_setup, CMOGRAFIKZ, s);
	sscanf (s, "%hd", &ed->rot_z);
	get_ptext (cmo_setup, CMOGRAFIKPERSP, s);
	sscanf (s, "%hd", &ed->persp);

	get_ptext (cmo_setup, CMOMASTERLEVEL, s);
	sscanf (s, "%hd", &ed->master);

	ed->innenraum = get_checkbox (cmo_setup, CMOINNENRAUM);

	if (get_checkbox (cmo_setup, CMOMONO))		ed->grafikmodus = MONO;
	if (get_checkbox (cmo_setup, CMOSTEREO))	ed->grafikmodus = STEREO;
	if (get_checkbox (cmo_setup, CMOQUADRO))	ed->grafikmodus = QUADRO;
	if (get_checkbox (cmo_setup, CMOOKTO))		ed->grafikmodus = OKTO;
	
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited 	= module->edited;
	SET_P		ed = edited->setup;
	STRING 	s;
	WORD		signal, offset;

	sprintf (s, "%d", ed->rot_x);
	set_ptext (cmo_setup, CMOGRAFIKX, s);
	sprintf (s, "%d", ed->rot_y);
	set_ptext (cmo_setup, CMOGRAFIKY, s);
	sprintf (s, "%d", ed->rot_z);
	set_ptext (cmo_setup, CMOGRAFIKZ, s);
	sprintf (s, "%d", ed->persp);
	set_ptext (cmo_setup, CMOGRAFIKPERSP, s);

	sprintf (s, "%d", ed->master);
	set_ptext (cmo_setup, CMOMASTERLEVEL, s);

	set_checkbox (cmo_setup, CMOINNENRAUM, ed->innenraum);
	set_checkbox (cmo_setup, CMOAUSSENWELT, !(ed->innenraum));

	set_checkbox (cmo_setup, CMOMONO,	ed->grafikmodus == MONO);
	set_checkbox (cmo_setup, CMOSTEREO,	ed->grafikmodus == STEREO);
	set_checkbox (cmo_setup, CMOQUADRO,	ed->grafikmodus == QUADRO);
	set_checkbox (cmo_setup, CMOOKTO,	ed->grafikmodus == OKTO);

	copy_icon (&cmo_setup[CMORAUMFORM], &cmo_raum[ed->raumform+1]);  /* Holen der Iconstruktur aus MTR_EBENEN Popup */
	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (cmo_setup, CMOSETNR , s);
} /* set_dbox */

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	return event;
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	STAT_P	status = module->status;
	WINDOWP	window = module->window;
	WORD		bank, input;
	
	for (bank = 0; bank < CMO_CHANNELS; bank++)
	{
		for (input = 0; input < MAXINPUTS; input++)
		{
			mem_set (&status->koor[bank][input], 0, (UWORD)sizeof(KOOR_SINGLE));
			status->kneu[bank][input] = TRUE;
		} /* for input */
	} /* for bank */

	status->reset_flag = TRUE;
	status->new = TRUE;
	if (window) window->milli = 1;
} /* reset */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	/* Vorausberechnung */
} /* precalc */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD 	variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	STAT_P	status = module->status;
	WORD		signal;

	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch (variable)
			{
				case VAR_SET_CMO:
					if (!sending_setup || value != module->actual->number)
						module->set_setnr(module, value);
					break;
			} /* switch */
			break;
	} /* switch */
} /* message */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;
	BOOLEAN	was_sending_setup = sending_setup;

	sending_setup = TRUE;
	send_variable(VAR_SET_CMO, module->actual->number);
	sending_setup = was_sending_setup;
	if (akt->midi_channel == 16)
		send_variable(VAR_CM_MASTER1, module->actual->setup->master);
	else
		send_variable(VAR_CM_MASTER2, module->actual->setup->master);
} /* send_messages */

/*****************************************************************************/
/* Setup-Behandlung                                                          */
/*****************************************************************************/

PUBLIC VOID    send_messages_setup	(RTMCLASSP module)
{
	RTMCLASSP refmodule = module->status->refmodule;
	
	if (refmodule)
		if (refmodule->send_messages)
			(refmodule->send_messages) (refmodule); 
} /* send_messages */

LOCAL VOID click_setup (WINDOWP window, MKINFO *mk)
{
	RTMCLASSP	module = Module(window);
	ED_P			edited 	= module->edited;
	SET_P			ed  = edited->setup;
	STRING 		s;
	WORD 			signal, offset;
	WORD			i, item;
	static		LONG x = 0;	

	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case CMOSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case CMOSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case CMOSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case CMOSETSTORE:
			module->set_store (window, edited->number);
			break;
		case CMOSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case CMOOK   :
			module->set_ok (window);
			break;
		case CMOCANCEL:
			module->set_cancel (window);
			break;
		case CMOHELP :
			module->help (module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case CMOSTANDARD:
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
				set_ptext (cmo_setup, CMOSETNR, s);
				draw_object(window, CMOSETNR);
			} /* if */
			switch (window->exit_obj)
			{
				case CMORAUMFORM:	
					i = ed->raumform +1;
					item = popup_menu (cmo_raum, ROOT, 0, 0, i, TRUE, mk->momask);
					if ((item != NIL) && (item != i))
					{
						ed->raumform = item-1;
						copy_icon (&cmo_setup[CMORAUMFORM], &cmo_raum[ed->raumform+1]);  /* Holen der Iconstruktur aus MTR_EBENEN Popup */
					} /* if */
	            undo_state (window->object, window->exit_obj, SELECTED);
					draw_object (window, ROOT);
					break;
				default:
					module->get_dbox(module);
					break;
			} /* switch */
		break;
	} /* switch */
} /* click_setup */

/*****************************************************************************/

PRIVATE VOID dsetup (WINDOWP refwindow)
{
	RTMCLASSP	module;
	WINDOWP		window;
	WORD			ret;
	
	window = search_window_object (CLASS_DIALOG, SRCH_ANY, cmo_setup);
	
	if (window == NULL)
	{
		form_center (cmo_setup, &ret, &ret, &ret, &ret);
		
		window = crt_dialog (cmo_setup, NULL, CLASS_CMO, cmo_text [FCMOSN].ob_spec.free_string, WI_MODELESS);
	} /* if */
		
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			module = define_setup(window, (RTMCLASSP)refwindow->module); 

			window->edit_obj = find_flags (cmo_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			
			module->set_edit (module);
			module->set_dbox (module);
			undo_state (window->object, CMOHELP, DISABLED);

			if (! open_window (window)) hndl_alert (ERR_NOOPEN);
		} /* if */
		else
			top_window (window);
	
	} /* if */
} /* dsetup */

PRIVATE RTMCLASSP define_setup (WINDOWP window, RTMCLASSP refmodule)
{
	static RTMCLASS setup_module;
	RTMCLASSP module = NULL;

	if (window != NULL)
	{
		window->click    = click_setup;
		window->key      = wi_key_obj;
		window->showinfo = info_mod;
		window->showhelp = showhelp_obj;
		module = &setup_module;
		mem_move (module, refmodule, (UWORD)sizeof (RTMCLASS));
		module->object_type	= MODULE_OTHER;
		module->send_messages	= send_messages_setup;
		module->get_edit	= get_edit_setup;
		module->set_setnr	= set_setnr_setup;
		module->window		= window;
		window->module		= (VOID*) module;
		
		module->status->refmodule = refmodule;
		sprintf(module->object_name, "%s", refmodule->object_name);
		sprintf(window->name, " %s Setups ", module->object_name);
	} /* if */
	return module;
} /* define_setup */

PUBLIC VOID    get_edit_setup	(RTMCLASSP module)
{
	RTMCLASSP refmodule = module->status->refmodule;

	get_edit_obj (module);
	refmodule->reset (refmodule);
		
} /* get_edit_obj */

PUBLIC BOOLEAN	set_setnr_setup	(RTMCLASSP module, LONG setupnr)
{
	RTMCLASSP refmodule = module->status->refmodule;
	BOOLEAN ret;
	
	ret = set_setnr_obj (module, setupnr);
	refmodule->reset (refmodule);
	return ret;
} /* set_setnr_setup */

/*****************************************************************************/
/* Menübehandlung                                                            */
/*****************************************************************************/

PRIVATE VOID update_menu_mod (WINDOWP window)
{
	SET_P		akt = Akt(window);
	
	/* Modus */
	menu_check(cmo_menu, MCMOMONO, akt->grafikmodus == MONO);
	menu_check(cmo_menu, MCMOSTEREO, akt->grafikmodus == STEREO);
	menu_check(cmo_menu, MCMOQUADRO, akt->grafikmodus == QUADRO);
	menu_check(cmo_menu, MCMOOKTO, akt->grafikmodus == OKTO);

	/* Raumform */
	menu_check(cmo_menu, MCMOTETRAEDER, akt->raumform == TETRAEDER);
	menu_check(cmo_menu, MCMOSECHSKANAL, akt->raumform == SECHSKANAL);
	menu_check(cmo_menu, MCMOOKTAEDER, akt->raumform == OKTAEDER);
	menu_check(cmo_menu, MCMOWUERFEL, akt->raumform == WUERFEL);
	menu_check(cmo_menu, MCMOWUERFELLANG, akt->raumform == WUERFELLANG);
	menu_check(cmo_menu, MCMOWUERFELHOCH, akt->raumform == WUERFELHOCH);
	menu_check(cmo_menu, MCMOWUERFELMITTE, akt->raumform == WUERFELMITTE);
	menu_check(cmo_menu, MCMOWUERFELDOPP, akt->raumform == WUERFELDOPP);
	menu_check(cmo_menu, MCMOKREISFORM, akt->raumform == KREISFORM);
	menu_check(cmo_menu, MCMOQUADROPHON, akt->raumform == QUADROPHON);

	menu_check(cmo_menu, MCMOCHANNEL15, akt->midi_channel == 15);
	menu_check(cmo_menu, MCMOCHANNEL16, akt->midi_channel == 16);

	/* Optionen */
	menu_check(cmo_menu, MCMOINNENRAUM, akt->innenraum);
} /* update_menu_mod */

/*****************************************************************************/

PRIVATE VOID handle_menu_mod (WINDOWP window, WORD title, WORD item)

{
	RTMCLASSP	module = Module(window);
	SET_P		akt = Akt(window);
	STAT_P	status = Status(window);
	
	if (window != NULL)
		menu_normal (window, title, FALSE);         /* Titel invers darstellen */

	switch (title)
	{
		case MCMOINFO: switch (item)
		{
			case MCMOINFOANZEIG: info_mod(window, NIL);
										break;
		} /* switch */
		break;

		case MCMOMODES: switch (item)
		{
			case MCMOMONO:		akt->grafikmodus = MONO; 	break;
			case MCMOSTEREO: 	akt->grafikmodus = STEREO;	break;
			case MCMOQUADRO: 	akt->grafikmodus = QUADRO;	break;
			case MCMOOKTO: 	akt->grafikmodus = OKTO;	break;
		} /* switch */
		status->new = TRUE;
		window->milli = 1;
		break;

		case MCMOFORMS: switch (item)
		{
			case MCMOTETRAEDER:		akt->raumform = TETRAEDER;		break;
			case MCMOSECHSKANAL:		akt->raumform = SECHSKANAL;	break;
			case MCMOOKTAEDER:		akt->raumform = OKTAEDER;		break;
			case MCMOWUERFEL:			akt->raumform = WUERFEL;		break;
			case MCMOWUERFELLANG:	akt->raumform = WUERFELLANG;	break;
			case MCMOWUERFELHOCH:	akt->raumform = WUERFELHOCH;	break;
			case MCMOWUERFELMITTE:	akt->raumform = WUERFELMITTE;	break;
			case MCMOWUERFELDOPP:	akt->raumform = WUERFELDOPP;	break;
			case MCMOQUADROPHON:		akt->raumform = QUADROPHON;	break;
			case MCMOKREISFORM:		akt->raumform = KREISFORM;		break;
		} /* switch */
		status->new = TRUE;
		window->milli = 1;
		break;

		case MCMOOPTIONS: switch (item)
		{
			case MCMOSETUPS	:
				dsetup(window);
				break;
			case MCMOINNENRAUM		:
				akt->innenraum = !akt->innenraum;
				window->milli = 1;
				break;
			case MCMOCHANNEL15		:
				akt->midi_channel = 15;
				window->milli = 1;
				break;
			case MCMOCHANNEL16		:
				akt->midi_channel = 16;
				window->milli = 1;
				break;
			case MCMOPERSPEKTIVE 	:
				break;
		} /* switch */
		status->new = TRUE;
		break;

		case MCMOACTIONS: switch (item)
		{
			case MCMORESET	:
				module->reset (module);
				break;
		} /* switch */
	} /* switch */
		
	if (window != NULL)
		menu_normal (window, title, TRUE);          /* Titel wieder normal darstellen */
} /* handle_menu_mod */

/*****************************************************************************/
/* Taste für Fenster                                                         */
/*****************************************************************************/

PRIVATE BOOLEAN wi_key_mod (WINDOWP window, MKINFO *mk)

{
	RTMCLASSP	module = Module(window);
	BOOLEAN		ok = FALSE;
	
	switch (mk->scan_code)
	{
		case	ESC:	
			module->reset(module);
			ok = TRUE;
			break;
	} /* switch */

	ok |= (menu_key (window, mk));
	return (ok);
} /* wi_key_mod */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

PRIVATE VOID wi_draw_mod (WINDOWP window)

{	
	RTMCLASSP	module = Module(window);
	STAT_P		status = module->status;
	
	if (status->new)
	{
		clr_scroll (window);
	} /* if */

} /* wi_draw_mod */

/*****************************************************************************/
/* Vor zeichnen Status verändern                                             */
/*****************************************************************************/

PRIVATE VOID wi_start_mod (WINDOWP window)

{	
	RTMCLASSP	module = Module(window);
	STAT_P	status = Status(window);
	SET_P		akt = Akt(window);
	DISPOBJP	dispobj = NULL;
	LIST_P	header, element;
	BOOLEAN	new = status->new || (window->flags & WI_JUNK);
	BOOLEAN	reset_pending = status->reset_flag;
	BOOLEAN	newkoor = FALSE;
	WORD		bank = akt->midi_channel == 15 ? 0 : 1;
	KOOR_SINGLE	*koor = status->koor[bank], *koor_s;
	BOOL		*kneu = status->kneu[bank];
	POINT_3D	*point;
	POS_3D	position;
	WORD		index, x, base_signal = 0, num_signals, num_objs, rows, cols, obj_count = 0;
	WORD		signal, obj_nr = 0;
	SPACESTATP	spacestatus;
	RECT		work, a;
	
	status->new = new;
	status->reset_flag = FALSE;

	if (new)
	{
		STRING title;
		WORD first_input = bank == 0 ? 33 : 1;

		snprintf ((char *)title, sizeof (title), " %s  CM %d (%d-%d) ",
		          (char *)module->object_name, bank == 0 ? 2 : 1,
		          first_input, first_input + MAXINPUTS - 1);
		if (strcmp ((char *)window->name, (char *)title) != 0)
		{
			strcpy ((char *)window->name, (char *)title);
			if (window->opened > 0)
				wind_set_str (window->handle, WF_NAME, window->name);
		}
	}
	
	/* Anzahl der Objekte berechnen */
	num_objs = ComputeNumDO (window);
	if (num_objs != status->num_spaces)
		create_displayobs (window);

	/* Anzahl der Signale pro Objekt */
	num_signals = MAXINPUTS / num_objs;
	
	/* Liste der Display-Objekte durchgehen und neue Koor eintragen */
	header = window->dispobjs;
	element = list_next(header);
	while (element != header) {
		dispobj = (DISPOBJP) element->key;
		if (new) {
			dispobj->new	= TRUE;
			/* Nur für 3D-Anzeigen */
			if (obj_nr < num_objs)
			{
				spacestatus = (SPACESTATP) dispobj->status;
				dispobj->set_uni  (dispobj, DOUniSpaceInside, akt->innenraum);
				dispobj->set_uni  (dispobj, DOUniSpaceArrows, akt->pfeile);
	
				/* Lookup Update verhindern */
				spacestatus->initializing = TRUE;
				dispobj->set_uni  (dispobj, DOUniRotationX, akt->rot_x);
				dispobj->set_uni  (dispobj, DOUniRotationY, akt->rot_y);
				if (akt->rot_z !=0)
					dispobj->set_uni  (dispobj, DOUniRotationZ, akt->rot_z);
				/* Lookup Update erlauben */
				spacestatus->initializing = FALSE;
				dispobj->set_uni  (dispobj, DOUniSpacePerspective, akt->persp);
	
				dispobj->set_type (dispobj, DOTypeSpaceForm, akt->raumform);
				dispobj->set_type (dispobj, DOTypeSpaceMode, akt->grafikmodus);
			
				ComputeWorkDO (window, obj_count, &work);
				dispobj->set_work (dispobj, &work);
	
				if (akt->grafikmodus == MONO)	
				{
					/* Mono: Fadenkreuze und keine Linien */
					dispobj->set_uni (dispobj, DOUniSpaceCrosshair, 0x1);
					dispobj->set_uni (dispobj, DOUniSpaceDisplay, 0x0);
				}
				else
				{
					/* Alle anderen: Nur Linien, keine FDK */
					dispobj->set_uni (dispobj, DOUniSpaceCrosshair, 0x0);
					dispobj->set_uni (dispobj, DOUniSpaceDisplay, 0xFF);
				}
				dispobj->set_uni (dispobj, DOUniSpaceInputBase, 0);
				
			} /* if obj_nr < num_objs */
			else
			{
				/* Höhe für Texte berechnen */
				a = dispobj->work;
				a.y = 2 * gl_hbox;
				dispobj->set_work (dispobj, &a);
			} /* else */
			
		} /* if new */		
	
		/* Nur für 3D-Anzeigen */
		if (obj_nr < num_objs)
		{
			/* Für jedes Objekt die entsprechenden Koordinaten einsetzen */
			spacestatus = (SPACESTATP) dispobj->status;
			for (signal = 0; signal < num_signals; signal++)
			{
				if (new || kneu[signal+base_signal])
				{
					kneu[signal+base_signal] = FALSE;
					koor_s = &koor[signal+base_signal];
					spacestatus->position[signal].x = (LONG)koor_s->koor.x * MAXDOKOOR / MAXKOOR;
					spacestatus->position[signal].y = (LONG)koor_s->koor.y * MAXDOKOOR / MAXKOOR;
					spacestatus->position[signal].z = (LONG)koor_s->koor.z * MAXDOKOOR / MAXKOOR;
					spacestatus->volume[signal]     = koor_s->volume;
					spacestatus->kneu = TRUE;
				} /* if kneu */
			} /* for */
	
			/* Point to next set of koordinates */
			base_signal += num_signals;
			obj_count++;
			
		} /* if obj_nr < num_objs */


		if (reset_pending && dispobj->reset)
			dispobj->reset (dispobj);
		obj_nr++;
		element = list_next(element);
	} /* while */

} /* wi_start_mod */

/*****************************************************************************/
/* Nach zeichnen Status verändern                                            */
/*****************************************************************************/

PRIVATE VOID wi_finished_mod (WINDOWP window)

{	
	STAT_P	status = Status(window);

	status->new = FALSE;

} /* wi_finished_mod */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (WINDOWP window, MKINFO *mk)
{
	RTMCLASSP	module = Module(window);
	STAT_P		status = module->status;
	BOOLEAN		new = status->new || (window->flags & WI_JUNK);
	
	/* Sicherheitshalber */
	status->new = TRUE;

	window->milli = 1;
} /* wi_click_mod */

/*****************************************************************************/
/* Einrasten des Fensters                                                    */
/*****************************************************************************/

PRIVATE VOID wi_snap_mod (WINDOWP window, RECT *new, WORD mode)

{
	STAT_P		status = Status(window);
	wi_snap_obj (window, new, mode);
	status->new = TRUE;
	window->milli = 1; 						/* Updaten ! */
} /* wi_snap_mod */

/*****************************************************************************/
/* Zeitablauf für Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (WINDOWP window)
{
	STAT_P status = Status(window);

	window->milli = 0; 			/* keine Timer-Funktion mehr bis
										zur nächsten Änderung */
	if (status->new || status->reset_flag || (window->flags & WI_JUNK))
		redraw_window(window, &window->scroll);
	else
	{
		hide_mouse ();
		wi_start_mod (window);
		draw_dispobjs (window);
		show_mouse ();
	}
} /* wi_timer_mod */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (OBJECT *obj, OBJECT *menu, WORD icon)
{
	WINDOWP window;
	WORD    menu_height, inx;
	
	inx    = num_windows (CLASS_CMO, SRCH_ANY, NULL);
	window = create_window_obj (KIND, CLASS_CMO);
	
	if (window != NULL)
	{
		menu_height = (menu != NULL) ? gl_hattr : 0;
		
		WINDOW_INITWIN_OBJ

		window->flags     = FLAGS;
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
		window->mousenum  = THICK_CROSS;
		window->mouseform = NULL;
		window->milli     = MILLI;
		window->module		= 0;
		window->object    = 0;
		window->menu      = menu;
		window->hndl_menu = handle_menu_mod;
		window->updt_menu = update_menu_mod;
		window->key			= wi_key_mod;
		window->draw		= wi_draw_mod;
		window->snap      = wi_snap_mod;
		window->timer     = wi_timer_mod;
		window->showinfo  = info_mod;
		window->click		= wi_click_mod;
		window->start		= wi_start_mod;
		window->finished  = wi_finished_mod;
			
		sprintf (window->name, cmo_text [FCMON].ob_spec.free_string, 0);
		sprintf (window->info, cmo_text [FCMOI].ob_spec.free_string, 0);
	} /* if */
	
	return (window);                      /* Fenster zurückgeben */
} /* crt_mod */

PRIVATE WORD ComputeNumDO (WINDOWP window)
{
	SET_P		akt = Akt(window);
	WORD		num;

	switch (akt->grafikmodus)
	{
		case MONO:	
			num = 1;
			break;
		case STEREO:	
			num = 2;
			break;
		case QUADRO:	
			num = 4;
			break;
		case OKTO:	
			num = 8;
			break;
		default:
			akt->grafikmodus = MONO;
			num = 1;
			break;
	}
	return MAXINPUTS/num;
} /* ComputeNumDO */

PRIVATE VOID ComputeWorkDO (WINDOWP window, WORD obj_num, RECT *work)
{
	/* setzen des Work-Bereiches eines Display-Objektes */
	SET_P		akt = Akt(window);
	RECT		*scroll = &window->scroll;
	WORD		rows, cols, row, col;
	
	switch (akt->grafikmodus)
	{
		case MONO:	
			rows			= 4;
			cols			= 8;
			break;
		case STEREO:	
			rows			= 4;
			cols			= 4;
			break;
		case QUADRO:	
			rows			= 4;
			cols			= 2;
			break;
		case OKTO:	
			rows			= 2;
			cols			= 2;
			break;
		default :
			akt->grafikmodus = MONO;
			rows			= 4;
			cols			= 8;
			break;
	} /* switch */

	col = obj_num%cols;
	row = (obj_num - col)/cols;
	
	/* Distribute rows and columns evenly */
	work->x = scroll->x + scroll->w*col/cols;
	work->w = scroll->w/cols;
	work->y = scroll->y + scroll->h*row/rows;
	work->h = scroll->h/rows;
} /* ComputeWorkDO */

PRIVATE VOID create_displayobs (WINDOWP window)
{	
	RTMCLASSP	module = Module(window);
	SET_P		akt = Akt(window);
	STAT_P	status = Status(window);
	WORD		obj_number = 0, num_signals, num_displayobs;
	RECT		work;
	WORD		signal, h = gl_hbox, w = gl_wbox, x0, y0;
	LONGSTR	s;
	RECT		a;
	LIST_P	header, element;
	DISPOBJP	dispobj;
	SPACESTATP	spacestatus;
	WORD		*xwin,		/* X auf Fenstergröße */
				*ywin,		/* y auf Fenstergröße */
				*xpersp,		/* X Verzerrung durch Z */
				*ypersp,		/* Y Verzerrung durch Z */
				*xzoff,		/* X-Offset durch Z */ 
				*yzoff;		/* Y-Offset durch Z */ 

	if (window->work.h > 400)
		h = 16;
	else
		h = 8;
	
	/* Liste der Display-Objekte durchgehen und löschen */
	header = window->dispobjs;
	element = list_next(header);
	while (element != header) {
		dispobj = (DISPOBJP) element->key;
		dispobj->delete (dispobj);
		element = list_next(element);
	} /* while */

	/* Alle objekte löschen */
	list_empty (window->dispobjs);
	
	num_displayobs = ComputeNumDO (window);
	/* Signale pro display */
	num_signals = MAXINPUTS / num_displayobs;

	for (obj_number = 0; obj_number < num_displayobs; obj_number++)
	{
		ComputeWorkDO (window, obj_number, &work);
		if (obj_number == 0)
		{
			dispobj = CreateCMOSpaceDispobj(window, WUERFEL, 0, &work, obj_number*num_signals);
			spacestatus = (SPACESTATP) dispobj->status;
			/* Pointer merken */
			xwin 		= spacestatus->xwin;
			ywin 		= spacestatus->ywin;
			xpersp 	= spacestatus->xpersp;
			ypersp 	= spacestatus->ypersp;
			xzoff 	= spacestatus->xzoff;
			yzoff 	= spacestatus->yzoff;
		}
		else
		{
			dispobj = CreateCMOSpaceDispobj(window, WUERFEL, SpaceModeSharedPersp, &work, obj_number*num_signals);
			spacestatus = (SPACESTATP) dispobj->status;
			/* Für jedes Objekt die entsprechenden Pointer einsetzen */
			spacestatus->xwin 	= xwin;
			spacestatus->ywin 	= ywin;
			spacestatus->xpersp 	= xpersp;
			spacestatus->ypersp 	= ypersp;
			spacestatus->xzoff 	= xzoff;
			spacestatus->yzoff 	= yzoff;
		}
		/* Element einsetzen */
		list_insert(window->dispobjs, list_new_el (dispobj));
	} /* for */

	status->num_spaces = num_displayobs;
} /* create_displayobs */

/*****************************************************************************/
/* Öffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (WORD icon)
{
	BOOLEAN 	ok;
	WINDOWP 	window;
	
	/* Fenster suchen */
	window = search_window (CLASS_CMO, SRCH_CLOSED, icon);
	/* Wenn nicht gefunden */
	if (window == NULL)
	{
		if (instance_count < max_instances && create() != NULL)
			window = search_window (CLASS_CMO, SRCH_CLOSED, icon);
		else if (instance_count >= max_instances)
		{
			window = search_window (CLASS_CMO, SRCH_OPENED, icon);
			if (window) top_window (window);
			return window != NULL;
		}
	} /* if */
	
	ok = window != NULL;
	
	if (ok)
	{
		ok = open_window (window);
	} /* if */
	
	return (ok);
} /* open_mod */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

PUBLIC BOOLEAN info_mod (WINDOWP window, WORD icon)
{
	RTMCLASSP	module = Module(window);
	WINDOWP		refwindow = window;
	WORD			ret;
	STRING		s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, ICMO);
		
	if (window == NULL)
	{
		 form_center (cmo_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (cmo_info, NULL, ICMO, refwindow->name, WI_MODELESS);
	} /* if */
		
	if (window != NULL)
	{
		window->object = cmo_info;
		sprintf(s, "%-20s", CMODATE);
		set_ptext (cmo_info, CMOIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (cmo_info, CMOCOMPILE, s);
		sprintf(s, "%-20s",  CMOVERSION);
		set_ptext (cmo_info, CMOIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (cmo_info, CMOISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (cmo_info, CMOIAKT, s);

		if (! open_dialog (ICMO)) hndl_alert (ERR_NOOPEN);
	} /* if */

  return (window != NULL);
} /* info_cmo */

/*****************************************************************************/
PRIVATE VOID init_standard (RTMCLASSP module, WORD instance)
{
	SET_P standard = module->standard;

	mem_set (standard, 0, (UWORD)sizeof (SETUP));
	standard->grafikmodus = MONO;
	standard->raumform = WUERFEL;
	standard->innenraum = TRUE;
	standard->master = 100;
	standard->midi_channel = instance == 0 ? 16 : 15;
	standard->rot_x = 30;
	standard->rot_y = 20;
	standard->rot_z = 200;
} /* init_standard */

PRIVATE	RTMCLASSP create (VOID)
{
	RTMCLASSP 	module;
	WINDOWP		window;
	STAT_P		status;
	FILE			*fp;
	WORD			signal;
	LIST_P		list;
	SHORT			refNum;
	
	daktstatus("Initialisierung CMO", "Start");
	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_CMO;
		module->icon				= &cmo_desk[CMOICON];
		module->icon_position 	= ICMO;
		module->icon_number		= ICMO;	/* Soll bei Init vergeben werden */
		module->menu_title		= MOUTPUTS;
		module->menu_position	= MCMO;
		module->menu_item			= MCMO;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_cmo;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_OTHER;
		module->apply				= apply;
		module->reset				= reset;
		module->precalc			= precalc;
		module->message			= message;
		module->create				= create;
		module->destroy			= destroy_obj;
		module->test				= test_obj;
	
		module->setup_length			= sizeof(SETUP);
		module->location				= SETUPS_EXTERN;
		module->ram_modified 		= FALSE;
		module->max_setups	 		= MAXSETUPS;
		module->standard				= (SET_P)mem_alloc(sizeof(SETUP));
		module->actual->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->edited->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->edited->modified	= FALSE;
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
		/* Prüfen, ob DEFAULT-Datei vorhanden */
		if((fp=fopen(module->file_name, "rb"))!=0)
		{
			/* Wenn vorhanden, laden */
			fclose(fp);
			module->load(module, module->file_name, FALSE);
		} /* if */

		/* Status-Struktur initialisieren */
		status = module->status;
		mem_set(status, 0, (UWORD) sizeof(STATUS));
				
		/* Setup-Strukturen initialisieren */
		init_standard (module, instance_count);
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Fenster generieren */
		window = crt_mod (cmo_setup, cmo_menu, ICMO);

		/* Modul-Struktur einbinden */
		window->module	= (VOID*) module;
		sprintf(window->name, " %s ", module->object_name);
		module->window		= window;
		module->status->refmodule = module;	
		module->status->refwindow = window;	

		/* Bei Midishare anmelden */
		refNum				= init_midishare(module);
		module->special	= (LONG) refNum;

		/* Display-Objekte einklinken */
		create_displayobs (window);

		add_rcv(VAR_SET_CMO,  module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_CMO, MAXSETUPS);
		instance_count++;
	} /* if */
	daktstatus("Initialisierung CMO", "Ende");
	return module;
} /* create */

/*****************************************************************************/

PRIVATE BOOLEAN init_rsc (VOID)

{
  WORD   i, iconw, iconh, iconr;
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
#if false
  alertmsg = &rs_strings [FREESTR];             /* Adresse der Fehlermeldungen */
#endif
  cmo_menu  = (OBJECT *)rs_trindex [CMO_MENU];  /* Adresse der CMO-Menüzeile */
  cmo_setup = (OBJECT *)rs_trindex [CMO_SETUP]; /* Adresse der CMO-Parameter-Box */
  cmo_shelp = (OBJECT *)rs_trindex [CMO_SHELP];	/* Adresse der CMO-Parameter-Hilfe */
  cmo_help  = (OBJECT *)rs_trindex [CMO_HELP];	/* Adresse der CMO-Hilfe */
  cmo_desk  = (OBJECT *)rs_trindex [CMO_DESK];	/* Adresse des CMO-Desktops */
  cmo_text  = (OBJECT *)rs_trindex [CMO_TEXT];	/* Adresse der CMO-Texte */
  cmo_info 	= (OBJECT *)rs_trindex [CMO_INFO];	/* Adresse der CMO-Info-Anzeige */
  cmo_raum	= (OBJECT *)rs_trindex [CMO_RAUM];	/* Adresse der CMO-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (cmo_rsc_ptr, rsc_name))
  {
    strcpy (s, "[3][Resource-File|");
    strcat (s, rsc_name);
    strcat (s, "?][ EXIT ]");
    beep ();
    form_alert (1, s);
    if (! deskacc) return (FALSE);
    menu_unregister (gl_apid);                  /* Wieder abmelden */
    while (TRUE) evnt_timer (65536L);             /* Lasse andere Prozesse ran */
  } /* if */

  rs_gaddr (cmo_rsc_ptr, R_TREE,  CMO_MENU,	&cmo_menu);    /* Adresse des CMO-Menüs */
  rs_gaddr (cmo_rsc_ptr, R_TREE,  CMO_SETUP,	&cmo_setup);   /* Adresse der CMO-Parameter-Box */
  rs_gaddr (cmo_rsc_ptr, R_TREE,  CMO_SHELP,	&cmo_shelp);   /* Adresse der CMO-Parameter-Hilfe */
  rs_gaddr (cmo_rsc_ptr, R_TREE,  CMO_HELP,	&cmo_help);    /* Adresse der CMO-Hilfe */
  rs_gaddr (cmo_rsc_ptr, R_TREE,  CMO_DESK,	&cmo_desk);    /* Adresse des CMO-Desktop */
  rs_gaddr (cmo_rsc_ptr, R_TREE,  CMO_TEXT,	&cmo_text);    /* Adresse der CMO-Texte */
  rs_gaddr (cmo_rsc_ptr, R_TREE,  CMO_INFO,	&cmo_info);    /* Adresse der CMO-Info-Anzeige */
  rs_gaddr (cmo_rsc_ptr, R_TREE,  CMO_RAUM,	&cmo_raum);    /* Adresse der CMO-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (cmo_menu, TRUE);
	fix_objs (cmo_setup, TRUE); 
	fix_objs (cmo_shelp, TRUE);
	fix_objs (cmo_help, TRUE);
	fix_objs (cmo_desk, TRUE);
	fix_objs (cmo_text, TRUE);
	fix_objs (cmo_info, TRUE);
	fix_objs (cmo_raum, TRUE);
	
	
	do_flags (cmo_setup, CMOCANCEL, UNDO_FLAG);
	do_flags (cmo_setup, CMOHELP, HELP_FLAG);

	/* The original ROM-port mixer and the unfinished preset actions are unavailable. */
	menu_enable (cmo_menu, MCMOMASTERDIV, FALSE);
	menu_enable (cmo_menu, MCMOMASTERDEC, FALSE);
	menu_enable (cmo_menu, MCMOMASTERINC, FALSE);
	menu_enable (cmo_menu, MCMOMASTERMUL, FALSE);
	menu_enable (cmo_menu, MCMOKREISEN, FALSE);
	menu_enable (cmo_menu, MCMOPRESSTEREO, FALSE);
	menu_enable (cmo_menu, MCMOPRESOKTO, FALSE);
	menu_enable (cmo_menu, MCMOPERSPEKTIVE, FALSE);
	do_state (cmo_setup, CMOMASTERLEVEL, DISABLED);
	undo_flags (cmo_setup, CMOMASTERLEVEL, EDITABLE);

	menu_enable(menu, MCMO, TRUE);

	return (TRUE);
} /* init_rsc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PRIVATE BOOLEAN term_rsc (VOID)

{
  BOOLEAN ok;

  ok = TRUE;

#if ((XRSC_CREATE|RSC_CREATE) == 0)
  ok = rs_free (cmo_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* MidiShare initialisieren                                                  */
/*****************************************************************************/

PRIVATE WORD init_midishare (RTMCLASSP module)
{
	SHORT		ref, refNum = 0;			/* temporäre Referenznummer */
	STRING	s;
	
	if (msh_available)
	{
		if (MidiShare())
		{
			if (instance_count < max_instances)
			{
				if (instance_count == 0)
					sprintf (s, "CMO");
				else
					sprintf (s, "CMO %d", instance_count + 1);
				refNum = MidiGetNamedAppl(s); /* Alte Applikation schliessen */
				if (refNum > 0) MidiClose(refNum);
				refNum = MidiOpen(s);				/* Applikation für MidiShare öffnen	*/
			} /* if */
		} /* if */
	
		if (refNum == 0)
			 hndl_alert (ERR_NOMIDISHARE);
	
		if (refNum == MIDIerrSpace)			/* Prüfen genug Platz war */
		{
			 hndl_alert (ERR_MIDISHAREFULL);
		} /* if */
	
		if (refNum > 0)							/* Prüfen ob alles klar */
		{
			modulep[refNum] = module;
			InstallFilter(refNum);
			MidiSetRcvAlarm (refNum, receive_evts_cmo);	/* Interrupt-Handler */		
			MidiSetApplAlarm (refNum, receive_alarm_cmo);	/* Alarm-Handler */
			/* An alle anschließen */
			try_all_connect (refNum);
			puf_refresh_output ();
		} /* else */
	} /* if */
	
	return refNum;
	
} /* init_midishare */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_cmo (VOID)
{
	BOOLEAN	ok = TRUE;

	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

	return (ok);
} /* init_cmo */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod (VOID)
{
	return (term_rsc ());
} /* term_mod */
