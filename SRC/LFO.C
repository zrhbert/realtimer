/*****************************************************************************/
/*                                                                           */
/* Modul: LFO.C                                                              */
/*                                                                           */
/* Low-Frequency-Oscillators                                                 */
/*                                                                           */
/*****************************************************************************/
#define LFOVERSION "V 1.03"
#define LFODATE "03.02.95"

/*****************************************************************************
V 1.03
- VOLUME AUSGESCHALTET! 03.02.95
- Bug in send_messages und message bei PAN berechnung beseitigt, 03.02.95
- minmax fÅr ctrl-port und channel, 02.02.95
- Status-Init mit ctrl-infos, 02.02.95
- LFO_RANDOM repariert, 02.02.95
- falsche Beschriftung der Import-Box korrigiert, 02.02.95
- ClickSetupField eingebaut, 30.01.95
- position_lfo umgebaut fÅr besseres Pausen-Handling, 14.01.95
- random eingebaut, 09.01.95
- pan_pan in quad_breite umbenannt, quad_pos eingebaut, 09.01.95
- bug in pan_pan beseitigt, 08.01.95
- ctrl_out_port/ch eingebaut
- pan_koor und pan-pan eingebaut, 04.01.95
- ClickValueField in wi_click_mod eingebaut, 03.01.95
V 1.02 14.12.94
- Bug in Vol berechnung in apply beseitigt
- Pause und Umkehrung eingebaut
V 1.01 14.07.94
- load_create_infos und instance_count eingebaut
- VAR Ausgabe wieder angeschaltet
- Zoom eingebaut
V 1.00
- eigene set_nr eingebaut
- panpos und panpan default auf 100
- Controller-Out eingebaut
V 0.11
- Senden von VAR_LFA0-9
- VOL-Berechnung mit Multiplikation
- Import mit Cont->Sysvars
- Sysvars eingeklinkt
- import modernisiert
- zoom herausgenommen
- Umstellung auf umgenannte PUF_INF Struktur
- message() optimiert
- module->window in create
- Umbau auf create_window_obj
- message: restliche VAR eingebaut
V 0.10
- copy_icon umgebaut
V 0.09
- Umstellung auf neue RTMCLASS Struktur
V 0.08
- LFO-Funktionen eingebaut
*****************************************************************************/



#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "lfo_mod.h"
#include "realtspc.h"
#include "var.h"

#include "desktop.h"
#include "resource.h"
#include "dialog.h"
#include "menu.h"
#include "errors.h"

#include "objects.h"
#include "math.h"			/* Wegen FLOAT-Routinen */
#include "msh_unit.h"	/* Wegen Controller-Out */

#include "export.h"
#include "lfo.h"

#if XRSC_CREATE
#include "lfo_mod.rsh"
#include "lfo_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 1 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 3 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangshîhe in Pixel */
#define MILLI  0						/* Millisekunden fÅr Zeitablauf */

#define MAXSETUPS 2001l				/* Anzahl der LFO-Setups */
#define MAXPATCH	MAXSIGNALS					/* Anzahl der Patch-Spalten */

#define MOD_RSC_NAME "LFO_MOD.RSC"		/* Name der Resource-Datei */
enum {LFO_STOP, LFO_SINUS, LFO_SQUARE, LFO_TRIANGLE, LFO_SAWUP, 
		LFO_SAWDOWN, LFO_RANDOM, LFO_VAR};	/* Quellen der LFO-Bewegung */

/* Geschwindigkeits->Winkel Umrechnung */
#define Stepwinkel(speed)	speed == 0 ? 0 : 2 * (QUANT * 360) / (speed * 10)
/****** TYPES ****************************************************************/
typedef	struct	lfoquelle
{
	WORD	form,			/* Schwingungsform/Quelle dieses LFOs */
			speed,		/* Geschwindigkeit */
			phase,		/* Phasenlage */
			ampli,		/* Amplitude */
			null,			/* Null-Punkt der Schwingung */
			pauso_zeit,	/* Pause am oberen Scheitelpunkt in Zeiteinheiten */
			pausu_zeit,	/* Pause am unteren Scheitelpunkt in Zeiteinheiten */
			var;			/* VAR-Systemvariable-Input */
	UINT	prop_speed	: 1,	/* Speed abhÑndig von Proportional-Faktor */
			prop_phase	: 1,	/* usw. */
			prop_ampli	: 1,
			prop_null	: 1,
			prop_pauso	: 1,
			prop_pausu	: 1;
} LFOQUELLE;

typedef	struct	lfopatch
{
	/* Geplante Auswirkungen der LFO auf ein Signal */
	WORD	pan_breite,	/* Koeffizient fÅr Panbreite */
			pan_pos,		/* ... Pan-Position (Balance) */ 
			quad_breite,/* Quadro-Pan Breite */
			quad_pos,	/* Quadro-Pan Position */
			versch_x,	/* Verschiebung der X-Koordinate */
			versch_y,	/* ... Y-Koor */
			versch_z,	/* ... Z-Koor */
			volume,		/* Modulation LautstÑrke */
			zoom,			/* Modulation Zoom */
			mtr_pos,		/* Vorgabe fÅr MTR-Position (in Grad) */
			mtr_spd,		/* Modulation MTR-Geschwindigkeit */
			vor_zur;		/* Ausleseposition fÅr Arbeitpuffer */
} LFOPATCH;

typedef	struct	setup
{
	LFOQUELLE	quelle[MAXLFOS];
	LFOPATCH		patch	[MAXPATCH];
} SETUP;

typedef	struct	setup	*SET_P;	/* Zeiger auf LFO-Setup */

typedef struct statqsingle
{
	/* Status eines LFO */
	WORD		pos;				/* Position innerhalb der Amplitude */
	DOUBLE	step_cumul;		/* Position 0..360 Grad der Rotation */
	DOUBLE	step;				/* Schrittweite pro Aufruf */
	WORD		speed_koeff;	/* Geschwindigkeits-Faktor */
	DOUBLE	position;		/* akt. Position 0..360 Grad incl. Phase */
	DOUBLE	pos_alt;			/* Position vor Aufruf der Pause */
	DOUBLE	pauso_cumul;	/* kumulierte Pausenzeit oben */
	DOUBLE	pausu_cumul;	/* kumulierte Pausenzeit oben */
	DOUBLE	pauso_step;		/* Schrittweite pro Aufruf */
	DOUBLE 	pausu_step;		/* Schrittweite pro Aufruf */	
	UINT		pauso_fertig : 1;		/* Pausen-Zeit abgelaufen */	
	UINT		pausu_fertig : 1;		/* Pausen-Zeit abgelaufen */	
	UINT		an 	: 1;		/* LFO in Betrieb */	
} STATQSINGLE;

typedef struct statzsingle
{
	/* Auswirkung aller LFO auf ein Signal */
	WORD	pan_breite,	/* Koeffizient fÅr Panbreite */
			pan_pos,		/* ... Pan-Position (Balance) */ 
			quad_breite,/* Quad Pan-Breite */ 
			quad_pos,	/* ... Pan-Position (Balance) */ 
			versch_x,	/* Verschiebung der X-Koordinate */
			versch_y,	/* ... Y-Koor */
			versch_z,	/* ... Z-Koor */
			volume,		/* Modulation LautstÑrke */
			zoom,			/* Modulation Zoom */
			mtr_pos,		/* Vorgabe fÅr MTR-Position (in Grad) */
			mtr_spd,		/* Modulation MTR-Geschwindigkeit */
			vor_zur;		/* Ausleseposition fÅr Arbeitspuffer */
} STATZSINGLE;

