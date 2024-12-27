/*****************************************************************************/
/*                                                                           */
/* Modul: VAR.C                                                              */
/*                                                                           */
/* VAR Systemvariablen                                                       */
/*****************************************************************************/
#define VARVERSION "V 1.06"
#define VARDATE "09.02.95"

/*****************************************************************************
02.12.2024
- zusÑtzliche daktstatus eingebaut in init
09.02.95
- kopieren den VAR info beim zuweisen von var->var_number in send_messages, 09.02.95
- update von Set-Vars in update_var nur bei VerÑnderungen, 05.02.95
- update_var bei receive_evts_var, 05.02.95
- var_get_relvalue eingebaut, 03.02.95
- énderungen in VAR initialisierung, 03.02.95
- relative Werte in Variablen eingebaut, 02.02.95
- Controller-Midi-In Addressierung um -1 verschoben, 02.02.95
- ClickSetupField eingebaut, 30.01.95
- quadpan eingebaut, 09.01.95
- VAR_CM_MASTER1/2 eingebaut
- Bug in wi_click inc/dec Var-Quelle beseitigt
- create_var in create umbenannt
V 1.06 19.05.94
- load_create_infos und instance_count eingebaut
- update_variable auf statische Speicher fÅr message umgebaut, wg. Interrupt
- var_get und var_set Makros eingebaut
- send_variable per update_var
V 1.05
- MS-Namen auf TRA geÑndert
- list-Funktionen ausgegliedert
V 1.04 16.08.93
- init_standard eingebaut
- Fehler in verwaltung der VAR-Variablen beseitigt (get_dbox, set_dbox, init_standard)
- DBox auf vierzeilig 10 Zeichen umgebaut
- var_get_value und var_get_name eingebaut
- msh_available eingebaut
- MAXSYSVARS und MAXSETVARS nun global
- window->module eingebaut
- try_all_connect eingebaut
- VAR_RECORD nun direkt in receive_evts_var
- destroy_mod eingebaut
- Umbau auf create_window_obj
V 1.03 20.06.93
- Abmelden alter MS-Applikationen
- wi_click_mod eingebunden
V 1.02
- send-variable per MidiShare, wenn angemeldet
- PlausibilitÑtsprÅfung fÅr Werte in send_message

V 1.01 23.05.93
- Umstellung auf neue RTMCLASS-Struktur
- Bei update wird jetzt DATA-Zeiger mitgeschickt, 
	als erster Parameter fÅr message()
V 1.00 17.04.93
- VAR_PUF_PLAY und VAR_BIG_PLAY eingebaut
*****************************************************************************/

#ifndef XRSC_CREATE
/* #define XRSC_CREATE TRUE                    /* X-Resource-File im Code */ */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"
#include "time.h"
#include "lists.h"

#include "realtim4.h"
#include "var_mod.h"
#include "realtspc.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"
#include "msh_unit.h"
#include "msh.h"

#include "export.h"
#include "var.h"

#if XRSC_CREATE
#include "var_mod.rsh"
#include "var_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 2 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangshîhe in Pixel */
#define MILLI  0                        /* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "VAR_MOD.RSC"		/* Name der Resource-Datei */

#define MAXSETUPS			201L			/* Anzahl der VAR-Setups */
#define MAXITERATIONS   4				/* Max. Tiefe der Update-Rekursion */
#define NO_REFNUM		-1

#define Timestamp() msh_available ? MidiGetTime() : clock ()
/****** TYPES ****************************************************************/
typedef struct setup
{
	UWORD	quelle [MAXSETVARS];			/* Nummern der Variablen die verwendet werden */
} SETUP;

typedef struct setup *SET_P;

typedef struct sysvar
{
	LONG		value;			/* akt. Wert der Variablen */
	LONG		min;				/* minimaler Wert der Variablen */
	LONG		max;				/* maximaler Wert der Variablen */
	FLOAT		rel_value;		/* akt. Wert der Variablen in Prozent (-100 .. +100)*/
	LONG		standard;		/* Default-Wert der Variablen */
	WORD		type;				/* Typ: Grad, Prozent, Zeit, Faktor */
	CHAR		name [16];		/* Nur 10 benutzen wg. LFO, Name dieser Variablen */
	UWORD		varnumber;		/* evtl. Verweis auf andere Variable */
	LIST_P	header;			/* Zeiger auf Liste fÅr message-Routinen */
	BOOLEAN	new;				/* Wert hat sich seit letztem Update verÑndert */
	clock_t	modified,		/* Letzte Modifikation */
				updated;			/* Letztes Update im System */
} SYSVAR;

typedef struct sysvar *SYS_P;

typedef struct status
{
	SYSVAR	sysvar [MAXSYSVARS];			/* Nummern der Variablen die verwendet werden */
	TFilter	filter;							/* Midi-In-Filter	*/
} STATUS;

typedef struct status *STAT_P;


/****** VARIABLES ************************************************************/
PRIVATE WORD	var_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*var_rsc_ptr = &var_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *var_setup;
PRIVATE OBJECT *var_help;
PRIVATE OBJECT *var_desk;
PRIVATE OBJECT *var_text;
PRIVATE OBJECT *var_info;

PRIVATE WORD		iterations;	/* Anzahl Iterationen fÅr VAR-Update */
PRIVATE WORD		instance_count = 0;			/* Anzahl der RTM Instanzen */
PRIVATE CONST WORD max_instances = 1;			/* Max Anzahl Instanzen */
PRIVATE CONST STRING module_name = "VAR";		/* Name, fÅr Extension etc. */

PRIVATE RTMCLASSP	modulep[MAXMSAPPLS];		/* Zeiger auf Modul-Strukturen */
PRIVATE WORD		refNums[1];					/* Referenznummern */

PRIVATE BOOL		var_watch = 0;				/* VAR ÅberprÅfen auf min/max etc. */
PRIVATE BOOL		var_msgs = 0;				/* Anzahl der Message-Routinen */
/****** FUNCTIONS ************************************************************/
/* MidiShare Funktionen */
PUBLIC VOID			cdecl	receive_evts_var	_((SHORT refNum));
PUBLIC VOID			cdecl play_task_var		_((LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3));
PRIVATE VOID		InstallFilter				_((SHORT refNum));

/* Interne VAR-Funktionen */
PRIVATE VOID	send_var	(RTMCLASSP module, UWORD variable, RTMCLASSP destmodule);
PRIVATE BOOL	init_rsc		_((VOID));
PRIVATE BOOL	term_rsc		_((VOID));

PRIVATE VOID	add_link			_((RTMCLASSP module, UWORD variable, KEYTYPE key));
PRIVATE VOID	del_link			_((RTMCLASSP module, UWORD variable, KEYTYPE key));

