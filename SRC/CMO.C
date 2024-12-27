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
- MenÅ-Abfrage fÅr Kreisen eingebaut
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
#include "math.h"

#include "realtim4.h"
#include "cmo_mod.h"
#include "realtspc.h"
#include "dispobj.h"
#include "var.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"
#include <msh_unit.h>		/* Deklarationen fÅr MidiShare Library */
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
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (50 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  (20 * gl_hbox)           /* Anfangshîhe in Pixel */
#define MILLI  100                     /* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "CMO_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS 100l					/* Anzahl der CMO-Setups */
#define KOORV (FBREIT2/MAXKOOR)			/* VerhÑltnis von 3D-Koordinaten zu 
														RTM-Koordinaten */
enum koor_states 
{USED, NEW, FREE};		/* Zustand der Koordinaten in den
														zwei Gruppen */

/* Sound-Ausgabe */
#define MAXOUTPUTS 8
#define ADRSTROBE 0xFA0000l		/* ROM4 Adresse = Offset fÅr faddresse[1][1] */
#define DATSTROBE 0xFB0000l		/* ROM3 Adresse = Offset fÅr vaddresse[0] */

/* Sound-Koordinaten */
#define FBREITE	126
#define FBREIT2	(FBREITE/2)
#define FEINH 		(FBREITE/3)
#define FEIN2 		(FBREITE/6)
#define FBKUBIK 	2000376L /* FBREITE ^3 */
#define KOFFSET 	(FBREIT2/2)
#define KOEFF		50

/* Grafik */
#define Sinq(x) sinus[x%360]
#define Cosq(x) sinus[(x+270)%360]
#define Sqrt(x) sqrt_array[(x)>>1]	

#define MUSTER1 0x5555 /*&X0101010101010101*/
#define MUSTER2 0xAAAA /*&X1010101010101010*/
#define MUSTER3 0xFFFF /*&X1111111111111111*/
#define MUSTER4 0x4444 /*&X0100010001000100*/

#define PUNKTEN 	0x9999
#define STRICHELN 	0x3333
#define STRICH 		0xffff
#define M (-FEIN2)
#define P FEIN2

/* Umwandlung von MIDI-Format auf interne Koordinatendarstellung */
#define MIDI_TO_INTERN(koor) (koor - 64)

#define PercentToMaster(percent) (percent*255/MAXPERCENT)

/****** TYPES ****************************************************************/

typedef struct status *STAT_P;
typedef struct status
{
	BOOLEAN	new;							/* Zwang zum Neuzeichnen */
	BOOLEAN	reset_flag;					/* Parameter wurden zurueckgesetzt */
	KOOR_SINGLE	koor[MAXINPUTS];		/* Aktuelle Koordinaten der Inputs */
	BOOL		kneu[MAXINPUTS];			/* TRUE, wenn Koordinate sich geÑndert hat */
	BOOLEAN	fader_neu[MAXINPUTS];	/* Fader mÅssen upgedated werden */
	BOOLEAN	master_neu[MAXINPUTS];	/* Master mÅssen upgedated werden */
	UWORD		fader[MAXINPUTS][MAXOUTPUTS];		/* Level fÅr Fader */
	UWORD		master[MAXOUTPUTS];		/* Level fÅr Master-Fader */
	BOOLEAN	kreisen[MAXINPUTS];		/* Rotation an/aus pro Kanal*/
	WORD		kreis_pos[MAXINPUTS];	/* Winkel-Positionen der Rotationen */
	WORD		kreis_radius[MAXINPUTS];	/* Radius der Rotationen */
	WINDOWP		refwindow;
	RTMCLASSP	refmodule;				/* Das Bezugsmodul */
	TFilter	filter;						/* Midi-In-Filter	*/
	WORD		num_spaces;					/* Anzahl der WÅrfel */
} STATUS;

typedef struct setup *SET_P;
typedef struct setup
{
	WORD		grafikmodus;				/* In diesem F. benutzte Darstellung */
	WORD		raumform;					/* Raumform fÅr dieses Fenster */
	WORD		rot_x,						/* Grafik-Parameter: Rotationen */
				rot_y,
				rot_z,
				distanz,						/* Distanz */
				persp;						/* Perspektive */
	WORD		master;						/* Master-Level */
	WORD		midi_channel;				/* Midi-Kanal fÅr eingehende Daten */
	BOOLEAN	innenraum;					/* Nur Innenraum anzeigen */
	BOOLEAN	pfeile;						/* Pfeile anzeigen */
} SETUP;	/* EnthÑlt alle Parameter einer kompletten CMO-Einstellung */

/****** VARIABLES ************************************************************/
PRIVATE WORD	cmo_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*cmo_rsc_ptr = &cmo_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *cmo_menu;
PRIVATE OBJECT *cmo_setup;
PRIVATE OBJECT *cmo_shelp;
PRIVATE OBJECT *cmo_help;
PRIVATE OBJECT *cmo_desk;
PRIVATE OBJECT *cmo_text;
PRIVATE OBJECT *cmo_info;
PRIVATE OBJECT *cmo_raum;

PRIVATE INT	xk [FBREITE+1] [FBREITE+1];
PRIVATE INT	yk [FBREITE+1] [FBREITE+1];
PRIVATE FLOAT	vx= 0.55,		/* Default-Werte fÅr Grafikausgabe */
			vy=-0.50,		/* Verzerrung x/y/z */
			vz= 0.20,
			persp=0.0012,	/* Perspektive */
			zoom=2.0;		/* Grîûe */

LOCAL LONG	maxsqrt = (1+3*(FBREIT2*FBREIT2))/2;
LOCAL WORD	*sqrt_array;		/* Wurzel Array */
LOCAL WORD	v_factor;
LOCAL int	kennlinie[256] = 
{
	  0,  31,  53,  87, 109, 114, 118, 121, 124, 126, 129, 131, 133, 135, 136, 138, 139, 140,
	142, 143, 144, 145, 147, 147, 149, 150, 150, 151, 152, 153, 154, 154, 155, 156, 157, 157,
	158, 158, 159, 160, 160, 161, 161, 162, 163, 163, 164, 164, 165, 165, 167, 167, 168, 168,
	169, 169, 169, 170, 170, 171, 171, 172, 172, 172, 173, 174, 174, 174, 175, 175, 176, 176,
	176, 177, 177, 177, 178, 178, 178, 179, 179, 179, 180, 180, 181, 181, 181, 182, 182, 182,
	183, 183, 183, 184, 184, 184, 186, 186, 186, 187, 187, 187, 188, 188, 188, 189, 189, 189,
	189, 190, 190, 190, 191, 191, 191, 192, 192, 192, 193, 193, 193, 194, 194, 194, 194, 195,
	195, 195, 195, 196, 196, 196, 197, 197, 197, 198, 198, 198, 198, 199, 199, 199, 200, 200,
	200, 201, 201, 201, 201, 202, 202, 202, 203, 203, 203, 203, 204, 204, 204, 205, 205, 204,
	205, 205, 205, 206, 206, 206, 207, 207, 207, 208, 208, 208, 209, 209, 209, 209, 210, 210,
	210, 211, 211, 211, 212, 212, 212, 213, 213, 214, 214, 214, 215, 215, 216, 216, 217, 217,
	217, 218, 218, 218, 219, 219, 219, 220, 220, 220, 220, 221, 221, 222, 222, 222, 223, 223,
	224, 224, 225, 225, 226, 226, 227, 227, 228, 229, 229, 230, 230, 231, 231, 232, 233, 233,
	234, 234, 234, 235, 236, 237, 238, 238, 239, 240, 241, 242, 243, 244, 245, 247, 248, 249,
	251, 252, 254, 255
};

LOCAL BYTE	*faddress[MAXINPUTS][MAXOUTPUTS];	/* Hardware-Adressen fÅr Input-Fader */
LOCAL BYTE	*maddress[MAXOUTPUTS];					/* Hardware-Adressen fÅr Master */
LOCAL BYTE	*vaddress[2000];							/* Hardware-Adressen Volume-Werte */
LOCAL WORD	sinus[360];									/* Sinus-Werte fÅr Sinq() */

PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 2;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "CMO";		/* Name, fÅr Extension etc. */

PRIVATE RTMCLASSP	modulep[MAXMSAPPLS];		/* Zeiger auf Modul-Strukturen */
PRIVATE WORD		refNums[2];					/* Referenznummern */
/****** FUNCTIONS ************************************************************/

PRIVATE VOID		dsetup			_((WINDOWP refwindow));
PRIVATE VOID		click_setup		_((WINDOWP window, MKINFO *mk));
PRIVATE RTMCLASSP define_setup	_((WINDOWP window, RTMCLASSP refmodule));
PUBLIC VOID    	get_edit_setup		_((RTMCLASSP module));
PUBLIC BOOLEAN		set_setnr_setup	_((RTMCLASSP module, LONG setupnr));

PRIVATE BOOLEAN	init_rsc			_((VOID));
PRIVATE BOOLEAN	term_rsc			_((VOID));
PRIVATE BOOLEAN	init_koor_cmo	_((VOID));

PRIVATE VOID		kreisen			_((RTMCLASSP module));
PRIVATE VOID		update			(RTMCLASSP module, BOOLEAN force);
PRIVATE VOID		update_single	(RTMCLASSP module, BOOLEAN force, WORD input);
PRIVATE VOID		set_input		(RTMCLASSP module, WORD input);
PRIVATE VOID		fader_calc		_((RTMCLASSP module, WORD input));
PRIVATE VOID		set_fader		_((RTMCLASSP module, WORD input,  WORD output));
PRIVATE VOID		set_master		_((RTMCLASSP module, WORD output));
PRIVATE VOID		init_fader		_((VOID));

PRIVATE WORD ComputeNumDO (WINDOWP window);
PRIVATE VOID ComputeWorkDO (WINDOWP window, WORD obj_num, RECT *work);
PRIVATE VOID create_displayobs (WINDOWP window);

/* MidiShare Funktionen */
PUBLIC VOID	cdecl	receive_evts_cmo	(SHORT refNum);
PUBLIC VOID cdecl receive_alarm_cmo (SHORT refNum, LONG code);
PRIVATE VOID		InstallFilter				_((SHORT refNum));
PRIVATE WORD		init_midishare 			_((VOID));

/*****************************************************************************/

PUBLIC VOID cdecl receive_evts_cmo (SHORT refNum)
{
	MidiEvPtr	event;
	LONG 			n;
	INT 			r;
	MidiEvPtr	myTask;
	RTMCLASSP	module = modulep[refNum];
	SET_P			akt = module->actual->setup;
	STAT_P		status = module->status;
	KOOR_SINGLE	*koor, *k;
	WINDOWP		window = module->window;
	WORD			signal, channel, type;
	UBYTE 		pitch, value;
	
	r = refNum;
	for (n = MidiCountEvs(r); n > 0; --n) 	/* Alle empfangenen Events abarbeiten */
	{
		event = MidiGetEv (r);				/*  Information holen */
		switch (EvType(event))
		{
			case typeKeyOn:
			case typeNote:
				channel = (WORD) Chan(event);
				if (channel == akt->midi_channel - 1)
				{
					pitch = Pitch(event);
					value	= Vel(event);
					
					/* Signal = bits 2 - 7 */
					signal = pitch>>2;
					/* X,Y,Z,Vol = bits 0 und 1 */
					type = pitch & 3;
	
					koor = &status->koor[signal];
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
					status->kneu[signal] = TRUE;
					update_single(module, signal, TRUE);
					window->milli = 1;
				} /* if midi_channel */
				break;
			case typeStop:
				module->reset(module);
				break;
		} /* switch */
		MidiFreeEv (event);
	} /* for */
} /* receive_evts_cmo */

PUBLIC VOID cdecl receive_alarm_cmo (SHORT refNum, LONG code)
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
* Cette procÇdure dÇfinit les valeurs du filtre de l'application. Un filtre *
* est composÇ de trois parties, qui sont trois tableaux de boolÇens :		 * 
* 															 *
*		un tableau de 256 bits pour les ports Midi acceptÇs			 *
*		un tableau de 256 bits pour les types d'ÇvÇnements acceptÇs		 *
*		un tableau de  16 bits pour les canaux Midi acceptÇs			 *
* 															 *
* Dans le code ci dessous, le filtre est paramÇtrÇ pour accepter n'importe	 *
* quel type d'ÇvÇnement. 										 *
* 															 *
* Les paramätres de l'appel :										 *
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
	{ 										
		AcceptBit(filter->evType,i);		/* accepte tous les types d'ÇvÇnements	*/
		AcceptBit(filter->port,i);		/* en provenance de tous les ports		*/
	} /* for */
											
	for (i = 0; i<16; i++)
		AcceptBit(filter->channel,i);	/* et sur tous les canaux Midi		*/
		
	MidiSetFilter( refNum, filter );   /* installe le filtre				*/
} /* InstallFilter */

