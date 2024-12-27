/*****************************************************************************/
/*                                                                           */
/* Modul: OBJECTS.C                                                          */
/* Datum: 02.02.95                                                           */
/*                                                                           */
/* Objekt- und Klassen-Routinen                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- speicher mit mem_lset gelîscht in set_setnr_obj, 02.02.95
- load_obj und save_obj ohne ftell und relativen seek, 24.01.95
- get/set_setnr_obj vor öberlauf auch bei SETUPS_EXTERN geschÅtzt, 18.01.95
- Fehler-Meldungen auf ERR_Mxxx umgestellt in get/set_setnr_obj sowie load_obj und save_obj, 18.01.95
28.11.94
- EC4 eingebaut
- ED4 eingebaut
- mem_lset fÅr rtmmodule initialisierung in init_modules
24.08.94
- CMO ans Ende der Initialisierung
03.05.94
- gmi eingebaut
- load/save_info_obj eingebaut und abgeschaltet
22.02.94
- LFO in Initialisierung nach hinten gelegt, wg. MAN Reihenfolge
- neue Fehler-Meldungen ERR_MOPEN, ERR_MFREAD, ERR_MWRITE
- CMO-Modul eingebaut
- Standard-Funktionen in module_create
- test umgebaut
- destroy_obj mit mem_free fÅr Datenstruktur-Pointer
- create_window_obj eingebaut
- save_obj ohne set_dakstat(0)
- showhelp_obj und help_obj getrennt
24.06.93
- RTMCLASS Manipulation eingebaut
- copy_icon bei init eingebaut
- help_obj mit NULL-öberprÅfung
17.04.93
- MSH als eigenstÑndiges Modul, Åbernimmt MidiShare Start und Initialisierung
*****************************************************************************/

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
#include "objects.h"

/****** DEFINES **************************************************************/
/* Macro fÅr Setup Addressierung, ersetzt setup[setupnr] */
#define Setup(setupnr) (VOID *)((BYTE *)module->setups + (module->setup_length * setupnr))

/****** TYPES ****************************************************************/
typedef struct setup
{
	VOID *dummy;
} SETUP;				/* Dummy Definition */

typedef struct status
{
	VOID *dummy;
} STATUS;			/* Dummy Definition */

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/
LOCAL WORD find_moduleslot	_((RTMCLASSP module));
LOCAL VOID box 				_((WINDOWP window, BOOLEAN grow));

/****** FUNCTIONS ************************************************************/
GLOBAL VOID    get_edit_obj	(RTMCLASSP module)
{
	ED_P		actual = module->actual;
	ED_P		edited = module->edited;

	/* Daten zur aktuellen Verwendung kopieren */
	mem_lmove(actual->setup, edited->setup, module->setup_length);
	actual->modified		= edited->modified;
	actual->number			= edited->number;

	if (module->send_messages) module->send_messages(module);
	if (module->reset) module->reset(module);
} /* get_edit_obj */

GLOBAL VOID	set_edit_obj		(RTMCLASSP module)
{
	ED_P	actual = module->actual;
	ED_P	edited = module->edited;

	mem_lmove(edited->setup, actual->setup, module->setup_length);
	edited->modified	= actual->modified;
	edited->number		= actual->number;
} /* set_edit_obj */

GLOBAL BOOLEAN	get_setnr_obj	(RTMCLASSP module, LONG setupnr)
{
	/* Setup-Speicher auf Platte / im RAM mit neuer Einstellung fÅllen */
	/* akt->setups[] , bzw. akt->Datei */
	
	ED_P		actual = module->actual;
	VOID		*akt = actual->setup;
	LONG		position, file_pos;
	BOOLEAN	ok = TRUE;
	
	if (setupnr>0)
	{
		minmaxsetup(&setupnr, module->max_setups);
		switch (module->location)
		{
			case SETUPS_INTERN:
				mem_lmove(Setup(setupnr), actual->setup, module->setup_length);
				module->ram_modified |= TRUE;
				break;
			case SETUPS_EXTERN:
				/* Hier speichern in Datei */
				if (module->file_pointer > 0L)
				{
					position = module->file_header_len + module->setup_length * setupnr;
#if TRUE
					/* Versuchsweise direkt */
					ok = ! fseek (module->file_pointer, position, SEEK_SET);
#else
					file_pos = ftell (module->file_pointer);
					if (file_pos < position)
					{
						ok = ! fseek (module->file_pointer, position-file_pos, SEEK_CUR);
					} /* else */
					else
					{
						ok = ! fseek (module->file_pointer, position, SEEK_SET);
					} /* else */
#endif
					if (ok)
					{
						/* fwrite liefert Anzhal der geschriebenen Records zurÅck */
						ok =  fwrite(akt, module->setup_length, 1, module->file_pointer);
						if (ok)
						{
							actual->number		= setupnr;
							actual->modified	= FALSE;
						} /* if */
					} /* if */
					if (!ok)	 hndl_alert_obj (module, ERR_MFWRITE);
				} /* if */
				else
				{
					ok = FALSE;
					hndl_alert_obj (module, ERR_NOEXTERN);
				} /* else */
		} /* switch */
	} /* if */
	else
	{
		/* Standard-Setup ist unverÑnderlich */
		ok = FALSE;
		hndl_alert_obj (module, ERR_STANDARD);
	} /* else */
	return ok;
} /* get_setnr_obj */