PRIVATE WORD	doffset			_((UWORD var));
PRIVATE VOID	init_variables	_((RTMCLASSP module));
PRIVATE VOID	init_standard	_((RTMCLASSP module));
PRIVATE SHORT	init_midishare 	_((VOID));
PRIVATE FLOAT SetValueVar (SYS_P var, LONG value);
PRIVATE VOID DefineVar (RTMCLASSP module, LONG var, CONST CHAR* text, WORD index, LONG minimum, LONG maximum, LONG def, WORD t);

/*****************************************************************************/
PUBLIC VOID cdecl receive_evts_var (int refNum)
{
	MidiEvPtr	event;
	LONG 			n;
	INT 			r;
	MidiEvPtr	myTask;
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	WINDOWP		window = module->window;
	SYS_P			sysvar = module->status->sysvar, var;
	UWORD			ctrl;
	UWORD			variable;
	LONG			value;

	r = refNum;
	for (n = MidiCountEvs(r); n > 0; --n) 	/* Alle empfangenen Events abarbeiten */
	{
		event = MidiGetEv (r);				/*  Information holen */
		switch (EvType(event))
		{
			case typeRTMSetVar:
				variable 					= (UWORD) get_var_number((MidiSTPtr)event);
				value							= get_var_value((MidiSTPtr)event);
				update_var (module, variable, value, FALSE);
				break;
			case typeRTMRecordOnOff:
				value								= get_record_on((MidiSTPtr)event);
				update_var (module, VAR_RECORD, value, FALSE);
				break;
			case typeCtrlChange:
				ctrl	= (UWORD)GetContID (event);
				value	= GetContValue (event);
				update_var (module, VAR_CONT0 + ctrl, value, FALSE);
		} /* switch */
		MidiFreeEv (event);
	} /* for */
} /* receive_evts_var */

PRIVATE VOID InstallFilter (SHORT refNum)
{
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	TFilter		*filter = &status->filter;

	register int i;

	for (i = 0; i<256; i++)
	{ 										
		AcceptBit(filter->evType,i);		/* accepte tous les types d'ÇvÇnements	*/
		AcceptBit(filter->port,i);		/* en provenance de tous les ports		*/
	} /* for */
											
	for (i = 0; i<16; i++)
		AcceptBit(filter->channel,i);	/* et sur tous les canaux Midi		*/
		
	MidiSetFilter( refNum, filter );   /* installe le filtre				*/
} /* InstallFilter */

/*****************************************************************************/

/*	Alternativen, die zu langsam bei hohen Datenmengen sind:

GLOBAL VOID send_variable	(UWORD variable, LONG value)
{
	/* Sendet eine Nachricht an das VAR-Modul, um eine
		Variable auf einen neuen Wert zu setzen.
		Dies ist die Standard-Methode um System-Variablen zu Ñndern */
		
	MidiSTPtr 	e;
	SHORT			refNum = refNums[1];
	MSG_SET_VAR	*msg;
	
	if (refNum)
	{
		/* Wenn MidiShare angemeldet */
		e = (MidiSTPtr) MidiNewEv(typeRTMSetVar);
		if (e)
		{
			set_var_number(e, variable);
			set_var_value(e, value);
			if (refNum > 0) MidiSendIm(refNum, e);
		} /* if */
	} /* if */
	else
	{
		/* MidiShare nicht angemeldet, senden per Subroutine-Call */
		/* Message Passing vorbereiten */
		msg = (MSG_SET_VAR *)mem_alloc(sizeof(MSG_SET_VAR));
		msg->variable	= variable;
		msg->value		= value;
		message(var_module, SET_VAR, (VOID *)msg);
		
		mem_free(msg);
	} /* else */
} /* send_variable */
*/

GLOBAL CHAR	*var_get_name	(RTMCLASSP module, UWORD variable, STRING s)
{
	/* VAR-Name in string kopieren und Pointer auf den String zurÅckgeben */
	if (variable < MAXSYSVARS)
		strcpy (s, module->status->sysvar[variable].name);
	else
		sprintf (s, "%d", variable);	/* Fehler, falsche Variable */
	return s;
} /* var_get_name */

GLOBAL LONG		var_get_min		(RTMCLASSP module, UWORD variable)
{
	return module->status->sysvar[variable].min;
}

GLOBAL VOID		var_set_min		(RTMCLASSP module, UWORD variable, LONG min)
{
	module->status->sysvar[variable].min = min;
}

GLOBAL LONG		var_get_max		(RTMCLASSP module, UWORD variable)
{
	return module->status->sysvar[variable].max;
}

GLOBAL VOID		var_set_max		(RTMCLASSP module, UWORD variable, LONG max)
{
	module->status->sysvar[variable].max = max;
}

GLOBAL LONG		var_get_default(RTMCLASSP module, UWORD variable)
{
	return module->status->sysvar[variable].standard;
}

GLOBAL VOID		var_set_default(RTMCLASSP module, UWORD variable, LONG def)
{
	module->status->sysvar[variable].standard = def;
}

GLOBAL WORD		var_get_type	(RTMCLASSP module, UWORD variable)
{
	return module->status->sysvar[variable].type;
}

GLOBAL VOID		var_set_type	(RTMCLASSP module, UWORD variable, WORD type)
{
	module->status->sysvar[variable].type = type;
}

GLOBAL LONG	var_get_value	(RTMCLASSP module, UWORD variable)
{
	/* Wert einer Systemvariablen zurÅckgeben, interrupt-tauglich */
	if (variable < MAXSYSVARS)
		return module->status->sysvar[variable].value;
	else
		return -1;
} /* var_get_value */

GLOBAL FLOAT	var_get_relvalue	(RTMCLASSP module, UWORD variable)
{
	/* Wert einer Systemvariablen zurÅckgeben, interrupt-tauglich */
	if (variable < MAXSYSVARS)
		return module->status->sysvar[variable].rel_value;
	else
		return -1;
} /* var_get_relvalue */

/* var_set_value defined as a Makro in VAR.H */

GLOBAL VOID	var_add_rcv		(RTMCLASSP module, UWORD variable, RTMCLASSP refmodule)
{
	/* Modulfunktion in VAR-Updateliste einfÅgen */
	/* Liste erweitern */
	add_link (module, variable, refmodule);
	
	/* Aktuellen Wert an Modul senden */
	send_var (module, variable, refmodule);

	/* Debug: ZÑhlen der Messages */
	var_msgs++;
} /* var_add_rcv */

GLOBAL VOID	var_del_rcv		(RTMCLASSP module, UWORD variable, RTMCLASSP refmodule)
{
	/* Modulfunktion aus VAR-Updateliste herausnehmen */
	del_link (module, variable, refmodule);

	/* Debug: ZÑhlen der Messages */
	var_msgs--;
} /* var_del_rcv */