/**** Steuer-Funktionen *******************************************************/

PRIVATE	VOID kreisen (RTMCLASSP module)
{
	STAT_P	status = module->status;
	KOOR_SINGLE	*koor = status->koor, *k;
	WORD		*pos = status->kreis_pos;
	WORD		*radius = status->kreis_radius;
	WORD		input;
	
	k = koor;
	for(input = 0; input < MAXINPUTS; input++)
	{
		if (status->kreisen[input])
		{
			*pos = *pos++ % 360;
			k->koor.x = *radius * Sinq(*pos * (6 + input)) / 100;
			k->koor.y = 0;
			k->koor.z = *radius * Cosq(*pos * (6 + input)) / 100;
			k->volume = 254;
			k++;
		} /* if */
	} /* for */

	update (module, FALSE);
} /* kreisen */

PRIVATE	VOID master	(RTMCLASSP module, UWORD volume)
{
	STAT_P	status = module->status;
	UWORD		*master = status->master;
	WORD		output, value;
	SET_P		akt = module->actual->setup;
	
	akt->master = min(MAXPERCENT, volume);
	value = PercentToMaster(volume);
	for (output = 0; output < MAXOUTPUTS; output++)
	{
		*master++ = value;
		set_master (module, output);
	} /* for */
	send_messages (module);
} /* master */