typedef struct status
{
	STATQSINGLE	quelle[MAXLFOS];		/* Status fÅr jeden LFO */
	STATZSINGLE	ziel[MAXSIGNALS];		/* Status der Auswirkungen */
	UINT			pause	 : 1;				/* LFO-System angehalten */
	UINT			umkehr : 1;				/* LFO-System umgedreht */
	WORD			prop;						/* Proportional-Faktor */
	WORD			var_values[MAXSETVARS];	/* Systemvariablen */
	WORD			ctrl_out_port;			/* Ausgabe-Port fÅr Controller */
	WORD			ctrl_out_ch;			/* Ausgabe-Kanal fÅr Controller */
} STATUS;

typedef struct status *STAT_P;	

/****** VARIABLES ************************************************************/
PRIVATE WORD	lfo_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*lfo_rsc_ptr = &lfo_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *lfo_setup;
PRIVATE OBJECT *lfo_help;
PRIVATE OBJECT *lfo_quelle;
PRIVATE OBJECT *lfo_select;
PRIVATE OBJECT *lfo_desk;
PRIVATE OBJECT *lfo_text;
PRIVATE OBJECT *lfo_info;

PRIVATE SHORT	refvar = 0;				/* MS-Referenz-Nummer des VAR-Moduls */
PRIVATE WORD		instance_count = 0;			/* Anzahl der Instanzen */
PRIVATE CONST WORD max_instances = 20;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "LFO";		/* Name, fÅr Extension etc. */

/****** FUNCTIONS ************************************************************/

/* Interne LFO-Funktionen */
PRIVATE WORD	item_quelle		(WORD quelle);
PRIVATE WORD	quelle_item		(WORD item);

PRIVATE UpdateFieldFn UpdateVARInField;

PRIVATE WORD	position_lfo 	(RTMCLASSP module, LFOQUELLE *set_s, STATQSINGLE *stat_qs, WORD proport);
PRIVATE VOID	use_lfos 		(RTMCLASSP module, LFOPATCH patches[], STATQSINGLE quell_s[], STATZSINGLE ziel_s[]);
PRIVATE WORD	select_lfo		(WORD *selection);
PRIVATE VOID 	panning 			(KOOR_ALL *koors, STATZSINGLE ziel_s[]);
PRIVATE VOID 	pan_koor (POINT_3D *k1, POINT_3D *k2, WORD breite, WORD pos);
PRIVATE VOID	pan 				(BYTE *k1, BYTE *k2, WORD breite, WORD pos);

PRIVATE BOOLEAN init_rsc		(VOID);
PRIVATE BOOLEAN term_rsc		(VOID);

/*****************************************************************************/
PRIVATE WORD	item_quelle		(WORD quelle)
{
	/* Umwandlung von internen Wellenformen in ICON-Nummern */
	switch (quelle)
	{
		case LFO_STOP:		return LFQSTOP;
		case LFO_SINUS:	return LFQSINUS;
		case LFO_SQUARE:	return LFQSQUAR;
		case LFO_TRIANGLE:return LFQDREIE;
		case LFO_SAWUP:	return LFQSAWUP;
		case LFO_SAWDOWN:	return LFQSAWDN;
		case LFO_RANDOM:	return LFQRAND;
		case LFO_VAR:		return LFQVAR;
	} /* switch */
	return 0;
	
} /* item_quelle */

PRIVATE WORD	quelle_item		(WORD item)
{
	/* Umwandlung von ICON-Nummern in interne Wellenformen */
	switch (item)
	{
		case LFQSTOP:		return LFO_STOP;
		case LFQSINUS:		return LFO_SINUS;
		case LFQSQUAR:		return LFO_SQUARE;
		case LFQDREIE:		return LFO_TRIANGLE;
		case LFQSAWUP:		return LFO_SAWUP;
		case LFQSAWDN:		return LFO_SAWDOWN;
		case LFQRAND:		return LFO_RANDOM;
		case LFQVAR:		return LFO_VAR;
	} /* switch */
	return 0;
	
} /* quelle_item */

PRIVATE WORD	lfo_offset	(WORD lfo)
{
	/* Berechnung des Objekt-Offsets fÅr zweireihige LFO Anzeige */
	/* Merke: Buttons sind von 1-12 numeriert, LFO's von 0-11 */
	if (lfo < 6)
		return (LFO2-LFO1)*lfo;
	else
		return (LFO7-LFO1)+(LFO8-LFO7)*(lfo-6);
} /* lfo_offset */