GLOBAL BOOLEAN	set_setnr_obj	(RTMCLASSP module, LONG setupnr)
{
	/* Akt. Setup von Platte / aus RAM mit neuer Einstellung fÅllen */
	/* setups[]->akt , bzw. Datei->akt */
	
	ED_P	actual = module->actual;
	ED_P	edited = module->edited;
	BOOL	update_edit = !edited->modified;
	LONG	position, file_pos;
	VOID	*akt = actual->setup;
	BOOLEAN	ok = TRUE;
	WINDOWP	window = Window(module);
	
	if (setupnr>0)
	{
		minmaxsetup(&setupnr, module->max_setups);
		switch (module->location)
		{
			case SETUPS_INTERN:
				/* Daten in Setup-Array kopieren */
				mem_lmove(actual->setup, Setup(setupnr), module->setup_length);
				actual->number		= setupnr;
				actual->modified	= FALSE;
				break;
			case SETUPS_EXTERN:
				/* Hier laden aus Datei */
				if (module->file_pointer > 0)
				{
					position = module->file_header_len + module->setup_length * setupnr;
#if TRUE
					/* Versuchsweise direkt */
					ok = ! fseek (module->file_pointer, position, SEEK_SET);
#else
					file_pos = ftell (module->file_pointer);
					if (file_pos < position)
					{
						ok = ! fseek (module->file_pointer, position-file_pos, SEEK_CUR);
					} /* else */
					else
					{
						ok = ! fseek (module->file_pointer, position, SEEK_SET);
					} /* else */
#endif
					if (ok)
					{
						/* Vorsichtshalber Speicherbereich lîschen */
						mem_lset(actual->setup, 0, module->setup_length);
						/* fread liefert Anzahl der geschriebenen Records zurÅck */
						ok =  fread(akt, module->setup_length, 1, module->file_pointer);
						if (ok)
						{
							actual->number		= setupnr;
							actual->modified	= FALSE;
						} /* if */
					} /* if */
					if (!ok) hndl_alert_obj (module, ERR_MFREAD);
				} /* if */
				else
				{
					hndl_alert_obj (module, ERR_NOEXTERN);
				} /* else */
				break;
		} /* switch */
	} /* if */
	else
	{
		/* 0-Setup wird ersetzt durch Standard-Setup */
		mem_lmove(actual->setup, module->standard, module->setup_length);
		setupnr = 0;
		actual->number		= setupnr;
		actual->modified	= FALSE;
	} /* else */

	/* Nur wenn nicht gerade importiert wird */
	if (!(module->flags & FLAG_IMPORTING))
	{
		/* Editor updaten */
		if (update_edit)
		{
			if (module->set_edit) module->set_edit (module);
			if (module->set_dbox) module->set_dbox (module);
			draw_object(window, ROOT);
		} /* if update_edit */
				
		if (module->send_messages) module->send_messages(module);
		if (module->reset) module->reset(module);
	} /* if */
	return ok;
} /* set_setnr_obj */

GLOBAL VOID set_nr_obj (WINDOWP window, LONG setupnr)
{
	RTMCLASSP	module = window->module;
	ED_P	edited = module->edited;
	BOOL	update_edit = edited->modified;
	
	undo_state (window->object, window->exit_obj, SELECTED);
	module->set_setnr(module, setupnr);
	if (update_edit)
	{
		module->set_edit(module);
		module->set_dbox(module);
		draw_object(window, ROOT);
	}
} /* set_nr_obj */

GLOBAL VOID set_store_obj (WINDOWP window, LONG setupnr)
{
	RTMCLASSP	module = window->module;
	STRING		title;
	
	undo_state (window->object, window->exit_obj, SELECTED);

	sprintf (title, "%s-Setup", module->object_name); 
	dzahl(&setupnr, window->work.x + window->object[window->exit_obj].ob_x, window->work.y + window->object[window->exit_obj].ob_y, title);
	
	module->get_dbox(module);
	module->get_edit(module);
	module->get_setnr(module, setupnr);
	module->set_setnr(module, setupnr);
	module->set_edit(module);
	module->set_dbox(module);
	draw_object(window, ROOT);
} /* set_store_obj */

GLOBAL VOID set_recall_obj (WINDOWP window, LONG setupnr)
{
	RTMCLASSP	module = window->module;
	STRING		title;
	
	undo_state (window->object, window->exit_obj, SELECTED);

	sprintf (title, "%s-Setup", module->object_name); 
	dzahl(&setupnr, window->work.x + window->object[window->exit_obj].ob_x, window->work.y + window->object[window->exit_obj].ob_y, title);

	module->set_setnr(module, setupnr);
	module->set_edit(module);
	module->set_dbox(module);
	draw_object(window, ROOT);
} /* set_recall_obj */

GLOBAL VOID set_ok_obj (WINDOWP window)
{
	RTMCLASSP	module = window->module;

	undo_state (window->object, window->exit_obj, SELECTED);

	module->get_dbox(module);
	module->get_edit(module);
	module->set_edit(module);
	module->set_dbox(module);
	draw_object(window, ROOT);
} /* set_ok_obj */