PRIVATE	VOID update	(RTMCLASSP module, BOOLEAN force)
{
	WORD input;
	SET_P		akt = module->actual->setup;
	
	for (input = 0; input < MAXINPUTS; input++)
		update_single (module, force, input);
		
	master (module, akt->master);
} /* update */

PRIVATE	VOID fader_calc	(RTMCLASSP module, WORD input)
{
	/* Fader-Position eines Inputs berechnen */
	STAT_P		status = module->status;
	KOOR_SINGLE	*koor = &status->koor[input];
	LONG 	x = (INT) koor->koor.x,
			y = (INT) koor->koor.y,
			z = (INT) koor->koor.z,
			rechts	= max(0,  x + KOFFSET),
			links 	= max(0, -x + KOFFSET),
			oben		= max(0,  y + KOFFSET),
			unten 	= max(0, -y + KOFFSET),
			hinten	= max(0,  z + KOFFSET),
			vorne 	= max(0, -z + KOFFSET);
	register UWORD *valp = status->fader[input];	/* Zeiger auf Fader-Werte */

	/* Vol in C: 0 ..127 , in Basic-Version 0 ..254 */
	LONG	vk = KOEFF*2*koor->volume*(FBREIT2-Sqrt((x*x)+(y*y)+(z*z)))/FBREIT2; 	/* Volume-Berechnung */
	if (vk > 0) 
		vk = (vk*vk) / (KOEFF * 255);
	
	if (vk >= KOEFF * 255)
		vk = KOEFF * 255;
	else if (vk < 0)
		vk=0;
	
	/* Hier werden die Faderwerte berechnet und gespeichert */
		
	*valp++ = (WORD) ((vk * links		* unten	* vorne	) / FBKUBIK);
	*valp++ = (WORD) ((vk * rechts	* unten	* vorne	) / FBKUBIK);
	*valp++ = (WORD) ((vk * rechts	* unten	* hinten	) / FBKUBIK);
	*valp++ = (WORD) ((vk * links		* unten	* hinten	) / FBKUBIK);

	*valp++ = (WORD) ((vk * links		* oben	* vorne	) / FBKUBIK);
	*valp++ = (WORD) ((vk * rechts	* oben	* vorne	) / FBKUBIK);
	*valp++ = (WORD) ((vk * rechts	* oben	* hinten	) / FBKUBIK);
	*valp++ = (WORD) ((vk * links		* oben	* hinten	) / FBKUBIK);

} /* fader_calc */

