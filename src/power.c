/*****************************************************************************/
/*                                                                           */
/* Modul: POWER.C                                                            */
/* Datum: 05/12/90                                                           */
/*                                                                           */
/*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"

#include "realtim4.h"

#include "resource.h"

#include "export.h"
#include "power.h"

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|INFO|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_MOUSE|WI_CURSKEYS)
#define XFAC   wbox                     /* X-Faktor */
#define YFAC   hbox                     /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  (12 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 7 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (18 * wbox)              /* Anfangsbreite in Pixel */
#define INITH  (10 * hbox)              /* Anfangshîhe in Pixel */
#define MILLI  2000                     /* Millisekunden fÅr Zeitablauf */

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

LOCAL WORD lines [] = {0, 0, 31, 20, 16, 14, 12, 12, 11, 10}; /* Zeilen pro Potenz */

LOCAL MFORM mform =
{
  0, 8, 1, WHITE, BLACK,
  {0x0000, 0x0030, 0x0038, 0x0FFC, 0x1FFC, 0x3FFC, 0x6FFC, 0xFFFC,
   0xFFF8, 0XFFF8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x0000, 0x0000, 0x0010, 0x0028, 0x0F88, 0x1FC8, 0x2FF8, 0x7FF8,
   0xFFF0, 0x1040, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}
}; /* mform */

/****** FUNCTIONS ************************************************************/

LOCAL LONG    power       _((WORD base, WORD p));

LOCAL VOID    update_menu _((WINDOWP window));
LOCAL VOID    handle_menu _((WINDOWP window, WORD title, WORD item));
LOCAL VOID    box         _((WINDOWP window, BOOLEAN grow));
LOCAL BOOLEAN wi_test     _((WINDOWP window, WORD action));
LOCAL VOID    wi_open     _((WINDOWP window));
LOCAL VOID    wi_close    _((WINDOWP window));
LOCAL VOID    wi_delete   _((WINDOWP window));
LOCAL VOID    wi_draw     _((WINDOWP window));
LOCAL VOID    wi_arrow    _((WINDOWP window, WORD dir, LONG oldpos, LONG newpos));
LOCAL VOID    wi_snap     _((WINDOWP window, RECT *new, WORD mode));
LOCAL VOID    wi_objop    _((WINDOWP window, SET objs, WORD action));
LOCAL WORD    wi_drag     _((WINDOWP src_window, WORD src_obj, WINDOWP dest_window, WORD dest_obj));
LOCAL VOID    wi_click    _((WINDOWP window, MKINFO *mk));
LOCAL VOID    wi_unclick  _((WINDOWP window));
LOCAL BOOLEAN wi_key      _((WINDOWP window, MKINFO *mk));
LOCAL VOID    wi_timer    _((WINDOWP window));
LOCAL VOID    wi_top      _((WINDOWP window));
LOCAL VOID    wi_untop    _((WINDOWP window));
LOCAL VOID    wi_edit     _((WINDOWP window, WORD action));

/*****************************************************************************/
/* Berechne Potenzen                                                         */
/*****************************************************************************/

LOCAL LONG power (base, p)
REG WORD base, p;

{
  REG LONG result;
 
  for (result = 1; p != 0; result *= base, p--);
  return (result);                  /* Potenz 'p' zur Basis 'base' */
} /* power */

/*****************************************************************************/
/* Box zeichnen                                                              */
/*****************************************************************************/

LOCAL VOID box (window, grow)
WINDOWP window;
BOOLEAN grow;

{
  RECT l, b;

  objc_rect (menu, MOPTIONS, &l, TRUE);

  wind_calc (WC_BORDER, window->kind,       /* Rand berechnen */
             window->work.x, window->work.y, window->work.w, window->work.h,
             &b.x, &b.y, &b.w, &b.h);

  if (grow)
    growbox (&l, &b);
  else
    shrinkbox (&l, &b);
} /* box */

/*****************************************************************************/
/* ôffne Fenster                                                             */
/*****************************************************************************/

LOCAL VOID wi_open (window)
WINDOWP window;

{
  box (window, TRUE);
} /* wi_open */

/*****************************************************************************/
/* Schlieûe Fenster                                                          */
/*****************************************************************************/

LOCAL VOID wi_close (window)
WINDOWP window;

