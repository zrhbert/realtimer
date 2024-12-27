/*****************************************************************************/
/*                                                                           */
/* Modul: INITERM.C                                                          */
/* Datum: 09.01.95                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- Icons hidden wenn disabled, 09.01.95
- Icon/MenÅ aktivierung fÅr Module eingebaut in init_initerm, 08.01.95
- dlogin eingebaut, 04.01.95
25.12.94
- terminierungs-Reihenfolge fÅr realtspc, objects und dispobj umgestellt, 25.12.94
- printer, edit, meta, clipboard, image und trash rausgenommen, 30.11.94
- ERR_NOSCRAP wird unterbunden 
09.03.94
- open_module_windows eingebaut
- init/term_dispobj eingebaut
03.06.93
- MAXRESWINDOWS auf 200 gesetzt
- Dummy Definitionen fÅr setup und status eingebaut
*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"

#include "realtspc.h"
#include "objects.h"
#include "init_obj.h"
#include "dispobj.h"
#include "realtim4.h"
#include "errors.h"

#include "resource.h"
#include "menu.h"
#include "event.h"
#include "dialog.h"
#include "desktop.h"
#include "trash.h"
#include "disk.h"
#include "printer.h"
#include "clipbrd.h"
#include "image.h"
#include "meta.h"
#include "edit.h"

#include "export.h"
#include "initerm.h"

/****** DEFINES **************************************************************/

#define ACC_MENU        "  Realtimer"
#define MAX_RESWIND     200 /* 40 GEM-Fenster fÅr A3D, 40 andere etc. */ 
#define CLASS_HELP      2               /* Muû in HELP.H, falls ein Help-Modul exisiert */

#ifndef ALERT_NAME
#define ALERT_NAME      "REALTIM4.ERR"
#endif

/****** TYPES ****************************************************************/

#if MSDOS | FLEXOS | DR_C | LASER_C | LATTICE_C | MW_C
typedef struct
{
  BYTE  d_reserved [21];
  UBYTE d_attrib;
  UWORD d_time;
  UWORD d_date;
  ULONG d_length;
  BYTE  d_fname [14];
} DTA;
#endif

typedef struct setup
{
	VOID *dummy;
} SETUP;

typedef struct status
{
	VOID *dummy;
} STATUS;

/****** VARIABLES ************************************************************/

LOCAL BOOLEAN gl_ok;            /* Initialisierung von global ok? */
LOCAL BOOLEAN rsc_ok;           /* Initialisierung von resource ok ? */
LOCAL BOOLEAN alert_ok;         /* Initialisierung von alert ok ? */
LOCAL BYTE	  *l_alert_msgs;		/* Lokale Definition fÅr init */
/****** FUNCTIONS ************************************************************/

LOCAL LONG    file_length _((BYTE *filename));
LOCAL BOOLEAN text_rdln   _((FILE *file, BYTE *s, WORD maxlen));
LOCAL BOOLEAN read_alerts _((VOID));
LOCAL VOID place_icons 		(VOID);

/*****************************************************************************/

LOCAL LONG file_length (filename)
BYTE *filename;

{
  LONG    length;
  BOOLEAN ok;
  BYTE    s [128];
  DTA     dta, *old_dta;

  old_dta = (DTA *)Fgetdta ();
  Fsetdta (&dta);
  strcpy (s, filename);

#if GEMDOS
  ok = Fsfirst (s, 0x00) == 0;
#else
  ok = Fsfirst (s, 0x00) > 0;
#endif

  length = ok ? dta.d_length : 0;

  Fsetdta (old_dta);

  return (length);
} /* file_length */

/*****************************************************************************/

LOCAL BOOLEAN text_rdln (file, s, maxlen)
FILE *file;
BYTE *s;
WORD maxlen;

{
  BYTE *res;
  LONG l;

  res = fgets (s, (INT)maxlen, file);
  if (res == NULL) return (FALSE);

  l = strlen (s) - 1;
  if (l >= 0)
    if (s [l] == '\n') s [l] = EOS;

  return (TRUE);
} /* text_rdln */

/*****************************************************************************/