/* jetzt als Makro in var.h 
GLOBAL VOID	add_rcv		(UWORD variable, RTMCLASSP module)
{
	/* Message Passing einklinken */
	MSG_ADD_VAR_RCV *msg;
	
	msg 				= (MSG_ADD_VAR_RCV *)mem_alloc(sizeof(MSG_ADD_VAR_RCV));
	msg->variable 	= variable;
	msg->module		= module;
	
	message(var_module, ADD_VAR_RCV, (VOID *)msg);
	
	mem_free(msg);
} /* add_rcv */
*/
/*****************************************************************************/

PRIVATE VOID	add_link		(RTMCLASSP module, UWORD variable, KEYTYPE key)
{
	LIST_P	element, header;

	header = module->status->sysvar[variable].header;

	element = list_search (header, key);
	if (element == header)
	{
		/* Nur einfÅvar, wenn noch nicht drin */
		element = list_create();
		element->key		= key;
		list_insert (header, element);	
	} /* if */
	
} /* add_link */

PRIVATE VOID	del_link		(RTMCLASSP module, UWORD variable, KEYTYPE key)
{
	LIST_P	element, header;
	
	header	= module->status->sysvar[variable].header;
	element	= list_search (header, key);
	list_delete (element);
	
	mem_free(element);
} /* del_link */

PRIVATE FLOAT SetValueVar (SYS_P var, LONG value)
{
	/* Setze den Wert der Variablen */
	var->value = value;
	/* Berechne relativen Wert */
	if (var->max - var->min != 0)
		var->rel_value = 100 * (var->value - var->min) / (var->max - var->min);
	else
		var->rel_value = 0;
	
	/* Zeit merken */
	var->updated	= Timestamp();
	
	return var->rel_value;
} /* SetValueVar */

GLOBAL VOID	update_var		(RTMCLASSP module, UWORD variable, LONG value, BOOL resend)
{
	LIST_P	element, header;
	SYS_P		var = &module->status->sysvar[variable];
	clock_t	modified, updated;
	MSG_SET_VAR	message, *msg = &message;
	RTMCLASSP	mod;

	iterations = 0;
	/* Sicherheitsabfrage */
	if (variable < MAXSYSVARS && value >= var->min && value <= var->max)
	{
		modified = var->modified;
		updated	= var->updated;
		/* Nur Update durchfÅhren, wenn Wert neuer als letztes Update ist */

		if (modified > updated || var->value != value || resend)
		{
			/* Message-Inhalt aufbauen */
			msg->variable	= variable;
			msg->value 		= value;
			
			/* neuen Wert merken */
			SetValueVar (var, value);

			header = var->header;
			
			element = header->next;
			/* Liste durcharbeiten */
			while (element != header)
			{
				mod = (RTMCLASSP)element->key;
				if (mod)
				{
					if (mod->message)
					{
						/* message-Routine anspringen */
						mod->message (mod, SET_VAR, msg);
					} /* if message */
				} /* if mod */
				
				/* PrÅfen, ob sich Wert verÑndert hat, z.B. durch minmaxsetup */
#if TRUE
				if (var->value != value)
					return;	/* Sofort beenden, weil rekursiv aufgerufen und schon geÑndert! */
				else
					element = element->next;	/* NÑchstes Element */
#else
				if (var->value != value && iterations < MAXITERATIONS )
				{
					/* Neuen Wert Åbernehmen */
					SetValueVar (var, value);
					msg->value = value;
					/* Liste noch einmal durcharbeiten */
					element = header->next;
					iterations++;
				} /* if modified */
				else
					element = element->next;	/* NÑchstes Element */
#endif
			} /* while */
			
			/* Evtl. interne Systemvariable anspringen (oder andere VAR) */
			if (var->varnumber>0 && var->varnumber != variable)
			{
				update_var(module, var->varnumber, value, resend);
			}
		} /* if modified */
		else
			var->updated	= Timestamp();
	} /* if gÅltig */
	else if (var_watch)
		hndl_alert_obj (module, ERR_NOMEMORY);	/* Dummy Fehlermeldung */
} /* update_var */

PRIVATE VOID send_var	(RTMCLASSP module, UWORD variable, RTMCLASSP destmodule)
{
	/* Wert einer Variablen an ein ANDERES Modul schicken */
	
	MSG_SET_VAR		message, *msg = &message;
	
	if (module != destmodule)	/* Nicht im eigenen Modul schicken, da unnîtig */
	{
	/* Message Passing vorbereiten */
		msg->variable	= variable;
		msg->value		= module->status->sysvar[variable].value;
	
		/* message-Routine anspringen */
		destmodule->message (destmodule, SET_VAR, msg);
	} /* if */
} /* send_var */

/*****************************************************************************/
PRIVATE VOID    get_dbox	(RTMCLASSP module)
{
	SET_P		ed = module->edited->setup;
	STRING 	s;
	UWORD		variable, offset;
	
	for (variable = 0; variable < MAXSETVARS/2; variable++)
	{
		offset = doffset(variable);
		get_ptext (var_setup, VARQUELLE1 + offset, s);
		sscanf (s, "%d", &ed->quelle[VAR_VAR0 + variable]);
	} /* for */
} /* get_dbox */

PRIVATE VOID    set_dbox	(RTMCLASSP module)
{
	SET_P		ed = module->edited->setup;
	STRING	s;
	UWORD		variable, offset, quelle;
	SYS_P		sysvar = module->status->sysvar;
	
	for (variable = 0; variable < MAXSETVARS; variable++)
	{
		offset = doffset(variable);
		sprintf (s, "%d", variable + 1);
		set_ptext (var_setup, VARNUMMER1 + offset, s);
		quelle = ed->quelle[VAR_VAR0 + variable];	/* Die Quelle dieser VAR */
		sprintf (s, "%d", quelle);
		set_ptext (var_setup, VARQUELLE1 + offset, s);
		set_ptext (var_setup, VARQUELLNAME1 + offset, sysvar[quelle].name);
	} /* for */
	
	if (module->edited->modified)
		sprintf (s, "%ld*", module->edited->number);
	else
		sprintf (s, "%ld", module->edited->number);
	set_ptext (var_setup, VARSETNR , s);
} /* set_dbox */

PRIVATE WORD doffset(UWORD var)
{
	/* Display Offset fÅr eine Anzeige mit vier Reihen berechnen */
	switch (var*4/MAXSETVARS)
	{
		case 0:
			return (VARNUMMER2-VARNUMMER1)*(var % (MAXSETVARS/4));
		case 1: 
			return (VARNUMMER2-VARNUMMER1)*(var % (MAXSETVARS/4)) + VARNUMMER11 - VARNUMMER1;
		case 2:
			return (VARNUMMER2-VARNUMMER1)*(var % (MAXSETVARS/4)) + VARNUMMER21 - VARNUMMER1;
		case 3:
			return (VARNUMMER2-VARNUMMER1)*(var % (MAXSETVARS/4)) + VARNUMMER31 - VARNUMMER1;
	} /* break */
	return 0;
} /* doffset */

