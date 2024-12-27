/*****************************************************************************/
/*                                                                           */
/* Modul: MTR.C                                                              */
/*                                                                           */
/* Multi-Rotationen                                                          */
/*                                                                           */
/*****************************************************************************/
#define MTRVERSION "V 1.06"
#define MTRDATE "02.02.95"

/*****************************************************************************
V 1.06
- minmax fÅr ctrl-port und channel, 02.02.95
- Status-Init mit ctrl-infos, 02.02.95
- ClickSetupField eingebaut, 30.01.95
- ctrl_out_port/ch eingebaut
- add_rcv fÅr MTR_ACC
- Pause und Umkehr eingebaut
V 1.05 19.05.94
- load_create_infos und instance_count eingebaut
- eigene set_nr eingebaut
- Import-Radius auf UMKREIS geÑndert
- Phasenverschiebung um 90 Grad in apply korrigiert, fÅr kompatibilitÑt mit V 3.0
- Controller-Out eingebaut
V 1.04
- Senden von VAR_MTR0-9
- import modernisiert und Fehler mit MTR_MODUS beseitigt
- window->module eingebaut
- module->window in create
- Umbau auf create_window_obj
V 1.03
- copy_icon umgebaut
V 1.02
- Umstellung auf neue RTMCLASS-Struktur
V 1.01 25.04.93
- Speed korrigiert
V 1.00 20.04.93
- einige Datentypen umbenannt
- Fehler in reset ed/akt beseitigt
*****************************************************************************/
#ifndef XRSC_CREATE
/* #define XRSC_CREATE */                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "mtr_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"
#include "math.h"
#include "msh_unit.h"			/* Wegen Controller-Out */

#include "export.h"
#include "mtr.h"

#if XRSC_CREATE
#include "mtr_mod.rsh"
#include "mtr_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  1								 /* X-Anfangsposition */
#define INITY  ( 3 * gl_hbox)				 /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangshîhe in Pixel */
#define MILLI  0                     /* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "MTR_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 2001L			/* Anzahl der MTR-Setups */
#define WAND MAXKOOR/3				/* Position eines Signales auf einer der WÑnde */
#define UMKREIS WAND*1.4

enum MTR_EBENEN	{MTR_VORNE, MTR_RECHTS, MTR_HINTEN, MTR_LINKS,
						 MTR_OBEN, MTR_UNTEN,
						 MTR_6741, MTR_8523, MTR_7812, MTR_5634,
						 MTR_1573, MTR_2684,
						 MTR_LORU, MTR_LVRH, MTR_VOHU};
enum MTR_RICHTUNG	{MTR_STOP, MTR_GEGENU, MTR_UHRZ};
enum MTR_MODUS		{MTR_AUTOSTOP, MTR_PENDEL, MTR_DURCHLAUF};
enum MTR_FORM		{MTR_KREISE, MTR_KANTEN};

/****** TYPES ****************************************************************/
typedef struct mtr_single
{
	WORD	ebene,			/* Rotationsebene der Multirotation */
			richtung,		/* Rechts/Stop/Links */
			modus,			/* Autostop/Pendel/Durchlauf */
			winkel,			/* Winkelausschnitt der Rotation */
			phase,			/* Phasenlage */
			speed,			/* Geschwindigkeit */
			radius_x,		/* Radius in imaginÑrer X-Richtung */
			radius_y;		/* Radius in imaginÑrer Y-Richtung */
	UINT	prop_winkel : 1,	/* Winkel proportional halten */
			prop_speed	: 1,	/* usw. */
			prop_phase	: 1,
			prop_radx	: 1,
			prop_rady	: 1;
} SINGLE;

typedef struct setup
{
	SINGLE		single [MAXSIGNALS]; /* EnthÑlt alle Informationen Åber MTR-Bewegungen der Signale */
	WORD			form;		/* Bewegungsform Kanten oder Kreise */
} SETUP;

typedef struct setup *SET_P;