GLOBAL VOID set_cancel_obj (WINDOWP window)
{
	RTMCLASSP	module = window->module;

	undo_state (window->object, window->exit_obj, SELECTED);

	module->set_edit(module);
	module->set_dbox(module);
	/* draw_object(window, ROOT); nicht nîtig, wird von WINDOWS Åbernommen */
} /* set_ok_obj */

GLOBAL VOID set_standard_obj (WINDOWP window)
{
	RTMCLASSP	module = window->module;

	undo_state (window->object, window->exit_obj, SELECTED);

	module->set_setnr(module, 0);
	module->set_edit(module);
	module->set_dbox(module);
	draw_object(window, ROOT);
} /* set_standard_obj */

GLOBAL BOOLEAN	load_obj	(RTMCLASSP module, STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	FILE		*in;
	STR128	s, title, filter, header, ext;
	LONG		x, max_setups = module->max_setups, setup_length = module->setup_length;
	LONG		number;
	BYTE		*setups;
				
	if (filename == NULL)
	{
		filename = s;
		strcpy (filename, module->file_name);
	} /* if */

	if (fileselect)
	{
		file_split (module->file_name, NULL, setup_path, filename, NULL);
		strcpy(ext, module->file_extension);
		sprintf (filter, "*.%s", ext);
		sprintf (title, "%s-Datei laden", module->object_name);
		ok = select_file (filename, setup_path, filter, title, module->file_name);
	} /* if */

	if (ok)
	{
		/* Alte Datei schliessen,  wenn nîtig */
		if (module->file_status & FILE_OPENED)
		{
			module->file_status &= ~FILE_OPENED;
			fclose (module->file_pointer);
		} /* if */

		/* Datei îffnen fÅr lesen+schreiben, wenn vorhanden */
		in = fopen(module->file_name, "r+b");
		if(in == 0)
		{	
			switch (module->location)
			{
				case SETUPS_INTERN:
		      	hndl_alert_obj (module, ERR_MFOPEN);
		      	ok = FALSE;
		      	break;
		      case SETUPS_EXTERN:
		      	switch (hndl_alert_obj (module, ERR_NOEXTERN))
		      	{
		      		case 1:	/* Neue Datei erstellen */
							sprintf (title, "%s-Datei wird erstellt", module->object_name);
							daktstatus (title, module->file_name);
							module->file_pointer = fopen(module->file_name, "w+b");
		      			fputs (module->file_version, module->file_pointer);
		      			for (x = 0; ok && x < max_setups; x++)
		      			{
								if (fwrite(module->standard, setup_length, 1, module->file_pointer) != 1)
								{
									hndl_alert_obj (module, ERR_FWRITE);
									ok = FALSE;
								} /* if */
								if (x % 100 == 0)
									set_daktstat((WORD)(x*100/max_setups));
							} /* for */
							/* Datei schliessen, Schutz vor AbstÅrzen */
							fclose (module->file_pointer);
							if (ok)
								set_daktstat(100);
							close_daktstat();
							in = fopen(module->file_name, "r+b");
		      			break;
		      		case 2:
		      			module->location = SETUPS_INTERN;
		      			break;
		      	} /* switch */
		      	break;
      	} /* switch */
		} /* if */
		
		if(in != 0)
		{
			module->file_status |= FILE_OPENED;
			fgets(header, (INT) sizeof(header), in);
			if (strcmp(header, module->file_version) != 0)
			{
				if (feof (in))
		      	hndl_alert_obj (module, ERR_MFREAD);
		      else
		      	hndl_alert_obj (module, ERR_FILEFORMAT);
				fclose(in);
			} /* if */
			else
			{
				switch (module->location)
				{
					case SETUPS_INTERN:
						if (max_setups > 0)
						{
							sprintf (title, "%s-Datei wird geladen", module->object_name);
							daktstatus (title, module->file_name);
							setups = (BYTE*)module->setups;
							for (x = 0; x < max_setups; x+=20)
							{
								set_daktstat((WORD)(x*100/max_setups));
								number = min(20, max_setups-x);
								fread(setups + x * setup_length, setup_length, number, in);
							} /* for */
							/* fread(module->setups, module->setup_length, max_setups, in); */
							set_daktstat(100);
						} /* if */
						module->ram_modified = FALSE;
						fclose(in);
						module->file_status &= ~FILE_OPENED;
						close_daktstat();
						break;
					case SETUPS_EXTERN:
						sprintf (title, "%s-Datei wird geîffnet", module->object_name);
						daktstatus (title, module->file_name);
						module->file_pointer = in;			/* Filepointer merken */
						module->file_header_len = strlen(header);	/* Vorspann */
						/* module->file_max = */
						set_daktstat(100);
						close_daktstat();
						break;
				} /* switch */
			} /* else */
		} /* if */
	} /* if */

	return (ok);
} /* load_obj */