PRIVATE	VOID update_single	(RTMCLASSP module, BOOLEAN recompute, WORD input)
{
	STAT_P		status 		= module->status;
	STAT_P		stat_alt 	= module->stat_alt;
	KOOR_SINGLE	*koor 		= &status->koor[input];
	KOOR_SINGLE	*koor_alt 	= &stat_alt->koor[input];
	BOOLEAN 		set = recompute;
	BOOLEAN		*fader_neu 	= &status->fader_neu[input];
	
	if (*fader_neu == FALSE || recompute)
	{
		/* Fader-Positionen neu berechnen */
		fader_calc (module, input);
		/*
		set |= koor->koor.x != koor_alt->koor.x;
		set |= koor->koor.y != koor_alt->koor.y;
		set |= koor->koor.z != koor_alt->koor.z;
		set |= koor->volume != koor_alt->volume;
		*/
		*fader_neu = FALSE;
	} /* if */
	set_input (module, input);

} /* update_single */

PRIVATE	VOID set_input (RTMCLASSP module, WORD input)
{
	/* Einen ganzen Input an die Hardware Åbegeben */
	volatile WORD peek;
	UWORD *valp, *value = module->status->fader[input];
	WORD *hardw_fader_adr;
	/* AdSpeed8(); */
		
	valp = value;
	hardw_fader_adr = (WORD *)faddress[input][0];
	
	peek = *hardw_fader_adr++;		/* Peek(faddress[input][0] */
	peek = *vaddress[*valp++];			/* Peek(vaddress[value[input][0] */

	peek = *hardw_fader_adr++;		/* Peek(faddress[input][1] */
	peek = *vaddress[*valp++];			/* Peek(vaddress[value[input][1] */

	peek = *hardw_fader_adr++;		/* Peek(faddress[input][2] */
	peek = *vaddress[*valp++];			/* Peek(vaddress[value[input][2] */

	peek = *hardw_fader_adr++;		/* Peek(faddress[input][3] */
	peek = *vaddress[*valp++];			/* Peek(vaddress[value[input][3] */

	peek = *hardw_fader_adr++;		/* Peek(faddress[input][4] */
	peek = *vaddress[*valp++];			/* Peek(vaddress[value[input][4] */

	peek = *hardw_fader_adr++;		/* Peek(faddress[input][5] */
	peek = *vaddress[*valp++];			/* Peek(vaddress[value[input][5] */

	peek = *hardw_fader_adr++;		/* Peek(faddress[input][6] */
	peek = *vaddress[*valp++];			/* Peek(vaddress[value[input][6] */

	peek = *hardw_fader_adr++;		/* Peek(faddress[input][7] */
	peek = *vaddress[*valp++];			/* Peek(vaddress[value[input][7] */

	/* AdSpeed16(); */

	/* activity-Anzeige : */
	/* activity(); */
} /* set_input */

