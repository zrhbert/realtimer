/*****************************************************************************/
/*                                                                           */
/* Modul: XYZ.C                                                              */
/*                                                                           */
/* XYZ Standard-Modul                                                       */
/*****************************************************************************/
#define XYZVERSION "V 0.03"
#define XYZDATE "19.05.94"

#ifndef XRSC_CREATE
/* #define XRSC_CREATE TRUE                    /* X-Resource-File im Code */ */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "xyz_mod.h"
#include "realtspc.h"
#include "var.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"
#include "mrosc.h"

#include "export.h"
#include "xyz.h"

#if XRSC_CREATE
#include "xyz_mod.rsh"
#include "xyz_mod.rh"
#endif
/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|MOVER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 3 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangshîhe in Pixel */
#define MILLI  0                        /* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "XYZ_MOD.RSC"		/* Name der Resource-Datei */

#define MAXXYZSETUPS		1l					/* Anzahl der XYZ-Setups */
/****** TYPES ****************************************************************/
typedef struct xyzset
{
	WORD	frames;			/* Nummern der Variablen die verwendet werden */
} XYZSETUP;

typedef struct xyzset *XYZ_P;

/****** VARIABLES ************************************************************/
PRIVATE WORD	xyz_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*xyz_rsc_ptr = &xyz_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *xyz_setup;
PRIVATE OBJECT *xyz_help;
PRIVATE OBJECT *xyz_desk;
PRIVATE OBJECT *xyz_text;
PRIVATE OBJECT *xyz_info;

PRIVATE	XYZSETUP	xyz_setups [MAXXYZSETUPS];
PRIVATE	LONG		xyz_setup_nr	= 1;	/* Nummer des aktuellen XYZ-Sets */
PRIVATE	LONG		xyz_setup_alt	= 1;	/* Nummer des alten XYZ-Sets */

PRIVATE	STR128	xyz_filename = "DEFAULT.XYZ";

/****** FUNCTIONS ************************************************************/

/* Interne XYZ-Funktionen */
PRIVATE BOOLEAN create_xyz 	_((VOID));

PRIVATE VOID    get_setup_xyz	_((VOID));
PRIVATE VOID    set_setup_xyz	_((VOID));

PRIVATE BOOLEAN init_rsc_xyz	_((VOID));
PRIVATE BOOLEAN term_rsc_xyz	_((VOID));

/*****************************************************************************/
PRIVATE VOID    get_setup_xyz	(VOID)
{
} /* get_xyz_setup */

PRIVATE VOID    set_setup_xyz	(VOID)
{
} /* set_xyz_setup */

PUBLIC PUF_INF *apply_xyz	(PUF_INF *koor)
{
	return koor;
} /* apply_xyz */

PUBLIC LONG		get_setnr_xyz	(VOID)
{
	return xyz_setup_nr;
} /* get_setnr_xyz */

PUBLIC LONG		set_setnr_xyz	(LONG setupnr)
{
	xyz_setup_nr = setupnr;

	minmaxsetup(&xyz_setup_nr, MAXXYZSETUPS);
	
	set_xyz();
	
	return xyz_setup_nr;
} /* set_setnr_xyz */

PUBLIC BOOLEAN	load_xyz	(STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	BYTE		ext [4];
	FILE		*in;
	STR128	file, path;
	STR128 s;
		
	if (filename == NULL)
	{
		filename = s;
		strcpy (filename, xyz_filename);
	} /* if */

	if (fileselect)
	{
		file_split (xyz_filename, NULL, path, filename, NULL);
		ok = select_file (filename, path, "*.XYZ", "LADEN von XYZ-Dateien", xyz_filename);
	} /* if */

	if (ok)
	{
		in = fopen(xyz_filename, "rb");
		if(in == 0)
		{	
	      hndl_alert (ERR_FOPEN);
	      ok = FALSE;
		} /* if */
		else
		{
			daktstatus(" XYZ-Datei wird geladen ... ", xyz_filename);
			fgets(s, (INT) sizeof(s), in);
			if (strcmp(s, "XYZ V 1.00\n") ==0)
				fread(xyz_setups, sizeof (xyz_setups[0]), MAXXYZSETUPS, in);
			fclose(in);
			close_daktstat();
		} /* else */
	} /* if */

	return (ok);
} /* load_xyz */