PRIVATE VOID    get_dbox	(RTMCLASSP module)
{

	SET_P			ed = module->edited->setup;
	STAT_P		status = module->status;
	WORD			offset;
	LFOQUELLE	*lfo_q;
	LFOPATCH		*lfo_p;
	UWORD			lfo, patch;
	
	lfo_q = ed->quelle;
	for(lfo=0; lfo<MAXLFOS; lfo++)
	{
		offset = lfo_offset(lfo);
		lfo_q->prop_speed	= get_checkbox(lfo_setup, LFOSPDPROP1 + offset);
		lfo_q->prop_phase	= get_checkbox(lfo_setup, LFOPHASPROP1 + offset);
		lfo_q->prop_ampli	= get_checkbox(lfo_setup, LFOAMPLPROP1 + offset);
		lfo_q->prop_null 	= get_checkbox(lfo_setup, LFONULLPROP1 + offset);
		lfo_q->prop_pauso	= get_checkbox(lfo_setup, LFOPAUSOPROP1 + offset);
		lfo_q->prop_pausu	= get_checkbox(lfo_setup, LFOPAUSUPROP1 + offset);

		GetPWord (lfo_setup, LFOSPEED1 + offset, &lfo_q->speed);
		GetPWord (lfo_setup, LFOPHASE1 + offset, &lfo_q->phase);
		GetPWord (lfo_setup, LFOAMPLI1 + offset, &lfo_q->ampli);
		GetPWord (lfo_setup, LFONULL1 + offset, &lfo_q->null);
		GetPWord (lfo_setup, LFOPAUSO1 + offset, &lfo_q->pauso_zeit);
		GetPWord (lfo_setup, LFOPAUSU1 + offset, &lfo_q->pausu_zeit);
		GetPWord (lfo_setup, LFOVARIN1 + offset, &lfo_q->var);
		lfo_q++;
	} /* for */

	lfo_p = ed->patch;
	offset= 0;
	for(patch=0; patch<MAXPATCH; patch++)
	{
		GetPWord (lfo_setup, LFOVERSCHX0 + offset, &lfo_p->versch_x);
		GetPWord (lfo_setup, LFOVERSCHY0 + offset, &lfo_p->versch_y);
		GetPWord (lfo_setup, LFOVERSCHZ0 + offset, &lfo_p->versch_z);
		GetPWord (lfo_setup, LFOZOOM0   + offset, &lfo_p->zoom);
		GetPWord (lfo_setup, LFOVOLUME0 + offset, &lfo_p->volume);
		GetPWord (lfo_setup, LFOMTRPOS0 + offset, &lfo_p->mtr_pos);
		GetPWord (lfo_setup, LFOMTRSPD0 + offset, &lfo_p->mtr_spd);
		GetPWord (lfo_setup, LFOVORZUR0 + offset, &lfo_p->vor_zur);
		lfo_p++;
		offset++;
	} /* for */

	offset= 0;
	for(patch=1; patch<MAXPATCH; patch+=2)
	{
		lfo_p = &ed->patch[patch];
		GetPWord (lfo_setup, LFOPANBREITE1 + offset, &lfo_p->pan_breite);
		GetPWord (lfo_setup, LFOPANPOS1 + offset, &lfo_p->pan_pos);
		offset++;
	} /* for */

	offset= 0;
	for(patch=1; patch<MAXPATCH; patch+=4)
	{
		lfo_p = &ed->patch[patch];
		GetPWord (lfo_setup, LFOQUADBREITE1 + offset, &lfo_p->quad_breite);
		GetPWord (lfo_setup, LFOQUADPOS1 + offset, &lfo_p->quad_pos);
		offset++;
	} /* for */

	GetPWord (lfo_setup, LFOCTRLOUTPORT, &status->ctrl_out_port);
	GetPWord (lfo_setup, LFOCTRLOUTCH,   &status->ctrl_out_ch);
	
	status->ctrl_out_port--;
	status->ctrl_out_ch--;
	
	status->ctrl_out_port 	= minmax(status->ctrl_out_port, 0, 31);
	status->ctrl_out_ch 		= minmax(status->ctrl_out_ch, 0, 15);

} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	ED_P			edited = module->edited;
	SET_P			ed = edited->setup;
	STAT_P		status = module->status;
	STRING		s;
	LFOQUELLE	*lfo_q;
	LFOPATCH		*lfo_p;
	WORD			lfo, patch, offset;
	
	lfo_q = ed->quelle;
	for(lfo=0; lfo<MAXLFOS; lfo++)
	{
		offset = lfo_offset(lfo);

		/* Holen der Iconstruktur aus LFO_QUELLE Popup */
		copy_icon (&lfo_setup[LFOQUELLE1 + offset], &lfo_quelle[item_quelle(lfo_q->form)]);

		SetCheck(lfo_setup, LFOSPDPROP1 + offset, lfo_q->prop_speed);
		SetCheck(lfo_setup, LFOPHASPROP1 + offset, lfo_q->prop_phase);
		SetCheck(lfo_setup, LFOAMPLPROP1 + offset, lfo_q->prop_ampli);
		SetCheck(lfo_setup, LFONULLPROP1 + offset, lfo_q->prop_null);
		SetCheck(lfo_setup, LFOPAUSOPROP1 + offset, lfo_q->prop_pauso);
		SetCheck(lfo_setup, LFOPAUSUPROP1 + offset, lfo_q->prop_pausu);

		SetPWordN (lfo_setup, LFOSPEED1 + offset, lfo_q->speed);
		SetPWordN (lfo_setup, LFOPHASE1 + offset, lfo_q->phase);
		SetPWordN (lfo_setup, LFOAMPLI1 + offset, lfo_q->ampli);
		SetPWordN (lfo_setup, LFONULL1 + offset,  lfo_q->null);
		SetPWordN (lfo_setup, LFOPAUSO1 + offset, lfo_q->pauso_zeit);
		SetPWordN (lfo_setup, LFOPAUSU1 + offset, lfo_q->pausu_zeit);

		/* VAR-Nummer und Name nur anzeigen, wenn VAR als Quelle selektiert */
		if (lfo_q->form == LFO_VAR)
		{
			SetPWordN (lfo_setup, LFOVARIN1 + offset, lfo_q->var);
			sprintf (s, "%s", var_get_name(var_module, lfo_q->var, s));
			set_ptext (lfo_setup, LFOVARNAME1 + offset, s);
		} /* else */
		else
		{
			sprintf (s, "          ");
			set_ptext (lfo_setup, LFOVARIN1 + offset, s);
			set_ptext (lfo_setup, LFOVARNAME1 + offset, s);
		} /* else */
		lfo_q++;
	} /* for */

	lfo_p = ed->patch;
	offset = 0;
	for(patch=0; patch<MAXPATCH; patch++)
	{
		SetPWord2N (lfo_setup, LFOVERSCHX0 + offset, lfo_p->versch_x);
		SetPWord2N (lfo_setup, LFOVERSCHY0 + offset, lfo_p->versch_y);
		SetPWord2N (lfo_setup, LFOVERSCHZ0 + offset, lfo_p->versch_z);
		SetPWord2N (lfo_setup, LFOZOOM0 + offset, lfo_p->zoom);
		SetPWord2N (lfo_setup, LFOVOLUME0 + offset, lfo_p->volume);
		SetPWord2N (lfo_setup, LFOMTRPOS0 + offset, lfo_p->mtr_pos);
		SetPWord2N (lfo_setup, LFOMTRSPD0 + offset, lfo_p->mtr_spd);
		SetPWord2N (lfo_setup, LFOVORZUR0 + offset, lfo_p->vor_zur);
		lfo_p++;
		offset++;
	} /* for */

	offset = 0;
	for(patch=1; patch<MAXPATCH; patch+=2)
	{
		lfo_p = &ed->patch[patch];
		SetPWord2N (lfo_setup, LFOPANBREITE1 + offset, lfo_p->pan_breite);
		SetPWord2N (lfo_setup, LFOPANPOS1 + offset, lfo_p->pan_pos);
		lfo_p++;
		offset++;
	} /* for */

	offset = 0;
	for(patch=1; patch<MAXPATCH; patch+=4)
	{
		lfo_p = &ed->patch[patch];
		SetPWord2N (lfo_setup, LFOQUADBREITE1 + offset, lfo_p->quad_breite);
		SetPWord2N (lfo_setup, LFOQUADPOS1 + offset, lfo_p->quad_pos);
		lfo_p++;
		offset++;
	} /* for */

	if (edited->modified)
		sprintf (s, "%ld*", edited->number);
	else
		sprintf (s, "%ld", edited->number);
	set_ptext (lfo_setup, LFOSETNR , s);

	SetPWordN (lfo_setup, LFOCTRLOUTPORT, status->ctrl_out_port+1);
	SetPWordN (lfo_setup, LFOCTRLOUTCH,   status->ctrl_out_ch+1);

} /* set_dbox */

/*****************************************************************************/

PUBLIC PUF_INF *apply	(RTMCLASSP module, PUF_INF *event)
{
	WORD			lfo = 0, pos, position, signal;	
	LONG 			temp;				/* Hilfsvariable um öberlauf zu vermeiden */
	SET_P			set 		= module->actual->setup;
	LFOQUELLE	*quelle	= set->quelle;
	LFOPATCH		*patch	= set->patch;
	STATUS 		*status	= module->status;
	STATQSINGLE	*stat_qs;
	STATZSINGLE	*z, *ziel = status->ziel; /* Zeiger auf ersten Ziel-Status */
	WORD 			proport = status->prop;
	KOOR_ALL		*koor = event->koors;
	KOOR_SINGLE	*k = koor->koor;
	WORD			*var = status->var_values;
	FLOAT			zoomv, volv;
		
	stat_qs = status->quelle;			/* Status fÅr ersten LFO */
	for(lfo=0; lfo < MAXLFOS; lfo++)
	{
		if (stat_qs->an && quelle->form != LFO_STOP)
		{
			/* LFO Position berechnen */
			position = position_lfo (module, quelle, stat_qs, proport);
			/* Amplitude berechnen */
			switch (quelle->form)
			{
				case LFO_SINUS:
					temp = wave [(WAVESTEPS*2 + position)% WAVESTEPS][WSINUS];
					temp *= (LONG)quelle->ampli;
					temp /= WAVEAMPL;
					temp /= 2;
					break;
				case LFO_SQUARE:
					temp = ((LONG)quelle->ampli * wave [(WAVESTEPS*2 + position)% WAVESTEPS][WSQUARE]) / WAVEAMPL;
					temp /= 2;
					break;
				case LFO_TRIANGLE:
					temp = ((LONG)quelle->ampli * wave [(WAVESTEPS*2 + position)% WAVESTEPS][WTRIANGLE]) / WAVEAMPL;
					temp /= 2;
					break;
				case LFO_SAWUP:
					temp = ((LONG)quelle->ampli * wave [(WAVESTEPS*2 + position)% WAVESTEPS][WSAWUP]) / WAVEAMPL;
					temp /= 2;
					break;
				case LFO_SAWDOWN:
					temp = ((LONG)quelle->ampli * wave [(WAVESTEPS*2 + position)% WAVESTEPS][WSAWDOWN]) / WAVEAMPL;
					temp /= 2;
					break;
				case LFO_VAR:
					temp = var_get_relvalue(var_module, quelle->var);
					temp *= quelle->ampli / 100;
					break;
				case LFO_RANDOM:
					temp = random (quelle->ampli) - quelle->ampli/2;
					break;
			} /* switch */

			/* Proportional-Faktoren einberechnen */
			if (quelle->prop_ampli)
				temp = temp * (LONG)proport / 100L;
				
			if (quelle->prop_null)
				pos = (WORD)temp + (WORD)(quelle->null * (LONG)proport / 100L);
			else
				pos = (WORD)temp + quelle->null;
			
			/* 'pos' enthÑlt nun aktuelle Amplitude */
			stat_qs->pos = pos;

			/* MIDI Controller Ausgabe der LFO Position */
			if (status->ctrl_out_ch > 0)
			{
				/* BD 2012_01_22: onlz if VAR available */
				if (refvar)
					controller_out (refvar, status->ctrl_out_port, status->ctrl_out_ch, 20 + lfo, pos);
			} /* if */
		} /* if */
		quelle++;
		patch++;
		stat_qs++;
	} /* for */
	
	patch = set->patch;
	stat_qs = status->quelle;
	/* Auswirkung der LFO berechnen */
	use_lfos (module, patch, stat_qs, ziel);
	
	panning (koor, ziel);
	z = ziel;		/* Zeiger auf ersten Zielstatus */
	/* Auswirkungen der Summe der LFO in die Koordinaten einberechnen */
	for (signal=0; signal < MAXSIGNALS; signal++)
	{

/* Volume abgeschaltet!
		/* Volume, wenn Patch Volume verÑndert */
		if (patch[signal].volume > 0) 
		{
			volv			= z->volume;
			volv			/= 127;
			k->volume	= max(min(volv * k->volume, 127),0);
		} /* if */
*/

		/* Zoom, wenn Patch Zoom verÑndert */
		if (patch[signal].zoom > 0) 
		{
			zoomv = z->zoom;
			zoomv /= 127;
			k->koor.x *= zoomv;
			k->koor.y *= zoomv;
			k->koor.z *= zoomv;
		} /* if */

		k->koor.x	+= z->versch_x;
		k->koor.x	= max(min(k->koor.x, 63),-63);
		k->koor.y	+= z->versch_y;
		k->koor.y	= max(min(k->koor.y, 63),-63);
		k->koor.z	+= z->versch_z;
		k->koor.z	= max(min(k->koor.z, 63),-63);
		k++;
		z++;
	} /* for */

	send_messages(module);	/* Werte aktualisieren */
	return event;
} /* apply */