GLOBAL BOOLEAN	save_obj	(RTMCLASSP module, STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	FILE		*out;
	STR128	s, title, filter, ext;
	LONG		x, max_setups = module->max_setups, setup_length = module->setup_length;
	LONG		number;
	BYTE		*setups;

	if (filename == NULL)
	{
		filename = s;
		strcpy (filename, module->file_name);
	} /* if */

	if (fileselect)
	{
		file_split (module->file_name, NULL, setup_path, filename, NULL);
		strncpy(ext, module->object_name, 3);	/* Erste 3 Zeichen */
		sprintf (title, "%s-Datei speichern", module->object_name);
		sprintf (filter, "*.%s", ext);
		ok = select_file (filename, setup_path, filter, title, module->file_name);
	} /* if */

	if (ok)
	{
		switch (module->location)
		{
			case SETUPS_INTERN:
				out = fopen(module->file_name, "r+b"); 
				if(out != 0)
				{
					sprintf (title, "%s-Datei wird gespeichert ...", module->object_name);
					daktstatus (title, module->file_name);
					fputs(module->file_version, out);
					setups = (BYTE*)module->setups;
					for (x = 0; x < max_setups; x+=20)
					{
						set_daktstat((WORD)(x*100/max_setups));
						number = min(20, max_setups-x);
						fwrite(setups + x * setup_length, setup_length, number, out);
					} /* for */
					/* fwrite(module->setups, module->setup_length, module->max_setups, out); */
					set_daktstat(100);
					module->ram_modified = FALSE;
					fclose(out);
					close_daktstat();
				} /* if */
				else
				{
		      	hndl_alert_obj (module, ERR_MFOPEN);
		      	ok = FALSE;
		      } /* else */	
				break;
			case SETUPS_EXTERN:
				if (module->file_status & FILE_OPENED)
				{
					sprintf (title, "%s-Datei wird geschlossen", module->object_name);
					daktstatus (title, module->file_name);
					fclose(module->file_pointer);
					set_daktstat(100);
					close_daktstat();
					/* Datei gleich wieder îffnen zur Weiterverwendung */
					module->file_pointer = fopen(module->file_name, "r+b");
				} /* if */
				break;
		} /* switch */
	} /* if */
	return (ok);
} /* save_obj */

GLOBAL BOOLEAN	test_obj		(RTMCLASSP module, WORD action)
{
	switch (action)
	{
		case DO_DELETE:
			/* PrÅfung vor DELETE des Modules */
			switch (module->location)
			{
				case SETUPS_INTERN:
					if (module->ram_modified)
					{
						switch (hndl_alert_obj (module, ERR_NOTSAVED))
						{
							case 1:
								module->save (module, module->file_name, FALSE);
								break;
							case 2:
								break;
						} /* switch */ 
					} /* if */
					break;
				case SETUPS_EXTERN:
					module->save (module, module->file_name, FALSE);
					break;
			} /* switch */ 
			if (module->import_status & FILE_OPENED) {
				 fclose (module->import_pointer);
				 module->import_status |= ~ FILE_OPENED;
			} /* if */
			if (module->export_status & FILE_OPENED) {
				 fclose (module->export_pointer);
				 module->export_status |= ~ FILE_OPENED;
			} /* if */
			break;
		} /* switch */
	return TRUE;
} /* test_obj */

GLOBAL WORD hndl_alert_obj (RTMCLASSP module, WORD alert_id)
{
	WORD		button;
	BYTE		*errstr = alert_msgs [alert_id];
	LONGSTR	alertstr;
	
	button = NIL;
	
	if (alert_msgs != NULL) 
	{
		switch (alert_id)
		{
			case ERR_NOEXTERN:
				sprintf(alertstr, errstr, module->object_name);
				break;
			case ERR_NOTSAVED:
				sprintf(alertstr, errstr, module->object_name, module->file_name);
				break;
			case ERR_SETCHANGE:
				sprintf(alertstr, errstr, module->object_name, module->file_name);
				break;
			case ERR_FILEFORMAT:
				sprintf(alertstr, errstr, module->object_name, module->file_name);
				break;
			case ERR_FIMPORTOPEN:
				sprintf(alertstr, errstr, module->object_name, module->file_name);
				break;
			case ERR_MFOPEN:
				sprintf(alertstr, errstr, module->object_name, module->file_name);
				break;
			case ERR_MFREAD:
				sprintf(alertstr, errstr, module->object_name, module->file_name);
				break;
			case ERR_MFWRITE:
				sprintf(alertstr, errstr, module->object_name, module->file_name);
				break;
			default:
				sprintf(alertstr, errstr);		/* Standard-Fehler */
		} /* switch */
		button = open_alert (alertstr);
	} /* if */
	return (button);
} /* hndl_alert_obj */

/*****************************************************************************/
/* MenÅbehandlung                                                            */
/*****************************************************************************/

GLOBAL VOID update_menu_obj (window)
WINDOWP window;

{
} /* update_menu_obj */

/*****************************************************************************/

GLOBAL VOID handle_menu_obj (window, title, item)
WINDOWP window;
WORD    title, item;

{
  if (window != NULL)
    menu_normal (window, title, FALSE);         /* Titel invers darstellen */
/*
	switch (title)
	{
		case MCMIINFO:	switch(item)
			{
				case MCMIINFOANZEIG: info_obj(window, NIL);
							break;
			}
			break;
	}
*/
  if (window != NULL)
    menu_normal (window, title, TRUE);          /* Titel wieder normal darstellen */
} /* handle_menu_obj */

/*****************************************************************************/
/* Box zeichnen                                                              */
/*****************************************************************************/