PUBLIC BOOLEAN	save_xyz	(STR128 filename, BOOLEAN fileselect)
{
	BOOLEAN	ok = TRUE; 
	BYTE		ext [4];
	FILE		*out;
	STR128	file, path;
	STR128 s;
		

	if (filename == NULL)
	{
		filename = s;
		strcpy (filename, xyz_filename);
	} /* if */

	if (fileselect)
	{
		file_split (filename, NULL, setup_path, filename, NULL);
		ok = select_file (filename, setup_path, "*.XYZ", "SPEICHERN von XYZ-Dateien", xyz_filename);
	} /* if */

	if (ok)
	{
		out = fopen(xyz_filename, "wb"); 
		if(out != 0)
		{
			daktstatus(" XYZ-Datei wird gespeichert ... ", xyz_filename);
			fputs("XYZ V 1.00\n", out);
			fwrite(xyz_setups, sizeof (xyz_setups[0]), MAXXYZSETUPS, out);
			fclose(out);
			close_daktstat();
		} /* if */
	} /* if */
	return (ok);
} /* save_xyz */

PUBLIC VOID		reset_xyz	(VOID)
{
	/* ZurÅcksetzen von Werten */
} /* reset_xyz */

PUBLIC VOID		precalc_xyz	(VOID)
{
	/* Vorausberechnung */
} /* precalc_xyz */

PUBLIC VOID		message_xyz	(WORD type, LONG param1, LONG param2)
{
	switch(type)
	{
		case SET_VAR:
		switch (param1)
		{
		} /* switch */
		break;
	} /* switch */
} /* message_xyz */

PUBLIC BOOLEAN	test_xyz		(RTMCLASSP rtmmodule, WORD action)
{
	/* PrÅfung vor DELETE des Modules */
	
	return TRUE;
} /* test_xyz */



/*****************************************************************************/
/* MenÅbehandlung                                                            */
/*****************************************************************************/

PRIVATE VOID update_menu (window)
WINDOWP window;

{
} /* update_menu */

/*****************************************************************************/

PRIVATE VOID handle_menu (window, title, item)
WINDOWP window;
WORD    title, item;

{
  if (window != NULL)
    menu_normal (window, title, FALSE);         /* Titel invers darstellen */
/*
	switch (title)
	{
		case MXYZINFO:	switch(item)
			{
				case MXYZINFOANZEIG: info_xyz(window, NIL);
							break;
			}
			break;
	}
*/
  if (window != NULL)
    menu_normal (window, title, TRUE);          /* Titel wieder normal darstellen */
} /* handle_menu */

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
/* Teste Fenster                                                             */
/*****************************************************************************/

PRIVATE BOOLEAN wi_test (window, action)
WINDOWP window;
WORD    action;

{
  BOOLEAN ret, ext;

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
    case DO_DELETE : /* Sicherheitsabfrage */ break;
  } /* switch */

  return (ret);
} /* wi_test */

/*****************************************************************************/
/* ôffne Fenster                                                             */
/*****************************************************************************/

PRIVATE VOID wi_open (window)
WINDOWP window;

{
  box (window, TRUE);
} /* wi_open */

/*****************************************************************************/
/* Schlieûe Fenster                                                          */
/*****************************************************************************/

PRIVATE VOID wi_close (window)
WINDOWP window;

{
  box (window, FALSE);
} /* wi_close */

/*****************************************************************************/
/* Lîsche Fenster                                                            */
/*****************************************************************************/

PRIVATE VOID wi_delete (window)
WINDOWP window;

{
} /* wi_delete */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

PRIVATE VOID wi_draw (window)
WINDOWP window;

{
	/* clr_scroll (window); */
} /* wi_draw */

/*****************************************************************************/
/* Reagiere auf Pfeile                                                       */
/*****************************************************************************/

PRIVATE VOID wi_arrow (window, dir, oldpos, newpos)
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
} /* wi_arrow */

