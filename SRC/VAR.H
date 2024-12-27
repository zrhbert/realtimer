/*****************************************************************************/
/*                                                                           */
/* Modul: VAR.H                                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- VAR_SYNC_IN, VAR_SYNC_IN_FRAMES, VAR_SYNC_IN_PORTS,	VAR_SYNC_IN_LOCKED,
  VAR_SYNC_OUT, VAR_SYNC_OUT_FRAMES, VAR_SYNC_OUT_PORTS, eingebaut, 04.03.95
- VAR_SYNC_SEND eingebaut, 23.02.95
- var_get_relvalue eingebaut, 03.02.95
- VAR_SET_SPD eingebaut, 15.01.95
- QUADBREITE und QUADPOS eingebaut, 09.01.95
14.12.94
- VAR_CM_MASTER1/2 eingeb.
- VAR_SET_EC4 eingebaut
- VAR_SET_ED4 eingebaut
18.05.94
- VAR_TRA_xxx eingebaut
- VAR_CYCLE eingebaut
- VAR_PUF_KOORX/Y/Z/V eingebaut
- update_var global gemacht, var_set_value und send_variable als Makro
- var_set_value eingebaut
- get_var_name und get_var_value eingebaut
- VAR_MTR0-9, VAR_POWF, VAR_POWF, VAR_PROP_SPS und VAR_VOLUME0 eingefÅhrt
- VARVAR0 auf 1
- MAXSYSVARS und MAXSETVARS nun global
- VAR_SET_ROT, VAR_REL_MTRLFO und VAR_ZOOM_PROP eingebaut
10.05.93
- CALLBACK-Pointer ersetzt durch RTMCLASSP
*****************************************************************************/

#ifndef __VAR__
#define __VAR__

/****** DEFINES **************************************************************/
#define MAXSETVARS		40				/* Anzahl der Variablen pro Setup */
#define MAXSYSVARS		999			/* Anzahl der Variablen im ges. System */

/* Senden von Variablen jetzt direkt an update_var, wg. Zeitersparnis */
#define send_variable(var, value) update_var (var_module, var, value, FALSE)
#define resend_variable(var, value) update_var (var_module, var, value, TRUE)
#define var_set_value(var, value) update_var (var_module, var, value, FALSE)
#define add_rcv(var, module) var_add_rcv (var_module, var, module)

enum {
	VAR_UNIT_INDEX = CLASS_VAR<<8,		/* FÅr Setup-Nummern, etc. */
	VAR_UNIT_MIDI,			/* FÅr Midi-Controller, etc. */
	VAR_UNIT_DEGREE,		/* Grad, z.B. bei Rotationen */
	VAR_UNIT_MILLISEC,	/* Zeit, in Millisekunden */
	VAR_UNIT_PERCENT,		/* Faktoren, in Prozent */
	VAR_UNIT_BOOL			/* Schalter, an=1/aus=0 */
};