PRIVATE VOID set_master (RTMCLASSP module, WORD output)
{
	VOLATILE BYTE peek;
	WORD		value = module->status->master[output];

	peek = *maddress[output];
	peek = *vaddress[value];

} /* set_master */

PRIVATE	VOID set_fader (RTMCLASSP module, WORD input,  WORD output)
{
	VOLATILE BYTE peek;
	WORD		value = module->status->fader[input][output];

	peek = *faddress[input][output];
	peek = *vaddress[value];
} /* set_fader */

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	SET_P	ed = module->edited->setup;
	STRING 	s;
	WORD	signal, offset;

	get_ptext (cmo_setup, CMOGRAFIKX, s);
	sscanf (s, "%d", &ed->rot_x);
	get_ptext (cmo_setup, CMOGRAFIKY, s);
	sscanf (s, "%d", &ed->rot_y);
	get_ptext (cmo_setup, CMOGRAFIKZ, s);
	sscanf (s, "%d", &ed->rot_z);
	get_ptext (cmo_setup, CMOGRAFIKPERSP, s);
	sscanf (s, "%d", &ed->persp);

	get_ptext (cmo_setup, CMOMASTERLEVEL, s);
	sscanf (s, "%d", &ed->master);

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
	WINDOWP	window = module->window;
	STAT_P	status = module->status;
	WORD		signal;
		
	window->milli = 1; 	/* Update so schnell wie mîglich */

	return event;
} /* apply */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	/* ZurÅcksetzen von Werten */
	STAT_P	status = module->status;
	WINDOWP	window = module->window;
	KOOR_SINGLE	*koor = status->koor, *k;
	WORD		input;
	
	k = status->koor;
	for (input = 0; input < MAXINPUTS; input++)
	{
		k->koor.x = 0;
		k->koor.y = 0;
		k->koor.z = 0;
		k->volume = 0;
		k++;
	} /* for input */
	update (module, FALSE);

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
					module->set_setnr(module, value);
					break;
				case VAR_CM_MASTER1:
					master (module, value);
					break;
			} /* switch */
			break;
	} /* switch */
} /* message */

PUBLIC VOID    send_messages	(RTMCLASSP module)
{
	STAT_P	status = module->status;
	SET_P			akt = module->actual->setup;

	send_variable(VAR_SET_CMO, module->actual->number);
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

LOCAL VOID click_setup (window, mk)
WINDOWP window;
MKINFO  *mk;
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
	
	window = search_window (CLASS_DIALOG, SRCH_CLOSED, CLASS_CMO);
	
	if (window == NULL)
	{
		form_center (cmo_setup, &ret, &ret, &ret, &ret);
		
		window = crt_dialog (cmo_setup, NULL, CLASS_CMO, (BYTE *)cmo_text [FCMOSN].ob_spec, WI_MODELESS);
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
		} /* if */
		else
			window->opened = 1;
		
		if (! open_window (window)) hndl_alert (ERR_NOOPEN);
	
	} /* if */
} /* dsetup */