/*****************************************************************************/
/* Einrasten des Fensters                                                    */
/*****************************************************************************/

PRIVATE VOID wi_snap (window, new, mode)
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
  diff.x = (new->x - r.x) / wbox * wbox;        /* Differenz berechnen */
  diff.y = (new->y - r.y) & 0xFFFE;
  diff.w = (new->w - r.w) / wbox * wbox;
  diff.h = (new->h - r.h) / hbox * hbox;

  if (wbox == 8) new->x = r.x + diff.x;         /* Schnelle Position */
  new->y = r.y + diff.y;                        /* Y immer gerade */
  new->w = r.w + diff.w;                        /* Arbeitsbereich einrasten */
  new->h = r.h + diff.h;

  if (mode & SIZED)
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
} /* wi_snap */

/*****************************************************************************/
/* Objektoperationen von Fenster                                             */
/*****************************************************************************/

PRIVATE VOID wi_objop (window, objs, action)
WINDOWP window;
SET     objs;
WORD    action;

{
} /* wi_objopen */

/*****************************************************************************/
/* Ziehen in das Fenster                                                     */
/*****************************************************************************/

PRIVATE WORD wi_drag (src_window, src_obj, dest_window, dest_obj)
WINDOWP src_window;
WORD    src_obj;
WINDOWP dest_window;
WORD    dest_obj;

{
  if (src_window->handle == dest_window->handle) return (DRAG_SWIND); /* Im gleichen Fenster */
  if (src_window->class == dest_window->class) return (DRAG_SCLASS);  /* Gleiche Fensterart */

  return (DRAG_NOACTN);
} /* wi_drag */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

PRIVATE VOID wi_click (window, mk)
WINDOWP window;
MKINFO  *mk;

{
	WORD   i, item;
	XYZ_P xyz;
	STRING s;
	UWORD signal, offset;
	LONG x;	
	
	xyz=&xyz_setups[xyz_setup_nr];

	
	if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
	switch (window->exit_obj)
	{
/*
		case XYZSETINC:
			x = xyz_setup_nr+=1;
			undo_state (window->object, window->exit_obj, SELECTED);
			minmaxsetup(&x, MAXXYZSETUPS);
			set_setnr_xyz(x);
			window->edit_obj = XYZSETNR;
			window->edit_inx = NIL;
			draw_object(window, ROOT);
			break;
		case XYZSETDEC:
			x = xyz_setup_nr-=1;
			undo_state (window->object, window->exit_obj, SELECTED);
			minmaxsetup(&x, MAXXYZSETUPS);
			window->edit_obj = XYZSETNR;
			window->edit_inx = NIL;
			set_setnr_xyz(x);
			draw_object(window, ROOT);
			break;
		case XYZSETSTORE:	get_xyz_setup();
			undo_state (window->object, window->exit_obj, SELECTED);
			dzahl(&xyz_setup_alt, -1, -1, "XYZ-Setup");
			/* Kopiere akt. Set in angegebenes Setup */
			xyz_setups[xyz_setup_alt]=xyz_setups[xyz_setup_nr];
			xyz_setup_nr=xyz_setup_alt;
			set_xyz_setup();
			draw_object(window, XYZSETNR);
			draw_object(window, XYZSETSTORE);
			break;
		case XYZSETRECALL:
			dzahl(&xyz_setup_alt, -1, -1, "XYZ-Setup");
			undo_state (window->object, window->exit_obj, SELECTED);
			/* Kopiere angegebenes Setup in akt. Setup */
			xyz_setups[xyz_setup_nr]=xyz_setups[xyz_setup_alt];
			set_xyz_setup();
			draw_object(window, ROOT);
			break;
		case XYZSETNR:
			window->edit_obj = XYZSETNR;
			window->edit_inx = NIL;
			undo_state (window->object, window->exit_obj, SELECTED);
			if (mk->breturn >1)	/* Nur bei Doppelklick */
			{
				get_ptext (xyz_setup, XYZSETNR, s);
				sscanf (s, "%ld", &x);
				set_setnr_xyz(x);
				draw_object(window, ROOT);
			} /* if */
			else
			{
				draw_object(window, XYZSETNR);
			} /* else */
			break;
*/
		case XYZOK   :
			get_xyz_setup ();
		   break;
		case XYZCANCEL:
			set_xyz_setup ();
		   break;
		case XYZHELP :
			help_xyz(window, NIL);
			undo_state (window->object, window->exit_obj, SELECTED);
			draw_object (window, window->exit_obj);
			break;
		default :	
/*
			if(xyz_setup_nr!=0)
			{
				xyz_setups[0]=xyz_setups[xyz_setup_nr];
				xyz_setup_alt=xyz_setup_nr;
				xyz_setup_nr=0;
				xyz=&xyz_setups[xyz_setup_nr];
				
				sprintf (s, "%ld", xyz_setup_nr);
				set_ptext (xyz_setup, XYZSETNR, s);
				draw_object(window, XYZSETNR);
			} /* if */
*/				
			break;						
	} /* switch */
} /* wi_click */