GLOBAL VOID box_obj (window, grow)
WINDOWP window;
BOOLEAN grow;
{
  RECT l, b;

  get_dxywh (window->icon, &l);

  wind_calc (WC_BORDER, window->kind,       /* Rand berechnen */
             window->work.x, window->work.y, window->work.w, window->work.h,
             &b.x, &b.y, &b.w, &b.h);

  if (grow)
    growbox (&l, &b);
  else
    shrinkbox (&l, &b);
} /* box_obj */

/*****************************************************************************/
/* Teste Fenster                                                             */
/*****************************************************************************/

GLOBAL BOOLEAN wi_test_obj (window, action)
WINDOWP window;
WORD    action;

{
  BOOLEAN ret, ext;
  RTMCLASSP	module = Module(window);

  ret = TRUE;
  ext = (action & DO_EXTERNAL) != 0;

  switch (action & 0x00FF)
  {
    case DO_UNDO   : break;
    case DO_CUT    : break;
    case DO_COPY   : break;
    case DO_PASTE  : break;
    case DO_CLEAR  : break;
    case DO_SELALL : break;
    case DO_CLOSE  : /* Sicherheitsabfrage */ break;
    case DO_DELETE  :  break;
  } /* switch */

  return (ret);
} /* wi_test_obj */

/*****************************************************************************/
/* ôffne Fenster                                                             */
/*****************************************************************************/

GLOBAL VOID wi_open_obj (window)
WINDOWP window;

{
  box (window, TRUE);
} /* wi_open_obj */

/*****************************************************************************/
/* Schlieûe Fenster                                                          */
/*****************************************************************************/

GLOBAL VOID wi_close_obj (window)
WINDOWP window;
{
  box (window, FALSE);
} /* wi_close_obj */

/*****************************************************************************/
/* Lîsche Fenster                                                            */
/*****************************************************************************/

GLOBAL VOID wi_delete_obj (window)
WINDOWP window;
{
} /* wi_delete_obj */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

GLOBAL VOID wi_draw_obj (window)
WINDOWP window;
{
  /* clr_scroll (window); */
} /* wi_draw_obj */

/*****************************************************************************/
/* Reagiere auf Pfeile                                                       */
/*****************************************************************************/

GLOBAL VOID wi_arrow_obj (window, dir, oldpos, newpos)
WINDOWP window;
WORD    dir;
LONG    oldpos, newpos;

{
  WORD w, h;
  LONG delta;

  w     = window->scroll.w / window->xfac;      /* Breite in Zeichen */
  h     = window->scroll.h / window->yfac;      /* Hîhe in Zeichen */
  delta = newpos - oldpos;

  if (dir & HORIZONTAL)         /* Horizontale Pfeile und Schieber */
  {
    if (delta != 0)                                    /* Scrolling nîtkg */
    {
      if (delta > 0)                                   /* Links-Scrolling */
      {
      } /* if */
      else                                             /* Rechts-Scrolling */
      {
      } /* else */

      window->doc.x = newpos;                          /* Neue Position */

      set_sliders (window, HORIZONTAL, SLPOS);         /* Schieber setzen */
      scroll_window (window, HORIZONTAL, delta * window->xfac);
    } /* if */
  } /* if */
  else                          /* Vertikale Pfeile und Schieber */
  {
    if (delta != 0)                                    /* Scrolling nîtig */
    {
      if (delta > 0)                                   /* AufwÑrts-Scrolling */
      {
      } /* if */
      else                                             /* AbwÑrts-Scrolling */
      {
      } /* else */

      window->doc.y = newpos;                          /* Neue Position */

      set_sliders (window, VERTICAL, SLPOS);           /* Schieber setzen */
      scroll_window (window, VERTICAL, delta * window->yfac);
    } /* if */
  } /* else */
} /* wi_arrow_obj */

/*****************************************************************************/
/* Einrasten des Fensters                                                    */
/*****************************************************************************/

GLOBAL VOID wi_snap_obj (window, new, mode)
WINDOWP window;
RECT    *new;
WORD    mode;

{
	RECT r, diff;
	WORD wbox, hbox;
	LONG max_xdoc, max_ydoc;
	
	wind_get (window->handle, WF_CURRXYWH, &r.x, &r.y, &r.w, &r.h);
	
	wbox   = window->xfac;
	hbox   = window->yfac;
	/* Allways snap the size to object format */
	if (window->object) 
	{
		/*
		r.w = min(window->object->ob_width, desk.w - 2*gl_wbox);
		r.h = min(window->object->ob_height, desk.h - 2*gl_hbox);
		*/
		new->w = window->object->ob_width;
		new->h = window->object->ob_height + hbox;
	} /* if */
	diff.x = (new->x - r.x) / wbox * wbox;        /* Differenz berechnen */
	diff.y = (new->y - r.y) & 0xFFFE;
	diff.w = (new->w - r.w) / wbox * wbox;
	diff.h = (new->h - r.h) / hbox * hbox;
	
	if (!window->object) 
	{
		if (wbox == 8) new->x = r.x + diff.x;         /* Schnelle Position */
		new->y = r.y + diff.y;                        /* Y immer gerade */
		new->w = r.w + diff.w;                        /* Arbeitsbereich einrasten */
		new->h = r.h + diff.h;
	} /* if */
	
	if (mode & SIZED)
	{
		if (window->object == NULL)
		{
			r.w      = (window->scroll.w + diff.w) / wbox; /* Neuer Scrollbereich */
			max_xdoc = window->doc.w - r.w;
			r.h      = (window->scroll.h + diff.h) / hbox;
			max_ydoc = window->doc.h - r.h;
			
			if (max_xdoc < 0) max_xdoc = 0;
			if (max_ydoc < 0) max_ydoc = 0;
			
			if (window->doc.x > max_xdoc)               /* Jenseits rechter Bereich */
			window->doc.x = max_xdoc;
			
			if (window->doc.y > max_ydoc)               /* Jenseits unterer Bereich */
			window->doc.y = max_ydoc;
		} /* if */
	} /* if */
} /* wi_snap_obj */