typedef struct statsingle
{
	BOOLEAN	pause;
	DOUBLE	position;		/* Position 0..360 Grad der Rotation */
	WORD		pos_x;			/* Letzte X-Position */
	WORD		pos_y;			/* Letzte Y-Position */
	DOUBLE	stepsize;		/* Schrittweite pro Aufruf */
	WORD		speed_koeff;	/* Geschwindigkeits-Faktor */
	WORD		pos_offset;		/* Offset fÅr absolute Positions-Steuerung */
} STATSINGLE;

typedef struct status
{
	STATSINGLE status[MAXSIGNALS];	/* Status fÅr jedes Signal */
	WORD		prop;							/* Proportional-Faktor */
	BOOLEAN	umkehrung;					/* Umkehrung der Drehrichtung */
	BOOLEAN	pause;						/* Anhalten der Drehun */
	WORD		ctrl_out_port;				/* Ausgabe-Port fÅr Controller */
	WORD		ctrl_out_ch;				/* Ausgabe-Kanal fÅr Controller */
} STATUS;

typedef struct status *STAT_P;	
/****** VARIABLES ************************************************************/
PRIVATE WORD	mtr_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*mtr_rsc_ptr = &mtr_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *mtr_setup;
PRIVATE OBJECT *mtr_help;
PRIVATE OBJECT *mtr_ebene;
PRIVATE OBJECT *mtr_desk;
PRIVATE OBJECT *mtr_text;
PRIVATE OBJECT *mtr_info;

PRIVATE SHORT	refvar = 0;				/* MS-Referenz-Nummer des VAR-Moduls */
PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "MTR";		/* Name, fÅr Extension etc. */

/****** FUNCTIONS ************************************************************/
PUBLIC BOOLEAN	set_setnr	(RTMCLASSP module, LONG setupnr);