/*****************************************************************************/

PRIVATE VOID wi_unclick (window)
WINDOWP window;

{
} /* wi_unclick */

/*****************************************************************************/
/* Taste fÅr Fenster                                                         */
/*****************************************************************************/

PRIVATE BOOLEAN wi_key (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  if (menu_key (window, mk)) return (TRUE);

  return (FALSE);
} /* wi_key */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer (window)
WINDOWP window;

{
  if (is_top (window))
  {
  } /* if */
} /* wi_timer */

/*****************************************************************************/
/* Fenster nach oben gebracht                                                */
/*****************************************************************************/

PRIVATE VOID wi_top (window)
WINDOWP window;

{
} /* wi_top */

/*****************************************************************************/
/* Fenster nach unten gebracht                                               */
/*****************************************************************************/

PRIVATE VOID wi_untop (window)
WINDOWP window;

{
} /* wi_untop */

/*****************************************************************************/
/* Cut/Copy/Paste fÅr Fenster                                                */
/*****************************************************************************/

PRIVATE VOID wi_edit (window, action)
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
} /* wi_edit */

/*****************************************************************************/
/* Iconbehandlung                                                            */
/*****************************************************************************/

PUBLIC BOOLEAN icons_xyz (src_obj, dest_obj)
WORD src_obj, dest_obj;