PRIVATE RTMCLASSP define_setup (WINDOWP window, RTMCLASSP refmodule)
{
	RTMCLASSP module;

	if (window != NULL)
	{
		window->click    = click_setup;
		window->key      = wi_key_obj;
		window->showinfo = info_mod;
		window->showhelp = showhelp_obj;
		module = create_module(module_name, instance_count);
		/* Informationen kopieren */
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
/* MenÅbehandlung                                                            */
/*****************************************************************************/

PRIVATE VOID update_menu_mod (window)
WINDOWP window;
{
	SET_P		akt = Akt(window);
	STAT_P	status = Status(window);
	BOOL		kreisen = FALSE;
	WORD		input;
	
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

	/* Aktionen */
	for(input = 0; input < MAXINPUTS; input++)
		kreisen |= 	status->kreisen[input];

	menu_check(cmo_menu, MCMOKREISEN, kreisen);
	menu_check(cmo_menu, MCMOCHANNEL15, akt->midi_channel == 15);
	menu_check(cmo_menu, MCMOCHANNEL16, akt->midi_channel == 16);

	/* Optionen */
	menu_check(cmo_menu, MCMOINNENRAUM, akt->innenraum);
} /* update_menu_mod */

/*****************************************************************************/

PRIVATE VOID handle_menu_mod (window, title, item)
WINDOWP window;
WORD    title, item;

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

		case MCMOACTIONS: switch (item)
		{
			case MCMOKREISEN	:
				kreisen(module);
				break;
			case MCMORESET	:
				module->reset (module);
				break;
			case MCMOMASTERINC:
				master (module, akt->master + 1);
				window->milli = 1;
				break;
			case MCMOMASTERDEC:
				master (module, akt->master - 1);
				window->milli = 1;
				break;
			case MCMOMASTERDIV:
				master (module, akt->master/2);
				window->milli = 1;
				break;
			case MCMOMASTERMUL:
				if (akt->master == 0)
					master (module, 1);
				else
					master (module, akt->master*2);
				window->milli = 1;
				break;
		} /* switch */
	} /* switch */
		
	if (window != NULL)
		menu_normal (window, title, TRUE);          /* Titel wieder normal darstellen */
} /* handle_menu_mod */

/*****************************************************************************/
/* Taste fÅr Fenster                                                         */
/*****************************************************************************/

PRIVATE BOOLEAN wi_key_mod (window, mk)
WINDOWP window;
MKINFO  *mk;

{
	RTMCLASSP	module = Module(window);
	WORD			refNum = (WORD)module->special;
	STAT_P		status = module->status;
	SET_P			akt = Akt(window);
	BOOLEAN		ok = FALSE;
	
	switch (mk->scan_code)
	{
		case	ESC:	
			module->reset(module);
			ok = TRUE;
			break;
		default:
			switch (mk->ascii_code)
			{
				case '+':
					master (module, akt->master + 1);
					window->milli = 1;
					break;
				case '-':
					master (module, akt->master - 1);
					window->milli = 1;
					break;
				case '/':
					master (module, akt->master/2);
					window->milli = 1;
					break;
				case '*':
					if (akt->master == 0)
						master (module, 1);
					else
						master (module, akt->master*2);
					window->milli = 1;
					break;
			} /* switch */
	} /* switch */

	ok |= (menu_key (window, mk));
	return (ok);
} /* wi_key_mod */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

PRIVATE VOID wi_draw_mod (window)
WINDOWP window;

{	
	RTMCLASSP	module = Module(window);
	STAT_P		status = module->status;
	
	if (status->new)
	{
		clr_scroll (window);
	} /* if */

} /* wi_draw_mod */

/*****************************************************************************/
/* Vor zeichnen Status verÑndern                                             */
/*****************************************************************************/

PRIVATE VOID wi_start_mod (window)
WINDOWP window;

{	
	RTMCLASSP	module = Module(window);
	STAT_P	status = Status(window);
	SET_P		akt = Akt(window);
	DISPOBJP	dispobj;
	LIST_P	header, element;
	BOOLEAN	new = status->new || (window->flags & WI_JUNK);
	BOOLEAN	newkoor = FALSE;
	KOOR_SINGLE	*koor = status->koor, *koor_s;
	BOOL		*kneu = status->kneu;
	POINT_3D	*point;
	POS_3D	position;
	WORD		index, x, base_signal = 0, num_signals, num_objs, rows, cols, obj_count = 0;
	WORD		signal, obj_nr = 0;
	SPACESTATP	spacestatus;
	RECT		work, a;
	
	status->new = new;
	
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
			/* Nur fÅr 3D-Anzeigen */
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
				/* Hîhe fÅr Texte berechnen */
				a = dispobj->work;
				a.y = 2 * gl_hbox;
				dispobj->set_work (dispobj, &a);
			} /* else */
			
		} /* if new */		
	
		/* Nur fÅr 3D-Anzeigen */
		if (obj_nr < num_objs)
		{
			/* FÅr jedes Objekt die entsprechenden Koordinaten einsetzen */
			spacestatus = (SPACESTATP) dispobj->status;
			for (signal = 0; signal < num_signals; signal++)
			{
				if (kneu[signal+base_signal])
				{
					koor_s = &koor[signal+base_signal];
					spacestatus->position[signal].x = koor_s->koor.x;
					spacestatus->position[signal].y = koor_s->koor.y;
					spacestatus->position[signal].z = koor_s->koor.z;
					spacestatus->volume[signal]     = koor_s->volume;
					spacestatus->kneu = TRUE;
				} /* if kneu */
			} /* for */
	
			/* Point to next set of koordinates */
			base_signal += num_signals;
			obj_count++;
			
		} /* if obj_nr < num_objs */


		obj_nr++;
		element = list_next(element);
	} /* while */

		if (status->reset_flag)
			if (dispobj->reset) (*dispobj->reset) (dispobj);
	status->reset_flag = FALSE;	

} /* wi_start_mod */