/* Systemvariablen */
enum {VAR_VAR0 = 1,						/* bis zu 128 Systemvariablen */
		VAR_CONT0 = VAR_VAR0 + 128,	/* 128 Controller */
		VAR_PITCH =	VAR_CONT0 + 128,	/* Pitch-Bend-Rad */
		VAR_MAAX, VAR_MAAY, VAR_MAAZ,
		VAR_MAEX, VAR_MAEY, VAR_MAEZ,
		VAR_POWX, VAR_POWY, VAR_POWZ, VAR_POWF, VAR_POWR, VAR_POWK,
		VAR_LFA1,
		VAR_LFB1 = VAR_LFA1 + MAXLFOS,
		VAR_MTR0 = VAR_LFB1 + MAXLFOS,
		VAR_PROP_LFA = VAR_MTR0 + MAXSIGNALS, VAR_PROP_LFB, VAR_PROP_MTR, VAR_PROP_SPG,
		VAR_PROP_SPO, VAR_PROP_SPS, VAR_PROP_MAA, VAR_PROP_MAE, VAR_PROP_POW,
		/* VOL-Direktsteuerung */
		VAR_VOLUME0 			= 512,
		/* MTR-Beschleunigungsfaktor */
		VAR_MTR_ACC0 			= VAR_VOLUME0 + MAXSIGNALS,		
		/* MTR-An/Aus einzelne Rotationen */
		VAR_MTR_ON0 			= VAR_MTR_ACC0 + MAXSIGNALS,
		/* MTR-Umkehrung */
		VAR_MTR_UMK 			= VAR_MTR_ON0 + MAXSIGNALS,		
		/* MTR angehalten */
		VAR_MTR_PAUSE,
		/* LFA-An/Aus einzelne LFA's*/
		VAR_LFA_ON1,
		/* LFA-Umkehrung */
		VAR_LFA_UMK				= VAR_LFA_ON1 + MAXLFOS,
		/* LFA angehalten */
		VAR_LFA_PAUSE,
		/* LFA-Beschleunigungsfaktor */
		VAR_LFA_ACC1,
		/* LFA MTR-Position */
		VAR_LFA_MTR_POS0 		= VAR_LFA_ACC1 + MAXLFOS,
		/* LFA Volumenvorgabe */
		VAR_LFA_VOLUME0		= VAR_LFA_MTR_POS0 + MAXSIGNALS,
		/* LFA Zoomvorgabe */
		VAR_LFA_ZOOM0			= VAR_LFA_VOLUME0 + MAXSIGNALS,
		/* LFA Vor-und-ZurÅck Ausleseposition */
		VAR_LFA_VORZUR0		= VAR_LFA_ZOOM0 + MAXSIGNALS,
		/* LFA Panbreite */
		VAR_LFA_PANBREITE1	= VAR_LFA_VORZUR0 + MAXSIGNALS,
		/* LFA Panposition */
		VAR_LFA_PANPOS1		= VAR_LFA_PANBREITE1 + 4,
		/* LFA Quadro-Breite */
		VAR_LFA_QUADBREITE1	= VAR_LFA_PANPOS1 + 4,
		/* LFA Quadro-Pos */
		VAR_LFA_QUADPOS1		= VAR_LFA_QUADBREITE1 + 2,
		/* LFB-An/Aus einzelne LFB's*/
		VAR_LFB_ON1				= VAR_LFA_QUADPOS1 + 2,
		/* LFB-Umkehrung */
		VAR_LFB_UMK				= VAR_LFB_ON1 + MAXLFOS,
		/* LFB angehalten */
		VAR_LFB_PAUSE,
		/* LFB-Beschleunigungsfaktor */
		VAR_LFB_ACC1,
		/* LFB MTR-Position */
		VAR_LFB_MTR_POS0 		= VAR_LFB_ACC1 + MAXLFOS,
		/* LFB Volumenvorgabe */
		VAR_LFB_VOLUME0		= VAR_LFB_MTR_POS0 + MAXSIGNALS,
		/* LFB Zoomvorgabe */
		VAR_LFB_ZOOM0			= VAR_LFB_VOLUME0 + MAXSIGNALS,
		/* LFB Vor-und-ZurÅck Ausleseposition */
		VAR_LFB_VORZUR0		= VAR_LFB_ZOOM0 + MAXSIGNALS,
		/* LFB Panbreite */
		VAR_LFB_PANBREITE1	= VAR_LFB_VORZUR0 +4,
		/* LFB Panposition */
		VAR_LFB_PANPOS1		= VAR_LFB_PANBREITE1 + 4,
		/* LFB Quadro-Breite */
		VAR_LFB_QUADBREITE1	= VAR_LFB_PANPOS1 + 4,
		/* LFA Quadro-Pos */
		VAR_LFB_QUADPOS1		= VAR_LFB_QUADBREITE1 + 2,
		/* allg. Record an/aus (z. B. Record per Tastatur angeschaltet */
		VAR_RECORD				= VAR_LFB_QUADPOS1 + 2,
		/* Sync extern on/off */
		VAR_SYNC_IN,
		VAR_SYNC_IN_FRAMES,
		VAR_SYNC_IN_PORTS,
		VAR_SYNC_IN_LOCKED,
		/* Sync send on/off */
		VAR_SYNC_OUT,
		VAR_SYNC_OUT_FRAMES,
		VAR_SYNC_OUT_PORTS,
		/* Cycle-Mode on/off */
		VAR_CYCLE,
		VAR_CYCLE_START,
		VAR_CYCLE_STOP,
		/* akt. SMPTE Zeit */
		VAR_SMPTE,
		/* Punch in, Aufnahmebeginn bei linkem Locator*/
		VAR_PUF_PUNCH_IN,
		/* Punch out, Aufnahmeende bei rechtem Locator*/
		VAR_PUF_PUNCH_OUT,
		/* PUF Ausgabe an/aus */
		VAR_PUF_PLAY,
		/* PUF Pause an/aus */
		VAR_PUF_PAUSE,
		/* PUF SMPTE Startzeit */
		VAR_PUF_START,
		/* PUF SMPTE Stopzeit */
		VAR_PUF_STOP,
		/* PUF Offset gegen SMPTE */
		VAR_PUF_OFFSET,
		/* PUF Zeitlupe an/aus */
		VAR_PUF_ZEITL,
		/* PUF Record an/aus fÅr einzelne Signale */
		VAR_PUF_REC_SIG0,
		/* PUF Ausgabe an/aus fÅr einzelne Signale */
		VAR_PUF_PLAY_SIG0 = VAR_PUF_REC_SIG0 + MAXSIGNALS,
		VAR_PUF_ZEITLUPE = VAR_PUF_PLAY_SIG0 + MAXSIGNALS,
		VAR_PUF_KOORX0,
		VAR_PUF_KOORY0 = VAR_PUF_KOORX0 + MAXSIGNALS,
		VAR_PUF_KOORZ0 = VAR_PUF_KOORY0 + MAXSIGNALS,
		VAR_PUF_VOL0 = VAR_PUF_KOORZ0 + MAXSIGNALS,
		/* BIG Ausgabe an/aus */
		VAR_BIG_PLAY = VAR_PUF_VOL0 + MAXSIGNALS,
		VAR_MAA_SPERRE_INNEN,
		VAR_MAA_SPERRE_AUSSEN,
		VAR_MAE_SPERRE_INNEN,
		VAR_MAE_SPERRE_AUSSEN,
		VAR_POW_SPERRE_INNEN,
		VAR_POW_SPERRE_AUSSEN,
		/* Ausgabe-Kanal fÅr die beiden CMI-Systeme */
		VAR_CMI_CHANNEL1,
		VAR_CMI_CHANNEL2,
		VAR_CM_MASTER1,
		VAR_CM_MASTER2,
		/* Ausgabe-Port fÅr die beiden CMI-Systeme */
		VAR_CMI_PORT1,
		VAR_CMI_PORT2,
		/* CMI Signal-Zuweisung */
		VAR_CMI_SIGNAL1,
		/* Zoom-Proportionalwert */
		VAR_ZOOM_PROP = VAR_CMI_SIGNAL1 + MAXSIGNALS -1,
		VAR_REL_MTRLFO,
		/* Transportleiste */
		VAR_TRA_PLAY,
		VAR_TRA_STOP,
		VAR_TRA_FFWD,
		VAR_TRA_REW,
		VAR_TRA_CLICK,
		/* 4D-Editor */
		VAR_ED4_PRESEL_INPUT,
		VAR_ED4_PRESEL_POSX,
		VAR_ED4_PRESEL_POSY,
		VAR_ED4_PRESEL_VOL,
		VAR_ED4_PRESEL_OBJSPD,
		VAR_ED4_SPEED,
		/* aktuelle Setups */
		VAR_SET_A3D,
		VAR_SET_BIG,
		VAR_SET_CMI,
		VAR_SET_CMO, 
		VAR_SET_CON, 
		VAR_SET_EC4, 
		VAR_SET_ED4, 
		VAR_SET_EFF, 
		VAR_SET_FAD, 
		VAR_SET_GEN, 
		VAR_SET_GEP, 
		VAR_SET_INI, 
		VAR_SET_INO, 
		VAR_SET_INT, 
		VAR_SET_KOO, 
		VAR_SET_LFA, 
		VAR_SET_LFB, 
		VAR_SET_MAA, 
		VAR_SET_MAE, 
		VAR_SET_MAN, 
		VAR_SET_MIN, 
		VAR_SET_MIO, 
		VAR_SET_MTR, 
		VAR_SET_OTU, 
		VAR_SET_PAR, 
		VAR_SET_PLE,
		VAR_SET_POW, 
		VAR_SET_PUF, 
		VAR_SET_ROT, 
		VAR_SET_SPD, 
		VAR_SET_SPG, 
		VAR_SET_SPO, 
		VAR_SET_SPS, 
		VAR_SET_SRR, 
		VAR_SET_STE, 
		VAR_SET_STR, 
		VAR_SET_SYN, 
		VAR_SET_TRA, 
		VAR_SET_VAR, 
		VAR_SET_VER
		};