PRIVATE VOID    send_messages	(RTMCLASSP module)
{
	SET_P		akt = module->actual->setup;
	SET_P		ed = module->edited->setup;
	SYS_P		sysvar = module->status->sysvar;
	UWORD		variable, alt, neu;

	/* Neue Zuweisungen initialisieren */
	/* Alle Variablen im Setup durchgehen */
	for (variable = VAR_VAR0; variable < MAXSETVARS + VAR_VAR0; variable++)
	{
		alt = akt->quelle[variable];	/* Alte Zuweisung */
		neu = ed->quelle[variable];	/* Neue Zuweisung */
		
		/* Schauen, ob sich die Zuweisung zu den sysvars geÑndert hat */
		if (neu < MAXSYSVARS)
		{
			/* Alte Nummer zulÑssig? */
			if (alt < MAXSYSVARS)
			{
				/* Ist noch die alte Nummer eingetragen, 
					d. h. noch nicht von einer anderen Var. Åberschrieben? */
				if(sysvar[alt].varnumber == variable)
					sysvar[alt].varnumber = 0;		/* Alte Nummer lîschen */
			} /* if */
			/* Neue Nummer eintragen */
			sysvar[neu].varnumber 	= variable;

			/* System-Variable mit Parametern der Quelle ausstatten */
			sysvar[variable].value			= sysvar[neu].value;
			sysvar[variable].rel_value		= sysvar[neu].rel_value;
			sysvar[variable].min				= sysvar[neu].min;
			sysvar[variable].max				= sysvar[neu].max;
			sysvar[variable].standard		= sysvar[neu].standard;
			sysvar[variable].type			= sysvar[neu].type;
			sysvar[variable].modified		= sysvar[neu].modified;
			sysvar[variable].varnumber		= 0;

			/* Wert Åbernehmen, aktualisieren und weitergeben */
			update_var(module, variable, sysvar[neu].value, TRUE);
		} /* if */
	} /* for */
	/* VAR-Setup-Nummer updaten */
	send_variable(VAR_SET_VAR, module->actual->number);
} /* send_messages */

/*
PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	SYS_P			sysvar = module->status->sysvar;
	UINT			variable;
	RTMCLASSP	ref_module;
	
	switch(type)
	{
		case SET_VAR:
			/* Systemvariable auf neuen Wert setzen */
			
			/* Wert verÑndern und Msg. senden */
			update_var (module, ((MSG_SET_VAR *)msg)->variable,
				((MSG_SET_VAR *)msg)->value, FALSE);
			
			break;
		case GET_VAR:
			/* Wert einer Systemvariablen abfragen */
			
			/* Der Wert wird in der durch den value-Zeiger angegebenen
				Adresse zurÅckgeliefert */
			((MSG_GET_VAR *)msg)->value =
				sysvar[((MSG_GET_VAR *)msg)->variable].value;
			
			break;
		case GET_VARNAME:			
			/* Name einer Systemvariablen holen */
			
			/* Der Name wird in der durch den name-Zeiger angegebenen
				Adresse zurÅckgeliefert */
			strcpy(((MSG_GET_VARNAME *)msg)->name, sysvar[((MSG_GET_VARNAME *)msg)->variable].name);
			
			break;
		case SET_VARNAME:
			/* Name einer Systemvariablen setzen */
			
			/* Den String aus msg->name als neuen Namen Åbernehmen */
			strcpy(sysvar[((MSG_SET_VARNAME *)msg)->variable].name, (((MSG_SET_VARNAME *)msg)->name));
			
			break;
		case ADD_VAR_RCV:
			/* Modulfunktion in VAR-Updateliste einfÅgen */
			
			ref_module 	= ((MSG_ADD_VAR_RCV *)msg)->module;
			variable		= ((MSG_ADD_VAR_RCV *)msg)->variable;

			/* Liste erweitern */
			add_link (module, variable, ref_module);
			
			/* Aktuellen Wert an Modul senden */
			send_var (module, variable, ref_module);
			
			break;
		case DEL_VAR_RCV:
			/* Modulfunktion aus VAR-Updateliste herausnehmen */
			del_link (module, ((MSG_DEL_VAR_RCV *)msg)->variable,
				((MSG_DEL_VAR_RCV *)msg)->module);
			break;
	} /* switch */
} /* message */
*/

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click_mod (window, mk)
WINDOWP window;
MKINFO  *mk;
{
	UWORD			i, item, variable;
	STRING		s;
	UWORD			signal, offset;
	static		LONG x = 0;	
	WORD			event, ret;
	RTMCLASSP	module = Module(window);
	ED_P			edited = module->edited;
	SET_P			ed = edited->setup;
	BOOL			found = FALSE;
	
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
		case VARSETINC:
			module->set_nr (window, edited->number+1);
			break;
		case VARSETDEC:
			module->set_nr (window, edited->number-1);
			break;
		case VARSETNR:
			ClickSetupField (window, window->exit_obj, mk);
			break;
		case VARSETSTORE:
			module->set_store (window, edited->number);
			break;
		case VARSETRECALL:
			module->set_recall (window, edited->number);
			break;
		case VAROK   :
			module->set_ok (window);
			break;
		case VARCANCEL:
			module->set_cancel (window);
		   break;
		case VARHELP :
			module->help(module);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		case VARSTANDARD:
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
				set_ptext (var_setup, VARSETNR, s);
				draw_object(window, VARSETNR);
			} /* if */
			undo_state (window->object, window->exit_obj, SELECTED);
			for (variable = 0; variable < MAXSETVARS && !found; variable++)
			{
				offset = doffset(variable);
				switch (window->exit_obj - offset)
				{
					case VARQUELLE1:
					case VARQUELLNAME1:
						found = TRUE;
						do{
							if (mk->momask == 0x001)		/* linke Taste */
								x = ed->quelle[VAR_VAR0 + variable]-1;
							else if (mk->momask == 0x002)	/* rechte Taste */
								x = ed->quelle[VAR_VAR0 + variable]+1;
							minmaxsetup(&x, MAXSYSVARS);
							ed->quelle[VAR_VAR0 + variable] = (UWORD) x;
							module->set_dbox(module);
							draw_object(window, VARQUELLE1 + offset);
							draw_object(window, VARQUELLNAME1 + offset);
							/* Maus noch gedrÅckt? */
							/* if(mk->mobutton>0 && mk->momask>0) */
								graf_mkstate(&ret, &ret, &mk->mobutton, &ret);
						} while (mk->mobutton>0 && mk->momask>0);
						break;
				} /* switch */
			} /* for */	
	} /* switch */
} /* wi_click_mod */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

GLOBAL VOID wi_timer_mod (window)
WINDOWP window;