/*****************************************************************************/
/* Nach zeichnen Status verÑndern                                            */
/*****************************************************************************/

PRIVATE VOID wi_finished_mod (window)
WINDOWP window;

{	
	STAT_P	status = Status(window);

	status->new = FALSE;

} /* wi_finished_mod */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
WINDOWP window;
MKINFO  *mk;
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

PRIVATE VOID wi_snap_mod (window, new, mode)
WINDOWP window;
RECT    *new;
WORD    mode;

{
	STAT_P		status = Status(window);
	wi_snap_obj (window, new, mode);
	status->new = TRUE;
	window->milli = 1; 						/* Updaten ! */
} /* wi_snap_mod */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (window)
WINDOWP window;
{
	redraw_window(window, &window->scroll);
	window->milli = 0; 			/* keine Timer-Funktion mehr bis
										zur nÑchsten énderung */
} /* wi_timer_mod */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;
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
			
		sprintf (window->name, (BYTE *)cmo_text [FCMON].ob_spec, 0);
		sprintf (window->info, (BYTE *)cmo_text [FCMOI].ob_spec, 0);
	} /* if */
	
	return (window);                      /* Fenster zurÅckgeben */
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
	WORD		*xwin,		/* X auf Fenstergrîûe */
				*ywin,		/* y auf Fenstergrîûe */
				*xpersp,		/* X Verzerrung durch Z */
				*ypersp,		/* Y Verzerrung durch Z */
				*xzoff,		/* X-Offset durch Z */ 
				*yzoff;		/* Y-Offset durch Z */ 

	if (window->work.h > 400)
		h = 16;
	else
		h = 8;
	
	/* Liste der Display-Objekte durchgehen und lîschen */
	header = window->dispobjs;
	element = list_next(header);
	while (element != header) {
		dispobj = (DISPOBJP) element->key;
		dispobj->delete (dispobj);
		element = list_next(element);
	} /* while */

	/* Alle objekte lîschen */
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
			/* FÅr jedes Objekt die entsprechenden Pointer einsetzen */
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

	/* Text-Objekte mÅssen immer nach den Grafiken kommen */
	a.x = 1 * w;
	a.y = 0;
	a.h = h;

	strcpy (s, "Master: %ld ");
	a.w = w * (WORD)strlen(s);
	CrtTextDOInsert (window, 0, 0, &a, VAR_CM_MASTER1, s);

	status->num_spaces = num_displayobs;
} /* create_displayobs */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;
{
	BOOLEAN 	ok;
	WINDOWP 	window;
	
	/* Fenster suchen */
	window = search_window (CLASS_CMO, SRCH_CLOSED, icon);
	/* Wenn nicht gefunden */
	if (window == NULL)
	{
		if (create()>0);	/* Neue Instanz */
			window = search_window (CLASS_CMO, SRCH_CLOSED, icon);
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

PUBLIC BOOLEAN info_mod (window, icon)
WINDOWP window;
WORD    icon;
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
PRIVATE	RTMCLASSP create ()
{
	RTMCLASSP 	module;
	WINDOWP		window;
	STAT_P		status;
	SET_P			standard;
	FILE			*fp;
	WORD			signal;
	LIST_P		list;
	SHORT			refNum;
	
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
	
		module->file_pointer			= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->file_pointer, 0, (UWORD)sizeof(FILE));
		module->import_pointer		= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->import_pointer, 0, (UWORD)sizeof(FILE));
		module->setup_length			= sizeof(SETUP);
		module->location				= SETUPS_EXTERN;
		module->ram_modified 		= FALSE;
		module->max_setups	 		= MAXSETUPS;
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

		/* Status-Struktur initialisieren */
		status = module->status;
		mem_set(status, 0, (UWORD) sizeof(STATUS));
				
		/* Setup-Strukturen initialisieren */
		standard = module->standard;
		mem_set(standard, 0, (UWORD) sizeof(SETUP));
		standard->grafikmodus	= MONO;
		standard->raumform		= WUERFEL;
		standard->innenraum		= TRUE;
		standard->master			= 100;
		standard->midi_channel	= 16;

		/* Fluchtpunkt */
		standard->rot_x			= 30;
		standard->rot_y			= 20;
		standard->rot_z			= 200;
		
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
		refNum				= init_midishare();
		module->special	= (LONG) refNum;
		modulep[refNum]	= module;
		if (refNum>0)
			InstallFilter(refNum);									/* Midi-Input-Filter einbauen */	

		/* Display-Objekte einklinken */
		create_displayobs (window);

		add_rcv(VAR_SET_CMO,  module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_CMO, MAXSETUPS);
	} /* if */
	return module;
} /* create */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