PRIVATE WORD position_lfo (RTMCLASSP module, LFOQUELLE *quelle, STATQSINGLE *stat_qs, WORD proport)
{
	/* Positionieren der LFO Quellen durch zeitlichen Ablauf */
	STATUS 	*status	= module->status;
	DOUBLE	*position = &stat_qs->position;
	WORD	step, phase;	/* Zwischenwerte evtl. incl. Proport */
	
	if (status->pause)
	{
		step = 0;
	}
	else
	{
		/* Schrittweite (Speed) und allgemeine MTR-Beschleunigung ber. */
		step = stat_qs->step * stat_qs->speed_koeff / 100;

		/* Wenn Umkerungs-Flag an, Drehrichtung umdrehen */
		if (status->umkehr)
			step *= -1;
	} /* else */
	
	/* Evtl. Proport-Faktor berÅcksichtigen */
	if (quelle->prop_speed)
			step *= proport / 100;
	
	/* Phase aus Setup  berÅcksichtigen */
	if (quelle->prop_phase)
		/* Evtl. Proport-Faktor berÅcksichtigen */
		phase = quelle->phase * proport / 100;
	else
		phase = quelle->phase;
	
	/* öberlauf verhindern */
	stat_qs->step_cumul = fmod(720 + stat_qs->step_cumul, 360);
	
	/* momentane Position neu berechnen */						
	*position = stat_qs->step_cumul + phase + step;
	
	/* 'stat_qs->position' enhÑlt nun endgÅltigen Wert fÅr diesen Durchlauf */
	/* Phase wird dynamisch berechnet fÅr jeden Durchlauf */
							
	/* FÅr internen Zeiger nur einen Schritt weiter gehen */
	if (stat_qs->an && stat_qs->pauso_cumul == 0 && stat_qs->pausu_cumul == 0)
	{
		/* LFO-Weiterstellen */
		stat_qs->step_cumul += step;
	} /* if */
	
	/* Position im oberen Pausenbereich ? */
	if (*position < 270 && *position >=90)
	{
		/* Pause-Oben Zeit eingestellt und Pause oben noch nicht abgelaufen? */
		if (quelle->pauso_zeit > 0  && !stat_qs->pauso_fertig)
		{
			if (stat_qs->pauso_cumul > 359) 	/* Schon fertig? */
			{
				/* Pause ist zu Ende */
				stat_qs->pauso_fertig = TRUE;
				stat_qs->pauso_cumul = 0;
				stat_qs->pausu_fertig = FALSE;
				/* Alte Position wieder einstellen, fÅr glatten Durchlauf */
				*position = stat_qs->pos_alt;
			} /* if */
			else
			{
				if (stat_qs->pauso_cumul == 0) /* Neu initialisieren? */
				{
					/* Merke alte Position */
					stat_qs->pos_alt = *position;
					*position = 90;
				} /* if */
				/* PausenzÑhler inkrementieren */
				if (quelle->prop_pauso)
					/* Evtl. Proport-Faktor berÅcksichtigen */
					stat_qs->pauso_cumul += stat_qs->pauso_step * stat_qs->speed_koeff/100 * proport/100;
				else
					stat_qs->pauso_cumul += stat_qs->pauso_step * stat_qs->speed_koeff/100;
			} /* else */
		} /* if */
		else
		{
			/* Wenn keine Pause eingestellt ist, Pause als abgelaufen betrachten */
			stat_qs->pauso_fertig = TRUE;
			stat_qs->pausu_fertig = FALSE;
		} /* else */
	} /* if */
	
	if (*position < 90 || *position >= 270)
	{
		if (quelle->pausu_zeit > 0  && !stat_qs->pausu_fertig)
		{
			if (stat_qs->pausu_cumul > 359)
			{
				/* Pause ist zu Ende */
				stat_qs->pausu_fertig = TRUE;
				stat_qs->pausu_cumul = 0;
				stat_qs->pauso_fertig = FALSE;
				/* Alte Position wieder einstellen, fÅr glatten Durchlauf */
				*position=stat_qs->pos_alt;
			} /* if */
			else
			{
				if (stat_qs->pausu_cumul == 0)
				{
					/* Merke alte Position */
					stat_qs->pos_alt = *position;
					*position = 270;
				} /* if */
				/* PausenzÑhler inkrementieren */
				if (quelle->prop_pausu)
					/* Evtl. Proport-Faktor berÅcksichtigen */
					stat_qs->pausu_cumul += stat_qs->pausu_step * stat_qs->speed_koeff/100 * proport/100;
				else
					stat_qs->pausu_cumul += stat_qs->pausu_step * stat_qs->speed_koeff/100;
			} /* else */
		} /* else */
		else
		{
			/* Wenn keine Pause eingestellt ist, Pause als abgelaufen betrachten */
			stat_qs->pauso_fertig = FALSE;
			stat_qs->pausu_fertig = TRUE;
		} /* else */
	} /* if */
	
	return ((WORD) *position);
} /* position_lfo */