/*****************************************************************************/
/* Objektoperationen von Fenster                                             */
/*****************************************************************************/

GLOBAL VOID wi_objop_obj (window, objs, action)
WINDOWP window;
SET     objs;
WORD    action;

{
} /* wi_objop_obj */

/*****************************************************************************/
/* Ziehen in das Fenster                                                     */
/*****************************************************************************/

GLOBAL WORD wi_drag_obj (src_window, src_obj, dest_window, dest_obj)
WINDOWP src_window;
WORD    src_obj;
WINDOWP dest_window;
WORD    dest_obj;

{
  if (src_window->handle == dest_window->handle) return (DRAG_SWIND); /* Im gleichen Fenster */
  if (src_window->class == dest_window->class) return (DRAG_SCLASS);  /* Gleiche Fensterart */

  return (DRAG_NOACTN);
} /* wi_drag_obj */
/*****************************************************************************/

GLOBAL VOID wi_click_obj (WINDOWP window, MKINFO *mk)
{
} /* wi_click_obj */

/*****************************************************************************/

GLOBAL VOID wi_unclick_obj (window)
WINDOWP window;

{
} /* wi_unclick_obj */

/*****************************************************************************/
/* Taste fÅr Fenster                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN wi_key_obj (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  if (menu_key (window, mk)) return (TRUE);

  return (FALSE);
} /* wi_key_obj */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

GLOBAL VOID wi_timer_obj (window)
WINDOWP window;

{
} /* wi_timer_obj */

/*****************************************************************************/
/* Fenster nach oben gebracht                                                */
/*****************************************************************************/

GLOBAL VOID wi_top_obj (window)
WINDOWP window;

{
} /* wi_top_obj */

/*****************************************************************************/
/* Fenster nach unten gebracht                                               */
/*****************************************************************************/

GLOBAL VOID wi_untop_obj (window)
WINDOWP window;

{
} /* wi_untop_obj */

/*****************************************************************************/
/* Cut/Copy/Paste fÅr Fenster                                                */
/*****************************************************************************/

GLOBAL VOID wi_edit_obj (window, action)
WINDOWP window;
WORD    action;

{
  BOOLEAN ext;

  ext = (action & DO_EXTERNAL) != 0;

  switch (action & 0x00FF)
  {
    case DO_UNDO   : break;
    case DO_CUT    : break;
    case DO_COPY   : break;
    case DO_PASTE  : break;
    case DO_CLEAR  : break;
    case DO_SELALL : break;
  } /* switch */
} /* wi_edit_obj */

/*****************************************************************************/
/* Iconbehandlung                                                            */
/*****************************************************************************/

GLOBAL BOOLEAN icons_obj (src_obj, dest_obj)
WORD src_obj, dest_obj;

{
  BOOLEAN result;
  WINDOWP window;

  result = FALSE;

  switch (src_obj)
  {
    case ICMI : window = search_window (CLASS_CMI, SRCH_ANY, src_obj);
                   switch (dest_obj)
                   {
                     case ITRASH   : break;
                     case IDISK    : break;
                     case IPRINTER : break;
                     case ICLIPBRD : break;
                   } /* switch */

                   result = TRUE;
                   break;
  } /* switch */

  return (result);
} /* icons_obj */

/*****************************************************************************/
/* Hilfen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN showhelp_obj (window, icon)
WINDOWP window;
WORD    icon;
{
	RTMCLASSP	module;
	STRING		name;
	CHAR			*c;
	INT			i;
	BOOLEAN		ret = FALSE;
	
	if (window)
	{
		for (i = 0; window->name[i] == ' '; i++); 	/* Space Åberspringen */
		strcpy (name, window->name+i);		/* Name kopieren */
		c = strchr (name, ' ');				/* Space	suchen */
		if (c) *c = 0;							/* String abschneiden */
		if (*name != 0)
			ret = help_rtm (name);
		else
		{
			ret = help_obj(window->module);
		} /* else */
	} /* if */
	if (!ret) hndl_alert (ERR_NOHELP);
	return (ret);
} /* showhelp_obj */

GLOBAL BOOLEAN help_obj (RTMCLASSP module)
{
	STRING		name;
	BOOLEAN		ret;
	
	if (module)
	{
		if (module->window)
			ret = showhelp_obj (module->window, NIL);
		else
			ret = help_rtm (module->object_name);
	} /* if */
	return (ret);
} /* help_obj */

/*****************************************************************************/
/* Box zeichnen                                                              */
/*****************************************************************************/

PRIVATE VOID box (window, grow)
WINDOWP window;
BOOLEAN grow;