{
	RTMCLASSP	module = Module(window);
	SYS_P			var;
	UWORD			variable;
	
	window->milli = 0;		/* Kein Update mehr b.a.w. */
	
	var = module->status->sysvar;
	for (variable = 0; variable < MAXSYSVARS; variable++)
	{
		/* Variable wurde geÑndert oder noch nie herausgeschickt */
		if (var->modified > var->updated || var->updated == 0)
			update_var (module, variable, var->value, FALSE);
		var++;
	} /* for */
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

  inx    = num_windows (CLASS_VAR, SRCH_ANY, NULL);
  window = create_window_obj (KIND, CLASS_VAR);

  if (window != NULL)
  {

		WINDOW_INITOBJ_OBJ

    window->flags     = FLAGS | WI_MODELESS;
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
    window->mousenum  = ARROW;
    window->mouseform = NULL;
    window->milli     = MILLI;
    window->module   = 0;
    window->object    = obj;
    window->menu      = menu;
    window->click     = wi_click_mod;
    window->timer     = wi_timer_mod;
    window->showinfo  = info_mod;

    sprintf (window->name, (BYTE *)var_text [FVARN].ob_spec);
    sprintf (window->info, (BYTE *)var_text [FVARI].ob_spec, 0);
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
	RTMCLASSP	module;
	
	window = search_window (CLASS_VAR, SRCH_ANY, VAR_SETUP);
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			window->edit_obj = find_flags (var_setup, ROOT, EDITABLE);
			window->edit_inx = NIL;
			module = Module(window);
			
			module->set_edit (module);
			module->set_dbox (module);
			if (! open_window (window)) hndl_alert (ERR_NOOPEN);
		} /* if */
		else 
			top_window (window);
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
	
	window = search_window (CLASS_DIALOG, SRCH_ANY, IVAR);
		
	if (window == NULL)
	{
		 form_center (var_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (var_info, NULL, IVAR, (BYTE *)var_text [FVARN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = var_info;
		sprintf(s, "%-20s", VARDATE);
		set_ptext (var_info, VARIVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (var_info, VARICOMPILE, s);
		sprintf(s, "%-20s", VARVERSION);
		set_ptext (var_info, VARIVERNR, s);
		sprintf(s, "%-20ld", MAXSETUPS);
		set_ptext (var_info, VARISETUPS, s);
		if (module)
			sprintf(s, "%-20ld", module->actual->number);
		else
			sprintf(s, "(kein Modul selektiert)");
		set_ptext (var_info, VARIAKT, s);

		if (! open_dialog (IVAR)) hndl_alert (ERR_NOOPEN);
	}

	return (window != NULL);
} /* info_mod */

/*****************************************************************************/
PRIVATE RTMCLASSP create ()
{
	RTMCLASSP 	module;
	WORD 			x;
	WINDOWP		window;
	FILE			*fp;
	LIST_P		element;
	SYS_P			sysvar;
	SHORT			refNum;
			
	module = create_module (module_name, instance_count);
	
	if (module != NULL)
	{
		module->class_number		= CLASS_VAR;
		module->icon				= &var_desk[VARICON];
		module->icon_position	= IVAR;
		module->icon_number		= IVAR;	/* Soll bei Init vergeben werden */
		module->menu_title		= MCONTROLS;
		module->menu_position	= MVAR;
		module->menu_item			= MVAR;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;
		module->open				= open_mod;
		module->info				= info_mod;
		module->init				= init_var;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_CONTROL;
		/* module->message			= message; */
		module->create				= create;
		module->destroy			= destroy_mod;

		var_module = module;	/* globaler Zeiger auf VAR-Modul */
	
		module->file_pointer			= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->file_pointer, 0, (UWORD)sizeof(FILE));
		module->import_pointer		= mem_alloc (sizeof (FILE));
		mem_set((VOID*)module->import_pointer, 0, (UWORD)sizeof(FILE));
		module->setup_length			= sizeof(SETUP);
		module->location				= SETUPS_INTERN;
		module->ram_modified 		= FALSE;
		module->max_setups		 		= MAXSETUPS;
		module->standard				= (SET_P)mem_alloc(sizeof(SETUP));
		module->actual->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->edited->setup		= (SET_P)mem_alloc(sizeof(SETUP));
		module->status 				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->stat_alt 				= 0;
		module->import					= 0;
		module->get_dbox				= get_dbox;
		module->set_dbox				= set_dbox;
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

		/* Speicher allozieren fÅr alle Header-Nodes */
		sysvar = module->status->sysvar;
		mem_lset (sysvar, 0, sizeof (SYSVAR) * MAXSYSVARS);
		
		element = (LIST_P) mem_alloc(MAXSYSVARS * sizeof(LIST));
		/* Initialisierung der Listen */
		for (x = 0; x < MAXSYSVARS; x++)
		{
			element->next 		= element;
			element->prev 		= element;
			sysvar[x].header	= element;
			sysvar[x].new 		= FALSE;
			sysvar[x].value	= 0;
			element++;			/* Auf nÑchsten Header-Node zeigen */
		} /* for */

		/* Namen fÅr Variablen initialisieren */
		init_variables (module);
		init_standard(module);
		
		/* Fenster generieren */
		window = crt_mod (var_setup, NULL, VAR_SETUP);
		/* Modul-Struktur einbinden */
		window->module		= (VOID*) module;
		module->window		= window;
		refNum				= init_midishare();
		module->special	= (LONG) refNum;
		modulep[refNum]	= module;
		if (refNum>0)
			InstallFilter(refNum);									/* Midi-Input-Filter einbauen */	

		var_add_rcv(var_module, VAR_SET_VAR, module);	/* Message einklinken */
		var_set_max(var_module, VAR_SET_VAR, MAXSETUPS);
		
		/* Initialisierung auf Standard-Setup */
		module->set_setnr(module, 0);		
	} /* if */
	
	return module;
} /* create */

/*****************************************************************************/
/* Lîsche Objekt                                                            */
/*****************************************************************************/
PUBLIC VOID destroy_mod (module)
RTMCLASSP module;
{
	INT	refNum;
	
	if (msh_available)
	{
		for (refNum = 0; module != modulep[refNum]; refNum++);
	
		if (refNum > 0)
		{
			refNums[instance_count--] = 0;
			MidiClose (refNum);				/* abmelden	*/
		} /* if */
	} /* if */
	
	destroy_obj (module);	/* weiter mit Standard-Routine */
} /* destroy_mod */

/*****************************************************************************/
/* MidiShare initialisieren                                                  */
/*****************************************************************************/

PRIVATE SHORT init_midishare ()
{
	/* Meldet ein neues Modul bei MidiShare an und gibt die refNum zurÅck */
	SHORT		ref, refNum = 0;			/* temporÑre Referenznummer */
	STRING	s;

	if (msh_available)
	{
		if (MidiShare())
		{	
			if (instance_count <= max_instances)
			{
				if (instance_count == 0)
					sprintf (s, "VAR");
				else
					sprintf (s, "VAR %d", instance_count + 1);
				refNum = MidiGetNamedAppl(s); /* Alte Applikation schliessen */
				if (refNum > 0) MidiClose(refNum);
				refNum = MidiOpen(s);				/* Applikation fÅr MidiShare îffnen	*/
			} /* if */
		} /* if */
		
		if (refNum == 0)
			 hndl_alert (ERR_NOMIDISHARE);
		else if (refNum == MIDIerrSpace)			/* PrÅfen genug Platz war */
			 hndl_alert (ERR_MIDISHAREFULL);
		else if (refNum > 0)							/* PrÅfen ob alles klar */
		{
			instance_count++;
			refNums[instance_count] = refNum;				/* Merken fÅr term_mod */
			MidiSetRcvAlarm(refNum, receive_evts_var);	/* Interrupt-Handler */		
			/* An alle anschlieûen */
			try_all_connect (refNum);
		} /* if */
		
	} /* if */
	return refNum;
	
} /* init_midishare */

/*****************************************************************************/

PRIVATE VOID init_standard (RTMCLASSP module)
{
	SET_P			standard;
	WORD			var, x;
	
	/* Initialisierung des Standard-Setups */
	standard = module->standard;
	mem_set(standard, 0, (UWORD) sizeof(SETUP));
	var = VAR_VAR0;
	for (x = 0; x < 32; x++)
	{
		standard->quelle[var++] = VAR_CONT0 + x;		/* 32 Controller */
	} /* for */
		
	standard->quelle[var++] = VAR_PITCH;
	standard->quelle[var++] = VAR_MAEX;
	standard->quelle[var++] = VAR_MAEY;
	standard->quelle[var++] = VAR_MAEZ;
	standard->quelle[var++] = VAR_POWX;
	standard->quelle[var++] = VAR_POWY;
	standard->quelle[var++] = VAR_POWZ;

	for (; x < VAR_VAR0 + MAXSETVARS; x++)
		standard->quelle[var++] = 0;			/* Rest nicht belegt */
} /* init_standard */

PRIVATE VOID DefineVar (RTMCLASSP module, LONG var, CONST CHAR* text, WORD index, LONG minimum, LONG maximum, LONG def, WORD t)
{
	SYS_P		sysvar = module->status->sysvar;
	sprintf(sysvar[var].name, text, index); 
	sysvar[var].min = minimum;
	sysvar[var].max = maximum;
	sysvar[var].standard = def;
	sysvar[var].type = t;
}

#define MINLONG -2147483648L
#define MAXLONG 2147483647L

#define DefinePercentVar(var, text, index)\
	DefineVar(module, var, text, index, -100, 100, 0, VAR_UNIT_PERCENT);

#define DefinePPercentVar(var, text, index)\
	DefineVar(module, var, text, index, 0, 100, 0, VAR_UNIT_PERCENT);

#define DefinePanVar(var, text, index)\
	DefineVar(module, var, text, index, -200, 200, 0, VAR_UNIT_PERCENT);

#define DefineAccVar(var, text, index)\
	DefineVar(module, var, text, index, -1000, 1000, 0, VAR_UNIT_PERCENT);

#define DefinePropVar(var, text, index)\
	DefineVar(module, var, text, index, MINLONG, MAXLONG, 0, VAR_UNIT_PERCENT);

#define DefineMidiVar(var, text, index)\
	DefineVar(module, var, text, index, 0, 127, 0, VAR_UNIT_MIDI);
	
#define DefineHiMidiVar(var, text, index)\
	DefineVar(module, var, text, index, 0, 6192, 0, VAR_UNIT_MIDI);

#define DefineSetupVar(var, text, index)\
	DefineVar(module, var, text, index, 0, MAXLONG, 0, VAR_UNIT_INDEX);

#define DefinePhaseVar(var, text, index)\
	DefineVar(module, var, text, index, MINLONG, MAXLONG, 0, VAR_UNIT_DEGREE);

#define DefineBoolVar(var, text, index)\
	DefineVar(module, var, text, index, 0, 1, 0, VAR_UNIT_BOOL);

#define DefineCMIVar(var, text, index)\
	DefineVar(module, var, text, index, 0, MAXINPUTS*2, 0, VAR_UNIT_INDEX);

#define DefineChanVar(var, text, index)\
	DefineVar(module, var, text, index, 0, 15, 0, VAR_UNIT_INDEX);

#define DefinePortVar(var, text, index)\
	DefineVar(module, var, text, index, 0, 31, 0, VAR_UNIT_INDEX);

#define DefineKoorVar(var, text, index)\
	DefineVar(module, var, text, index, -100, 100, 0, VAR_UNIT_PERCENT);

PRIVATE VOID init_variables (RTMCLASSP module)
{
	SYS_P		sysvar = module->status->sysvar;
	WORD		x;
	
	/* Initialisierung der Namen */
	daktstatus(" VAR-Modul Initialisierung ", "Variablen-Namen und -Werte werden generiert ...");
	strcpy(sysvar[0].name, "(leer)"); 

	for (x = 1; x < MAXSYSVARS; x++)
	{
		sysvar[x].min = MINLONG;	/* minlong */
		sysvar[x].max = MAXLONG;	/* maxlong */
		sysvar[x].standard = 0;
	}

	set_daktstat(5);
	for (x = VAR_VAR0; x < VAR_VAR0 + MAXSETVARS; x++)
		DefineSetupVar(x, "VAR %3d", x - VAR_VAR0);

	set_daktstat(10);
		for (x = VAR_CONT0; x < VAR_MAAX; x++)
		DefineMidiVar(x, "Ctrl %3d", x - VAR_CONT0); 

	set_daktstat(15);
	DefineHiMidiVar(VAR_PITCH, "Pi-Bend", 0); 

	DefinePercentVar(VAR_MAAX, "ST-Maus->X", 0); 
	DefinePercentVar(VAR_MAAY, "ST-Maus->Y", 0); 
	DefinePercentVar(VAR_MAAZ, "ST-Maus->Z", 0); 

	DefinePercentVar(VAR_MAEX, "PC-Maus->X", 0); 
	DefinePercentVar(VAR_MAEY, "PC-Maus->Y", 0); 
	DefinePercentVar(VAR_MAEZ, "PC-Maus->Z", 0); 

	DefinePercentVar(VAR_POWX, "POW X", 0); 
	DefinePercentVar(VAR_POWY, "POW Y", 0); 
	DefinePercentVar(VAR_POWZ, "POW Z", 0); 
	DefinePercentVar(VAR_POWR, "POW Rot", 0); 
	DefinePercentVar(VAR_POWF, "POW Fin", 0); 
	DefinePercentVar(VAR_POWK, "POW Key", 0); 

	for (x = VAR_LFA1; x < VAR_LFA1 + MAXLFOS; x++)
		DefinePhaseVar(x, "LFA Pos %2d", x - VAR_LFA1); 

	for (x = VAR_LFB1; x < VAR_LFB1 + MAXLFOS; x++)
		DefinePhaseVar(x, "LFB Pos %2d", x - VAR_LFB1); 

	for (x = VAR_MTR0; x < VAR_MTR0 + MAXSIGNALS; x++)
		DefinePhaseVar(x, "MTR Pos %2d", x - VAR_MTR0); 

	set_daktstat(20);
	DefinePropVar(VAR_PROP_MAA, "MAA Prop", 0);
	DefinePropVar(VAR_PROP_MAE, "MAE Prop", 0);
	DefinePropVar(VAR_PROP_POW, "POW Prop", 0);
	DefinePropVar(VAR_PROP_LFA, "LFA Prop", 0);
	DefinePropVar(VAR_PROP_LFB, "LFB Prop", 0);
	DefinePropVar(VAR_PROP_MTR, "MTR Prop", 0);
	DefinePropVar(VAR_PROP_SPG, "SPG Prop", 0);
	DefinePropVar(VAR_PROP_SPO, "SPO Prop", 0);
	DefinePropVar(VAR_PROP_SPS, "SPS Prop", 0);

	for (x = 0; x < MAXSIGNALS; x++) {
		DefineMidiVar(VAR_VOLUME0 + x, "Volume %2d", x); 
		DefineBoolVar(VAR_MTR_ON0 + x, "MTR-An %2d", x); 
		DefineAccVar(VAR_MTR_ACC0 + x, "MTR-Spd %2d", x); 
	}
	
	for (x = 0; x < MAXLFOS; x++) {
		DefineBoolVar(VAR_LFA_ON1 + x, "LFA-An %2d", x +1); 
		DefineAccVar(VAR_LFA_ACC1 + x, "LFA-Spd %2d", x +1); 
	}

	for (x = 0; x < MAXSIGNALS; x++) {
		DefinePropVar(VAR_LFA_MTR_POS0 + x, "LFA-MTRP%2d", x); 
		DefineMidiVar(VAR_LFA_VOLUME0 + x, "LFA->Vol%2d", x); 
		DefinePropVar(VAR_LFA_ZOOM0 + x, "LFA->Zoo%2d", x); 
		DefinePhaseVar(VAR_LFA_VORZUR0 + x, "LFA->V+Z%2d", x); 
	}

	set_daktstat(25);
	for (x = 0; x < 4; x++) {
		DefinePanVar(VAR_LFA_PANBREITE1 + x, "LFA->PBr%2d", x); 
		DefinePanVar(VAR_LFA_PANPOS1 + x, "LFA->PPo%2d", x); 
	}
	DefinePanVar(VAR_LFA_QUADBREITE1, "LFA-QuBr %d", 1 );
	DefinePanVar(VAR_LFA_QUADBREITE1 + 1, "LFA-QuBr %d", 2 );
	DefinePanVar(VAR_LFA_QUADPOS1, "LFA-QuPos %d", 1 );
	DefinePanVar(VAR_LFA_QUADPOS1 + 1, "LFA-QuPos %d", 2 );

	for (x = 0; x < MAXLFOS; x++) {
		DefineBoolVar(VAR_LFB_ON1 + x, "LFB-An %2d", x +1); 
		DefineAccVar(VAR_LFB_ACC1 + x, "LFB-Spd %2d", x +1); 
	}

	for (x = 0; x < MAXSIGNALS; x++) {
		DefinePropVar(VAR_LFB_MTR_POS0 + x, "LFB-MTRP%2d", x); 
		DefineMidiVar(VAR_LFB_VOLUME0 + x, "LFB->Vol%2d", x); 
		DefinePanVar(VAR_LFB_ZOOM0 + x, "LFB->Zoo%2d", x); 
		DefinePhaseVar(VAR_LFB_VORZUR0 + x, "LFB->V+Z%2d", x); 
	}

	for (x = 0; x < 4; x++) {
		DefinePanVar(VAR_LFB_PANBREITE1 + x, "LFB->PBr%2d", x); 
		DefinePanVar(VAR_LFB_PANPOS1 + x, "LFB->PPo%2d", x); 
	}
	DefinePanVar(VAR_LFB_QUADBREITE1, "LFB-QuBr %d", 1 );
	DefinePanVar(VAR_LFB_QUADBREITE1 + 1, "LFB-QuBr %d", 2 );
	DefinePanVar(VAR_LFB_QUADPOS1, "LFB-QuPos %d", 1 );
	DefinePanVar(VAR_LFB_QUADPOS1 + 1, "LFB-QuPos %d", 2 );

	set_daktstat(30);
	DefineBoolVar(VAR_RECORD, 			"RECORD", 0);
	DefineBoolVar(VAR_PUF_PLAY, 		"PUF Sync", 0);
	DefineBoolVar(VAR_PUF_ZEITLUPE, 	"PUF Zeitl.", 0);
	DefineBoolVar(VAR_PUF_PAUSE, 		"PUF Pause", 0);

	set_daktstat(40);
	for (x = 0; x < 8; x++)
	{
		DefineKoorVar(VAR_PUF_KOORX0 + x, "PUF Koor X %d", x);
		DefineKoorVar(VAR_PUF_KOORY0 + x, "PUF Koor Y %d", x);
		DefineKoorVar(VAR_PUF_KOORZ0 + x, "PUF Koor Z %d", x);
		DefineKoorVar(VAR_PUF_VOL0 + x,   "PUF Volume %d", x);
	}
	DefineBoolVar(VAR_BIG_PLAY, 		"BIG Sync", 0);

	DefineBoolVar(VAR_MAA_SPERRE_INNEN, 	"MAA SpIn", 0);
	DefineBoolVar(VAR_MAA_SPERRE_AUSSEN, 	"MAA SpAu", 0);
	DefineBoolVar(VAR_MAE_SPERRE_INNEN, 	"MAE SpIn", 0);
	DefineBoolVar(VAR_MAE_SPERRE_AUSSEN, 	"MAE SpAu", 0);

	set_daktstat(50);
	for (x = 0; x < 8; x++)
		DefineCMIVar(VAR_CMI_SIGNAL1 + x, "CMI-Sig.%d", x +1); 

	set_daktstat(60);
	DefinePPercentVar(VAR_CM_MASTER1, "CM-Master %d", 1);
	DefinePPercentVar(VAR_CM_MASTER2, "CM-Master %d", 2);

	DefineChanVar(VAR_CMI_CHANNEL1, 	"CMI Chan %d", 1);
	DefineChanVar(VAR_CMI_CHANNEL2, 	"CMI Chan %d", 1);
	DefinePortVar(VAR_CMI_PORT1, 	"CMI Port %d", 1);
	DefinePortVar(VAR_CMI_PORT2, 	"CMI Port %d", 2);

	set_daktstat(70);
	DefineBoolVar(VAR_TRA_PLAY, 			"TRA Play", 0);
	DefineBoolVar(VAR_TRA_STOP, 			"TRA Stop", 0);
	DefineBoolVar(VAR_TRA_FFWD, 			"TRA FFwd", 0);
	DefineBoolVar(VAR_TRA_REW, 			"TRA Rew", 0);
	DefineBoolVar(VAR_TRA_CLICK, 			"TRA Click", 0);

	set_daktstat(80);
	DefineCMIVar(VAR_ED4_PRESEL_INPUT, 	"ED4 Input", 0); 
	DefineKoorVar(VAR_ED4_PRESEL_POSX, 	"ED4 Input", 0); 
	DefineKoorVar(VAR_ED4_PRESEL_POSY,	"ED4 Input", 0); 
	DefineMidiVar(VAR_ED4_PRESEL_VOL, 	"ED4 Input", 0); 
	DefineAccVar(VAR_ED4_PRESEL_OBJSPD, "ED4 Input", 0); 
	DefineAccVar(VAR_ED4_SPEED, 			"ED4 Input", 0); 

	set_daktstat(90);
	DefineSetupVar(VAR_SET_A3D, "A3D Setup", 0);
	DefineSetupVar(VAR_SET_BIG, "BIG Setup", 0);
	DefineSetupVar(VAR_SET_CMI, "CMI Setup", 0);
	DefineSetupVar(VAR_SET_CMO, "CMO Setup", 0);
	DefineSetupVar(VAR_SET_CON, "CON Setup", 0);
	DefineSetupVar(VAR_SET_EFF, "EFF Setup", 0);
	DefineSetupVar(VAR_SET_FAD, "FAD Setup", 0);
	DefineSetupVar(VAR_SET_GEN, "GEN Setup", 0);
	DefineSetupVar(VAR_SET_INI, "INI Setup", 0);
	DefineSetupVar(VAR_SET_INO, "INO Setup", 0);
	DefineSetupVar(VAR_SET_INT, "INT Setup", 0);
	DefineSetupVar(VAR_SET_KOO, "KOO Setup", 0);
	DefineSetupVar(VAR_SET_LFA, "LFA Setup", 0);
	DefineSetupVar(VAR_SET_LFB, "LFB Setup", 0);
	DefineSetupVar(VAR_SET_MAA, "MAA Setup", 0);
	DefineSetupVar(VAR_SET_MAE, "MAE Setup", 0);
	DefineSetupVar(VAR_SET_MAN, "MAN Setup", 0);
	DefineSetupVar(VAR_SET_MIN, "MIN Setup", 0);
	DefineSetupVar(VAR_SET_MIO, "MIO Setup", 0);
	DefineSetupVar(VAR_SET_MTR, "MTR Setup", 0);
	DefineSetupVar(VAR_SET_OTU, "OTU Setup", 0);
	DefineSetupVar(VAR_SET_PAR, "PAR Setup", 0);
	DefineSetupVar(VAR_SET_PLE, "PLE Setup", 0);
	DefineSetupVar(VAR_SET_PUF, "PUF Setup", 0);
	DefineSetupVar(VAR_SET_SPG, "SPG Setup", 0);
	DefineSetupVar(VAR_SET_SPO, "SPO Setup", 0);
	DefineSetupVar(VAR_SET_SPS, "SPS Setup", 0);
	DefineSetupVar(VAR_SET_SRR, "SRR Setup", 0);
	DefineSetupVar(VAR_SET_STE, "STE Setup", 0);
	DefineSetupVar(VAR_SET_STR, "STR Setup", 0);
	DefineSetupVar(VAR_SET_SYN, "SYN Setup", 0);
	DefineSetupVar(VAR_SET_TRA, "TRA Setup", 0);
	DefineSetupVar(VAR_SET_VAR, "VAR Setup", 0);
	DefineSetupVar(VAR_SET_VER, "VER Setup", 0);
	
	set_daktstat(100);
	close_daktstat();
	
} /* init_var_names */
/*****************************************************************************/
PRIVATE BOOLEAN init_rsc ()

{
  WORD   i;
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
  alevarsg = &rs_strings [FREESTR];             /* Adresse der Fehlermeldungen */
*/
/*
	var_menu  = (OBJECT *)rs_trindex [VAR_SETUP]; /* Adresse des VAR-MenÅs */
*/
	var_setup = (OBJECT *)rs_trindex [VAR_SETUP]; /* Adresse der VAR-Parameter-Box */
	var_help  = (OBJECT *)rs_trindex [VAR_HELP];	/* Adresse der VAR-Hilfe */
	var_desk  = (OBJECT *)rs_trindex [VAR_DESK];	/* Adresse des VAR-Desktops */
	var_text  = (OBJECT *)rs_trindex [VAR_TEXT];	/* Adresse der VAR-Texte */
	var_info 	= (OBJECT *)rs_trindex [VAR_INFO];	/* Adresse der VAR-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (var_rsc_ptr, rsc_name))
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
/*
	rs_gaddr (var_rsc_ptr, R_TREE,  VAR_SETUP,	&var_menu);    /* Adresse des VAR-MenÅs */
*/
	rs_gaddr (var_rsc_ptr, R_TREE,  VAR_SETUP,	&var_setup);   /* Adresse der VAR-Parameter-Box */
	rs_gaddr (var_rsc_ptr, R_TREE,  VAR_HELP,	&var_help);    /* Adresse der VAR-Hilfe */
	rs_gaddr (var_rsc_ptr, R_TREE,  VAR_DESK,	&var_desk);    /* Adresse des VAR-Desktop */
	rs_gaddr (var_rsc_ptr, R_TREE,  VAR_TEXT,	&var_text);    /* Adresse der VAR-Texte */
	rs_gaddr (var_rsc_ptr, R_TREE,  VAR_INFO,	&var_info);    /* Adresse der VAR-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	/*fix_objs (var_menu, TRUE);*/
	fix_objs (var_setup, TRUE);
	fix_objs (var_help, TRUE);
	fix_objs (var_desk, TRUE);
	fix_objs (var_text, TRUE);
	fix_objs (var_info, TRUE);
	
	/*
	do_flags (var_setup, VARCANCEL, UNDO_FLAG);
	do_flags (var_setup, VARHELP, HELP_FLAG);
	*/
	menu_enable(menu, MVAR, TRUE);

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
  ok = rs_free (var_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_var ()
{
	BOOLEAN	ok = TRUE;
	
	ok &= init_rsc ();
	instance_count = load_create_infos (create, module_name, max_instances);
	
	return ok;
} /* init_var */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()
{
	BOOLEAN	ok = TRUE;

	ok &= term_rsc ();
	return (ok);
} /* term_mod */