PRIVATE VOID use_lfos (RTMCLASSP module, LFOPATCH patches[], STATQSINGLE quell_s[], STATZSINGLE ziel_s[])
{
	/* Berechnung der LFO-Auswirkung auf die Signale */
	LFOPATCH		*patch;
	STATZSINGLE *z;
	WORD			signal;
	
	/* Alten Ziel-Status aller LFO lîschen */
	mem_setx (ziel_s, 0, (UWORD) sizeof(STATZSINGLE)*MAXSIGNALS);
	/* Default-Werte fÅr Pan's */
	for (signal=0; signal < MAXSIGNALS; signal++)
	{
			ziel_s[signal].pan_breite = 100;
			ziel_s[signal].quad_breite= 100;
	} /* for */
			
	/* Jeder LFO kann alle Signale beeinflussen. Der Gesamteffekt besteht aus
	der Summe der Auswirkungen */
	
	z = ziel_s;		/* Adresse des ersten Zielstatus */
	patch = patches;
	for (signal=0; signal < MAXSIGNALS; signal++)
	{
		if (patch->mtr_spd > 0)		
		{
			/* MTR-Speed */
			/* Wirkung des zugeordneten LFO addieren */
			z->mtr_spd += quell_s[patch->mtr_spd-1].pos;
		} /* if */
		if (patch->volume > 0)
		{	
			/* Volume */
			/* Wirkung des zugeordneten LFO addieren */
			z->volume += quell_s[patch->volume-1].pos;
		} /* if */
		if (patch->zoom > 0)
		{
			/* Zoom */
			/* usw. */
			z->zoom += quell_s[patch->zoom-1].pos;
		} /* if */
		if (patch->vor_zur > 0)
		{
			/* Vor- und ZurÅck */
			z->vor_zur += quell_s[patch->vor_zur-1].pos;
		} /* if */
		if (patch->mtr_pos > 0)
		{
			/* MTR-Position */
			z->mtr_pos += quell_s[patch->mtr_pos-1].pos;
		} /* if */
		if (patch->versch_x > 0)
		{
			/* Verschieben in X-Richtung */
			z->versch_x += quell_s[patch->versch_x-1].pos;
		} /* if */
		if (patch->versch_y > 0)
		{
			/* Verschieben in X-Richtung */
			z->versch_y += quell_s[patch->versch_y-1].pos;
		} /* if */
		if (patch->versch_z > 0)
		{
			/* Verschieben in X-Richtung */
			z->versch_z += quell_s[patch->versch_z-1].pos;
		} /* if */
		z++;		/* nÑchster Zielstatus */
		patch++;
	} /* for */

	/* ACHTUNG: Nur ein LFO pro Pan */
	for (signal=1; signal < MAXSIGNALS; signal+=2)
	{
		/* Absolute Addressierung wegen Åbersprungener Patches */
		patch = &patches[signal];
		z = &ziel_s[signal];		/* Adresse des Zielstatus */
		if (patch->pan_breite > 0)
		{
			/* Panorama-Breite fÅr Stereo-Panoramen */
			/* LFO-Standard: Ampl 100= Panpos: -50% .. 50% */
			z->pan_breite = quell_s[patch->pan_breite-1].pos;
		} /* if */
		if (patch->pan_pos > 0)
		{
			/* Panorama-Position fÅr Stereo-Panoramen */
			/* LFO-Standard: Ampl 100= Panpos: -50% .. 50% */
			z->pan_pos = quell_s[patch->pan_pos-1].pos;
		} /* if */
		z++;		/* nÑchster Zielstatus */
		patch++;
	} /* for */

	for (signal=1; signal < MAXSIGNALS; signal+=4)
	{
		/* Absolute Addressierung wegen Åbersprungener Patches */
		patch = &patches[signal];
		z = &ziel_s[signal];		/* Adresse des Zielstatus */
		if (patch->quad_breite > 0)
		{
			/* Panorama-Position fÅr Quadro-Panoramen */
			z->quad_breite = quell_s[patch->quad_breite-1].pos;
		} /* if */
		if (patch->quad_pos > 0)
		{
			/* Panorama-Position fÅr Quadro-Panoramen */
			z->quad_pos = quell_s[patch->quad_pos-1].pos;
		} /* if */
	} /* for */
} /* use_lfos */

PRIVATE VOID panning (KOOR_ALL *koors, STATZSINGLE ziel_s[])
{
	/* Pan-Funktion fÅr alle LFO-Ziele */
	STATZSINGLE *z;
	KOOR_SINGLE	*koor = koors->koor;
	WORD			signal;
	KOOR_SINGLE	*k1, *k2;
	
	/* Stereo Panning */
	for (signal=1; signal < MAXSIGNALS; signal+=2)
	{
		k1 = &koor[signal];	/* Pointer auf erste Koordinaten-Gruppe */
		k2 = k1 + 1;			/* Pointer auf zweite Koordinaten-Gruppe */
		z = &ziel_s[signal];
		pan_koor (&k1->koor, &k2->koor, z->pan_breite, z->pan_pos);
	} /* for */
	
	/* Quadro Pan (Pan-Pan) */
	for (signal=1; signal < MAXSIGNALS; signal+=4)
	{
		k1 = &koor[signal];	/* Pointer auf erste Koordinaten-Gruppe */
		z = &ziel_s[signal];
		pan_koor (&koor[signal].koor, &koor[signal + 2].koor, z->quad_breite, z->quad_pos);
		pan_koor (&koor[signal + 1].koor, &koor[signal + 3].koor, z->quad_breite, z->quad_pos);
	} /* for */

} /* panning */

PRIVATE VOID pan_koor (POINT_3D *k1, POINT_3D *k2, WORD breite, WORD pos)
{
	/* Zwei Signale zueinander pannen */
	pan (&k1->x, &k2->x, breite, pos);
	pan (&k1->y, &k2->y, breite, pos);
	pan (&k1->z, &k2->z, breite, pos);
} /* pan_koor */

PRIVATE VOID pan (BYTE *k1, BYTE *k2, WORD breite, WORD pos)
{
	/* Pan-Funktion fÅr eine Dimension */
	WORD	dist, newdist;

	/* Abstand berechnen */
	dist	= *k1 - *k2;

	/* Halbe Panbreite */
	newdist	= dist * (100 - breite) / 100;

	/* Neue Koor = Mitte + Halbe Breite - Verschiebung */
	*k1 += dist * pos/100 - newdist/2;

	/* Zweiter Kanal = Erster Kanal um eine Panbreite verschoben */
	*k2 = *k1 - dist + newdist;
} /* pan */

PRIVATE VOID    send_messages	(RTMCLASSP module)
{
	STAT_P		status = module->status;
	STATQSINGLE	*q, *quelle = status->quelle; /* Zeiger auf ersten Quell-Status */
	STATZSINGLE	*z, *ziel = status->ziel; 		/* Zeiger auf ersten Ziel-Status */
	WORD 			lfo, signal, x=0;

	send_variable(VAR_LFA_UMK, 	status->umkehr);
	send_variable(VAR_LFA_PAUSE,	status->pause);
	send_variable(VAR_SET_LFA, 	module->actual->number);
	
	q = quelle;
	for (lfo = 0; lfo < MAXLFOS; lfo++)
	{
		send_variable(VAR_LFA1 				+ lfo, q->step_cumul);
		send_variable(VAR_LFA_ON1 			+ lfo, q->an);
		send_variable(VAR_LFA_ACC1 		+ lfo, q->speed_koeff);
		q++;
	} /* for */
	
	z = ziel;
	for (signal=0; signal < MAXSIGNALS; signal++)
	{
		send_variable(VAR_MTR_ACC0 		+ signal, z->mtr_spd);
		send_variable(VAR_LFA_VOLUME0 	+ signal, z->volume);
		send_variable(VAR_LFA_ZOOM0 		+ signal, z->zoom);
		send_variable(VAR_LFA_VORZUR0		+ signal, z->vor_zur);
		send_variable(VAR_LFA_MTR_POS0 	+ signal, z->mtr_pos);
		z++;		/* Auf nÑchsten Signal-Setup zeigen */
	} /* for */	
	x = 0;
	z = ziel;
	for (signal=1; signal < MAXSIGNALS; signal+=2)
	{
		z = &ziel[signal];
		send_variable(VAR_LFA_PANBREITE1 + x, z->pan_breite);
		send_variable(VAR_LFA_PANPOS1 	+ x++, z->pan_pos);
	} /* for */	
	x = 0;
	for (signal=1; signal < MAXSIGNALS; signal+=4)
	{
		z = &ziel[signal];
		send_variable(VAR_LFA_QUADBREITE1 	+ x, z->quad_breite);
		send_variable(VAR_LFA_QUADPOS1 		+ x++, z->quad_pos);
	} /* for */	
} /* send_messages */