{
  box (window, FALSE);
} /* wi_close */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

LOCAL VOID wi_draw (window)
WINDOWP window;

{
  WORD   i, base;
  WORD   font, point;
  WORD   wchar, hchar;
  WORD   wbox, hbox;
  WORD   x;
  RECT   r;
  STRING s;

  clr_scroll (window);
  text_default (vdi_handle);
  rc_copy (&window->scroll, &r);

  base  = (WORD)(window->special & 0xFF);
  font  = (WORD)(window->special >>  8) & 0xFF;
  point = (WORD)(window->special >> 16) & 0xFF;

  x = window->scroll.x - window->doc.x * window->xfac;

  vst_font (vdi_handle, font);
  vst_point (vdi_handle, point, &wchar, &hchar, &wbox, &hbox);

  for (i = window->doc.y; (i < lines [base]) && (r.y - window->scroll.y < r.h); i++, r.y += window->yfac)
  {
    sprintf (s, "%2d -> %11ld", i, power (base, i));   /* Textzeile bestimmen */
    v_gtext (vdi_handle, x, r.y, s);                   /* Textzeile ausgeben */
  } /* for */
} /* wi_draw */

/*****************************************************************************/
/* Reagiere auf Pfeile                                                       */
/*****************************************************************************/

LOCAL VOID wi_arrow (window, dir, oldpos, newpos)
WINDOWP window;
WORD    dir;
LONG    oldpos, newpos;

{
  LONG delta;

  delta = newpos - oldpos;

  if (delta != 0)
  {
    if (dir & HORIZONTAL)       /* Horizontale Pfeile und Schieber */
    {
      window->doc.x = newpos;                          /* Neue Position */

      set_sliders (window, HORIZONTAL, SLPOS);         /* Schieber setzen */
      scroll_window (window, HORIZONTAL, delta * window->xfac);
    } /* if */
    else                        /* Vertikale Pfeile und Schieber */
    {
      window->doc.y = newpos;                          /* Neue Position */

      set_sliders (window, VERTICAL, SLPOS);           /* Schieber setzen */
      scroll_window (window, VERTICAL, delta * window->yfac);
    } /* else */
  } /* if */
} /* wi_arrow */

/*****************************************************************************/
/* Einrasten des Fensters                                                    */
/*****************************************************************************/

LOCAL VOID wi_snap (window, new, mode)
WINDOWP window;
RECT    *new;
WORD    mode;

