/*****************************************************************************/
/*                                                                           */
/* Modul: WINCLASS.H                                                         */
/* Datum: 03.01.95                                                           */
/*                                                                           */
/* Definitionen der Objekt-Nummern                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- COI, COO und ROT, SPD eingebaut, 03.01.95
- EC4 eingebaut, 29.11.94
- ED4 eingebaut, 29.11.94
*****************************************************************************/

#ifndef __WINCLASS__
#define __WINCLASS__

/****** DEFINES **************************************************************/

/* Bezeichner fÅr die RTM-Fensterklassen */
enum windowclass
{	
	CLASS_A3D = 219,
	CLASS_BIG,
	CLASS_CMI,
	CLASS_CMO,
	CLASS_COI,
	CLASS_COO,
	CLASS_ED4,
	CLASS_EC4,
	CLASS_EFF,
	CLASS_GEN,
	CLASS_GMI,
	CLASS_IMI,
	CLASS_IMO,
	CLASS_KOO,
	CLASS_LFO,
	CLASS_MAA,
	CLASS_MAE,
	CLASS_MAN,
	CLASS_MSH,
	CLASS_MTR,
	CLASS_SPD,
	CLASS_SPG,
	CLASS_SPO,
	CLASS_SPS,
	CLASS_PAR,
	CLASS_POW,
	CLASS_PUF,
	CLASS_ROT,
	CLASS_SYN,
	CLASS_TRA,
	CLASS_VAR,
	NUM_CLASSES
};
		
/* Angabe, was fÅr eine Art Modul es ist */
enum MODULE_TYPE	
{
	MODULE_CALC,
	MODULE_INPUT,
	MODULE_CONTROL,
	MODULE_OUTPUT,
	MODULE_OTHER,
	NUM_MODULE_TYPES
};


#endif /* __WINCLASS__ */