PUBLIC VOID		reset	(RTMCLASSP module)
{
	/* ZurÅcksetzen von Werten */
	SET_P			akt = module->actual->setup;
	STAT_P		status = module->status;
	LFOQUELLE	*quelle = akt->quelle; 		/* Zeiger auf ersten Signal-Setup */
	STATQSINGLE	*q = status->quelle; 		/* Zeiger auf ersten Quell-Status */
	STATZSINGLE	*z = status->ziel; 			/* Zeiger auf ersten Quell-Status */
	WORD			lfo, signal;
	
	for(lfo=0; lfo < MAXLFOS; lfo++)
	{
		q->pos			= 0;
		q->step_cumul	= 0;
		q->position		= 0;
		q->speed_koeff	= 100;
		q->pauso_fertig = FALSE;	
		q->pausu_fertig = FALSE;	
		q->pauso_cumul	= 0;	
		q->pausu_cumul	= 0;	
		q->an				= quelle->form != LFO_STOP;
		
		/* Schrittweite feststellen */
		q->step			= Stepwinkel (quelle->speed);
		q->pauso_step	= Stepwinkel (quelle->pauso_zeit);
		q->pausu_step	= Stepwinkel (quelle->pausu_zeit);
		quelle++;	/* Auf nÑchstes Signal-Setup zeigen */
		q++;			/* Auf nÑchsten Signal-Status zeigen */
	} /* for */

	for(signal=0; signal < MAXSIGNALS; signal++)
	{
		z->mtr_spd 		= 0;
		z->volume 		= 127;
		z->zoom 			= 127;
		z->vor_zur		= 0;
		z->mtr_pos 		= 0;
		z->pan_pos 		= 0;
		z->pan_breite	= 100;
		z->quad_breite	= 100;
		z->quad_pos 		= 0;
		z++;			/* Auf nÑchsten Ziel-Status zeigen */
	} /* for */

	send_messages (module);
	/* FÅr Controller-Ausgabe */
		/* BD 2012_01_22: disable VAR
		if (!refvar) refvar = MidiGetNamedAppl("VAR");
	 */
} /* reset */