{
  RECT l, b;

  get_dxywh (window->icon, &l);

  wind_calc (WC_BORDER, window->kind,       /* Rand berechnen */
             window->work.x, window->work.y, window->work.w, window->work.h,
             &b.x, &b.y, &b.w, &b.h);

  if (grow)
    growbox (&l, &b);
  else
    shrinkbox (&l, &b);
} /* box */

/*****************************************************************************/
/* Suche Slot einer Klasse                                                   */
/*****************************************************************************/

GLOBAL WORD find_classlot (rtmclass)
INT rtmclass;
{
	INT slot;
	
	/* ... den passenden Modulzeiger finden ... */
	for (slot = 0; slot < rtmtop; slot++)
	{
		if(rtmclass == rtmmodules [slot]->class_number)
			return (slot);
	} /* for */
	
	return (FAILURE);
} /* find_classlot */

/*****************************************************************************/
/* Suche Slot von Modul                                                      */
/*****************************************************************************/

LOCAL WORD find_moduleslot (module)
RTMCLASSP module;
{
  REG WORD slot;

  for (slot = 0; slot < rtmtop; slot++)
    if (rtmmodules [slot] == module) return (slot);

  return (FAILURE);
} /* find_moduleslot */

/*****************************************************************************/
/* Kreire Objekt                                                             */
/*****************************************************************************/
GLOBAL WINDOWP create_window_obj (UWORD kind, WORD class)
{
	WINDOWP	window;
	window = create_window (kind, class);
	if (window)
	{
		/* Standard-Bindings */
		window->hndl_menu = handle_menu_obj;
		window->updt_menu = update_menu_obj;
		window->test      = wi_test_obj;
		window->open      = wi_open_obj;
		window->close     = wi_close_obj;
		window->delete    = wi_delete_obj;
		window->draw      = wi_draw_obj;
		window->arrow     = wi_arrow_obj;
		window->snap      = wi_snap_obj;
		window->objop     = wi_objop_obj;
		window->drag      = wi_drag_obj;
		window->click     = wi_click_obj;
		window->unclick   = wi_unclick_obj;
		window->key       = wi_key_obj;
		window->timer     = wi_timer_obj;
		window->top       = wi_top_obj;
		window->untop     = wi_untop_obj;
		window->edit      = wi_edit_obj;
		window->showhelp  = showhelp_obj;
	} /* if */
	return window;
} /* create_window_obj */

/*****************************************************************************/
/* Lîsche Objekt                                                            */
/*****************************************************************************/
GLOBAL VOID destroy_obj (module)
RTMCLASSP module;
{
	if (module)
	{
		mem_free(module->setups);
		mem_free(module->standard);
		if (module->edited) mem_free(module->edited->setup);
		mem_free(module->edited);
		if (module->actual) mem_free(module->actual->setup);
		mem_free(module->actual);
		mem_free(module->status);
		mem_free(module->stat_alt);
	} /* if */
} /* destroy_obj */

/*****************************************************************************/
/* Kreiere Modul                                                           */
/*****************************************************************************/

GLOBAL RTMCLASSP create_module (CHAR *module_name, WORD instance_count)

{
	/* REG */ RTMCLASSP module;
	/* REG */ WORD i;
	
	module = NULL;                /* ZunÑchst kein Modul zur VerfÅgung */
	
	for (i = 0; (i < max_rtmmodules) && setin (used_rtmmodules, i); i++);
	
	if (i == max_rtmmodules)
	{
		if (nortmmodules >= 0) error (1, nortmmodules, NIL, NULL);
	} /* if */
	else
	{
		setincl (used_rtmmodules, i);  /* Setze Modul als benutzt */
		module = &rtmmrec [i];      /* Merke Adresse */
		
		mem_set (module, 0, (UWORD)sizeof (RTMCLASS));

		rtmmodules [rtmtop++]		= module; /* Neues Modul in Keller */
		sprintf(module->object_name, "%s_%d", module_name, instance_count+1);
		sprintf(module->menu_text, "%s", module_name);
		sprintf(module->file_name, 		"%sDEFAULT.%s", setup_path, module_name);
		sprintf(module->file_extension,	"%s", module_name);
		sprintf(module->file_version,		"%s V 1.00\n", module_name);
		sprintf(module->import_name, 		"%s%s.EXP", import_path, module_name);
		sprintf(module->export_name, 		"%s%s.EXP", export_path, module_name);
		sprintf(module->info_name, 		"%s%s_%d.INF", info_path, module_name, instance_count+1);
		module->actual 			= mem_alloc (sizeof(EDIT));
		module->edited 			= mem_alloc (sizeof(EDIT));
		module->get_edit			= get_edit_obj;
		module->set_edit			= set_edit_obj;
		module->get_setnr			= get_setnr_obj;
		module->set_setnr			= set_setnr_obj;
		module->set_nr				= set_nr_obj;
		module->set_store			= set_store_obj;
		module->set_recall		= set_recall_obj;
		module->set_ok				= set_ok_obj;
		module->set_cancel		= set_cancel_obj;
		module->set_standard		= set_standard_obj;
		module->load				= load_obj;
		module->save				= save_obj;
		module->test				= test_obj;
		module->destroy			= destroy_obj;
		module->icons				= icons_obj;
		module->help				= help_obj;
	} /* else */
	
	return (module);              /* Gib kreiertes Modul zurÅck */
} /* create_module */