/****** TYPES ****************************************************************/

/* Message-Kommunikation */

typedef struct{
	UWORD		variable;	/* FÅr diese Variable ist der Wert bestimmt */
	LONG		value;		/* Das ist der Wert */
}	MSG_SET_VAR;			/* Systemvariable auf neuen Wert setzen */

typedef struct{
	UWORD		variable;	/* Nach dieser Variablen wird gefragt */
	LONG		value;		/* Hierin wird der Wert zurÅckgeliefert */
}	MSG_GET_VAR;			/* Wert einer Systemvariablen abfragen */

typedef struct{
	UWORD		variable;	/* FÅr diese Variable ist der Name bestimmt */
	STRING	name;			/* Dies ist der Name */
}	MSG_SET_VARNAME;		/* Name einer Systemvariablen geben */

typedef struct{
	UWORD		variable;	/* Nach dieser Variablen wird gefragt */
	STRING	name;			/* Hierin wird der Name zurÅckgeliefert */
}	MSG_GET_VARNAME;		/* Name einer Systemvariablen holen */

typedef struct{
	UWORD			variable;	/* Auf diese Variable soll reagiert werden */
	struct rtmclass	*module;	/* Dieses Modul soll immer angesprungen werden */
}	MSG_ADD_VAR_RCV;		/* Modul in VAR-Updateliste einfÅgen */