PUBLIC BOOLEAN	import	(RTMCLASSP module, STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	STRING	ext, title, filter;
	FILE		*in;
	STR128	s;
	WORD		x, muell = 0;
	SET_P		akt = module->actual->setup;
	LONG		setnr = 1, max_setups = module->max_setups;
	LFOQUELLE	*quelle;
	LFOPATCH		*patch, muell_patch;
		
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
			daktstatus(" LFO-Datei wird importiert ... ", module->import_name);
			module->flags |= FLAG_IMPORTING;
			while (ok != EOF && setnr < max_setups)
			{
				/* Zeiger auf Info des ersten LFO */
				quelle = akt->quelle;
				for (x = 0; x < MAXLFOS; x++)
				{
					ok = fscanf(in, "%d", &(muell));
					ok = fscanf(in, "%d", &(quelle->form));
					ok = fscanf(in, "%d", &(quelle->speed));
					ok = fscanf(in, "%d", &(quelle->phase));
					ok = fscanf(in, "%d", &(quelle->ampli));
					ok = fscanf(in, "%d", &(quelle->null));
					ok = fscanf(in, "%d", &(quelle->pauso_zeit));
					ok = fscanf(in, "%d", &(quelle->pausu_zeit));
					ok = fscanf(in, "%d", &(quelle->var));
					ok = fscanf(in, "%d", &(muell));
					
					/* Umwandlung von Quelle und Controller-Daten aus RTM3 */
					switch(quelle->form)
					{
						case 0:	quelle->form	= LFO_STOP;
									break;
						case 1:	quelle->form	= LFO_VAR;
									quelle->var		= VAR_MAEX;
									break;
						case 2:	quelle->form	= LFO_VAR;
									quelle->var		= VAR_MAEY;
									break;
						case 3:	quelle->form	= LFO_VAR;
									/* Controller nun Systemvariablen */
									quelle->var		= quelle->var + VAR_VAR0;
									break;
						case 4:	quelle->form	= LFO_SINUS;
									break;
						case 5:	quelle->form	= LFO_TRIANGLE;
									break;
						case 6:	quelle->form	= LFO_SAWUP;
									break;
						case 7:	quelle->form	= LFO_SAWDOWN;
									break;
						case 8:	quelle->form	= LFO_RANDOM;
									break;
						case 9:	quelle->form	= LFO_SQUARE;
									break;
						case 10:	quelle->form	= LFO_VAR; /* Pitchbend eigentlich */
									quelle->var		= VAR_PITCH;
									break;
					}								
					quelle++;	/* Auf Info fÅr nÑchste Signal zeigen */
				} /* for */
				patch = akt->patch;
				for (x = 0; x < MAXPATCH; x++)
				{
					/* Signal 0 Info Åberspringen */
					if (x == 0) patch = &muell_patch;
					ok = fscanf(in, "%d", &muell);
					ok = fscanf(in, "%d", &patch->pan_breite);
					ok = fscanf(in, "%d", &patch->pan_pos);
					ok = fscanf(in, "%d", &patch->mtr_spd);
					ok = fscanf(in, "%d", &patch->volume);
					ok = fscanf(in, "%d", &patch->zoom);
					ok = fscanf(in, "%d", &patch->vor_zur);
					ok = fscanf(in, "%d", &patch->versch_x);
					ok = fscanf(in, "%d", &patch->versch_y);
					ok = fscanf(in, "%d", &patch->versch_z);
					ok = fscanf(in, "%d", &muell);
					if (x == 0) patch = akt->patch;
					patch++;	 /* Auf Info fÅr nÑchste Signal zeigen */
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

PUBLIC VOID		precalc	(RTMCLASSP module)
{
	/* Vorausberechnung */
} /* precalc */

PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD 		variable = ((MSG_SET_VAR *)msg)->variable;
	LONG			value		= ((MSG_SET_VAR *)msg)->value;
	STAT_P		status = module->status;
	STATQSINGLE	*quelle 	= status->quelle;
	STATZSINGLE	*ziel 	= status->ziel;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			switch (variable)
			{
				case VAR_SET_LFA:	
					module->set_setnr(module, value);
					break;
				case VAR_PROP_LFA:
					module->status->prop = (WORD)value;
					break;
				case VAR_LFA_UMK:
					module->status->umkehr = (WORD)value;
					break;
				default:
					if ((variable >= VAR_LFA_ON1) 
						&& (variable < VAR_LFA_ON1 + MAXLFOS))
						quelle[variable - VAR_LFA_ON1 ].an = (WORD)value;
					else if ((variable >= VAR_LFA_ACC1) 
						&& (variable < VAR_LFA_ACC1 + MAXLFOS))
						quelle[variable - VAR_LFA_ACC1 ].speed_koeff = (WORD)value;
					else if ((variable >= VAR_LFA_MTR_POS0) 
						&& (variable < VAR_LFA_MTR_POS0 + MAXSIGNALS))
						ziel[variable - VAR_LFA_MTR_POS0 ].mtr_pos = (WORD)value;
					else if ((variable >= VAR_LFA_VOLUME0) 
						&& (variable < VAR_LFA_VOLUME0 + MAXSIGNALS))
						ziel[variable - VAR_LFA_VOLUME0 ].volume = (WORD)value;
					else if ((variable >= VAR_LFA_ZOOM0) 
						&& (variable < VAR_LFA_ZOOM0 + MAXSIGNALS))
						ziel[variable - VAR_LFA_ZOOM0 ].zoom = (WORD)value;
					else if ((variable >= VAR_LFA_VORZUR0) 
						&& (variable < VAR_LFA_VORZUR0 + MAXSIGNALS))
						ziel[variable - VAR_LFA_VORZUR0 ].vor_zur = (WORD)value;
					else if ((variable >= VAR_LFA_PANBREITE1) 
						&& (variable < VAR_LFA_PANBREITE1 + MAXSIGNALS/2)) {
						ziel[(variable - VAR_LFA_PANBREITE1+1)*2+0].pan_breite = (WORD)value;
						ziel[(variable - VAR_LFA_PANBREITE1+1)*2+1].pan_breite = (WORD)value;
					} /* else if */
					else if ((variable >= VAR_LFA_PANPOS1) 
						&& (variable < VAR_LFA_PANPOS1 + MAXSIGNALS/2)) {
						ziel[(variable - VAR_LFA_PANPOS1+1)*2+0].pan_pos = (WORD)value;
						ziel[(variable - VAR_LFA_PANPOS1+1)*2+1].pan_pos = (WORD)value;
					} /* else if */
					else if ((variable >= VAR_LFA_QUADBREITE1) 
						&& (variable < VAR_LFA_QUADBREITE1 + MAXSIGNALS/4)) {
						ziel[(variable - VAR_LFA_QUADBREITE1+1)*4+0].quad_breite = (WORD)value;
						ziel[(variable - VAR_LFA_QUADBREITE1+1)*4+1].quad_breite = (WORD)value;
						ziel[(variable - VAR_LFA_QUADBREITE1+1)*4+2].quad_breite = (WORD)value;
						ziel[(variable - VAR_LFA_QUADBREITE1+1)*4+3].quad_breite = (WORD)value;
					} /* else if */
					else if ((variable >= VAR_LFA_QUADPOS1) 
						&& (variable < VAR_LFA_QUADPOS1 + MAXSIGNALS/4)) {
						ziel[(variable - VAR_LFA_QUADPOS1+1)*4+0].quad_pos = (WORD)value;
						ziel[(variable - VAR_LFA_QUADPOS1+1)*4+1].quad_pos = (WORD)value;
						ziel[(variable - VAR_LFA_QUADPOS1+1)*4+2].quad_pos = (WORD)value;
						ziel[(variable - VAR_LFA_QUADPOS1+1)*4+3].quad_pos = (WORD)value;
					} /* else if */
/*
					else if (variable < VAR_VAR0 + MAXSETVARS )
						status->var_values[variable - VAR_VAR0] = (WORD)value;
*/
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
	WORD			i, item, offset;
	STRING		s;
	static		LONG x = 0;	
	RTMCLASSP	module = (RTMCLASSP)window->module;
	ED_P			edited = module->edited;
	SET_P			ed = edited->setup;
	LFOQUELLE 	*lfo_q;
	LFOPATCH		*lfo_p = ed->patch;
	WORD 			lfo, signal;
	BOOLEAN		found = FALSE;
	
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case LFOSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case LFOSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case LFOSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case LFOSETSTORE:
			module->set_store (window, edited->number);
			break;
		case LFOSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case LFOOK   :
			module->set_ok (window);
			break;
		case LFOCANCEL:
			module->set_cancel (window);
		   break;
		case LFOHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case LFOCLEAR:
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
				set_ptext (lfo_setup, LFOSETNR, s);
				draw_object(window, LFOSETNR);
			} /* if */
			
			module->get_dbox(module);
			for(lfo=0; lfo<MAXLFOS && !found; lfo++)
			{
				lfo_q = &ed->quelle[lfo];
				switch (window->exit_obj-lfo_offset(lfo))
				{
					case LFOQUELLE1:
			        	undo_state (window->object, window->exit_obj, SELECTED);
						i = item_quelle(lfo_q->form);
						item = popup_menu (lfo_quelle, ROOT, 0, 0, i, TRUE, mk->momask);
						if ((item != NIL) && (item != i))
						{
							lfo_q->form = quelle_item(item);
							/* Holen der Iconstruktur aus LFO_QUELLE Popup */
							copy_icon (&lfo_setup[LFOQUELLE1 + lfo_offset(lfo)], &lfo_quelle[item_quelle(lfo_q->form)]);
							found = TRUE;
						} /* if */
						break;
					case LFOVARIN1:
#if TRUE
						lfo_q->form = LFO_VAR;
						copy_icon (&lfo_setup[LFOQUELLE1 + lfo_offset(lfo)], &lfo_quelle[item_quelle(lfo_q->form)]);
						draw_object(window, LFOQUELLE1 + lfo_offset(lfo));
						ClickValueField (window, window->exit_obj, mk, 0, MAXSYSVARS, UpdateVARInField);
						module->get_dbox (module);
#else
						do {
							if (mk->momask == 0x001)		/* linke Taste */
								x = lfo_q->var + 1;
							else if (mk->momask == 0x002)	/* rechte Taste */
								x = lfo_q->var - 1;
							minmaxsetup(&x, MAXSYSVARS);
							lfo_q->var = (WORD) x;
							module->set_dbox(module);
							draw_object(window, LFOVARIN1 + lfo_offset(lfo));
							/* Maus noch gedrÅckt? */
							if(mk->mobutton>0 && mk->momask>0)
								graf_mkstate(&ret, &ret, &mk->mobutton, &ret);
						} while (mk->mobutton>0 && mk->momask>0);
#endif
						found = TRUE;
						break;
				} /* switch */
			} /* for */

			offset = 0;
			for (signal = 0; !found && signal < MAXSIGNALS; signal++)
			{
				/* Absolute Addressierung wegen Åbersprungener Patches */
				lfo_p = &ed->patch[signal];					/* Patch holen */
				switch (window->exit_obj-signal)
				{
					case  LFOZOOM0:
						select_lfo (&lfo_p->zoom);
						found = TRUE;
						break;
					case LFOVOLUME0:
						select_lfo (&lfo_p->volume);
						found = TRUE;
						break;
					case LFOVERSCHX0:
						select_lfo (&lfo_p->versch_x);
						found = TRUE;
						break;
					case LFOVERSCHY0:
						select_lfo (&lfo_p->versch_y);
						found = TRUE;
						break;
					case LFOVERSCHZ0:
						select_lfo (&lfo_p->versch_z);
						found = TRUE;
						break;
					case LFOMTRPOS0:
						select_lfo (&lfo_p->mtr_pos);
						found = TRUE;
						break;
					case LFOMTRSPD0:
						select_lfo (&lfo_p->mtr_spd);
						found = TRUE;
						break;
					case LFOVORZUR0:
						select_lfo (&lfo_p->vor_zur);
						found = TRUE;
						break;
				} /* switch */
			} /* for */

			offset = 0;
			for (signal = 1; !found && signal < MAXSIGNALS; signal+=2)
			{
				lfo_p = &ed->patch[signal];					/* Patch holen */
				switch (window->exit_obj - offset)
				{
					case  LFOPANBREITE1:
						select_lfo (&lfo_p->pan_breite);
						found = TRUE;
						break;
					case LFOPANPOS1:
						select_lfo (&lfo_p->pan_pos);
						found = TRUE;
						break;
				} /* switch */
				offset++;
			} /* for */

			offset = 0;
			for (signal = 1; !found && signal < MAXSIGNALS; signal+=4)
			{
				/* Absolute Addressierung wegen Åbersprungener Patches */
				lfo_p = &ed->patch[signal];					/* Patch holen */
				switch (window->exit_obj - offset)
				{
					case  LFOQUADBREITE1:
						select_lfo (&lfo_p->quad_breite);
						found = TRUE;
						break;
					case  LFOQUADPOS1:
						select_lfo (&lfo_p->quad_pos);
						found = TRUE;
						break;
				} /* switch */
				offset++;
			} /* for */

			module->set_dbox(module);
			if (found)
			{
				undo_state (window->object, window->exit_obj, SELECTED);
				draw_object (window, ROOT);
			} /* if */
	} /* switch */
} /* wi_click_mod */

PRIVATE VOID UpdateVARInField (WINDOWP window, WORD obj, LONG value)
{
	/* Funktion fÅr Werte-Update,
		Setzt neuen Wert in DBOX und den Text fÅr VAR Name ein
		und fÅr Update auf Screen durch */
		
	WORD		offset = obj - LFOVARIN1;
	STRING 	s;
	
	SetPLong (window->object, obj, value);
	draw_object(window, obj);
	var_get_name(var_module, value, s);
	SetPText (window->object, LFOVARNAME1 + offset, s);
	draw_object(window, LFOVARNAME1 + offset);

} /* UpdateVARInField */

PRIVATE WORD select_lfo (WORD *selection)
{
	/* default ist ein Zeiger auf den aktuellen Wert */
	*selection = popup_menu (lfo_select, ROOT, 0, 0, *selection + 1, TRUE, 0x1) - 1;
	if (*selection < 0) *selection = 0;
	return (*selection);
} /* select_lfo */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;
{
  WINDOWP window;
  WORD    menu_height, inx;

  inx    = num_windows (CLASS_LFO, SRCH_ANY, NULL);
  window = create_window_obj (KIND, CLASS_LFO);

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

    sprintf (window->name, (BYTE *)lfo_text [FLFON].ob_spec);
    sprintf (window->info, (BYTE *)lfo_text [FLFOI].ob_spec, 0);
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
	
	window = search_window (CLASS_LFO, SRCH_ANY, LFO_SETUP);
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (lfo_setup, ROOT, EDITABLE);
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
	WINDOWP	refwindow = window;
		
	window = search_window (CLASS_DIALOG, SRCH_ANY, ILFO);
		
	if (window == NULL)
	{
		 form_center (lfo_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (lfo_info, NULL, ILFO, refwindow->name, WI_MODELESS);
	} /* if */
		
	if (window != NULL)
	{
		window->object = lfo_info;
		sprintf(s, "%-20s", LFODATE);
		set_ptext (lfo_info, LFOIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (lfo_info, LFOCOMPILE, s);
		sprintf(s, "%-20s", LFOVERSION);
		set_ptext (lfo_info, LFOIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (lfo_info, LFOISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (lfo_info, LFOIAKT, s);

		if (! open_dialog (ILFO)) hndl_alert (ERR_NOOPEN);
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
	WORD			lfo, x;
	STAT_P		status;
	
	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module -> class_number	= CLASS_LFO;
		module -> icon				= &lfo_desk[LFOICON];
		module -> icon_position = ILFO;
		module -> icon_number	= ILFO;	/* Soll bei Init vergeben werden */
		module -> menu_title		= MSETUPS;
		module -> menu_position	= MLFO;
		module -> menu_item		= MLFO;	/* Soll bei Init vergeben werden */
		module -> multiple		= FALSE;
		
		module -> crt				= crt_mod;
		module -> open				= open_mod;
		module -> info				= info_mod;
		module -> init				= init_lfo;
		module -> term				= term_mod;

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
		window = module->crt (lfo_setup, NULL, ILFO);
	
		/* Setup-Strukturen initialisieren */
		mem_set(module->standard, 0, (UWORD) sizeof(SETUP));
		
		/* Initialisierung auf erstes Setup */
		module->set_setnr(module, 0);		
	
		/* Initialisierung der Status-Struktur */
		status = module->status;
		status->ctrl_out_port = 0;
		status->ctrl_out_ch = 0;

		/* Fenster generieren */
		window = crt_mod (lfo_setup, NULL, LFO_SETUP);
		/* Modul-Struktur einbinden */
		window->module = (VOID*) module;
		module->window = window;
				
		for (lfo = 0; lfo < MAXLFOS; lfo++)
		{
			add_rcv(VAR_LFA_ON1 + lfo, module);
			add_rcv(VAR_LFA_ACC1 + lfo, module);
		} /* for */
		add_rcv(VAR_LFA_UMK, module);
		add_rcv(VAR_LFA_PAUSE, module);
		add_rcv(VAR_SET_LFA, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_LFA, MAXSETUPS);
		add_rcv(VAR_PROP_LFA, module);	/* Message einklinken */
	
		for (lfo = 0; lfo < MAXLFOS; lfo++)
		{
			add_rcv(VAR_LFB_ON1 + lfo, module);
			add_rcv(VAR_LFB_ACC1 + lfo, module);
		} /* for */
		add_rcv(VAR_LFB_UMK, module);
		add_rcv(VAR_LFB_PAUSE, module);
		add_rcv(VAR_SET_LFB, module);	/* Message einklinken */
		add_rcv(VAR_PROP_LFB, module);	/* Message einklinken */

/*
		for (x = 0; x < MAXSETVARS; x++)
			add_rcv(VAR_VAR0 + x, module);	/* Message einklinken */
*/
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
  alertmsg = &rs_strings [FREESTR];             	/* Adresse der Fehlermeldungen */
*/
  lfo_setup = (OBJECT *)rs_trindex [LFO_SETUP]; 	/* Adresse der LFO-Parameter-Box */
  lfo_help  = (OBJECT *)rs_trindex [LFO_HELP];		/* Adresse der LFO-Hilfe */
  lfo_quelle= (OBJECT *)rs_trindex [LFO_QUELLE];	/* Adresse des LFO-Quellen */
  lfo_select= (OBJECT *)rs_trindex [LFO_SELECT];	/* Adresse des LFO-Selektion */
  lfo_desk  = (OBJECT *)rs_trindex [LFO_DESK];		/* Adresse des LFO-Desktops */
  lfo_text  = (OBJECT *)rs_trindex [LFO_TEXT];		/* Adresse der LFO-Texte */
  lfo_info 	= (OBJECT *)rs_trindex [LFO_INFO];	/* Adresse der LFO-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (lfo_rsc_ptr, rsc_name))
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

  rs_gaddr (lfo_rsc_ptr, R_TREE,  LFO_SETUP,	&lfo_setup);   /* Adresse der LFO-Parameter-Box */
  rs_gaddr (lfo_rsc_ptr, R_TREE,  LFO_HELP,	&lfo_help);    /* Adresse der LFO-Hilfe */
  rs_gaddr (lfo_rsc_ptr, R_TREE,  LFO_QUELLE,&lfo_quelle);  /* Adresse der LFO-Quellen */
  rs_gaddr (lfo_rsc_ptr, R_TREE,  LFO_SELECT,&lfo_select);  /* Adresse der LFO-Auswahl */
  rs_gaddr (lfo_rsc_ptr, R_TREE,  LFO_DESK,	&lfo_desk);    /* Adresse des LFO-Desktop */
  rs_gaddr (lfo_rsc_ptr, R_TREE,  LFO_TEXT,	&lfo_text);    /* Adresse der LFO-Texte */
  rs_gaddr (lfo_rsc_ptr, R_TREE,  LFO_INFO,	&lfo_info);    /* Adresse der LFO-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	fix_objs (lfo_setup, TRUE);
	fix_objs (lfo_help, TRUE);
	fix_objs (lfo_quelle, TRUE);
	fix_objs (lfo_select, TRUE);
	fix_objs (lfo_desk, TRUE);
	fix_objs (lfo_text, TRUE);
	fix_objs (lfo_info, TRUE);
	
	do_flags (lfo_setup, LFOCANCEL, UNDO_FLAG);
	do_flags (lfo_setup, LFOHELP, HELP_FLAG);
	
	menu_enable(menu, MLFO, TRUE);

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
  ok = rs_free (lfo_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_lfo ()
{
	BOOLEAN	ok = TRUE;

	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);

 	return (ok);
} /* init_lfo */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()

{
	BOOLEAN ok = TRUE;
	
	ok &= term_rsc ();
	return (ok);
} /* term_mod */