{
  RECT r, diff;
  WORD wbox, hbox;
  LONG max_xdoc, max_ydoc;

  wind_get (window->handle, WF_CXYWH, &r.x, &r.y, &r.w, &r.h);

  wbox   = window->xfac;
  hbox   = window->yfac;
  diff.x = (new->x - r.x) / 8 * 8;              /* Differenz berechnen */
  diff.w = (new->w - r.w) / 8 * 8;
  diff.h = (new->h - r.h) / hbox * hbox;

  new->x = r.x + diff.x;                        /* Schnelle Position */
  new->w = r.w + diff.w;                        /* Immer auf 8 Bit */

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
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

LOCAL VOID wi_click (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  WORD   i, item;
  MKINFO key;

  if (sel_window != window) unclick_window (sel_window); /* Deselektieren */

  for (i = POWERS + 1; i < POWERS + 9; i++) menu_icheck (popup, i, FALSE);

  i = POWERS + (WORD)(window->special & 0xFF) - 1;

  menu_icheck (popup, i, TRUE);
  item = popup_menu (popup, POWERS, 0, 0, i, TRUE, mk->momask);

  if ((item != NIL) && (item != i))
  {
    mem_set (&key, 0, (LONG)sizeof (key));
    key.ascii_code = '0' + item - POWERS + 1;

    key_window (window, &key);
  } /* if */
} /* wi_click */

/*****************************************************************************/

LOCAL VOID wi_unclick (window)
WINDOWP window;

{
} /* wi_unclick */

/*****************************************************************************/
/* Taste fÅr Fenster                                                         */
/*****************************************************************************/

LOCAL BOOLEAN wi_key (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  WORD wh;
  WORD key, base;

  if (menu_key (window, mk)) return (TRUE);

  key = mk->ascii_code;                         /* Nur ASCII-Wert interessant */

  if (('2' <= key) && (key <= '9'))             /* Neue Basis */
  {
    wh   = window->handle;
    base = key - '0';

    window->doc.x   = 0;
    window->doc.y   = 0;
    window->doc.h   = lines [base];
    window->special = (window->special & 0xFFFF00L) + base;

    sprintf (window->info, (BYTE *)freetext [FPOWERIN].ob_spec, base); /* Infozeile */

    wind_set (wh, WF_INFO, ADR (window->info), 0, 0); /* Infozeile neu setzen */
    set_sliders (window, HORIZONTAL + VERTICAL, SLPOS + SLSIZE);
    redraw_window (window, &window->scroll);

    return (TRUE);
  } /* if */

  return (FALSE);
} /* wi_key */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

GLOBAL WINDOWP crt_power (obj, menu, icon, font, point)
OBJECT *obj, *menu;
WORD   icon, font, point;

{
  WINDOWP window;
  WORD    inx;
  WORD    wchar, hchar;
  WORD    wbox, hbox;
  WORD    initw, inith;

  vst_font (vdi_handle, font);
  vst_point (vdi_handle, point, &wchar, &hchar, &wbox, &hbox);

  initw = min (desk.w - 4 * gl_wattr, INITW);
  inith = min (desk.h - 4 * gl_hattr, INITH);

  inx    = num_windows (CLASS_POWER, SRCH_ANY, NULL);
  window = create_window (KIND, CLASS_POWER);

  if (window != NULL)
  {
    window->flags     = FLAGS;
    window->icon      = icon;
    window->doc.x     = 0;
    window->doc.y     = 0;
    window->doc.w     = INITW / XFAC;
    window->doc.h     = lines [2];
    window->xfac      = XFAC;
    window->yfac      = YFAC;
    window->xunits    = XUNITS;
    window->yunits    = YUNITS;
    window->scroll.x  = INITX + inx * gl_wbox;
    window->scroll.y  = INITY + inx * gl_hbox;
    window->scroll.w  = initw;
    window->scroll.h  = inith;
    window->work.x    = window->scroll.x;
    window->work.y    = window->scroll.y;
    window->work.w    = window->scroll.w;
    window->work.h    = window->scroll.h;
    window->mousenum  = USER_DEF;
    window->mouseform = &mform;
    window->milli     = MILLI;
    window->special   = 2 + ((LONG)font << 8) + ((LONG)point << 16);
    window->object    = obj;
    window->menu      = menu;
    window->hndl_menu = NULL;
    window->updt_menu = NULL;
    window->test      = NULL;
    window->open      = wi_open;
    window->close     = wi_close;
    window->delete    = NULL;
    window->draw      = wi_draw;
    window->arrow     = wi_arrow;
    window->snap      = wi_snap;
    window->objop     = NULL;
    window->drag      = NULL;
    window->click     = wi_click;
    window->unclick   = wi_unclick;
    window->key       = wi_key;
    window->timer     = NULL;
    window->top       = NULL;
    window->untop     = NULL;
    window->edit      = NULL;
    window->showinfo  = info_power;
    window->showhelp  = help_power;

    strcpy (window->name, (BYTE *)freetext [FPOWERNA].ob_spec);
    sprintf (window->info, (BYTE *)freetext [FPOWERIN].ob_spec, 2);
  } /* if */

  return (window);                      /* Fenster zurÅckgeben */
} /* crt_power */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN open_power (icon, font, point)
WORD icon, font, point;

{
  BOOLEAN ok;
  WINDOWP window;

  window = search_window (CLASS_POWER, SRCH_CLOSED, icon);
  if (window == NULL) window = crt_power (NULL, NULL, icon, font, point);

  ok = window != NULL;

  if (ok) ok = open_window (window);

  return (ok);
} /* open_power */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

GLOBAL BOOLEAN info_power (window, icon)
WINDOWP window;
WORD    icon;

{
  return (FALSE);
} /* info_power */

/*****************************************************************************/
/* Hilfe des Objekts                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN help_power (window, icon)
WINDOWP window;
WORD    icon;

{
  return (FALSE);
} /* help_power */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_power ()

{
  return (TRUE);
} /* init_power */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_power ()

{
  return (TRUE);
} /* term_power */