/* INTERNE MTR-Funktionen */
PRIVATE BOOLEAN init_rsc	_((VOID));
PRIVATE BOOLEAN term_rsc	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	SET_P		ed = module->edited->setup;
	STAT_P	status = module->status;
	STRING 	s;
	WORD 		offset, signal;
	SINGLE	*set_s;

	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		offset=(MTR1-MTR0)*signal;
		set_s=&(ed->single[signal]);
		if (GetCheck(mtr_setup, MTRRIL0 + offset, NULL)) (set_s->richtung =MTR_GEGENU);
		if (GetCheck(mtr_setup, MTRRIS0 + offset, NULL)) (set_s->richtung =MTR_STOP);
		if (GetCheck(mtr_setup, MTRRIR0 + offset, NULL)) (set_s->richtung =MTR_UHRZ);
		
		if (GetCheck(mtr_setup, MTRMOAUTOS0 + offset, NULL))  (set_s->modus =MTR_AUTOSTOP);
		if (GetCheck(mtr_setup, MTRMOPENDEL0 + offset, NULL)) (set_s->modus =MTR_PENDEL);
		if (GetCheck(mtr_setup, MTRMODURCHL0 + offset, NULL)) (set_s->modus =MTR_DURCHLAUF);

		set_s->prop_winkel = GetCheck (mtr_setup, MTRWINKPROP0 + offset, NULL);
		set_s->prop_speed  = GetCheck (mtr_setup, MTRSPEEDPROP0 + offset, NULL);
		set_s->prop_phase  = GetCheck (mtr_setup, MTRPHASPROP0 + offset, NULL);
		set_s->prop_radx   = GetCheck (mtr_setup, MTRRADIUSXPROP0 + offset, NULL);
		set_s->prop_rady   = GetCheck (mtr_setup, MTRRADIUSYPROP0 + offset, NULL);
		
		GetPWord (mtr_setup, MTRWINK0 + offset, &set_s->winkel);
		GetPWord (mtr_setup, MTRSPEE0 + offset, &set_s->speed);
		GetPWord (mtr_setup, MTRPHAS0 + offset, &set_s->phase);
		GetPWord (mtr_setup, MTRRADIUSX0 + offset, &set_s->radius_x);
		GetPWord (mtr_setup, MTRRADIUSY0 + offset, &set_s->radius_y);
		
	}
	if(GetCheck (mtr_setup, MTRKANTE, NULL)) ed->form = MTR_KANTEN;
	if(GetCheck (mtr_setup, MTRKREIS, NULL)) ed->form = MTR_KREISE;

	GetPWord (mtr_setup, MTRCTRLOUTPORT, &status->ctrl_out_port);
	GetPWord (mtr_setup, MTRCTRLOUTCH,   &status->ctrl_out_ch);
	
	status->ctrl_out_port--;
	status->ctrl_out_ch--;
		
	status->ctrl_out_port	= minmax(status->ctrl_out_port, 0, 31);
	status->ctrl_out_ch		= minmax(status->ctrl_out_ch, 0, 15);
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P		edited = module->edited;
	SET_P		ed = edited->setup;
	STAT_P	status = module->status;
	STRING	s;
	WORD		offset, signal;
	SINGLE	*set_s;
	
	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		offset=(MTR1-MTR0)*signal;
		set_s=&(ed->single[signal]);

		copy_icon (&mtr_setup[MTREBEN0 + offset], &mtr_ebene[set_s->ebene+1]);  /* Holen der Iconstruktur aus MTR_EBENEN Popup */
		SetCheck (mtr_setup, MTRRIL0  + offset, set_s->richtung == MTR_GEGENU);
		SetCheck (mtr_setup, MTRRIS0  + offset, set_s->richtung == MTR_STOP);
		SetCheck (mtr_setup, MTRRIR0  + offset, set_s->richtung == MTR_UHRZ);
		
		SetCheck (mtr_setup, MTRMOAUTOS0 + offset, set_s->modus == MTR_AUTOSTOP);
		SetCheck (mtr_setup, MTRMOPENDEL0 + offset, set_s->modus == MTR_PENDEL);
		SetCheck (mtr_setup, MTRMODURCHL0 + offset, set_s->modus == MTR_DURCHLAUF);
		
		SetCheck (mtr_setup, MTRWINKPROP0 + offset, set_s->prop_winkel);
		SetCheck (mtr_setup, MTRSPEEDPROP0 + offset, set_s->prop_speed);
		SetCheck (mtr_setup, MTRPHASPROP0 + offset, set_s->prop_phase);
		SetCheck (mtr_setup, MTRRADIUSXPROP0 + offset, set_s->prop_radx);
		SetCheck (mtr_setup, MTRRADIUSYPROP0 + offset, set_s->prop_rady);

		SetPWord (mtr_setup, MTRWINK0 + offset, set_s->winkel);
		SetPWord (mtr_setup, MTRSPEE0 + offset, set_s->speed);
		SetPWord (mtr_setup, MTRPHAS0 + offset, set_s->phase);
		SetPWord (mtr_setup, MTRRADIUSX0 + offset, set_s->radius_x);
		SetPWord (mtr_setup, MTRRADIUSY0 + offset, set_s->radius_y);
		
	} /* for */
	
	SetCheck (mtr_setup, MTRKANTE, ed->form == MTR_KANTEN);
	SetCheck (mtr_setup, MTRKREIS, ed->form == MTR_KREISE);

	SetPWord (mtr_setup, MTRCTRLOUTPORT, status->ctrl_out_port+1);
	SetPWord (mtr_setup, MTRCTRLOUTCH,   status->ctrl_out_ch+1);
	
	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (mtr_setup, MTRSETNR , s);
} /* set_dbox */

PUBLIC BOOLEAN	set_setnr	(RTMCLASSP module, LONG setupnr)
{
	STAT_P 	status	= module->status;

	/* Standard-Setup-Routine aufrufen */
	return set_setnr_obj (module, setupnr);	
} /* set_setnr */