PRIVATE	VOID init_fader ()
{
	INT 	x, input, output;
	LONG	i;
	
	for(x = 0; x < 2000; x++)
	{
		vaddress[x] = (BYTE *)(0xfb0000L + 510);		/* DATSTROBE + Maximum */
	} /* for */

	for(x = 0; x < 256; x++)
	{
		vaddress[x] = (BYTE *)(0xfb0000L + kennlinie[x]*2);	/* Jetzt echte Werte */
		vaddress[x+1] = (BYTE *)vaddress[x];
	} /* for */
	
	/* Adressen der Input-Fader */
	for(input = 0; input < MAXINPUTS; input++)
	{
		for(output = 0; output < MAXOUTPUTS; output++)
			faddress[input][output] = (BYTE *)(ADRSTROBE + 2 * (0x100 + MAXOUTPUTS * input + output));
	} /* for */

	/* Adressen der Master-Fader */
	for(output = 0; output < MAXOUTPUTS; output++)
		maddress[output] = (BYTE *)(ADRSTROBE + 2 * output);

	for(x = 0; x < 360; x++)
	{
		sinus[x] = 100 * sin((double) x / 360 * 2 * M_PI);
		/*printf("\033HSinus(%d)=%d\n",x,sinq(x));*/
	} /* for */

	daktstatus (" CMO Initialisierung", "Wurzel-Array-Berechnung ...");
	sqrt_array = (WORD*) mem_alloc (maxsqrt * sizeof (WORD));
	for(i = 0; i < maxsqrt; i++)
	{
		if( i % (maxsqrt/10)== 0)
					set_daktstat((WORD)(i * 100 / maxsqrt));
		sqrt_array[i] = (WORD)sqrt((double)(i<<1));
	} /* for */
	set_daktstat(100);
	close_daktstat();

	v_factor = sqrt((double)(3 * FEIN2 * FEIN2)) * 10;
} /* init_fader */

/*****************************************************************************/

PRIVATE BOOLEAN init_rsc ()

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
/*
  alertmsg = &rs_strings [FREESTR];             /* Adresse der Fehlermeldungen */
*/
  cmo_menu  = (OBJECT *)rs_trindex [CMO_MENU];  /* Adresse der CMO-MenÅzeile */
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
    while (TRUE) evnt_timer (0, 1);             /* Lasse andere Prozesse ran */
  } /* if */

  rs_gaddr (cmo_rsc_ptr, R_TREE,  CMO_MENU,	&cmo_menu);    /* Adresse des CMO-MenÅs */
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
	
	menu_enable(menu, MCMO, TRUE);

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
  ok = rs_free (cmo_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* MidiShare initialisieren                                                  */
/*****************************************************************************/

PRIVATE WORD init_midishare ()
{
	SHORT		ref, refNum = 0;			/* temporÑre Referenznummer */
	STRING	s;
	
	if (msh_available)
	{
		if (MidiShare())
		{
			if (instance_count <= max_instances)
			{
				if (instance_count == 0)
					sprintf (s, "CMO");
				else
					sprintf (s, "CMO %d", instance_count + 1);
				refNum = MidiGetNamedAppl(s); /* Alte Applikation schliessen */
				if (refNum > 0) MidiClose(refNum);
				refNum = MidiOpen(s);				/* Applikation fÅr MidiShare îffnen	*/
			} /* if */
		} /* if */
	
		if (refNum == 0)
			 hndl_alert (ERR_NOMIDISHARE);
	
		if (refNum == MIDIerrSpace)			/* PrÅfen genug Platz war */
		{
			 hndl_alert (ERR_MIDISHAREFULL);
		} /* if */
	
		if (refNum > 0)							/* PrÅfen ob alles klar */
		{
			instance_count++;
			refNums[instance_count] = refNum;				/* Merken fÅr term_mod */
			MidiSetRcvAlarm (refNum, receive_evts_cmo);	/* Interrupt-Handler */		
			MidiSetApplAlarm (refNum, receive_alarm_cmo);	/* Alarm-Handler */
			/* An alle anschlieûen */
			try_all_connect (refNum);
		} /* else */
	} /* if */
	
	return refNum;
	
} /* init_midishare */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_cmo ()
{
	BOOLEAN	ok = TRUE;

	ok &= init_rsc ();
	init_fader ();
	instance_count = load_create_infos (create, module_name, max_instances);

	return (ok);
} /* init_cmo */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	return (term_rsc ());
} /* term_mod */