{
  BOOLEAN result;
  WINDOWP window;

  result = FALSE;

  switch (src_obj)
  {
    case IXYZ : window = search_window (CLASS_XYZ, SRCH_ANY, src_obj);
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
} /* icons_clipbrd */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_xyz (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
  WINDOWP window;
  WORD    menu_height, inx;

  inx    = num_windows (CLASS_XYZ, SRCH_ANY, NULL);
  window = create_window (KIND, CLASS_XYZ);

  if (window != NULL)
  {
    menu_height = (menu != NULL) ? gl_hattr : 0;

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
    window->scroll.x  = INITX + gl_wbox;
    window->scroll.y  = INITY + gl_hbox + odd (menu_height);
    window->scroll.w  = obj->ob_width;
    window->scroll.h  = obj->ob_height;
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
    window->hndl_menu = handle_menu;
    window->updt_menu = update_menu;
    window->test      = wi_test;
    window->open      = wi_open;
    window->close     = wi_close;
    window->delete    = wi_delete;
    window->draw      = wi_draw;
    window->arrow     = wi_arrow;
    window->snap      = wi_snap;
    window->objop     = wi_objop;
    window->drag      = wi_drag;
    window->click     = wi_click;
    window->unclick   = wi_unclick;
    window->key       = wi_key;
    window->timer     = wi_timer;
    window->top       = wi_top;
    window->untop     = wi_untop;
    window->edit      = wi_edit;
    window->showinfo  = info_xyz;
    window->showhelp  = help_xyz;

    sprintf (window->name, (BYTE *)xyz_text [FXYZN].ob_spec);
    sprintf (window->info, (BYTE *)xyz_text [FXYZI].ob_spec, 0);
  } /* if */

  return (window);                      /* Fenster zurÅckgeben */
} /* crt_xyz */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_xyz (icon)
WORD icon;

{
  BOOLEAN ok;
  WINDOWP window;

  window = search_window (CLASS_XYZ, SRCH_ANY, XYZ_SETUP);
  if (window == NULL)
  {
    window = crt_xyz (xyz_setup, NULL, XYZ_SETUP);

    if (window != NULL)
    {

      undo_state (window->object, XYZHELP, DISABLED);
    } /* if */
  } /* if */

  if (window != NULL)
  {
    if (window->opened == 0)
    {
      window->edit_obj = find_flags (xyz_setup, ROOT, EDITABLE);
      window->edit_inx = NIL;

		set_xyz_setup ();
		
		if (! open_window (window)) hndl_alert (ERR_NOOPEN);
    } /* if */
	else top_window (window);
  } /* if */
	
	ok= window !=0;
	
	return (ok);
} /* open_xyz */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

PUBLIC BOOLEAN info_xyz (window, icon)
WINDOWP window;
WORD    icon;

{
	WORD		ret;
	STRING	s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, IXYZ);
		
	if (window == NULL)
	{
		 form_center (xyz_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (xyz_info, NULL, IXYZ, (BYTE *)xyz_text [FXYZN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = xyz_info;
		sprintf(s, XYZDATE);
		set_ptext (xyz_info, XYZIVERDA, s);
		sprintf(s, __DATE__);
		set_ptext (xyz_info, XYZICOMPILE, s);
		sprintf(s, XYZVERSION);
		set_ptext (xyz_info, XYZIVERNR, s);
		sprintf(s, "%d", MAXXYZSETUPS);
		set_ptext (xyz_info, XYZISETUPS, s);
		sprintf(s, "%d", xyz_setup_nr);
		set_ptext (xyz_info, XYZIAKT, s);

		if (! open_dialog (IXYZ)) hndl_alert (ERR_NOOPEN);
	}

	return (window != NULL);
} /* info_xyz */

/*****************************************************************************/
/* Hilfe des Objekts                                                         */
/*****************************************************************************/

PUBLIC BOOLEAN help_xyz (window, icon)
WINDOWP window;
WORD    icon;

{
  help ("XYZ-Setup");

  return (TRUE);
} /* help_xyz */

/*****************************************************************************/
PRIVATE	BOOLEAN create_xyz ()
{
	RTMCLASSP 	rtmmodule;
	BOOLEAN		ok;
	STRING		s;

	rtmmodule = create_module ();
	
	if (rtmmodule != NULL)
	{
		rtmmodule -> class_number	= CLASS_XYZ;
		rtmmodule -> icon				= &xyz_desk[XYZICON];
		rtmmodule -> icon_position = IXYZ;
		rtmmodule -> icon_number	= IXYZ;	/* Soll bei Init vergeben werden */
		rtmmodule -> menu_title		= MWINDOWS;
		rtmmodule -> menu_position	= MXYZ;
		rtmmodule -> menu_item		= MXYZ;	/* Soll bei Init vergeben werden */
		rtmmodule -> multiple		= FALSE;
		
		rtmmodule -> create			= crt_xyz;

		rtmmodule -> open				= open_xyz;
		rtmmodule -> info				= info_xyz;
		rtmmodule -> help				= help_xyz;
		rtmmodule -> icons			= icons_xyz;

		rtmmodule -> init				= init_xyz;
		rtmmodule -> term				= term_xyz;

		rtmmodule -> priority		= 50000l;
		rtmmodule -> apply			= apply_xyz;
		rtmmodule -> get_setnr		= get_setnr_xyz;
		rtmmodule -> set_setnr		= set_setnr_xyz;
		rtmmodule -> load				= load_xyz;
		rtmmodule -> save				= save_xyz;
		rtmmodule -> reset			= reset_xyz;

		rtmmodule -> precalc			= precalc_xyz;
		rtmmodule -> message			= message_xyz;
		rtmmodule -> test				= test_xyz;
	
		ok = TRUE;
	}
	else ok=FALSE;
	
	return ok;
}

/*****************************************************************************/
PRIVATE BOOLEAN init_rsc_xyz ()

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
  alexyzsg = &rs_strings [FREESTR];             /* Adresse der Fehlermeldungen */
*/
/*
	xyz_menu  = (OBJECT *)rs_trindex [XYZ_SETUP]; /* Adresse des XYZ-MenÅs */
*/
	xyz_setup = (OBJECT *)rs_trindex [XYZ_SETUP]; /* Adresse der XYZ-Parameter-Box */
	xyz_help  = (OBJECT *)rs_trindex [XYZ_HELP];	/* Adresse der XYZ-Hilfe */
	xyz_desk  = (OBJECT *)rs_trindex [XYZ_DESK];	/* Adresse des XYZ-Desktops */
	xyz_text  = (OBJECT *)rs_trindex [XYZ_TEXT];	/* Adresse der XYZ-Texte */
	xyz_info 	= (OBJECT *)rs_trindex [XYZ_INFO];	/* Adresse der XYZ-Info-Anzeige */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (xyz_rsc_ptr, rsc_name))
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
	rs_gaddr (xyz_rsc_ptr, R_TREE,  XYZ_SETUP,	&xyz_menu);    /* Adresse des XYZ-MenÅs */
*/
	rs_gaddr (xyz_rsc_ptr, R_TREE,  XYZ_SETUP,	&xyz_setup);   /* Adresse der XYZ-Parameter-Box */
	rs_gaddr (xyz_rsc_ptr, R_TREE,  XYZ_HELP,	&xyz_help);    /* Adresse der XYZ-Hilfe */
	rs_gaddr (xyz_rsc_ptr, R_TREE,  XYZ_DESK,	&xyz_desk);    /* Adresse des XYZ-Desktop */
	rs_gaddr (xyz_rsc_ptr, R_TREE,  XYZ_TEXT,	&xyz_text);    /* Adresse der XYZ-Texte */
	rs_gaddr (xyz_rsc_ptr, R_TREE,  XYZ_INFO,	&xyz_info);    /* Adresse der XYZ-Info-Anzeige */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

	/*fix_objs (xyz_menu, TRUE);*/
	fix_objs (xyz_setup, TRUE);
	fix_objs (xyz_help, TRUE);
	fix_objs (xyz_desk, TRUE);
	fix_objs (xyz_text, TRUE);
	fix_objs (xyz_info, TRUE);
	
	/*
	do_flags (xyz_setup, XYZCANCEL, UNDO_FLAG);
	do_flags (xyz_setup, XYZHELP, HELP_FLAG);
	*/
	menu_enable(menu, MXYZ, TRUE);

	return (TRUE);
} /* init_rsc_xyz */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PUBLIC BOOLEAN term_rsc_xyz ()

{
  BOOLEAN ok;

  ok = TRUE;

#if ((XRSC_CREATE|RSC_CREATE) == 0)
  ok = rs_free (xyz_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc_xyz */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_xyz ()

{
	STR128	s;
	FILE		*test;
	BOOLEAN	ok = TRUE;
	XYZ_P		xyz;
	WORD		x;
	
	
	ok &= init_rsc_xyz ();
	ok &= create_xyz ();
	if ((strchr (xyz_filename, DRIVESEP) == NULL) && (strchr (xyz_filename, PATHSEP) == NULL))
		strcpy (s, setup_path);
	else
		s [0] = EOS;
	
	strcat (s, xyz_filename);
	strcpy (xyz_filename, s);
	
	/* PrÅfen, ob DEFAULT-Datei vorhanden */
	if((test=fopen(xyz_filename, "rb"))!=0)
	{
		/* Wenn vorhanden, laden */
		fclose(test);
		load_xyz(xyz_filename, FALSE);
	} /* if */

	/* Initialisierung des Standard-Setups */
	xyz_setup_nr = 1;

 	return (ok);
} /* init_xyz */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_xyz ()
{
	BOOLEAN ok = TRUE;
	ok &= term_rsc_xyz ();
	mem_free(xyz_setups);
	return (ok);
} /* term_xyz */
