/*****************************************************************************/
/*                                                                           */
/* Modul: MENU.C                                                             */
/*                                                                           */
/*****************************************************************************/

#define RTMVERSION "V 4.04"
#define RTMDATE "09.01.95"

/* HISTORY *******************************************************************
V 4.04
- CUT/COPY/PASTE und FONTS herausgenommen, 09.01.95
- printer, edit, meta, clipboard, image und trash rausgenommen
- MBIG auf MGMI geÑndert in init_menu fÅr F-Tasten
V 4.03
- Bug in hndl_menu bei top->module beseitigt
- msettings auf neue Namen umgebaut
V 4.02
- help umgebaut
- mabout als globale Info-Funktion fÅr RTM4
V 4.01
- Modul-Helpfunktion eingebaut
- Ende-Alert eingebaut
- Umstellung auf neue RTMCLASS-Struktur
*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"

#include "realtim4.h"
#include "errors.h"

#include "clipbrd.h"
#include "desktop.h"
#include "dialog.h"
#include "disk.h"
#include "resource.h"
#include "realtspc.h"

#include "objects.h"

#include "export.h"
#include "menu.h"

/****** DEFINES **************************************************************/

#ifndef OUTPUT
#define OUTPUT      0                   /* TRUE, wenn dies ein Ausgabeprogramm ist */
#endif

#define GEM_OUTPUT  "OUTPUT.APP"        /* Name des GEM-Output Programms */

#define MAX_FONTS       99              /* maximale Anzahl von Fonts */
#define MAX_POINTS     256              /* maximale Anzahl von Punktgrî·en */

#define FONT_SWAPSIZE 3072              /* 3072 * 16 = 48 KByte font swapping */

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

/****** VARIABLES ************************************************************/

LOCAL BOOLEAN  fonts_loaded;    /* Fonts schon geladen ? */
LOCAL WORD     num_fonts;       /* Anzahl verfÅgbarer Fonts */
LOCAL WORD     ccp_ext;         /* Cut/Copy/Paste extern auf Klemmbrett */
LOCAL WORD     g_font;          /* Aktueller Font */
LOCAL WORD     g_point;         /* Aktuelle Punktgrî·e */

LOCAL LISTBOX  lnames;          /* Liste der Fontnamen */
LOCAL LISTBOX  lsizes;          /* Liste der Fontgrî·en */
LOCAL BYTE     *fnames;         /* Zeiger auf Fontnamen */
LOCAL BYTE     *fsizes;         /* Zeiger auf Fontgrî·en */
LOCAL WORD     wnames;          /* Breite der Scrollbox der Fontnamen */
LOCAL WORD     wsizes;          /* Breite der Scrollbox der Fontgrî·en */
LOCAL WORD     nlines;          /* Anzahl Zeilen der Fontnamen */
LOCAL WORD     slines;          /* Anzahl Zeilen der Fontgrî·en */
LOCAL WORD     sel_font;        /* selektierter Fontname */
LOCAL WORD     sel_point;       /* selektierte Fontgrî·e */

LOCAL WORD     font_table [MAX_FONTS];
LOCAL WORD     point_table [MAX_POINTS];

LOCAL WORD     edit_inx;        /* Index Passwort fÅr edit_noecho */
LOCAL BYTE     password [MAX_PASSWORD + 1];

/****** FUNCTIONS ************************************************************/


