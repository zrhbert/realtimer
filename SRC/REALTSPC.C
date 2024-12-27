/*****************************************************************************/
/*                                                                           */
/* Modul: REALTSPC.C                                                         */
/*                                                                           */
/* Spezialfunktionen                                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- Datenreduktion in SetPTime, 04.03.95
- GetPText als Funktion, 04.03.95
- SetPLongN eingebaut, 19.02.95
- Sinq und Cosq eingebaut, 19.02.95
- Window-Update bei ClickSetupField, 03.02.95
- WTRIANGLE repariert, 05.02.95
- ClickValueField mit min/max bei Direkt-Eingabe, 02.02.95
- set_cancel und window-update in ClickSetupField, 02.02.95
- ClickSetupField eingebaut, 30.01.95
- GetCheck mit Shortcut eingebaut, 30.01.95
- SetPxxx auf atoi/l/f umgebaut, 30.01.95
- SetPWord2 und SetPWord3 eingebaut, 09.01.95
- ClickTimeField eingebaut, 08.01.95
- dlogin eingebaut, 04.01.95
- GetPxxx mit optionalem drittem Parameter, 03.01.95
- SetPxxx/GetPxxx eingebaut, 23.12.94
- SetVertex etc. eingebaut, 20.12.94
17.01.94
- controller_out eingebaut
- update_text und update_window gegen window=0 geschÅtzt
- akstatus mit malloc fÅr window->special
24.06.93
- Help mit Space-Stripping
- copy_icon modifiziert
- daktstat mit Mindestzeit
- Definitionen fÅr SETUP und STATUS eingebaut
- Help-Funktion fÅr Alert-Boxen eingeklinkt

*****************************************************************************/

#define XRSC_CREATE 1

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"
#include "dialog.h"
#include "resource.h"

#include "realtim4.h"
#include "objects.h"
#include "errors.h"

#include <msh_unit.h>
#include "msh.h"
#include "math.h"
#include "time.h"
#include "ext.h"			/* wg. delay() */

#include "export.h"
#include "realtspc.h"

#if XRSC_CREATE
#include "spc_mod.rsh"
#include "spc_mod.rh"
#endif

/****** DEFINES **************************************************************/

#define MAXLOGINTRIALS 3
#define MAX_PASSWORD     8              /* Maximal 8 Buchstaben im Passwort */

/****** TYPES ****************************************************************/
typedef struct setup
{
	VOID *dummy;
} SETUP;

typedef struct status
{
	VOID *dummy;
} STATUS;

typedef struct aktstatus
{
	BOOLEAN	close;			/* Flag, ob geschlossen werden soll */
	LONG		start_time;		/* Erste Aufrufzeit */
	LONG		time;				/* verbrauchte Zeit */
	WORD		percent;			/* erreichter Anteil */
} AKTSTATUS;

/****** VARIABLES ************************************************************/
LOCAL WORD	spc_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
LOCAL WORD	*spc_rsc_ptr = &spc_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */

LOCAL OBJECT *zahl;
LOCAL OBJECT *zeit;
LOCAL OBJECT *save;
LOCAL OBJECT *login;
LOCAL OBJECT *aktstat;

LOCAL WORD     edit_inx;        /* Index Passwort fÅr edit_noecho */
LOCAL BYTE     password [MAX_PASSWORD + 1];
LOCAL BYTE     user [MAX_PASSWORD + 1];
LOCAL WORD     log_count;			/* Anzahl der Login Versuche */

/****** FUNCTIONS ************************************************************/
LOCAL VOID time_strings (STRING hours, STRING minutes, STRING seconds, STRING milli, LONG time);
LOCAL LONG strings_time (STRING hours, STRING minutes, STRING seconds, STRING milli);