LOCAL BOOLEAN read_alerts ()
{
  BOOLEAN  ok;
  LONG     len;
  STR128   filename;
  FILE     *stream;
  WORD     numerr, count, i;
  LONGSTR  s;
  BYTE     *p, *ap;

  strcpy (filename, ALERT_NAME);

  if (! shel_find (filename))
  {
    strcpy (filename, app_path);
    strcat (filename, ALERT_NAME);
  } /* if */

  l_alert_msgs = NULL;
  alerts     = NULL;
  len        = file_length (filename);
  ok         = len != 0;

  if (! ok)
    error (1, NOERRMSG, NIL, NULL);
  else
  {
    l_alert_msgs = (BYTE *)mem_alloc (len);
    ok         = l_alert_msgs != NULL;

    if (! ok)
      error (1, NOMEMORY, NIL, NULL);
    else
    {
      stream = fopen (filename, "r");
      ok     = stream != NULL;

      if (! ok)
      {
        error (1, NOERRMSG, NIL, NULL);
        mem_free (l_alert_msgs);
      } /* if */
      else
      {
        numerr = count = 0;
        ap     = l_alert_msgs;

        while (text_rdln (stream, s, LONGSTRLEN))
          if (*s != EOS)
          {
            p = s;
            while ((p = strchr (p, SEP_CLOSE)) != NULL) /* Separatoren zÑhlen */
            {
              p++;
              count++;
            } /* while */

            strcpy (ap, s);
            ap += strlen (ap);

            if (count >= NUM_SEP)                       /* Ende der Fehlermeldung */
            {
              numerr++;
              ap++;
              count = 0;
            } /* if */
          } /* if, while */

        fclose (stream);

        alerts = (BYTE **)mem_alloc ((LONG)numerr * sizeof (BYTE *));
        ok     = alerts != NULL;

        if (! ok)
        {
          error (1, NOMEMORY, NIL, NULL);
          mem_free (l_alert_msgs);
        } /* if */
        else
          for (i = 0, ap = l_alert_msgs; i < numerr; i++)  /* Zeiger vergeben */
          {
            alerts [i]  = ap;
            ap         += strlen (ap) + 1;
          } /* for, else */
      } /* else */
    } /* else */
  } /* else */

  return (ok);
} /* read_alerts */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_initerm (argc, argv)
INT  argc;
BYTE *argv [];

{
  BOOLEAN ok;
  BYTE    *p;
  WORD    i;
  STR128  s;
  STRING  prefix;
  WORD    drive;
  STRING  path;
  BYTE    filename [14];
  BYTE    ext [4];
  BYTE    sep [2];
	REG RTMCLASSP module;
  
  ok    = TRUE;
  
  gl_ok = init_global (argc, argv, ACC_MENU, CLASS_DESK); /* Initialisiere global */

  if (! gl_ok) return (FALSE);                  /* Keine Applikation mehr mîglich */

  rsc_ok = init_resource ();                    /* Initialisiere resource */

  if (! rsc_ok) return (FALSE);                 /* Resourcen nicht ok */

  alert_ok = read_alerts ();                    /* Lies Fehlermeldungen */

  if (! alert_ok) return (FALSE);

  ok &= init_windows (NOWINDOW, MAX_RESWIND, CLASS_HELP); /* Initialisiere windows */
  ok &= init_menu ();                           /* Initialisiere menu */
  ok &= init_event ();                          /* Initialisiere event */
  ok &= init_dialog (alerts, alert, ALERT, (BYTE *)freetext [FDESKNAM].ob_spec);
  ok &= init_desktop ();                        /* Initialisiere desktop */
  ok &= init_disk ();                           /* Initialisiere disk */
/*
  ok &= init_trash ();                          /* Initialisiere trash */
  ok &= init_printer ();                        /* Initialisiere printer */
  ok &= init_clipbrd ();                        /* Initialisiere clipboard */
  ok &= init_image ();                          /* Initialisiere image */
  ok &= init_meta ();                           /* Initialisiere image */
  ok &= init_edit ();                           /* Initialisiere edit */
*/
  ok &= init_realtspc ();                     	/* Initialisiere RTM-Spezialfunktionen */
  ok &= init_dispobj ();	                    	/* Initialisiere Display-Objekte */
  ok &= init_modules ();                     	/* Initialisiere RTM-Module */
	
	/* Icons und MenÅs aller Module aktivieren */
	for (i = 0; i < max_rtmmodules; i++)         	
	{
		module = rtmmodules [i];
		
		if (module)
		{
			undo_state (desktop, module->icon_number, DISABLED);
			undo_state (menu, module->menu_item, DISABLED);
		} /* if */
	} /* for */

	place_icons();
	
  /* if (*scrapdir == EOS) hndl_alert (ERR_NOSCRAP); unterbinden, BD */
		
  if (ok)
    if (! deskacc)
    {
      wind_update (BEG_UPDATE);                 /* Benutzer darf nicht agieren */
      busy_mouse ();                            /* Biene zeigen */
      if (menu_fits) menu_bar (menu, TRUE);     /* MenÅzeile darstellen */
      open_desktop (NIL);                       /* Desktop îffnen */
		/*
      open_clipbrd (ICLIPBRD);                  /* Klemmbrett îffnen */
		*/
      if (*tail)                                /* ParamterÅbergabe */
      {
        p          = tail;
        prefix [0] = EOS;
        sep [0]    = SUFFSEP;
        sep [1]    = EOS;

        while (*p)
        {
          i = 0;
          while (*p && (*p != SP)) s [i++] = *p++;      /* Suche Leerzeichen */

          s [i] = EOS;
          if (*p) p++;

          file_split (s, &drive, path, filename, ext);

          if (*prefix == EOS)                           /* Merke PrÑfix */
          {
            strcpy (prefix, "A:");
            prefix [0] += (BYTE)drive;
            strcat (prefix, path);
          } /* if */

          if (strchr (s, PATHSEP) == NULL)              /* Kein Pfad vorhanden */
          {
            strcpy (s, prefix);                         /* Benutze letzten Pfad */
            strcat (s, filename);
            strcat (s, sep);
            strcat (s, ext);
          } /* if */

          /*
          if (strcmp (ext, "IMG") == 0)
            open_image (NIL, s);
          else
            if (strcmp (ext, "GEM") == 0)
              open_meta (NIL, s);
            else
              open_edit (NIL, s);
          */
        } /* while */

        tail [0] = EOS;                                 /* Nicht mehr benîtigt */
      } /* if */

		open_module_windows ();
      arrow_mouse ();                           /* Wieder Pfeil zeigen */
      wind_update (END_UPDATE);                 /* Benuzter darf wieder agieren */
    } /* if, if */

/*
	if (ok) 
		ok = dlogin();		/* Login durchfÅhren */
*/
  return (ok);                                  /* Alles gut verlaufen */
} /* init_initerm */