LOCAL VOID    get_settings   _((VOID));
LOCAL VOID    set_settings   _((VOID));
LOCAL VOID    click_settings _((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN key_settings   _((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN help_settings  _((WINDOWP window, WORD icon));
LOCAL VOID    msettings      _((VOID));


LOCAL VOID    set_fonttable  _((WORD sel_font));
LOCAL VOID    box            _((WINDOWP window, BOOLEAN grow));
LOCAL VOID    get_font       _((VOID));
LOCAL VOID    set_font       _((WORD font, WORD point));
LOCAL VOID    close_font     _((WINDOWP window));
LOCAL VOID    click_font     _((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN help_font      _((WINDOWP window, WORD icon));
LOCAL VOID    draw_font      _((WINDOWP window));

LOCAL VOID    set_lnames     _((WORD num_fonts));
LOCAL VOID    set_lsizes     _((WORD num_points));

LOCAL WORD    font2inx       _((WORD font));
LOCAL WORD    point2inx      _((WORD point));
LOCAL VOID    mselfont       _((VOID));

LOCAL BOOLEAN load_fonts     _((WORD vdi_handle));
LOCAL VOID    unload_fonts   _((WORD vdi_handle));

/*****************************************************************************/

GLOBAL VOID mabout (title)
WORD title;

{
	WINDOWP	window;
	WORD		ret;
	STRING	s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, ABOUT);
		
	if (window == NULL)
	{
		 form_center (about, &ret, &ret, &ret, &ret);
		 window = crt_dialog (about, NULL, ABOUT, (BYTE *)freetext [FABOUT].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		sprintf(s, "%-20s", RTMDATE);
		set_ptext (about, ABOVERDA, s);
		sprintf(s, "%-20s", __DATE__);
		set_ptext (about, ABOCOMPILE, s);
		sprintf(s, "%-20s", RTMVERSION);
		set_ptext (about, ABOVERNR, s);
		sprintf(s, "%-20d", max_rtmmodules);
		set_ptext (about, ABOMSLOTS, s);
		sprintf(s, "%-20d", setcard(used_rtmmodules));
		set_ptext (about, ABOMODULES, s);

		if (! open_dialog (ABOUT)) hndl_alert (ERR_NOOPEN);
	}
} /* mabout */

/*****************************************************************************/

LOCAL VOID get_settings ()
{
  STRING s;

  get_ptext (settings, SETBLINK, s);
  sscanf (s, "%d", &blinkrate);

  ring_bell   = get_checkbox (settings, SETBEEP);
  grow_shrink = get_checkbox (settings, SETGROW);
} /* get_settings */

/*****************************************************************************/

LOCAL VOID set_settings ()

{
  STRING s;

  sprintf (s, "%d", blinkrate);
  set_ptext (settings, SETBLINK, s);
  set_ptext (settings, SETPASSWD, "");

  set_checkbox (settings, SETBEEP, ring_bell);
  set_checkbox (settings, SETGROW, grow_shrink);

  undo_state (settings, SETOK, DISABLED);
} /* set_settings */

/*****************************************************************************/

LOCAL VOID click_settings (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  switch (window->exit_obj)
  {
    case SETPASSWD : edit_inx = window->edit_inx;
                   break;
    case SETOK     : get_settings (); /* Hier kînnte man das Paûwort abfragen */
                   break;           /* Wenn es falsch ist, kînnte man z.B. WI_DLCLOSE zurÅcksetzen */
    case SETCANCEL : set_settings ();
                   break;
    case SETHELP   : help_settings (NULL, NIL);
                   undo_state (window->object, window->exit_obj, SELECTED);
                   draw_object (window, window->exit_obj);
                   break;
  } /* switch */
} /* click_settings */

/*****************************************************************************/

LOCAL BOOLEAN key_settings (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  BYTE *p;

  switch (window->edit_obj)
  {
    case SETBLINK  : p = ((TEDINFO *)settings [SETBLINK].ob_spec)->te_ptext;
                   if ((*p == EOS) == ! is_state (settings, SETOK, DISABLED))
                   {
                     flip_state (settings, SETOK, DISABLED);
                     draw_object (window, SETOK);
                   } /* if */
                   break;
    case SETPASSWD : if (window->exit_obj == 0)  /* durch Tastatur, nicht Maus */
                      edit_noecho (mk, edit_inx, password, MAX_PASSWORD);
                    edit_inx = window->edit_inx;
                    break;
  } /* switch */

  return (FALSE);
} /* key_settings */

/*****************************************************************************/

LOCAL BOOLEAN help_settings (window, icon)
WINDOWP window;
WORD    icon;

{
  BOOLEAN ok;
  WINDOWP helpwin;

  helpwin = search_window (CLASS_DIALOG, SRCH_ANY, SETTINGHELP);

  if (helpwin == NULL)
  {
    settinghelp->ob_x = desk.x + desk.w - settinghelp->ob_width;
    settinghelp->ob_y = desk.y + desk.h - settinghelp->ob_height;
    helpwin = crt_dialog (settinghelp, NULL, SETTINGHELP, (BYTE *)freetext [FHELPSET].ob_spec, WI_MODELESS);
  } /* if */

  ok = helpwin != NULL;

  if (ok)
  {
    if (helpwin->opened == 0)
    {
      helpwin->work.x = helpwin->scroll.x = desk.x + desk.w - settinghelp->ob_width;
      helpwin->work.y = helpwin->scroll.y = desk.y + desk.h - settinghelp->ob_height;
    } /* if */

    if (! open_dialog (SETTINGHELP)) hndl_alert (ERR_NOOPEN);
  } /* if */

  return (ok);
} /* help_settings */

/*****************************************************************************/

LOCAL VOID msettings ()

{
  WINDOWP window;
  WORD    ret;

  window = search_window (CLASS_DIALOG, SRCH_ANY, SETTINGS);

  if (window == NULL)
  {
    form_center (settings, &ret, &ret, &ret, &ret);
    window = crt_dialog (settings, NULL, SETTINGS, (BYTE *)freetext [FSETTING].ob_spec, WI_MODELESS);

    if (window != NULL)
    {
      window->click    = click_settings;
      window->key      = key_settings;
      window->showhelp = help_settings;
      password [0]     = EOS;

      undo_state (window->object, SETTINGHELP, DISABLED);
    } /* if */
  } /* if */

  if (window != NULL)
  {
    if (window->opened == 0)
    {
      window->edit_obj = find_flags (settings, ROOT, EDITABLE);
      window->edit_inx = NIL;

      set_settings ();
    } /* if */

    if (! open_dialog (SETTINGS)) hndl_alert (ERR_NOOPEN);
  } /* if */
} /* msettings */


/*****************************************************************************/

LOCAL VOID set_fonttable (sel_font)
WORD sel_font;

{
  WORD   point, new_point;
  WORD   font, max_fonts, i;
  WORD   char_w, char_h, cell_w, cell_h;
  SET    size;
  STRING name;

  max_fonts = min (num_fonts, MAX_FONTS);

  for (i = 0; i < MAX_FONTS; i++) font_table [i] = FONT_SYSTEM;
  for (i = 0; i < MAX_POINTS; i++) point_table [i] = 0;

  for (font = FONT_SYSTEM; font <= max_fonts; font++)   /* set font indexes */
    font_table [font - 1] = vqt_name (vdi_handle, font, name);

  setclr (size);
  vst_font (vdi_handle, sel_font);
  new_point = 999;

  do                                                    /* find point sizes */
  {
    point     = --new_point;
    new_point = vst_point (vdi_handle, point, &char_w, &char_h, &cell_w, &cell_h);
    setincl (size, new_point);
  } while (new_point <= point);

  for (point = i = 0; point < MAX_POINTS; point++)
    if (setin (size, point)) point_table [i++] = point;
} /* set_fonttable */

/*****************************************************************************/

LOCAL VOID box (window, grow)
WINDOWP window;
BOOLEAN grow;

{
  RECT l, b;

  xywh2rect (0, 0, 0, 0, &l);

  wind_calc (WC_BORDER, window->kind,       /* Rand berechnen */
             window->work.x, window->work.y, window->work.w, window->work.h,
             &b.x, &b.y, &b.w, &b.h);

  if (grow)
    growbox (&l, &b);
  else
    shrinkbox (&l, &b);
} /* box */

/*****************************************************************************/

LOCAL VOID get_font ()

{
  g_font  = font_table [lnames.active];
  g_point = point_table [lsizes.active];
} /* get_font */

/*****************************************************************************/

LOCAL VOID set_font (font, point)

{
  WORD size, inx;
  WORD num_points;

  sel_font  = font;
  sel_point = point;

  set_fonttable (sel_font);
  num_points = 0;
  while (point_table [num_points] != 0) num_points++;
  num_points = min (num_points, MAX_POINTS);

  inx  = font2inx (sel_font);
  size = wnames + 1;
  if (fnames == NULL) fnames = (BYTE *)mem_alloc ((LONG)num_fonts * size);

  lnames.window     = search_window (CLASS_DIALOG, SRCH_ANY, SELFONT);
  lnames.tree       = selfont;
  lnames.itemlist   = (VOID *)fnames;
  lnames.itemsize   = size;
  lnames.indirect   = FALSE;
  lnames.num_items  = num_fonts;
  lnames.first_item = (inx >= nlines) ? inx : 0;
  lnames.active     = inx;
  lnames.sel_state  = CHECKED;
  lnames.root       = SFROOT;
  lnames.items      = SFITEMS;
  lnames.up         = SFUP;
  lnames.down       = SFDOWN;
  lnames.parent     = SFPARENT;
  lnames.slider     = SFSLIDER;

  if (fnames != NULL) set_lnames (num_fonts);
  listbox (&lnames, LIST_INIT, NULL);

  inx  = point2inx (sel_point);
  size = wsizes + 1;
  if (fsizes == NULL) fsizes = (BYTE *)mem_alloc ((LONG)MAX_POINTS * size);

  lsizes.window     = search_window (CLASS_DIALOG, SRCH_ANY, SELFONT);
  lsizes.tree       = selfont;
  lsizes.itemlist   = (VOID *)fsizes;
  lsizes.itemsize   = size;
  lsizes.indirect   = FALSE;
  lsizes.num_items  = num_points;
  lsizes.first_item = (inx >= slines) ? inx : 0;
  lsizes.active     = inx;
  lsizes.sel_state  = CHECKED;
  lsizes.root       = SSROOT;
  lsizes.items      = SSITEMS;
  lsizes.up         = SSUP;
  lsizes.down       = SSDOWN;
  lsizes.parent     = SSPARENT;
  lsizes.slider     = SSSLIDER;

  if (fsizes != NULL) set_lsizes (num_points);
  listbox (&lsizes, LIST_INIT, NULL);
} /* set_font */

/*****************************************************************************/

LOCAL VOID close_font (window)
WINDOWP window;

{
  mem_free (fnames);
  mem_free (fsizes);

  fnames = NULL;
  fsizes = NULL;

  box (window, FALSE);
} /* close_nkey */

/*****************************************************************************/

LOCAL VOID click_font (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  WORD    num_points;
  WORD    active;
  BOOLEAN dclick;

  dclick = FALSE;

  if ((SFROOT <= window->exit_obj) && (window->exit_obj <= SFDOWN))
  {
    active = lnames.active;
    dclick = listbox (&lnames, LIST_CLICK, mk);
    set_fonttable (font_table [lnames.active]);

    if (active != lnames.active)
    {
      lsizes.first_item = 0;
      lsizes.active     = 0;
      sel_point         = point_table [0];
      num_points        = 0;
      while (point_table [num_points] != 0) num_points++;

      set_lsizes (num_points);
      listbox (&lsizes, LIST_INIT | LIST_DRAW, mk);
      draw_font (window);
    } /* if */
  } /* if */

  if ((SSROOT <= window->exit_obj) && (window->exit_obj <= SSDOWN))
  {
    active = lsizes.active;
    dclick = listbox (&lsizes, LIST_CLICK, mk);
    if (active != lsizes.active) draw_font (window);
  } /* if */

  if (dclick)
  {
    window->exit_obj  = SFOK;
    window->flags    |= WI_DLCLOSE;
  } /* if */

  switch (window->exit_obj)
  {
    case SFOK     : get_font ();
                    break;
    case SFCANCEL : set_font (g_font, g_point);
                    break;
    case SFHELP   : help_font (NULL, NIL);
                    undo_state (window->object, window->exit_obj, SELECTED);
                    draw_object (window, window->exit_obj);
                    break;
  } /* switch */
} /* click_font */

/*****************************************************************************/

LOCAL BOOLEAN help_font (window, icon)
WINDOWP window;
WORD    icon;

{
  BOOLEAN ok;
  WINDOWP helpwin;

  helpwin = search_window (CLASS_DIALOG, SRCH_ANY, FONTHELP);

  if (helpwin == NULL)
  {
    fonthelp->ob_x = desk.x + desk.w - fonthelp->ob_width;
    fonthelp->ob_y = desk.y + desk.h - fonthelp->ob_height;
    helpwin = crt_dialog (fonthelp, NULL, FONTHELP, (BYTE *)freetext [FHELPFON].ob_spec, WI_MODELESS);
  } /* if */

  ok = helpwin != NULL;

  if (ok)
  {
    if (helpwin->opened == 0)
    {
      helpwin->work.x = helpwin->scroll.x = desk.x + desk.w - fonthelp->ob_width;
      helpwin->work.y = helpwin->scroll.y = desk.y + desk.h - fonthelp->ob_height;
    } /* if */

    if (! open_dialog (FONTHELP)) hndl_alert (ERR_NOOPEN);
  } /* if */

  return (ok);
} /* help_font */

/*****************************************************************************/

LOCAL VOID draw_font (window)
WINDOWP window;

{
  WORD font, point;
  WORD wchar, hchar, wbox, hbox;
  WORD x, y, w, h, diff;
  RECT r, old_clip;

  font  = font_table [lnames.active];
  point = point_table [lsizes.active];

  text_default (vdi_handle);
  vst_color (vdi_handle, RED);
  vst_font (vdi_handle, font);
  vst_point (vdi_handle, point, &wchar, &hchar, &wbox, &hbox);

  objc_offset (selfont, SFXAMPLE, &x, &y);
  w = selfont [SFXAMPLE].ob_width;
  h = selfont [SFXAMPLE].ob_height;
  xywh2rect (x, y, w, h, &r);

  diff = h - hbox;
  if (diff < 0) diff = 0;

  if (find_top () == window) set_clip (TRUE, &r);

  if (rc_intersect (&clip, &r))
  {
    old_clip = clip;
    set_clip (TRUE, &r);
    clr_area (&r);
    v_gtext (vdi_handle, x, y + diff / 2, (BYTE *)freetext [FTXTDEMO].ob_spec);
    set_clip (TRUE, &old_clip);
  } /* if */
} /* draw_font */

/*****************************************************************************/

LOCAL VOID set_lnames (num_fonts)
WORD num_fonts;

{
  WORD   font;
  STRING name;
  BYTE   *mem;

  lnames.num_items = num_fonts;

  for (font = FONT_SYSTEM, mem = fnames; font <= num_fonts; font++)
  {
    vqt_name (vdi_handle, font, name);
    if (font == FONT_SYSTEM) strcpy (name, "System");
    name [wnames - 2] = EOS;            /* Name muû mit 2 Leerzeichen beginnen */

    sprintf (mem, "  %s", name);
    mem += lnames.itemsize;
  } /* for */
} /* set_lnames */

/*****************************************************************************/

LOCAL VOID set_lsizes (num_points)
WORD num_points;

{
  WORD index, point;
  BYTE *mem;

  lsizes.num_items = num_points;

  for (index = 0, mem = fsizes; index < num_points; index++)
  {
    point = point_table [index];
    if (wsizes - 3 >= 0) sprintf (mem, " %*d", wsizes - 3, point);
    mem += lsizes.itemsize;
  } /* for */
} /* set_lsizes */

/*****************************************************************************/

LOCAL WORD font2inx (font)
WORD font;

{
  WORD i;

  for (i = 0; i < MAX_FONTS; i++)
    if (font_table [i] == font) return (i);

  return (0);
} /* font2inx */

/*****************************************************************************/

LOCAL WORD point2inx (point)
WORD point;

{
  WORD i;

  for (i = 0; i < MAX_POINTS; i++)
    if (point_table [i] == point) return (i);

  return (0);
} /* point2inx */

/*****************************************************************************/

LOCAL VOID mselfont ()

{
  WINDOWP window;
  WORD    ret;

  window = search_window (CLASS_DIALOG, SRCH_ANY, SELFONT);

  if (window == NULL)
  {
    form_center (selfont, &ret, &ret, &ret, &ret);
    window = crt_dialog (selfont, NULL, SELFONT, (BYTE *)freetext [FSELFONT].ob_spec, WI_MODELESS);

    if (window != NULL)
    {
      window->draw     = draw_font;
      window->close    = close_font;
      window->click    = click_font;
      window->showhelp = help_font;

      undo_state (window->object, SFHELP, DISABLED);

      fnames = NULL;
      fsizes = NULL;
    } /* if */
  } /* if */

  if (window != NULL)
  {
    if (window->opened == 0)
    {
      window->edit_obj = find_flags (selfont, ROOT, EDITABLE);
      window->edit_inx = NIL;

      set_font (g_font, g_point);
    } /* if */

    if (! open_dialog (SELFONT)) hndl_alert (ERR_NOOPEN);
  } /* if */
} /* mselfont */

/*****************************************************************************/

LOCAL BOOLEAN load_fonts (vdi_handle)
WORD vdi_handle;

{
  if (! fonts_loaded && gdos_ok ())
  {
    busy_mouse ();

    num_fonts    = vst_ex_load_fonts (vdi_handle, 0, FONT_SWAPSIZE, 0);
    fonts_loaded = num_fonts > 0;

    if (fonts_loaded)
      num_fonts++;                                      /* add system font */
    else
    {
      if (num_fonts < 0) unload_fonts (vdi_handle);     /* release font memory */
      num_fonts = 1;                                    /* leave system font */
    } /* else */

    arrow_mouse ();
  } /* if */

  return (fonts_loaded);
} /* load_fonts */

/*****************************************************************************/

LOCAL VOID unload_fonts (vdi_handle)
WORD vdi_handle;

{
  if (fonts_loaded && gdos_ok ())
  {
    vst_unload_fonts (vdi_handle, 0);

    num_fonts    = 1;
    fonts_loaded = FALSE;
  } /* if */
} /* unload_fonts */

/*****************************************************************************/
/* MenÅ-Verarbeitung                                                         */
/*****************************************************************************/

GLOBAL VOID updt_menu (window)
WINDOWP window;

{
  WORD    i;
  SET     after;
  LONGSTR s;
  WINDOWP top;
  BOOLEAN ccp;

  if (menu_ok && updtmenu)
  {
    top = find_top ();
    ccp = (top != NULL) && (top->test != NULL) && (top->edit != NULL);

    menu_enable (menu, MCLOSE,   any_open (FALSE, TRUE, FALSE));
    menu_enable (menu, MPRINT,   (sel_window != NULL) && (sel_window->class == CLASS_CLIPBRD));
/*
    menu_enable (menu, MUNDO,    ccp && (*top->test) (top, DO_UNDO   | ccp_ext));
    menu_enable (menu, MCUT,     ccp && (*top->test) (top, DO_CUT    | ccp_ext));
    menu_enable (menu, MCOPY,    ccp && (*top->test) (top, DO_COPY   | ccp_ext));
    menu_enable (menu, MPASTE,   ccp && (*top->test) (top, DO_PASTE  | ccp_ext));
    menu_enable (menu, MCLEAR,   ccp && (*top->test) (top, DO_CLEAR  | ccp_ext));
    menu_enable (menu, MSELALL,  ccp && (*top->test) (top, DO_SELALL | ccp_ext));
    menu_enable (menu, MLOADFNT, gdos_ok () && ! fonts_loaded);
    menu_enable (menu, MUNLOADF, gdos_ok () && fonts_loaded);
*/
    setclr (after);

    for (i = 0; i < 10; i++)
      if ((funcmenus [i].item == 0) ||
          is_state (menu, funcmenus [i].title, DISABLED) ||
          is_state (menu, funcmenus [i].item, DISABLED))
        setexcl (after, i);
      else
        setincl (after, i);

    setxor (menus, after);

    if (! setcmp (menus, NULL))         /* Es hat sich etwas geÑndert */
    {
      for (i = 0, *s = EOS; i < 10; i++)
      {
        if (setin (after, i)) strcat (s, (BYTE *)freetext [FM1 + i].ob_spec);
        strcat (s, ",");
      } /* for */

      set_func (s);
      hide_mouse ();

      for (i = 0; i < 10; i++)
        if (setin (menus, i)) draw_key (i + 1);

      show_mouse ();
    } /* if */

    setcpy (menus, after);
  } /* if */

  updtmenu = TRUE;      /* MenÅs immer auf neuesten Stand bringen */
} /* updt_menu */

/*****************************************************************************/

GLOBAL VOID hndl_menu (window, title, item)
WINDOWP window;
WORD    title, item;

{
  WINDOWP top;
  BOOLEAN to_clip;
  WORD    obj;
  MKINFO  mk;
  REG WORD i;
  REG RTMCLASSP rtmmodule;
  BOOLEAN ok = FALSE;

  if (is_state (menu, title, DISABLED) ||       /* Accessory kînnte Nachricht geschickt haben */
      is_state (menu, item, DISABLED)) return;

  menu_normal (window, title, FALSE);           /* Titel invers darstellen */

  top = find_top ();

	/* Untersuche alle RTM-Module auf passenden MenÅpunkt */
	 	
	for (i = 0; i < rtmtop && !ok; i++)         	/* Untersuche alle Module */
	{
		rtmmodule = rtmmodules [i];
	
		switch (item)
		{
			case MOPEN :
				if(rtmmodule == Module(top))
					if(rtmmodule->open != 0)
					{
						(rtmmodule->open) (rtmmodule->icon_position);
						ok = TRUE;	/* Passendes Modul gefunden */
					}
				break;
			case MLOAD :
				if(rtmmodule == Module(top))
					if(rtmmodule->load != 0)
					{
						(rtmmodule->load) (rtmmodule, NULL, TRUE);
						ok = TRUE;	/* Passendes Modul gefunden */
					}
				break;
			case MSAVE :
				if(rtmmodule == Module(top))
					if(rtmmodule->save != 0)
					{
						(rtmmodule->save) (rtmmodule, NULL, FALSE);
						ok = TRUE;	/* Passendes Modul gefunden */
					}
				break;
			case MSAVEAS :
				if(rtmmodule == Module(top))
					if(rtmmodule->save != 0)
					{
						(rtmmodule->save) (rtmmodule, NULL, TRUE);
						ok = TRUE;	/* Passendes Modul gefunden */
					}
				break;
			case MIMPORT :
				if(rtmmodule == Module(top))
					if(rtmmodule->import != 0)
					{
						(rtmmodule->import) (rtmmodule, NULL, TRUE);
						ok = TRUE;	/* Passendes Modul gefunden */
					}
				break;
			case MEXPORT :
				if(rtmmodule == Module(top))
					if(rtmmodule->export != 0)
					{
						(rtmmodule->export) (rtmmodule, NULL, TRUE);
						ok = TRUE;	/* Passendes Modul gefunden */
					}
				break;
			case MHELP :
				if(rtmmodule == Module(top))
				{
					if(rtmmodule->help != 0)
					{
						ok = (rtmmodule->help) (rtmmodule);
					} /* if */
				} /* if */
				/*
				else
					if (top)
						if (top->name) help_rtm (top->name);
				*/
				break;
			default:
				if ((rtmmodule->menu_item == item))	/* MenÅpunkt identisch? */
				{
					(rtmmodule->open) (rtmmodule->icon_position);
					ok = TRUE;	/* Passendes Modul gefunden */
				} /* if */
				break;
		} /* switch */
	} /* for */

	if (ok==FALSE) /* kein Modul gefunden, "normale" MenÅabfrage */
	{
		switch (title)
		{
		 case MDESK    :
		 	if (item == MABOUT) mabout (title);
		   break;
		 case MFILE    :
		 switch (item)
		 {
		 	case MOPEN   :
		 		if (sel_window == NULL)
		      open_disk (NIL);
		      else 
		      	if (sel_window->objop != NULL)
		      		(*sel_window->objop) (sel_window, sel_objs, OBJ_OPEN);
		      break;
		   case MCLOSE  :
		   	close_top ();
		      break;
		   case MINFO   :
		   	if (sel_window != NULL)
		      {
		      	if (sel_window->objop != NULL)
		         	(*sel_window->objop) (sel_window, sel_objs, OBJ_INFO);
		      } /* if */
		      else
		      	if ((top == NULL) || (top->opened == 0))
		         	mabout (title);
		         else
						if ((top->showinfo == NULL) || ! (*top->showinfo) (top, NIL)) hndl_alert (ERR_NOINFO);
		      break;
		    case MHELP   :
		    if ((top == NULL) || (top->opened == 0))
				hndl_alert (ERR_NOHELP);
		    else
		    	if (top->flags & (WI_MODAL | WI_MODELESS))
		      {
		      	obj = find_flags (top->object, ROOT, HELP_FLAG);
					if (obj == NIL)
		         	hndl_alert (ERR_NOHELP);
		         else
		         {
		         	top->exit_obj = obj;
		            do_state (top->object, obj, SELECTED);
		            draw_object (top, obj);
		            mem_set (&mk, 0, (UWORD)sizeof (mk));
		            if (top->click != NULL) (*top->click) (top, &mk);
		          } /* else */
		       } /* if */
		       else
		       if (sel_window != NULL)
		       {
		       	if (sel_window->objop != NULL)
		         	(*sel_window->objop) (sel_window, sel_objs, OBJ_HELP);
		       } /* if */
		       else
					if ((top->showhelp == NULL) || ! (*top->showhelp) (top, NIL)) hndl_alert (ERR_NOHELP);
		    break;
		 case MNWINDOW :
		   cycle_window ();
    		break;
		 case MPRINT  :
		 	/*
		  	print_clipfiles (sel_window, sel_objs);
		  	*/
		  	break;
/*
		 case MCALLER :
		 	done = TRUE;       /* ZurÅck zum Aufrufer */
		   break;
*/
		 case MQUIT   :
		 	if (hndl_alert(ERR_QUIT) == 2)
		   {
		   	done = TRUE;
		      called_by [0] = EOS;  /* Programm ganz beenden */
		   } /* if */
		  	break;
		  } /* switch */
		  break;
/*
		 case MEDIT    :
		 	switch (item)
		   {
		   	case MUNDO   :
		   		(*top->edit) (top, DO_UNDO   | ccp_ext); break;
		      case MCUT    :
		      	(*top->edit) (top, DO_CUT    | ccp_ext); break;
		      case MCOPY   :
		      	(*top->edit) (top, DO_COPY   | ccp_ext); break;
		      case MPASTE  :
		      	(*top->edit) (top, DO_PASTE  | ccp_ext); break;
		      case MCLEAR  :
		      	(*top->edit) (top, DO_CLEAR  | ccp_ext); break;
		      case MSELALL :
		      	(*top->edit) (top, DO_SELALL | ccp_ext); break;
		      case MTOCLIP :
		      	to_clip = ccp_ext == 0;
		         menu_check (menu, MTOCLIP, to_clip);
		         ccp_ext = to_clip ? DO_EXTERNAL : 0;
		         break;
		   } /* switch */
		   break;
*/
		case MCONTROLS :
			switch (item)
			{
			} /* switch */
			break;
		case MSETUPS :
			switch (item)
			{
			}
		case MINPUTS :
			switch (item)
			{
			}
		case MOUTPUTS : 
			switch (item)
			{
			}
		case MOPTIONS :
			switch (item)
			{
			case MSETTING :
				msettings ();
				break;
/*
			case MLOADFNT :
				load_fonts (vdi_handle);
				set_meminfo ();
				break;
			case MUNLOADF :
				unload_fonts (vdi_handle);
				set_meminfo ();
				break;
			case MSELFONT :
				mselfont ();
				break;
*/
			} /* switch */
			break;
		} /* switch */
	} /* ok */
	
  menu_normal (window, title, TRUE);            /* Titel wieder normal darstellen */
} /* hndl_menu */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_menu ()

{
  WORD   title, menubox, i;
  WORD   ddiff;
  RECT   mbox;
#if OUTPUT
  BYTE   *p;
  STRING s, ext;
#endif

  get_settings ();

  funcmenus [0].title = MFILE;            /* MenÅs der Funktionstasten */
  funcmenus [0].item  = MHELP;

  funcmenus [1].title = MFILE;
  funcmenus [1].item  = MOPEN;

  funcmenus [2].title = MFILE;
  funcmenus [2].item  = MCLOSE;

  funcmenus [3].title = MFILE;
  funcmenus [3].item  = MINFO;

  funcmenus [4].title = MOUTPUTS;
  funcmenus [4].item  = MA3D;

  funcmenus [5].title = MPUFS;
  funcmenus [5].item  = MPUF;

  funcmenus [6].title = MPUFS;
  funcmenus [6].item  = MGMI;

  funcmenus [7].title = MOUTPUTS;
  funcmenus [7].item  = MKOO;

  funcmenus [8].title = MCONTROLS;
  funcmenus [8].item  = MTRA;

  funcmenus [9].title = MFILE;
  funcmenus [9].item  = MQUIT;

  setclr (menus);                         /* Keine MenÅs auf Funktionstasten */

  menu_ok      = (menu != NULL);
  menu_fits    = FALSE;
  fonts_loaded = FALSE;
/*
  ccp_ext      = (menu_ok && is_state (menu, MTOCLIP, CHECKED)) ? DO_EXTERNAL : 0;
*/
  if (menu_ok) menu_fits = menu [THEACTIVE].ob_x + menu [THEACTIVE].ob_width <= desk.w;
  if ((class_desk == DESK) && menu_ok && ! menu_fits) class_desk = DESKWINDOW; /* MenÅzeile im Fenster */

  if (menu_ok)
  {
#if OUTPUT
    if (*called_by)     /* Ersetze "An Ausgabe" durch "An Called_by" */
    {
      file_split (called_by, NULL, NULL, s, ext);
      str_lower (s + 1);
      p = (BYTE *)menu [MCALLER].ob_spec;
      for (p += 2; *p != SP; p++);
      mem_move (p + 1, s, strlen (s));
      for (p += strlen (s) + 1; *p && (*p != SP); p++) *p = SP;
    } /* if */
#else
    strcpy (called_by, GEM_OUTPUT);
    shel_find (called_by);
#endif

/*
    if (deskacc || (*called_by == EOS))
    {
      i = MCALLER;
      while (menu [i].ob_next > i) menu [++i].ob_y -= gl_hbox;
      menu [menu [i].ob_next].ob_height -= gl_hbox;
      objc_delete (menu, MCALLER);
    } /* if */
*/
  } /* if */

  if (menu_ok && menu_fits)
  {
    menubox = menu [ROOT].ob_tail;
    menubox = menu [menubox].ob_head;
    title   = THEFIRST;

    do
    {
      objc_rect (menu, menubox, &mbox, FALSE);

      ddiff = mbox.x + mbox.w + gl_wbox - (desk.x + desk.w); /* Differenz zum Desktop (gl_wbox wegen OUTLINED) */

      if (ddiff > 0) menu [menubox].ob_x -= ddiff;      /* Hing rechts heraus */

      menubox = menu [menubox].ob_next;                 /* NÑchstes Drop-Down-MenÅ */
      title   = menu [title].ob_next;                   /* NÑchster Titel */
    } while (title != THEACTIVE);
  } /* if */

  g_font    = FONT_SYSTEM;
  g_point   = gl_point;
  num_fonts = 1;

  set_fonttable (FONT_SYSTEM);

  wnames = selfont [SFITEMS].ob_width / gl_wbox;
  wsizes = selfont [SSITEMS].ob_width / gl_wbox;

  nlines = selfont [SFITEMS].ob_height / gl_hbox;
  slines = selfont [SSITEMS].ob_height / gl_hbox;
  return (TRUE);
} /* init_menu */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_menu ()

{
  unload_fonts (vdi_handle);
  return (TRUE);
} /* term_menu */