PRIVATE VOID    send_messages	(RTMCLASSP module)
{
	WORD		signal;
	SET_P		akt = module->actual->setup;
	STAT_P	status = module->status;
	SINGLE	*single = akt->single;

	for (signal = 0; signal < MAXSIGNALS; signal++)
	{
		send_variable(VAR_MTR_ON0 + signal, 
			single[signal].richtung != MTR_STOP &&
			! status->status[signal].pause);
	} /* for */
	send_variable(VAR_SET_MTR, module->actual->number);
		
} /* send_messages */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD 	variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	STAT_P	status	= module->status;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch (variable)
			{
				case VAR_SET_MTR:	
					if ((module->actual->number != value) || module->actual->modified)
						module->set_setnr(module, value);
					break;
				case VAR_PROP_MTR:
					status->prop = (WORD) value;
					break;
				case VAR_MTR_UMK:
					status->umkehrung = (WORD) value;
					break;
				case VAR_MTR_PAUSE:
					status->pause = (WORD) value;
					break;
				default:
					if ((variable >= VAR_MTR_ACC0) && (variable < VAR_MTR_ACC0 + MAXSIGNALS))
						/* Beschleunigung fÅr diesen Kanal setzen */
						status->status[variable-VAR_MTR_ACC0].speed_koeff = (WORD)value; 
					else if ((variable >= VAR_MTR_ON0) && (variable < VAR_MTR_ON0 + MAXSIGNALS))
						/* Pause fÅr diesen Kanal setzen */
						status->status[variable-VAR_MTR_ON0].pause = (WORD)value; 
					else if ((variable >= VAR_LFA_MTR_POS0) && (variable < VAR_LFA_MTR_POS0 + MAXSIGNALS))
						/* Positions-Offset fÅr diesen Kanal setzen */
						status->status[variable-VAR_LFA_MTR_POS0].pos_offset = (WORD)value; 
					else if ((variable >= VAR_LFB_MTR_POS0) && (variable < VAR_LFB_MTR_POS0 + MAXSIGNALS))
						/* Positions-Offset fÅr diesen Kanal setzen */
						status->status[variable-VAR_LFB_MTR_POS0].pos_offset = (WORD)value; 
					break;
			} /* switch */
		break;
	} /* switch */
} /* message */