LOCAL VOID		get_zahl		_((LONG *number));
LOCAL VOID		set_zahl   	_((LONG number));
LOCAL VOID		click_zahl 	_((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN	key_zahl   	_((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN	help_zahl  	_((WINDOWP window, WORD icon));

LOCAL VOID		get_zeit		_((LONG *time));
LOCAL VOID		set_zeit   	_((LONG time));
LOCAL VOID		click_zeit 	_((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN	key_zeit   	_((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN	help_zeit  	_((WINDOWP window, WORD icon));

LOCAL VOID		get_save		_((VOID));
LOCAL VOID		set_save   	_((STR128 file));
LOCAL VOID		click_save 	_((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN	key_save   	_((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN	help_save  	_((WINDOWP window, WORD icon));

LOCAL BOOL try_login (CHAR *user, CHAR *password);
LOCAL VOID get_login (CHAR *user, CHAR *password);
LOCAL VOID set_login (VOID);
LOCAL VOID click_login (WINDOWP window, MKINFO *mk);
LOCAL BOOLEAN key_login (WINDOWP window, MKINFO  *mk);
LOCAL BOOLEAN help_login (WINDOWP window, WORD icon);

LOCAL VOID		timer_daktstat	_((WINDOWP window));

LOCAL VOID		init_waveform	_((VOID));
LOCAL VOID 		init_sinq		(VOID);

LOCAL BOOLEAN	init_rsc_realtspc _((VOID));
LOCAL BOOLEAN	term_rsc_realtspc _((VOID));


/*** Zeit-Gruppen ***********************************************************/

LOCAL VOID time_strings (STRING hours, STRING minutes, STRING seconds, STRING milli, LONG time)
{
	/* Wandelt einen Long-Wert fÅr die Anzahl der Millisekunden */
	/* in eine Reihe von Zahlen um */
	REG LONG temp;
	REG WORD hh, mm, ss, ll;
	
	temp = time /1000;						/* Millisekunden abschneiden */
	ll = (WORD) (time - temp*1000);			/* ms merken */
	sprintf(milli, "%3d", ll);

	time /= 1000;
	temp /= 60;
	ss = (WORD) (time - temp*60);
	sprintf(seconds, "%2d", ss);

	time /= 60;
	temp /= 60;
	mm = (WORD) (time - temp*60);
	sprintf(minutes, "%2d", mm);

	time /= 60;
	temp /= 24;
	hh = (WORD) (time - temp*24);
	time /= 24;
	sprintf(hours, "%2d", hh);

} /* time_strings */

LOCAL LONG strings_time (STRING hours, STRING minutes, STRING seconds, STRING milli)
{
	/* Wandelt eine Reihe von Strings */
	/* in einen Long-Wert fÅr die Anzahl der Millisekunden um */
	REG LONG time;
	WORD hh = 0, mm = 0, ss = 0, ll = 0;

	hh = atoi (hours);
	mm = atoi (minutes);
	ss = atoi (seconds);
	ll = atoi (milli);

	time = hh;
	time *= 60;
	time += mm;
	time *= 60;
	time += ss;
	time *= 1000;
	time += ll;

	return (time);
} /* strings_time */

/*** D-Boxen ***********************************************************/

GLOBAL BOOL GetCheck (OBJECT *tree, WORD obj, BOOL *val)
{
	BOOL	value;
	/* Schneller, aber equivalent zu: value = get_checkbox (tree, obj); */

	/* Vermeiden von Werten != 0 oder 1, wg. Bitfeldern */
	if (is_state (tree, obj, SELECTED)) 
		value = TRUE;
	else
		value = FALSE;
	
	if (val) *val = value;
	return value;
} /* GetCheck */

GLOBAL WORD GetPWord (OBJECT *tree, WORD obj, WORD *val)
{
	STRING 	s;
	WORD		value;
	
	get_ptext (tree, obj, s);
	value = atoi(s);
	
	if (val) *val = value;
	return value;
} /* GetPWord */

GLOBAL LONG GetPTime (OBJECT *tree, WORD obj, LONG *val)
{
	LONG		value;
/*
	STRING 	s;
		
	get_ptext (tree, obj, s);
	value = strtim(s);
*/
	STRING		hours, minutes, seconds, milli;

	GetPText (tree, obj + 1, hours);
	GetPText (tree, obj + 2, minutes);
	GetPText (tree, obj + 3, seconds);
	GetPText (tree, obj + 4, milli);

	value = strings_time (hours, minutes, seconds, milli);
	if (val) *val = value;
	return value;
} /* GetPTime */

GLOBAL LONG GetPLong (OBJECT *tree, WORD obj, LONG *val)
{
	STRING 	s;
	LONG		value;
	
	get_ptext (tree, obj, s);
	value = atol(s);
	
	if (val) *val = value;
	return value;
} /* GetPLong */

GLOBAL FLOAT GetPFloat (OBJECT *tree, WORD obj, FLOAT *val)
{
	STRING 	s;
	FLOAT		value;
	
	get_ptext (tree, obj, s);
	value = atof(s);

	if (val) *val = value;
	return value;
} /* GetPFloat */

GLOBAL CHAR *GetPText (OBJECT *tree, WORD obj, CHAR *val)
{
	get_ptext (tree, obj, val);
	return val;
}

GLOBAL VOID SetCheck (OBJECT *tree, WORD obj, CONST BOOL val)
{
	/* Schneller, aber equivalent zu: set_checkbox (tree, obj, val); */
	if (val)
		tree [obj].ob_state |= SELECTED;         /* Status im Objekt setzen */
	else
		tree [obj].ob_state &= ~ SELECTED;       /* Status im Objekt lîschen */
	
} /* SetCheck */

GLOBAL VOID SetPWord (OBJECT *tree, WORD obj, CONST WORD value)
{
	STRING 	s;
	
	itoa (value, s, 10);
	set_ptext (tree, obj, s);
} /* SetPWord */

GLOBAL VOID SetPWordN (OBJECT *tree, WORD obj, CONST WORD value)
{
	/* Zahlen mit 0 als Leerstellen */
	STRING 	s;
	
	if (value !=0)
		itoa (value, s, 10);
	else
		sprintf (s, "");

	set_ptext (tree, obj, s);
} /* SetPWordN */

GLOBAL VOID SetPWord2 (OBJECT *tree, WORD obj, CONST WORD value)
{
	/* Zweistellige Zahlen */
	STRING 	s, format = "%2d";
	
	sprintf (s, format, value);
	set_ptext (tree, obj, s);
} /* SetPWord2 */

GLOBAL VOID SetPWord2N (OBJECT *tree, WORD obj, CONST WORD value)
{
	/* Zweistellige Zahlen mit 0 als Leerstellen */
	STRING 	s, format = "%2d";
	
	if (value !=0)
		sprintf (s, format, value);
	else
		sprintf (s, "");

	set_ptext (tree, obj, s);
} /* SetPWord2N */

GLOBAL VOID SetPWord3 (OBJECT *tree, WORD obj, CONST WORD value)
{
	/* Dreistellige Zahlen */
	STRING 	s, format = "%3d";
	
	sprintf (s, format, value);
	set_ptext (tree, obj, s);
} /* SetPWord3 */

GLOBAL VOID SetPWord3N (OBJECT *tree, WORD obj, CONST WORD value)
{
	/* Zweistellige Zahlen mit 0 als Leerstellen */
	STRING 	s, format = "%3d";
	
	if (value !=0)
		sprintf (s, format, value);
	else
		sprintf (s, "");

	set_ptext (tree, obj, s);
} /* SetPWord3N */

GLOBAL VOID SetPTime (OBJECT *tree, WORD obj, CONST LONG value)
{
	STRING 	s;
	STRING	hours, minutes, seconds, milli;

	time_strings (hours, minutes, seconds, milli, value);

	/* Nur neu einsetzen, wenn verÑndert */
	if (strcmp (GetPText(tree, obj + 1, s), hours)   != 0) SetPText (tree, obj + 1, hours);
	if (strcmp (GetPText(tree, obj + 2, s), minutes) != 0) SetPText (tree, obj + 2, minutes);
	if (strcmp (GetPText(tree, obj + 3, s), seconds) != 0) SetPText (tree, obj + 3, seconds);
	if (strcmp (GetPText(tree, obj + 4, s), milli)   != 0) SetPText (tree, obj + 4, milli);
} /* SetPTime */

GLOBAL VOID SetPLong (OBJECT *tree, WORD obj, CONST LONG value)
{
	STRING 	s, format = "%ld";
	
	sprintf (s, format, value);
	set_ptext (tree, obj, s);
} /* SetPLong */

GLOBAL VOID SetPLongN (OBJECT *tree, WORD obj, CONST LONG value)
{
	/* Zahlen mit 0 als Leerstellen */
	STRING 	s, format = "%ld";
	
	if (value !=0)
		sprintf (s, format, value);
	else
		sprintf (s, "");

	set_ptext (tree, obj, s);
} /* SetPLongN */

GLOBAL VOID SetPFloat (OBJECT *tree, WORD obj, CONST FLOAT value)
{
	STRING 	s, format = "%.0f";
	
	sprintf (s, format, value);
	set_ptext (tree, obj, s);
} /* SetPFloat */

/*** Polygone ***********************************************************/

GLOBAL VOID SetVertex (POS_3DP v, FLOAT x, FLOAT y, FLOAT z)
{
	/* Definiere einen Eckpunkt in einem Polygon */
	v->x = x;
	v->y = y;
	v->z = z;
} /* SetVertex	*/

GLOBAL VOID AddVertex (POLY_P poly, FLOAT x, FLOAT y, FLOAT z)
{
	/* Neuen Eckpunkt definieren */
	SetVertex (&poly->vertex[poly->num_vertices++], x, y, z);
} /* AddVertex */

GLOBAL VOID SetEdge (EDGE_P e, WORD from, WORD to, WORD style, WORD begin, WORD end, WORD color, WORD width)
{
	/* Definiere eine Kante in einem Polygon */
	e->from = from;
	e->to = to;
	e->line_style = style;
	e->begin_style = begin;
	e->end_style = end;
	e->color = color;
	e->width = width;
} /* SetEdge */

GLOBAL VOID AddEdge (POLY_P poly, WORD from, WORD to, WORD style, WORD begin, WORD end, WORD color, WORD width)
{
	/* Neue Kante einsetzen */
	SetEdge (&poly->edge[poly->num_edges++], from, to, style, begin, end, color, width);
} /* AddEdge */

/***Akt-Status-Window **********************************************************/

GLOBAL VOID daktstatus (STRING title, STRING text)
{
	/* Fenster zum Anzeigen des aktuellen Status */
	WINDOWP	window;
	WORD		ret;
	STRING	s; 
	static	STRING stitle;
	AKTSTATUS	*status;
	window = search_window (CLASS_DIALOG, SRCH_ANY, 100+AKTSTAT);
	
	/* Leerstellen an Titel anfÅgen */
	sprintf(stitle, " %s ", title);
	if (window == NULL)
	{
		form_center (aktstat, &ret, &ret, &ret, &ret);
		
		window = crt_dialog (aktstat, NULL, 100+AKTSTAT, stitle, WI_MODELESS);
		
	} /* if */
	
	if (window != NULL)
	{
		sprintf (window->name, stitle);
		if (window->opened == 0)
		{
			window->edit_obj	= NIL;
			window->edit_inx	= NIL;
			window->timer		= timer_daktstat;
			window->special	= (LONG)mem_alloc(sizeof(AKTSTATUS));
			window->milli		= 0;
		} /* if */
		else
			wind_set (window->handle, WF_NAME, ADR (stitle), 0, 0); /* Name setzen */

		window->special = (LONG)mem_alloc (sizeof (AKTSTATUS));
		status = (AKTSTATUS*) window->special;
		mem_set ((VOID*)status, 0, (UWORD)sizeof (AKTSTATUS));
		status->close		= 0;
		status->start_time= clock();
		status->time		= 0;
		status->percent	= 0;
		sprintf (s, "%s",  text);
		set_ptext (aktstat, AKTTEXT, s);
		set_daktstat (0);
				
		if (! open_dialog (100+AKTSTAT)) hndl_alert (ERR_NOOPEN);
		
		draw_object(window, ROOT);
	} /* if */
} /* daktstatus */

GLOBAL VOID set_daktstat	(WORD percent)
{
	WINDOWP		window;
	LONG			temp;
	LONG			proj_time, elapsed_time, time, start_time;
	WORD			elapsed_percent;
	AKTSTATUS	*status;
	STRING		s;
	
	window = search_window (CLASS_DIALOG, SRCH_ANY, 100+AKTSTAT);

	status = (AKTSTATUS*) window->special;
	start_time			= status->start_time;
	elapsed_time		= status->time;
	elapsed_percent	= status->percent;
	status->percent	= percent;
	time 					= clock() - start_time;
	status->time		= time;	/* verbrauchte Zeit */

	/* PrÅfen, ob sich schon etwas getan hat */
	if (percent > elapsed_percent && time > elapsed_time)
	{	
		/* Vorhersage berechnen */
		/* Geschwindigkeit * Rest-Arbeit */
		/* proj_time = (time - elapsed_time) / (percent - elapsed_percent); */
		proj_time = time / percent;
		proj_time *= (100 - percent);
		proj_time /= CLK_TCK;
		timstr (proj_time * 1000, s);
		set_ptext (aktstat, AKTERWARTZEIT, s);
	} /* if */
	else if (elapsed_time == 0)
	{
		sprintf (s, "        ");
		set_ptext (aktstat, AKTERWARTZEIT, s);
	} /* else */

	/* Laufzeit eintragen */
	timstr (elapsed_time/CLK_TCK* 1000, s);
	set_ptext (aktstat, AKTLAUFZEIT, s);
	/* Erreichten Anteil eintragen */
	sprintf (s, "%d%%", percent);
	set_ptext (aktstat, AKTPROZENT, s);
	
	/* SchieberlÑnge anpassen */
	temp = (LONG)aktstat[AKTWORKBOX].ob_width;
	temp *= (LONG)percent;
	temp /= 100;
	aktstat[AKTWORKSLIDER].ob_width = (WORD)temp;
	/* Text im Schieber justieren */
	if (temp < aktstat[AKTPROZENT].ob_width / 2)
		temp = 0;	/* Wenn Schieber zu klein fÅr Anzeiger */
	aktstat[AKTPROZENT].ob_x = ((WORD)temp)/2 + aktstat[AKTWORKSLIDER].ob_x;
	
	if (window != 0)	
	{
		draw_object (window, AKTWORKSLIDER);
		draw_object (window, AKTLAUFZEIT);
		draw_object (window, AKTERWARTZEIT);
	} /* if */	
} /* set_daktstat */

LOCAL VOID timer_daktstat (WINDOWP window)
{
	AKTSTATUS	*status;

	if (window)
	{
		status = (AKTSTATUS*) window->special;
		if (status->close) /* Schliessen ? */
		{
			close_window(window);
			mem_free ((VOID*)status);
		} /* if */
	} /* if */
} /* timer_daktstat */

GLOBAL VOID close_daktstat()
{
	WINDOWP	window;
	AKTSTATUS	*status;
	BOOLEAN	immediate = FALSE;

	window = search_window (CLASS_DIALOG, SRCH_OPENED, 100+AKTSTAT);

	if (window)
	{
		if (immediate)
		{	
			close_window(window);
		} /* if */
		else
		{	
			status = (AKTSTATUS*) window->special;
			status->close = TRUE;		/* Schliessen befehlen */
			window->milli = 300;			/* etwas stehen lassen */
			/* SpÑter wird per timer_daktstat geschlossen */
		} /* else */
	} /* if */
} /* close_daktstat */
/*****************************************************************************/
LOCAL VOID get_zahl (number)
LONG *number;
{
  STRING s;

  get_ptext (zahl, ZAHZAHL, s);
  sscanf (s, "%ld", number);

} /* get_zahl */

/*****************************************************************************/

LOCAL VOID set_zahl (number)
LONG number;

{
  STRING s;

  sprintf (s, "%ld",  number);
  set_ptext (zahl, ZAHZAHL, s);
} /* set_zahl */

/*****************************************************************************/

LOCAL VOID click_zahl (window, mk)
WINDOWP window;
MKINFO  *mk;

{
} /* click_zahl */

/*****************************************************************************/

LOCAL BOOLEAN key_zahl (window, mk)
WINDOWP window;
MKINFO  *mk;

{

  switch (window->edit_obj)
  {
  } /* switch */

  return (FALSE);
} /* key_zahl */

/*****************************************************************************/

LOCAL BOOLEAN help_zahl (window, icon)
WINDOWP window;
WORD    icon;

{

  return (TRUE);
} /* help_zahl */

/*****************************************************************************/

GLOBAL VOID dzahl (LONG *number, INT posx, INT posy, STRING title)
{
	WINDOWP window;
	WORD    ret;
	
	window = search_window (CLASS_DIALOG, SRCH_ANY, 100+ZAHL);
	
	if (posx==-1 && posy==-1)
		form_center (zahl, &ret, &ret, &ret, &ret);
	if (window == NULL)
	{
		window = crt_dialog (zahl, NULL, 100+ZAHL, title, WI_MODAL);
		
		if (window != NULL)
		{
			window->click    = click_zahl;
			window->key      = key_zahl;
			window->showhelp = help_zahl;
		} /* if */
	} /* if */
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			sprintf (window->name, title);
			window->edit_obj	= find_flags (zahl, ROOT, EDITABLE);
			window->edit_inx	= NIL;
			window->scroll.x  = posx;
			window->scroll.y  = posy;
			window->work.x    = window->scroll.x;
			window->work.y    = window->scroll.y;
			window->work.w    = window->scroll.w;
			window->work.h    = window->scroll.h;
			set_zahl (*number);
		} /* if */
		
		if (! open_dialog (100+ZAHL)) hndl_alert (ERR_NOOPEN);
		
		/* Zahl Åbernehmen klappt nur, wenn D-Box modal! */
		switch (window->exit_obj)
		{
			case ZAHZAHL :	get_zahl(number);
		               		break;
		} /* switch */
	} /* if */
} /* dzahl */

/*****************************************************************************/
GLOBAL VOID dzeit (LONG *time, INT posx, INT posy, STRING title)
{
	WINDOWP window;
	WORD    ret;
	
	window = search_window (CLASS_DIALOG, SRCH_CLOSED, 100+ZEIT);
	
	if (window == NULL)
	{
		if (posx==-1 && posy==-1)
		form_center (zeit, &ret, &ret, &ret, &ret);
		
		window = crt_dialog (zeit, NULL, 100+ZEIT, title, WI_MODAL);
		
		if (window != NULL)
		{
			window->click    = click_zeit;
			window->key      = key_zeit;
			window->showhelp = help_zeit;
		} /* if */
	} /* if */
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			sprintf (window->name, title);
			window->edit_obj	= find_flags (zeit, ROOT, EDITABLE);
			window->edit_inx	= NIL;
			window->scroll.x  = posx;
			window->scroll.y  = posy;
			window->work.x    = window->scroll.x;
			window->work.y    = window->scroll.y;
			window->work.w    = window->scroll.w;
			window->work.h    = window->scroll.h;
			set_zeit (*time);
		} /* if */
		
		if (! open_dialog (100+ZEIT)) hndl_alert (ERR_NOOPEN);
		
		/* Zahl Åbernehmen klappt nur, wenn D-Box modal! */
		switch (window->exit_obj)
		{
			case ZEIZEIT :	get_zeit(time);
			            		break;
		} /* switch */
	} /* if */
} /* dzeit */
/*****************************************************************************/

LOCAL VOID get_zeit (LONG *time)
{
	STRING s;
	
	get_ptext (zeit, ZEIZEIT, s);
	*time = strtim(s);		/* Make SMPTE-String out of time-LONG */
} /* get_zeit */

/*****************************************************************************/

LOCAL VOID set_zeit (LONG time)
{
	STRING s;

  timstr (time, s);	/* Make String out of long */
  set_ptext (zeit, ZEIZEIT, s);
} /* set_zeit */

/*****************************************************************************/

LOCAL VOID click_zeit (window, mk)
WINDOWP window;
MKINFO  *mk;

{
} /* click_zeit */

/*****************************************************************************/

LOCAL BOOLEAN key_zeit (window, mk)
WINDOWP window;
MKINFO  *mk;

{

  switch (window->edit_obj)
  {
  } /* switch */

  return (FALSE);
} /* key_zahl */

/*****************************************************************************/

LOCAL BOOLEAN help_zeit (window, icon)
WINDOWP window;
WORD    icon;

{

  return (TRUE);
} /* help_zahl */

/*****************************************************************************/

GLOBAL VOID dsave (STR128 filename, RTMCLASSP module)
{
	WINDOWP window;
	WORD    ret, menu_height;
	
	window = search_window (CLASS_DIALOG, SRCH_ANY, 100+SAVE);
	
	form_center (save, &ret, &ret, &ret, &ret);
	if (window == NULL)
	{
		window = crt_dialog (save, NULL, 100+SAVE, module->object_name, WI_MODAL);
		
		if (window != NULL)
		{
			window->click    = click_save;
			window->key      = key_save;
			window->showhelp = help_save;
		} /* if */
	} /* if */
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			sprintf (window->name, module->object_name);
			window->edit_obj	= find_flags (save, ROOT, EDITABLE);
			window->edit_inx	= NIL;
			set_save (filename);
		} /* if */
		
		if (! open_dialog (100+SAVE)) hndl_alert (ERR_NOOPEN);
		
		/* Zahl Åbernehmen klappt nur, wenn D-Box modal! */
		switch (window->exit_obj)
		{
			case SAVSAVE   :	(module->save)(module, filename, FALSE);	break;
			case SAVSAVEAS :	(module->save)(module, filename, TRUE);	break;
			case SAVCANCEL :	break;
		} /* switch */
	} /* if */
} /* dsave */
/*****************************************************************************/
LOCAL VOID get_save ()
{
} /* get_save */

/*****************************************************************************/

LOCAL VOID set_save (STR128 filename)
{
  set_ptext (save, SAVFILE, filename);
} /* set_save */

/*****************************************************************************/

LOCAL VOID click_save (window, mk)
WINDOWP window;
MKINFO  *mk;

{
} /* click_save */

/*****************************************************************************/

LOCAL BOOLEAN key_save (window, mk)
WINDOWP window;
MKINFO  *mk;

{

  switch (window->edit_obj)
  {
  } /* switch */

  return (FALSE);
} /* key_save */

/*****************************************************************************/

LOCAL BOOLEAN help_save (window, icon)
WINDOWP window;
WORD    icon;

{
	help_rtm ("SPEICHERN");
	return (TRUE);
} /* help_save */

/*****************************************************************************/

GLOBAL BOOL dlogin ()
{
	WINDOWP window;
	WORD    ret;
	STRING	title;
	
	sprintf (title, " Login ");
	
	window = search_window (CLASS_DIALOG, SRCH_ANY, 100+LOGIN);
	
	form_center (login, &ret, &ret, &ret, &ret);

	if (window == NULL)
	{
		window = crt_dialog (login, NULL, 100+LOGIN, title, WI_MODAL);
		
		if (window != NULL)
		{
			/* window->showhelp = help_login; */
		} /* if */
	} /* if */
	
	if (window != NULL)
	{
		if (window->opened == 0)
		{
			sprintf (window->name, title);
			window->edit_obj	= LOGUSERNAME;
			window->edit_inx	= NIL;
			window->work.x    = window->scroll.x;
			window->work.y    = window->scroll.y;
			window->work.w    = window->scroll.w;
			window->work.h    = window->scroll.h;
			window->special	= FALSE;
			window->click		= click_login;
			window->key			= key_login;
			window->showhelp = help_login;
			set_login ();
		} /* if */
		
		/* Ergebnis Åbernehmen klappt nur, wenn D-Box modal! */
		while (!window->special && log_count < MAXLOGINTRIALS)
		{
			if (! open_dialog (100+LOGIN)) hndl_alert (ERR_NOOPEN);
			switch (window->exit_obj)
			{
				case LOGCANCEL :
					return FALSE;
			} /* switch */
		} /* while */
	} /* if */
	return window->special;
} /* dlogin */

LOCAL VOID get_login (CHAR *user, CHAR *password)
{
  GetPText (login, LOGUSERNAME, user);
  /* GetPText (login, LOGPASSWORD, password); */
} /* get_login */

LOCAL VOID set_login ()
{
  SetPText (login, LOGUSERNAME, "");
  SetPText (login, LOGPASSWORD, "");
} /* set_login */

LOCAL VOID click_login (WINDOWP window, MKINFO *mk)
{
	switch (window->exit_obj)
	{
		case LOGPASSWORD : 
			edit_inx = window->edit_inx;
			break;
		case LOGOK     :
			get_login (user, password);
			window->special = try_login (user, password);
			if (!window->special)
			{
				set_login();
				redraw_window (window, &window->work);
				log_count++;
			}
			break;
		case LOGCLEAR :
			undo_state (window->object, window->exit_obj, SELECTED);
			window->edit_obj	= LOGUSERNAME;
			window->edit_inx	= NIL;
			set_login ();
			draw_object (window, ROOT);
			break;
		case LOGHELP   :
			help_login (NULL, NIL);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
	} /* switch */
} /* click_login */

LOCAL BOOL try_login (CHAR *user, CHAR *password)
{
	/* Provisorisch: einfacher String-Vergleich */
	if (strcmp (user, password) == 0)
		return TRUE;
	else
		return FALSE;
} /* try_login */

LOCAL BOOLEAN key_login (WINDOWP window, MKINFO  *mk)
{
	
	switch (window->edit_obj)
	{
		case LOGPASSWORD :
			if (window->exit_obj == 0)  /* durch Tastatur, nicht Maus */
			edit_noecho (mk, edit_inx, password, MAX_PASSWORD);
			edit_inx = window->edit_inx;
			break;
	} /* switch */
	
	return (FALSE);
} /* key_login */

LOCAL BOOLEAN help_login (WINDOWP window, WORD icon)
{
	help_rtm ("LOGIN");
	return (TRUE);
} /* help_login */

/*****************************************************************************/

GLOBAL VOID mkstate( MKINFO *mk)
{
	/* Holt aktuelle MKINFO */
	WORD ret;
	
	graf_mkstate (&ret, &ret, &mk->mobutton, &mk->kstate); /* Werte nach Ereignis */

#if MSDOS
    if (mk.momask == 0x0000) mk.momask = 0x0001;         /* Irgendein Knopf ist linker Knopf */
#endif

    mk->shift  = (mk->kstate & (K_RSHIFT | K_LSHIFT)) != 0;
    mk->ctrl   = (mk->kstate & K_CTRL) != 0;
    mk->alt    = (mk->kstate & K_ALT) != 0;
} /* mkstate */

GLOBAL LONG ClickValueField (WINDOWP window, WORD object, MKINFO *mk, LONG min_val, LONG max_val, UpdateFieldFn update)
{
	/* Ein Zahlen-Feld in einer DBOX wird per Mausclick verÑndert */
	LONG	x, step;
	BOOL	repeat;
	
	undo_state (window->object, object, SELECTED);
	if (mk->breturn < 2)
	{
		/* Einfach-Click links oder rechts = dec/inc */
		GetPLong (window->object, object, &x);
		do{
			step = 1;
			if (mk->alt)	step *= 10;
			if (mk->shift)	step *= 10;
			if (mk->ctrl)	step *= 10;
			switch (mk->momask)
			{
				case 0x001:		/* linke Taste */
					x-= step;
					break;
				case 0x002:	/* rechte Taste */
					x+= step;
					break;
			} /* switch */
			
			/* Grenzen berÅcksichtigen */
			if (x > max_val) x = min_val;
			if (x < min_val) x = max_val;
			update (window, object, x);

			/* PrÅfen ob Taste gedrÅckt */
			mkstate(mk);
			if (mk->mobutton>0 && mk->momask>0)	delay (100);
			
			/* PrÅfen ob Taste immernoch gedrÅckt */
			mkstate(mk);
			repeat =  (mk->mobutton>0 && mk->momask>0);
		} while (repeat);
	} /* if */
	else if (mk->breturn == 2)
	{
		/* Doppel-Click = Eingabe Åber Tastatur */
		GetPLong (window->object, object, &x);
		dzahl (&x, mk->mox, mk->moy, " Zahl ");
		if (x > max_val) x = max_val;
		if (x < min_val) x = min_val;
		update (window, object, x);
	} /* else */

	return x;
} /* ClickValueField */

GLOBAL LONG ClickSetupField (WINDOWP window, WORD object, MKINFO *mk)
{
	/* Setup-Nummern-Feld verwalten */
	RTMCLASSP	module = (RTMCLASSP)window->module;
	LONG	x, step;
	BOOL	repeat;
	LONG	max_val = module->max_setups;
	
	undo_state (window->object, object, SELECTED);
	if (mk->breturn < 2)
	{
		/* Einfach-Click links oder rechts = dec/inc */
		GetPLong (window->object, object, &x);
		do{
			step = 1;
			if (mk->alt)	step *= 10;
			if (mk->shift)	step *= 10;
			if (mk->ctrl)	step *= 10;
			switch (mk->momask)
			{
				case 0x001:		/* linke Taste */
					x-= step;
					break;
				case 0x002:	/* rechte Taste */
					x+= step;
					break;
			} /* switch */
			
			/* Grenzen berÅcksichtigen */
			if (x > max_val) x = 0;
			if (x < 0) x = max_val;
			module->set_nr (window, x);
			timer_all(1);

			/* PrÅfen ob Taste gedrÅckt */
			mkstate(mk);
			if (mk->mobutton>0 && mk->momask>0)	delay (100);
			
			/* PrÅfen ob Taste immernoch gedrÅckt */
			mkstate(mk);
			repeat =  (mk->mobutton>0 && mk->momask>0);
		} while (repeat);
	} /* if */
	else if (mk->breturn == 2)
	{
		/* Doppel-Click = Eingabe Åber Tastatur */
		GetPLong (window->object, object, &x);
		/*
			dzahl (&x, mk->mox, mk->moy, " Zahl ");
		*/
		module->set_nr (window, x);
		draw_object (window, ROOT);
	} /* else */
	return x;
} /* ClickSetupField */

GLOBAL VOID UpdateValueField (WINDOWP window, WORD obj, LONG value)
{
	/* Standard-Funktion fÅr Werte-Update,
		Setzt neuen Wert in DBOX ein und fÅr Update auf Screen durch */
		
	SetPLong (window->object, obj, value);
	draw_object(window, obj);
} /* UpdateValueField */

GLOBAL LONG ClickTimeField (WINDOWP window, WORD object, MKINFO *mk, LONG min_val, LONG max_val, UpdateFieldFn update)
{
	/* Ein Zeit-Feld in einer DBOX wird per Mausclick verÑndert */
	LONG	x, step, unit;
	BOOL	repeat;
	
	switch (window->exit_obj - object)
	{
		case 1:
			unit = 3600000L;	/* Stunden */
			break;
		case 2:
			unit = 60000L;		/* Minuten */
			break;
		case 3:
			unit = 1000;			/* Sekunden */
			break;
		case 4:
			unit = 1;				/* Milli-Sekunden */
			break;
		default:
			unit = 1;				/* */
	} /* switch */

	undo_state (window->object, object, SELECTED);
	if (mk->breturn < 2)
	{
		/* Einfach-Click links oder rechts = dec/inc */
		GetPTime (window->object, object, &x);
		do{
			step = unit;
			if (mk->shift)	step = unit*10;
			if (mk->ctrl)	step = unit*100;
			if (mk->alt)	step = QUANT;
			switch (mk->momask)
			{
				case 0x001:		/* linke Taste */
					x-= step;
					break;
				case 0x002:	/* rechte Taste */
					x+= step;
					break;
			} /* switch */
			
			if (max_val > min_val)
			{
				/* Grenzen berÅcksichtigen */
				if (x > max_val) x = min_val;
				if (x < min_val) x = max_val;
			} /* if */
			update (window, object, x);

			/* PrÅfen ob Taste gedrÅckt */
			mkstate(mk);
			if (mk->mobutton>0 && mk->momask>0)	delay (100);
			
			/* PrÅfen ob Taste immernoch gedrÅckt */
			mkstate(mk);
			repeat =  (mk->mobutton>0 && mk->momask>0);
		} while (repeat);
	} /* if */
	else if (mk->breturn == 2)
	{
		/* Doppel-Click = Eingabe Åber Tastatur */
		GetPTime (window->object, object, &x);
		dzeit (&x, mk->mox, mk->moy, " Zahl ");
		update (window, object, x);
	} /* else */

	return x;
} /* ClickTimeField */

GLOBAL VOID UpdateTimeField (WINDOWP window, WORD obj, LONG value)
{
	/* Standard-Funktion fÅr Werte-Update,
		Setzt neuen Wert in DBOX ein und fÅr Update auf Screen durch */
		
	SetPTime (window->object, obj, value);
	draw_object(window, obj);
} /* UpdateValueField */

/*****************************************************************************/

GLOBAL BOOLEAN help_rtm (STRING keyword )
{
	INT		msg_buff[8], acc_id;
	BOOLEAN	ok = TRUE;
	static STRING help_key;		/* Speichert dauerhaft den Helptext 
											fÅr ACC-Zugriff */
	CHAR		*c;
	INT		i;
		
	acc_id = appl_find (AC_NAME);

	if( acc_id < 0 )
	{	/* non, therefore no help available */
		form_alert( 1, "[3][|" AC_NAME "|not found.][ OK ]" );
		ok = FALSE;
	}
	else
	{
		if (keyword)
		{
			for (i = 0; keyword[i] == ' '; i++); 	/* Space Åberspringen */
			strcpy (help_key, keyword+i);		/* Name kopieren */
			c = strrchr (help_key, ' ');		/* Hinteres Space	suchen */
			if (c) *c = 0;							/* String abschneiden */
			
			msg_buff[0] = AC_HELP;					/* magic message number 	*/
			msg_buff[1] = gl_apid;					/* my own id				*/
			msg_buff[2] = 0;							/* no more than 16 bytes	*/
			*(char **)&msg_buff[3] = help_key;	/* the Key Word			*/
	
			ok &= appl_write( acc_id, 16, msg_buff );  /* write message			*/
	/*
			/* wait for reply */
			do
			{
				evnt_mesag( msg_buff );
			} while ( msg_buff[0] != AC_REPLY );
	*/	
		
		} /* if */
	} /* else */
	return ok;
} /* help */

GLOBAL VOID update_ptext (WINDOWP window, WORD object, INT newval, INT oldval, STRING format, BOOLEAN force_draw)
{
	STRING s;
	
	if (window)
	{
		if (newval != oldval)
		{
			sprintf (s, format, newval);
			set_ptext (window->object, object, s);
		} /* if */
		if (newval != oldval || force_draw)
			draw_object (window, object);
	} /* if */
} /* update_ptext */

GLOBAL VOID update_checkbox (WINDOWP window, WORD object, BOOLEAN new, BOOLEAN old, BOOLEAN force_draw)
{
	if (window)
	{
		if (new != old)
			set_checkbox (window->object, object, new);
		if (new != old || force_draw)
			draw_object (window, object);
	} /* if */
} /* update_checkbox */

/*****************************************************************************/

GLOBAL BOOLEAN controller_out (SHORT refnum, WORD port, WORD channel, WORD controller, WORD value)
{
	MidiEvPtr 	e;
	REG LONG 	id = controller, val = value;
	
	/* Ausgabe eines Control-Change Events */
	e = MidiNewEv (typeCtrlChange);
	if (e)
	{
		Chan (e)	= channel;
		Port (e) = port;
		SetContID (e, id);
		SetContValue (e, val);
		MidiSendIm (refnum, e);
		return TRUE;
	} /* if e */
	return FALSE;
} /* controller_out */

/*****************************************************************************/

GLOBAL VOID swap_int (INT *i1, INT *i2)
{
	INT temp = *i1;
	
	*i1 = *i2;
	*i2 = temp;
} /*  swap_int */

GLOBAL VOID swap_word (WORD *w1, WORD *w2)
{
	WORD temp = *w1;
	
	*w1 = *w2;
	*w2 = temp;
} /*  swap_word */

GLOBAL VOID swap_byte (BYTE *w1, BYTE *w2)
{
	BYTE temp = *w1;
	
	*w1 = *w2;
	*w2 = temp;
} /*  swap_byte */
/*****************************************************************************/

GLOBAL LONG		minmaxsetup _((LONG *value, LONG maxvalue))
{
	/* Grenzt Werte ab, Åber maxvalue gibt 0, -1 gibt wieder maxvalue */
	*value= (*value<maxvalue) ? *value : 0;
	*value= (*value>=0)       ? *value : maxvalue+*value;
	return *value;
}

/*****************************************************************************/

GLOBAL VOID copy_icon _((OBJECT *dobj, OBJECT *sobj))
{
	/* ICONBLK *siconblk, *diconblk;

	siconblk = (ICONBLK *) sobj.ob_spec;
	diconblk = (ICONBLK *) dobj.ob_spec;
	
	*diconblk = *siconblk;*/

	(ICONBLK *)dobj->ob_spec = (ICONBLK *) sobj->ob_spec;
} /* copy_icon */

/*****************************************************************************/
GLOBAL	void	getpath(char *s)	/* holt den ganzen Pfad incl. Laufwerksbuchstaben */
{
	int drv;

	drv = Dgetdrv();										/* aktuellen Pfad setzen */
	s[0] = 'A'+drv;
	s[1] = ':';
	Dgetpath(s + 2, drv + 1);
}

GLOBAL	void	cutpath(char *s)
{
	s=strrchr(s, '\\');	/* Extender oder Filename abschneiden */
	if(s)
		*s=0;
}
/*****************************************************************************/
/* Speicherverwaltung ohne TOS-Aufrufe                                       */
/*****************************************************************************/

GLOBAL VOID *mem_setx (dest, val, len)
VOID  *dest;
WORD  val;
UWORD len;

{
  REG UBYTE *d;

  for (d = (UBYTE *)dest; len > 0; len--) *d++ = (UBYTE)val;

  return (dest);
} /* mem_set */

/*****************************************************************************/

GLOBAL VOID *mem_movex (dest, src, len)
VOID       *dest;
CONST VOID *src;
UWORD      len;

{
  REG UBYTE *s, *d;
  REG UWORD l;

  s = (UBYTE *)src;
  d = (UBYTE *)dest;
  l = len;

  if ((s < d) && (s + l > d))
    for (d += l, s += l; l > 0; l--) *(--d) = *(--s);
  else 
    for (; l > 0; l--) *d++ = *s++;

  return (dest);
} /* mem_movex */

/*****************************************************************************/

GLOBAL VOID *mem_lsetx (dest, val, len)
VOID  *dest;
WORD  val;
ULONG len;

{
  REG UBYTE HUGE *d;

  if (len < 0x00010000L)
    mem_setx (dest, val, (UWORD)len);
  else
    for (d = (UBYTE HUGE *)dest; len > 0; len--) *d++ = (UBYTE)val;


  return (dest);
} /* mem_lsetx */

/*****************************************************************************/

GLOBAL VOID *mem_lmovex (dest, src, len)
VOID       *dest;
CONST VOID *src;
ULONG      len;

{
  REG UBYTE HUGE *s;
  REG UBYTE HUGE *d;
  REG ULONG       l;

  if (len < 0x00010000L)
    mem_movex (dest, src, (UWORD)len);
  else
  {
    s = (UBYTE HUGE *)src;
    d = (UBYTE HUGE *)dest;
    l = len;

    if ((s < d) && (s + l > d))
      for (d += l, s += l; l > 0; l--) *(--d) = *(--s);
    else 
      for (; l > 0; l--) *d++ = *s++;
  } /* else */

  return (dest);
} /* mem_lmovex */

LOCAL VOID init_waveform()
{
	WORD winkel, waveform;
	
	daktstatus(" Oszillatoren ", "Wellenformen berechnen ...");

	/* Wertebereich = -Ampli/2 ... +Ampli/2 */
	for (waveform = 0; waveform < WAVES; waveform++)
	{
		for (winkel = 0; winkel < WAVESTEPS; winkel++)
		{
			switch (waveform)
			{
				case WSINUS:
					wave[winkel][waveform] = sin(2*M_PI*winkel/WAVESTEPS)*WAVEAMPL;
					break;
				case WTRIANGLE:
					/* Auf-Ab-Ab-Auf */
					if (winkel < WAVESTEPS/4)
						wave[winkel][waveform] = winkel * 4 * WAVEAMPL / WAVESTEPS;
					else if (winkel >= WAVESTEPS/4 && winkel <(3*WAVESTEPS/4))
						wave[winkel][waveform] = WAVEAMPL - (winkel * 4 * WAVEAMPL) / WAVESTEPS;
					else
						wave[winkel][waveform] = (winkel * 4 * WAVEAMPL) / WAVESTEPS - 4* WAVEAMPL;
					break;
				case WSAWUP:
					wave[winkel][waveform] =  winkel * 2 * WAVEAMPL/WAVESTEPS - WAVEAMPL;
					break;
				case WSAWDOWN:
					wave[winkel][waveform] =  (WAVESTEPS - winkel) * 2 * WAVEAMPL/WAVESTEPS - WAVEAMPL;
					break;
				case WSQUARE:
					if (winkel < WAVESTEPS/2)
						wave[winkel][waveform] =  WAVEAMPL;
					else
						wave[winkel][waveform] = -WAVEAMPL;
					break;
				case WKANTE:
					/* Spezial-Welle fÅr "Kanten-Rotation" */
					if ((winkel <= WAVESTEPS/8) || (winkel > (7*WAVESTEPS)/8))
						wave[winkel][WKANTE] =   WAVEAMPL;
					if ((winkel >  WAVESTEPS/8) && (winkel <= (3*WAVESTEPS)/8))
						wave[winkel][WKANTE] =   WAVEAMPL*((WAVESTEPS/4)-winkel)/(WAVESTEPS/8);
					if ((winkel >  (3*WAVESTEPS/8)) && (winkel <= (5*WAVESTEPS)/8))
						wave[winkel][WKANTE] = - WAVEAMPL;
					if ((winkel >  (5*WAVESTEPS/8)) && (winkel <= (7*WAVESTEPS)/8))
						wave[winkel][WKANTE] = - WAVEAMPL*((3*WAVESTEPS/4)-winkel)/45;
					break;
			} /* switch */
		} /* for */
		set_daktstat(100*(waveform+1)/WAVES);
	} /* for */
	close_daktstat();
} /* init_waveform */

LOCAL VOID init_sinq()
{
	WORD winkel;
	
	/* Lookup-Tables fÅr Sinus und Cosinus generieren */
	for (winkel = 0; winkel < 360; winkel++)
	{
		sinq[winkel] = sin(Rad(winkel));
		cosq[winkel] = cos(Rad(winkel));
	} /* for winkel */
} /* init_sinq */

/*****************************************************************************/
LOCAL BOOLEAN init_rsc_realtspc ()
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
  zahl	   = (OBJECT *)rs_trindex [ZAHL];		/* Adresse der Zahl-Eingabebox */
  zeit	   = (OBJECT *)rs_trindex [ZEIT];		/* Adresse der Zahl-Eingabebox */
  save	   = (OBJECT *)rs_trindex [SAVE];		/* Adresse der Save-Eingabebox */
  aktstat	= (OBJECT *)rs_trindex [AKTSTAT];	/* Adresse der Aktuell-Status-Box */
  login	 	= (OBJECT *)rs_trindex [LOGIN];		

#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (rtm_rsc_ptr, rsc_name))
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

  rs_gaddr (spc_rsc_ptr, R_TREE,  ZAHL,   	&zahl);			 /* Adresse der Zahl-Eingabebox */
  rs_gaddr (spc_rsc_ptr, R_TREE,  ZEIT,   	&zeit);			 /* Adresse der Zeit-Eingabebox */
  rs_gaddr (spc_rsc_ptr, R_TREE,  SAVE,   	&save);			 /* Adresse der Save-Eingabebox */
  rs_gaddr (spc_rsc_ptr, R_TREE,  AKTSTAT, &aktstat);		 /* Adresse der Aktuell-Status-Box */
  rs_gaddr (spc_rsc_ptr, R_TREE,  DLOGIN,	&login);
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif


  fix_objs (zahl,	  	TRUE);
  fix_objs (zeit,	 	TRUE);
  fix_objs (save,	 	TRUE);
  fix_objs (login, 	TRUE);
  fix_objs (aktstat,	TRUE);

  do_flags (login, LOGPASSWORD, NOECHO_FLAG);
  do_flags (login, LOGCANCEL, UNDO_FLAG);
  do_flags (login, LOGHELP,   HELP_FLAG);

  do_flags (aktstat, AKTCANCEL, UNDO_FLAG);

/*
	menu_enable(menu, MRTM, TRUE);
*/
	return (TRUE);
} /* init_rsc_realtspc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

LOCAL BOOLEAN term_rsc_realtspc ()

{
  BOOLEAN ok = TRUE;
/*
#if ((XRSC_CREATE|RSC_CREATE) == 0)
  ok = rs_free (spc_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif
*/
  return (ok);
} /* term_rsc_rtm */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_realtspc ()

{
	BOOLEAN ok = TRUE;
	ok &= init_rsc_realtspc ();
	init_waveform ();	
	init_sinq();
	set_helpfunc (help_rtm);		/* Help-Funktion fÅr Geiss-Paket einklinken */	
	return (ok);
} /* init_realtspc */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_realtspc ()
{
	BOOLEAN ok = TRUE;
	ok &= term_rsc_realtspc ();
	return (ok);
} /* term_realtspc */