LOCAL VOID place_icons ()
{
	WINDOWP	desk_win = find_desk ();
	WORD iconw, iconh, iconr, i, y, count = 0;
	
	if (desktop != NULL)                          /* Eigener Desktop */
	{
		iconw = desktop [ITRASH].ob_width +4;
		iconh = desktop [ITRASH].ob_height +4;
		iconr = desk.w/iconw; /* Icons pro Zeile */
		
		for (i = ITRASH; i < FKEYS; i++)
		{
			if (is_state (desktop, i, DISABLED))
				do_flags (desktop, i, HIDETREE);
			else
			{			
				if (is_flags (desktop, FKEYS, HIDETREE))
				y = desk.y + desk.h ;
				else
				y = desktop [FKEYS].ob_y;
				
				/* Erstes Icon in oberste Reihe, links */
				/*   Hîhe  mal (Anzahl       + Rundung)                   pro Zeile */
				y -= iconh * (((FKEYS-ITRASH)+(iconr-((FKEYS-ITRASH+1)%(iconr+1)))) / iconr);
				/* Evtl. nÑchste Zeile */
				y += iconh * (((count-ITRASH)/ iconr)-1);
				/* y -= 4; */                         /* 4 Bits freilassen */
				y -= odd (y);                   /* nur gerade Zahlen */
				desktop [i].ob_y = y;
				/* ... und immer schîn nebeneinander */
				desktop [i].ob_x = iconw * ((count-ITRASH) % iconr);
				count++;
			} /* else */
			draw_object(desk_win, i);
		} /* for */
	} /* if desktop */
} /* place_icons  */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_initerm ()

{
  BOOLEAN ok;
  STR128  s;
  BYTE    sep [2];

  ok = TRUE;

  if (gl_ok && rsc_ok && alert_ok)
  {
    wind_update (BEG_UPDATE);
    busy_mouse ();

    if (*called_by)                             /* ZurÅck zum Aufrufer bzw. an OUTPUT */
    {
      sep [0] = PROGSEP;
      sep [1] = EOS;

      strcpy (s + 1, tail);
      strcat (s + 1, sep);
      strcat (s + 1, app_path);
      strcat (s + 1, app_name);
      s [0] = (BYTE)strlen (s + 1);

      shel_write (TRUE, TRUE, 1, called_by, s);
    } /* if */

    if (menu_fits) menu_bar (menu, FALSE);      /* MenÅzeile freigeben */
/*
    ok &= term_edit ();                         /* Terminiere edit */
    ok &= term_meta ();                         /* Terminiere meta */
    ok &= term_image ();                        /* Terminiere image */
    ok &= term_clipbrd ();                      /* Terminiere clipboard */
    ok &= term_printer ();                      /* Terminiere printer */
    ok &= term_trash ();                        /* Terminiere trash */
*/

    ok &= term_modules ();                     	/* Terminiere RTM-Module */
	 ok &= term_dispobj ();                    	/* Terminiere Display-Objekte */
	 ok &= term_realtspc ();                    	/* Terminiere RTM-Spezialfunktionen */

    ok &= term_disk ();                         /* Terminiere disk */
    ok &= term_desktop ();                      /* Terminiere desktop */
    ok &= term_dialog ();                       /* Terminiere dialog */
    ok &= term_event ();                        /* Terminiere event */
    ok &= term_menu ();                         /* Terminiere menu */
    ok &= term_windows ();                      /* Terminiere windows */
    ok &= term_resource ();                     /* Terminiere resource */

    mem_free (alerts);
    mem_free (l_alert_msgs);

    arrow_mouse ();
    wind_update (END_UPDATE);
  } /* if */

  ok &= term_global ();                         /* Terminiere global */

  return (ok);
} /* term_initerm */