/*****************************************************************************/

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	WORD				signal = 0, pos_x, pos_y, x, y, z;
	LONG 				temp_x, temp_y;	/* Hilfsvariable um öberlauf zu vermeiden */
	SET_P				set = module->actual->setup;
	SINGLE			*set_s;
	STAT_P 			status = module->status;
	STATSINGLE 		*stat_s;
	WORD 				proport = status->prop, cont_val, cont_pos;
	WORD				position, step, phase, winkel;	/* Zwischenwerte evtl. incl. Proport */
	KOOR_ALL			*koor = event->koors;
	POINT_3D 		*koo;

	/* Zeiger auf Setup fÅr dieses Signal */
	set_s = set->single;
	/* Zeiger auf Status fÅr dieses Signal */
	stat_s = status->status;
	for(signal=0; signal<MAXSIGNALS; signal++)
	{
		if (! stat_s->pause)
		{
			/* Zeiger auf Koordinaten holen */
			koo = &koor->koor[signal].koor;

			if (status->pause)
			{
				/* Bei Pause, auf nicht weiterbewegen */
				step = 0;
			} /*if */
			else
			{ 
				/* Schrittweite (Speed) und allgemeine MTR-Beschleunigung ber. */
				step = stat_s->stepsize * stat_s->speed_koeff / 100;
				if (status->umkehrung)
				{
					/* Richtung umdrehen */
					step *= -1;
				} /* if */
			} /* else */

			/* Evtl. Proport-Faktor berÅcksichtigen */
			if (set_s->prop_speed)
					step = (step * proport) / 100;

			/* Phase aus MTR-Setup  berÅcksichtigen */
			if (set_s->prop_phase)
				/* Evtl. Proport-Faktor berÅcksichtigen */
				phase = (set_s->phase * proport) / 100;
			else
				phase = set_s->phase;
			
			position = stat_s->position + phase + step;

			/* Winkel aus MTR-Setup  berÅcksichtigen */
			if (set_s->prop_winkel)
				/* Evtl. Proport-Faktor berÅcksichtigen */
				winkel	= (set_s->winkel * proport) / 100;
			else
				winkel	= set_s->winkel;
				
			switch(set_s->modus)
			{
				case MTR_AUTOSTOP:					/* Bewegung anhalten */
					if((position > winkel) || (position < -winkel))
						stat_s->stepsize = 0;
					break;
				case MTR_PENDEL:						/* Richtung umkehren */
					if((position >(phase + winkel/2)) || (position < (phase - winkel/2)))
						stat_s->stepsize *= -1;	
					break;
				case MTR_DURCHLAUF:
				default:									/* öberlauf verhindern */
					stat_s->position = fmod(stat_s->position, winkel);
					/* momentane Position neu berechnen */						
					position = stat_s->position + phase + step;
					break;			
			} /* switch */
			
			/* Momentanen Offset aus VAR berÅcksichtigen */
			position += stat_s->pos_offset;

			/* 'position' enhÑlt nun endgÅltigen Wert fÅr diesen Durchlauf */

			/* MIDI Controller Ausgabe der MTR Position */
			if (status->ctrl_out_ch > 0)
			{
				cont_pos = stat_s->position;
				cont_val = 180 - abs( (cont_pos % 360) - 180)*127/180;
				/* BD 2012_01_21 check on VAR first */
				if (refvar)
					controller_out (refvar, status->ctrl_out_port, status->ctrl_out_ch, 20 + signal, cont_val);
			} /* if */
				
			/* FÅr internen Zeiger nur einen Schritt weiter gehen */
			stat_s->position += step;

			switch (set->form)
			{
				/* Positionen aus Wave-Array um 90 Grad Phasenverschoben auslesen */
				case MTR_KANTEN:
					temp_x = ((LONG)set_s->radius_x * wave [(WAVESTEPS*4 + (INT) position + 2*WAVESTEPS/4)% WAVESTEPS][WKANTE]) / WAVEAMPL;
					temp_y = ((LONG)set_s->radius_y * wave [(WAVESTEPS*4 + (INT) position + WAVESTEPS/4)% WAVESTEPS][WKANTE]) / WAVEAMPL;
					break;
				case MTR_KREISE:
					temp_x = ((LONG)set_s->radius_x * wave [(WAVESTEPS*4 + (INT) position + 2*WAVESTEPS/4)% WAVESTEPS][WSINUS]) / WAVEAMPL;
					temp_y = ((LONG)set_s->radius_y * wave [(WAVESTEPS*4 + (INT) position + WAVESTEPS/4)% WAVESTEPS][WSINUS]) / WAVEAMPL;
					break;
			} /* switch */

			/* Proportional-Faktor einberechnen */
			if (set_s->prop_radx)
				pos_x = (WORD) (temp_x * (LONG)proport / 100L);
			else
				pos_x = (WORD) temp_x;
			stat_s->pos_x = pos_x;

			if (set_s->prop_rady)
				pos_y = (WORD) (temp_y * (LONG)proport / 100L);
			else
				pos_y = (WORD) temp_y;
			stat_s->pos_y = pos_y;
			
			switch (set_s->ebene)
			{
				case MTR_VORNE:
					x =  pos_x;
					y =  pos_y;
					z = -WAND;
					break;
				case MTR_RECHTS:
					x =  WAND;
					y =  pos_y;
					z =  pos_x;
					break;
				case MTR_HINTEN:
					x = -pos_x;
					y =  pos_y;
					z =  WAND;
					break;
				case MTR_LINKS:
					x = -WAND;
					y =  pos_y;
					z = -pos_x;
					break;
				case MTR_OBEN:
					x =  pos_x;
					y =  WAND;
					z =  pos_y;
					break;
				case MTR_UNTEN:
					x =  pos_x;
					y = -WAND;
					z = -pos_y;
					break;
				case MTR_6741:
					x =  pos_y;
					y =  pos_y;
					z =  pos_x;
					break;
				case MTR_8523:
					x =  pos_y;
					y = -pos_y;
					z =  pos_x;
					break;
				case MTR_7812:
					x =  pos_x;
					y =  pos_y;
					z =  pos_y;
					break;
				case MTR_5634:
					x = -pos_x;
					y = -pos_y;
					z =  pos_y;
					break;
				case MTR_1573:
					x = -pos_y;
					y =  pos_x;
					z = -pos_y;
					break;
				case MTR_2684:
					x = -pos_y;
					y =  pos_x;
					z =  pos_y;
					break;
				case MTR_LORU:
					x =  pos_x;
					y =  pos_y;
					z =  0;
					break;
				case MTR_LVRH:
					x = pos_y;
					y = 0;
					z = pos_x;
					break;
				case MTR_VOHU:
					x = 0;
					y = pos_y;
					z = pos_x;
					break;
				default:
					x = 0;
					y = 0;
					z = 0;
			} /* switch */
			koo->x += x;
			koo->y += y;
			koo->z += z;
		} /* if */
		set_s++;
		stat_s++;
		send_variable(VAR_MTR0 + signal, status->status[signal].position);
	} /* for signal */
	/* VAR-Update */
	return event;
} /* apply */

