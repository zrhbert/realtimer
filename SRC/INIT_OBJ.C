#include "import.h"
#include "global.h"
#include "windows.h"
#include "desktop.h"
#include "dialog.h"
#include "initerm.h"						/* wegen alert_msgs */
#include "errors.h"

#include "realtim4.h"
#include "realtspc.h"
#include <msh_unit.h>					/* Deklarationen fÅr MidiShare */

#include "var.h"

#include "a3d.h"
#include "big.h"
#include "cmi.h"
#include "cmo.h"
#include "ed4.h"
#include "eff.h"
#include "gen.h"
#include "gmi.h"
#include "koo.h"
#include "lfo.h"
#include "maa.h"
#include "mae.h"
#include "man.h"
#include "msh.h"
#include "mtr.h"
#include "par.h"
#include "pow.h"
#include "puf.h"
#include "spg.h"
#include "spo.h"
#include "sps.h"
#include "syn.h"
#include "tra.h"

#include "export.h"
#include "init_obj.h"

/*****************************************************************************/
/* Initialisierung fÅr alle Module                                           */
/*****************************************************************************/

GLOBAL BOOLEAN init_modules ()

{	
	WORD i;
	BOOLEAN ok = TRUE;
	STR128 s;
	RTMCLASSP module;

	strcpy (setup_path,		"SETUPS\\");
	strcpy (import_path, 	"IMPORT\\");
	strcpy (export_path, 	"EXPORT\\");
	strcpy (sysdata_path,	"SYSDATA\\");
	strcpy (midishare_path,	"MIDSHARE\\");
	strcpy (info_path,		"INFOS\\");

	/* Pfade Initialisieren */
	strcpy (s, app_path);
	strcat (s, setup_path);
	strcpy (setup_path, s);

	strcpy (s, app_path);
	strcat (s, import_path);
	strcpy (import_path, s);

	strcpy (s, app_path);
	strcat (s, export_path);
	strcpy (export_path, s);

	strcpy (s, app_path);
	strcat (s, sysdata_path);
	strcpy (sysdata_path, s);

	strcpy (s, app_path);
	strcat (s, midishare_path);
	strcpy (midishare_path, s);

	strcpy (s, app_path);
	strcat (s, info_path);
	strcpy (info_path, s);
	
	rtmmodules = NULL;                     /* Kein Modulkeller */
	rtmmrec    = NULL;                     /* Keine Module */
	
	max_rtmmodules = 50;

	rtmmodules = (RTMCLASSP *)mem_alloc ((LONG)max_rtmmodules * sizeof (RTMCLASSP));
	mem_lset(rtmmodules, 0, (max_rtmmodules * sizeof (RTMCLASSP)));

	rtmmrec = (RTMCLASSP)mem_alloc ((LONG)max_rtmmodules * sizeof (RTMCLASS));
	mem_lset(rtmmrec, 0, (max_rtmmodules * sizeof (RTMCLASS)));


	if(init_msh) ok &= init_msh ();		/* Initialisiere MSH */

	/* VAR als erstes Modul Initialisieren, wg. msg. */
	if(init_var) ok &= init_var ();		/* Initialisiere var */
	if(init_a3d) ok &= init_a3d ();		/* Initialisiere 3D-Anzeige */
	if(init_cmi) ok &= init_cmi ();		/* Initialisiere cmi */
	if(init_eff) ok &= init_eff ();		/* Initialisiere eff */
	if(init_gen) ok &= init_gen ();		/* Initialisiere gen */
	if(init_gmi) ok &= init_gmi ();		/* Initialisiere gen */
	if(init_koo) ok &= init_koo ();		/* Initialisiere koo */
	if(init_maa) ok &= init_maa ();		/* Initialisiere maa */
	if(init_mae) ok &= init_mae ();		/* Initialisiere mae */
	if(init_mtr) ok &= init_mtr ();		/* Initialisiere mtr */
	if(init_par) ok &= init_par ();		/* Initialisiere par */
	if(init_pow) ok &= init_pow ();		/* Initialisiere pow */
	if(init_spg) ok &= init_spg ();		/* Initialisiere spg */
	if(init_spo) ok &= init_spo ();		/* Initialisiere spo */
	if(init_sps) ok &= init_sps ();		/* Initialisiere sps */
/* LFO muss im MAN-Standard-Setup hinten an liegen */
	if(init_lfo) ok &= init_lfo ();		/* Initialisiere lfo */
	if(init_syn) ok &= init_syn ();		/* Initialisiere syn */

/* MAN als letztes initialisieren, braucht Obj-Infos der anderen Module */
	if(init_man) ok &= init_man ();		/* Initialisiere man */

/* Ganz zum Schluû die MidiShare-Applikationen */
	if(init_tra) ok &= init_tra ();		/* Initialisiere tra */
	if(init_puf) ok &= init_puf ();		/* Initialisiere puf */
	if(init_big) ok &= init_big ();		/* Initialisiere big */
	if(init_cmo) ok &= init_cmo ();		/* Initialisiere cmo */
	if(init_ed4) ok &= init_ed4 ();		/* Initialisiere 4D-Editor */
	if(init_ec4) ok &= init_ec4 ();		/* Initialisiere 4D-Cue-List */

	for (i = 0; i < max_rtmmodules; i++)         	/* Untersuche alle Module */
	{
		module = rtmmodules [i];
		if (module)								/* Modul vorhanden? */
		{
			copy_icon (&desktop[module->icon_position], module->icon);
		} /* if */
	} /* for */
	
	return (ok);
	
} /* init_modules */