/*****************************************************************************/
/* Lîsche Modul                                                            */
/*****************************************************************************/

GLOBAL VOID delete_module (module)
RTMCLASSP module;

{
  WORD    slot, i;
  BOOLEAN cont;

  if (module != NULL)
  {
    cont = (module->test != NULL) ? (*module->test) (module, DO_DELETE) : TRUE;

    if (cont)
    {

        /* close_rtmmodule (module); */                  /* Schlieûe Modul */

      slot = find_moduleslot (module);               /* Nicht nur oberstes Modul kann gelîscht werden */

      for (i = slot + 1; i < rtmtop; i++) rtmmodules [i - 1] = rtmmodules [i];

      slot = (WORD)(module - rtmmrec);          /* Zeiger Arithmetik */
      setexcl (used_rtmmodules, slot);             /* Modul freigeben */
      rtmtop--;                                    /* Slot freigeben */
    } /* if */
  } /* if */
} /* delete_module */


/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_modules ()

{
	BOOLEAN ok = TRUE;
	REG WORD i;
	REG RTMCLASSP module;
	
	for (i = 0; i < max_rtmmodules; i++)         	/* Untersuche alle Module */
	{
		module = rtmmodules [i];
		if (module)								/* Modul vorhanden? */
		{
			if (module->test != 0)       	/* Test vorhanden? */
				ok &= (module->test) (module, DO_DELETE);	/* Beenden vorbereiten */
			if (module->term != 0)       	/* Term vorhanden? */
				ok &= (module->term) ();	/* Modul beenden */
			if (module->destroy)
				module->destroy(module);	/* Komplett lîschen */
		} /* if */
	} /* for */

	if (rtmmrec != NULL) mem_free (rtmmrec); /* Speicher freigeben */
	if (rtmmodules != NULL) mem_free (rtmmodules); /* Speicher freigeben */
	return ( ok );
} /* term_modules */

GLOBAL BOOL load_info_obj (CONST CHAR *file_name, CHAR *setup_name, LONG *setup_nr, RECT *scroll, BOOL *opened)
{
	/* Open an info file, read in the data and assign it to the 
		pointer-parameters */
		
	FILE		*info;

	info = fopen(file_name, "r"); 
	if (info)
	{
		if (setup_name == NULL)
			setup_name = (CHAR*)new(STRING);
		
		fscanf (info, "%s",	setup_name);
		fscanf (info, "%ld",		setup_nr);
		fscanf (info, "%d %d %d %d",	&scroll->x, &scroll->y, &scroll->w, &scroll->h);
		fscanf (info, "%d",	opened);

		fclose (info);
		return TRUE;
	} /* if */

	return FALSE;
} /* load_info_obj */

GLOBAL BOOL save_info_obj (CONST CHAR *file_name, CONST CHAR *setup_name, CONST LONG setup_nr, CONST RECT *scroll, BOOL opened)
{
	/* Open an info file, write in the data and assign it to the 
		pointer-parameters */
		
	FILE		*info;

	info = fopen(file_name, "w"); 
	if (info)
	{
		fprintf (info, "%s\n",	setup_name);
		fprintf (info, "%ld\n",		setup_nr);
		fprintf (info, "%d %d %d %d\n", scroll->x, scroll->y, scroll->w, scroll->h);
		fprintf (info, "%d",	opened);
		
		fclose (info);
		return TRUE;
	} /* if */
	
	return FALSE;
} /* save_info_obj */

GLOBAL WORD load_create_infos (CreateFn *create, CONST CHAR *type, CONST WORD max_instances)
{
	/* Go through all info files of a type and create instances for them */
	WORD			instance;
	BOOL			opened;
	LONGSTR		file_name, setup_name;
	LONG			setup_nr;
	RECT			scroll;
	RTMCLASSP	module;
	WINDOWP		window;
	
	for (instance = 1; instance <= max_instances; instance++)
	{
		sprintf (file_name, "%s%s_%d.INF", info_path, type, instance);

		if (load_info_obj (file_name, setup_name, &setup_nr, &winit, &opened))
		{
			module = create ();
			strcpy (module->info_name, file_name);
			module->set_setnr (module, setup_nr);
			window= Window(module);
			module->start_open = opened;
		} /* if load */
		else 
		{
			/* Stop iteration, no more .INF available */
			/* Create at least one module, for compatibility */
			winit.x = 0;
			winit.y = 0;
			winit.w = 0;
			winit.h = 0;
			if (instance == 1)
			{
				module = create ();
				strcpy (module->info_name, file_name);
				return 1;
			} /* if */
			else
				/* Return the number of the valid instances */
				return instance-1;	
		} /* else */
	} /* for */
	
	/* All .INF have been created */
	return instance-1;	
} /* load_create_infos */

GLOBAL BOOLEAN open_module_windows ()
{
	BOOLEAN ok = TRUE;
	REG WORD i;
	REG RTMCLASSP module;
	
	for (i = 0; i < max_rtmmodules; i++)         	/* Untersuche alle Module */
	{
		module = rtmmodules [i];
		
		if (module)
			if (module->start_open)
				module->open (module->icon_number);
	} /* for */
	return TRUE;
} /* open_module_windows */