PUBLIC BOOLEAN	import	(RTMCLASSP module, STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	STRING	ext, title, filter;
	FILE		*in;
	STR128	s;
	WORD		signal;
	SINGLE	*single;
	SET_P		akt = module->actual->setup;
	LONG		setnr = 1, max_setups = module->max_setups;
		
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
			/* Zeiger auf erstes Setup nochmal holen, wegen Supervisor-MÅll in file_split */
			akt = module->actual->setup;
			daktstatus(" MTR-Datei wird importiert ... ", module->import_name);
			module->flags |= FLAG_IMPORTING;
			while (ok != EOF && setnr < max_setups)
			{
				/* Zeiger auf Info des ersten Signals */
				single = akt->single;
				for (signal = 0; signal < MAXSIGNALS; signal++)
				{
					ok = fscanf(in, "%d", &single->ebene);
					ok = fscanf(in, "%d", &single->richtung);
					switch(single->richtung)
					{
						case 0:
						case 2:
							single->richtung = MTR_STOP;
							break;
						case 1:
							single->richtung = MTR_GEGENU;
							break;
						case 3:
							single->richtung = MTR_UHRZ;
							break;
					} /* switch */
					ok = fscanf(in, "%d", &(single->modus));
					switch(single->modus)
					{
						case 0:
						case 1:
							single->modus = MTR_AUTOSTOP;
							break;
						case 2:
							single->modus = MTR_PENDEL;
							break;
						case 3:
							single->modus = MTR_DURCHLAUF;
							break;
					} /* switch */
					ok = fscanf(in, "%d", &single->speed);
					ok = fscanf(in, "%d", &single->phase);
					ok = fscanf(in, "%d", &single->winkel);
					single->radius_x = UMKREIS;	/* neu in RTM4 */
					single->radius_y = UMKREIS;	/* neu in RTM4 */
					single++;	/* Auf Info fÅr nÑchste Signal zeigen */
				} /* for */
				ok = fscanf(in, "%d", &(akt->form));	/* Kreis/Kante */
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
	SET_P				akt		= module->actual->setup;
	SINGLE			*set_s	= akt->single; /* Zeiger auf ersten Signal-Setup */
	STAT_P			status	= module->status;
	STATSINGLE		*stat_s	= status->status; /* Zeiger auf ersten Signal-Status */
	WORD 				signal;
		
	/* ZurÅcksetzen von Werten */
	for (signal = 0; signal < MAXSIGNALS; signal++)
	{
		
		stat_s->position		= 0;
		stat_s->speed_koeff	= 100;
	
		/* Schrittweite feststellen */
		if (set_s->speed !=0)
			stat_s->stepsize = 2 * (QUANT * 360) / (set_s->speed * 10);
		else
			stat_s->stepsize = 0;
		
		if (set_s->richtung == MTR_GEGENU) stat_s->stepsize *= -1;

		/* Pausen bei jedem Reset wieder ausschalten */
		stat_s->pause =  ! (set_s->richtung == MTR_GEGENU || set_s->richtung == MTR_UHRZ);

		stat_s++;	/* Auf nÑchsten Signal-Status zeigen */
		set_s++;		/* Auf nÑchsten Signal-Setup zeigen */
		
		
	} /* for */
	
	/* FÅr Controller-Ausgabe */
/* BD 2012_01_22	refvar = MidiGetNamedAppl("VAR"); */
} /* reset */

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	/* Vorausberechnung */
} /* precalc_mtr */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
WINDOWP window;
MKINFO  *mk;
{
	WORD			i, item;
	STRING		s;
	UWORD			signal, offset;
	static		LONG x = 0;	
	RTMCLASSP	module = Module(window);
	STAT_P		status = module->status;
	ED_P			edited = module->edited;
	SET_P			ed = edited->setup;
	SINGLE		*set_s = ed->single; /* Zeiger auf ersten Signal-Setup */

	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case MTRSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case MTRSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case MTRSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case MTRSETSTORE:
			module->set_store (window, edited->number);
			break;
		case MTRSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case MTROK   :
			module->set_ok (window);
			break;
		case MTRCANCEL:
			module->set_cancel (window);
		   break;
		case MTRHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case MTRSTANDARD:
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
				set_ptext (mtr_setup, MTRSETNR, s);
				draw_object(window, MTRSETNR);
			} /* if */
			for(signal=0; signal<MAXSIGNALS; signal++)
			{
				offset=(MTR1-MTR0)*signal;
				set_s=&(ed->single[signal]);
				switch (window->exit_obj-offset)
				{
					case MTREBEN0:
						i = set_s->ebene +1;
						item = popup_menu (mtr_ebene, ROOT, 0, 0, i, TRUE, mk->momask);
						undo_state (window->object, window->exit_obj, SELECTED);
						if ((item != NIL) && (item != i))
						{
							set_s->ebene = item-1;
							copy_icon (&mtr_setup[MTREBEN0 + offset], &mtr_ebene[set_s->ebene+1]);  /* Holen der Iconstruktur aus MTR_EBENEN Popup */
						} /* if */
						draw_object (window, MTREBEN0 + offset);
				} /* switch */
			} /* for */
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

  inx    = num_windows (CLASS_MTR, SRCH_ANY, NULL);
  window = create_window_obj (KIND, CLASS_MTR);

  if (window != NULL)
  {
		WINDOW_INITOBJ_OBJ
    window->flags     = FLAGS | WI_MODELESS;
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

    sprintf (window->name, (BYTE *)mtr_text [FMTRN].ob_spec);
    sprintf (window->info, (BYTE *)mtr_text [FMTRI].ob_spec, 0);
  } /* if */

  return (window);                      /* Fenster zurÅckgeben */
} /* crt_mod */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN 		ok;
	WINDOWP 		window;
	RTMCLASSP	module;
	
	window = search_window (CLASS_MTR, SRCH_ANY, MTR_SETUP);
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (mtr_setup, ROOT, EDITABLE);
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
	WORD			ret;
	STRING		s;
	RTMCLASSP	module = Module(window);

	window = search_window (CLASS_DIALOG, SRCH_ANY, IMTR);
		
	if (window == NULL)
	{
		 form_center (mtr_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (mtr_info, NULL, IMTR, (BYTE *)mtr_text [FMTRN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = mtr_info;
		sprintf(s, MTRDATE);
		set_ptext (mtr_info, MTRIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (mtr_info, MTRCOMPILE, s);
		sprintf(s, "%-20s", MTRVERSION);
		set_ptext (mtr_info, MTRIVERNR, s);
		sprintf(s, "%-20ld", module->file_max);
		set_ptext (mtr_info, MTRISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (mtr_info, MTRIAKT, s);

		if (! open_dialog (IMTR)) hndl_alert (ERR_NOOPEN);
	}

  return (window != NULL);
} /* info_mod */

/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	WINDOWP		window;
	RTMCLASSP 	module;
	BOOLEAN		ok;
	STRING		s;
	FILE			*fp;
	WORD			signal;
	STAT_P		status;
	
	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_MTR;
		module->icon				= &mtr_desk[MTRICON];
		module->icon_position 	= IMTR;
		module->icon_number		= IMTR;	/* Soll bei Init vergeben werden */
		module->menu_title		= MSETUPS;
		module->menu_position	= MMTR;
		module->menu_item			= MMTR;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_mtr;
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
		mtr_module = module;	/* globaler Zeiger auf MTR-Modul */
	
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
		module->set_setnr				= set_setnr;
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
	
		/* Initialisierung der Status-Struktur */
		status = module->status;
		status->ctrl_out_port = 0;
		status->ctrl_out_ch = 0;

		/* Fenster generieren */
		window = crt_mod (mtr_setup, NULL, MTR_SETUP);
		/* Modul-Struktur einbinden */
		window->module = (VOID*) module;
		module->window = window;
	
		add_rcv(VAR_SET_MTR,  module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_MTR, MAXSETUPS);
		add_rcv(VAR_PROP_MTR, module);	/* Message einklinken */
		add_rcv(VAR_MTR_PAUSE, module);	/* Message einklinken */
		add_rcv(VAR_MTR_UMK, module);	/* Message einklinken */
		for (signal = 0; signal < MAXSIGNALS; signal++)
		{
			add_rcv(VAR_MTR_ACC0 + signal, module);	/* Message einklinken */
			add_rcv(VAR_MTR_ON0 + signal, module);	/* Message einklinken */
			add_rcv(VAR_LFA_MTR_POS0 + signal, module);	/* Message einklinken */
			/* add_rcv(VAR_LFB_MTR_POS0 + signal, module);	/* Message einklinken */ */
		} /* for */
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
  mtr_setup = (OBJECT *)rs_trindex [MTR_SETUP]; /* Adresse der MTR-Parameter-Box */
  mtr_ebene = (OBJECT *)rs_trindex [MTR_EBENE];	/* Adresse der MTR-Ebenen */
  mtr_help  = (OBJECT *)rs_trindex [MTR_HELP];	/* Adresse der MTR-Hilfe */
  mtr_desk  = (OBJECT *)rs_trindex [MTR_DESK];	/* Adresse des MTR-Desktops */
  mtr_text  = (OBJECT *)rs_trindex [MTR_TEXT];	/* Adresse der MTR-Texte */
  mtr_info 	= (OBJECT *)rs_trindex [MTR_INFO];	/* Adresse der MTR-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (mtr_rsc_ptr, rsc_name))
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

  rs_gaddr (mtr_rsc_ptr, R_TREE,  MTR_SETUP,	&mtr_setup);   /* Adresse der MTR-Parameter-Box */
  rs_gaddr (mtr_rsc_ptr, R_TREE,  MTR_EBENE,	&mtr_ebene);   /* Adresse der MTR-Ebenen */
  rs_gaddr (mtr_rsc_ptr, R_TREE,  MTR_HELP,	&mtr_help);    /* Adresse der MTR-Hilfe */
  rs_gaddr (mtr_rsc_ptr, R_TREE,  MTR_DESK,	&mtr_desk);    /* Adresse des MTR-Desktop */
  rs_gaddr (mtr_rsc_ptr, R_TREE,  MTR_TEXT,	&mtr_text);    /* Adresse der MTR-Texte */
  rs_gaddr (mtr_rsc_ptr, R_TREE,  MTR_INFO,	&mtr_info);    /* Adresse der MTR-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (mtr_setup, TRUE);
	fix_objs (mtr_ebene, TRUE);
	fix_objs (mtr_help, TRUE);
	fix_objs (mtr_desk, TRUE);
	fix_objs (mtr_text, TRUE);
	fix_objs (mtr_info, TRUE);
	
	
	do_flags (mtr_setup, MTRCANCEL, UNDO_FLAG);
	do_flags (mtr_setup, MTRHELP, HELP_FLAG);
	
	menu_enable(menu, MMTR, TRUE);

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
  ok = rs_free (mtr_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_mtr ()
{
	BOOLEAN 	ok = TRUE;

	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

 	return (ok);
} /* init_mtr */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN ok = TRUE;
	ok &= term_rsc ();
	return (ok);
} /* term_mod */