typedef struct{
	UWORD		variable;	/* Auf diese Variable soll nicht mehr reagiert werden */
	struct rtmclass	*module;	/* Dieses Modul soll immer angesprungen werden */
}	MSG_DEL_VAR_RCV;		/* Modul aus VAR-Updateliste herausnehmen */

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

GLOBAL VOID		update_var		(struct rtmclass* module, UWORD variable, LONG value, BOOL resend);
GLOBAL CHAR		*var_get_name	(struct rtmclass* module, UWORD variable, STRING s);
GLOBAL LONG		var_get_value	(struct rtmclass* module, UWORD variable);
GLOBAL FLOAT	var_get_relvalue	(struct rtmclass* module, UWORD variable);
GLOBAL LONG		var_get_min		(struct rtmclass* module, UWORD variable);
GLOBAL VOID		var_set_min		(struct rtmclass* module, UWORD variable, LONG min);
GLOBAL LONG		var_get_max		(struct rtmclass* module, UWORD variable);
GLOBAL VOID		var_set_max		(struct rtmclass* module, UWORD variable, LONG max);
GLOBAL LONG		var_get_default(struct rtmclass* module, UWORD variable);
GLOBAL VOID		var_set_default(struct rtmclass* module, UWORD variable, LONG def);
GLOBAL WORD		var_get_type	(struct rtmclass* module, UWORD variable);
GLOBAL VOID		var_set_type	(struct rtmclass* module, UWORD variable, WORD type);
GLOBAL VOID		var_add_rcv		(struct rtmclass* module, UWORD variable, struct rtmclass *refmodule);
GLOBAL VOID		var_del_rcv		(struct rtmclass* module, UWORD variable, struct rtmclass *refmodule);
GLOBAL BOOLEAN	init_var  		(VOID);

#endif /* __VAR__ */

