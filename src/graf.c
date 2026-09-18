/*****************************************************************************/
/*                                                                           */
/* Modul: GRAF.C                                                             */
/* Datum: 02/02/90                                                           */
/*                                                                           */
/*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"

#include "realtim4.h"

#include "export.h"
#include "graf.h"

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|INFO|SIZER)
#define FLAGS  (WI_RESIDENT)
#define XFAC   1                        /* X-Faktor */
#define YFAC   1                        /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  (desk.x + desk.w / 4)    /* X-Anfangsposition */
#define INITY  (desk.y + desk.h / 6)    /* Y-Anfangsposition */
#define INITW  (desk.w / 3)             /* Anfangsbreite in Pixel */
#define INITH  (desk.h / 3)             /* Anfangshîhe in Pixel */
#define MILLI  1000                     /* Millisekunden fÅr Zeitablauf */

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

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
  WORD style, inx;

  clr_scroll (window);

  style = (WORD)(window->special >> 8);
  inx   = (WORD)(window->special & 0xFFL);

  vsf_interior (vdi_handle, style);             /* Muster */
  vsf_style (vdi_handle, inx);                  /* Kugeln usw. */
  vsf_color (vdi_handle, RED);                  /* rot */
  vsf_perimeter (vdi_handle, TRUE);             /* Umrandung */
  v_ellipse (vdi_handle,                        /* Ellipse malen */
             window->scroll.x + window->scroll.w / 2,
             window->scroll.y + window->scroll.h / 2,
             window->scroll.w / 2,
             window->scroll.h / 2);
} /* wi_draw */

/*****************************************************************************/
/* Taste fÅr Fenster                                                         */
/*****************************************************************************/

LOCAL BOOLEAN wi_key (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  WORD wh;
  WORD key, style, inx, max_inx;

  if (menu_key (window, mk)) return (TRUE);

  key = mk->ascii_code;                         /* Nur ASCII-Wert interessant */

  if ((key == '+') || (key == '-') ||           /* Neuer Index */
      ('0' <= key) && (key <= '3'))             /* Neues Muster */
  {
    wh    = window->handle;
    style = (WORD)(window->special >> 8);
    inx   = (WORD)(window->special & 0xFFL);

    if (('0' <= key) && (key <= '3'))
      style  = key - '0';
    else
      inx += (key == '+') ? 1 : -1;

    switch (style)                              /* Maximaler Index */
    {
      case 2  : max_inx = 24; break;
      case 3  : max_inx = 12; break;
      default : max_inx = 1;  break;
    } /* switch */

    if (inx < 1) inx = max_inx;                 /* Grenzen */
    if (inx > max_inx) inx = 1;

    window->special = style * 256 + inx;
    sprintf (window->info, (BYTE *)freetext [FGRAFINF].ob_spec, style, inx); /* Infozeile */

    wind_set (wh, WF_INFO, ADR (window->info), 0, 0); /* Infozeile neu setzen */
    redraw_window (window, &window->scroll);

    return (TRUE);
  } /* if */
  else
    if (key == '*')
    {
      window->milli = (window->milli == 0) ? MILLI : 0;
      return (TRUE);
    } /* if, else */

  return (FALSE);
} /* wi_key */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

LOCAL VOID wi_timer (window)
WINDOWP window;

{
  MKINFO mk;

  mem_set (&mk, 0, sizeof (mk));
  mk.ascii_code = '+';

  key_window (window, &mk);
} /* wi_timer */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

GLOBAL WINDOWP crt_graf (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
  WINDOWP window;
  WORD    inx;

  inx    = num_windows (CLASS_GRAF, SRCH_ANY, NULL);
  window = create_window (KIND, CLASS_GRAF);

  if (window != NULL)
  {
    window->flags     = FLAGS;
    window->icon      = icon;
    window->doc.x     = 0;
    window->doc.y     = 0;
    window->doc.w     = INITW / XFAC;
    window->doc.h     = INITH / YFAC;
    window->xfac      = XFAC;
    window->yfac      = YFAC;
    window->xunits    = XUNITS;
    window->yunits    = YUNITS;
    window->scroll.x  = INITX + inx * gl_wbox;
    window->scroll.y  = INITY + inx * gl_hbox;
    window->scroll.w  = INITW;
    window->scroll.h  = INITH;
    window->work.x    = window->scroll.x;
    window->work.y    = window->scroll.y;
    window->work.w    = window->scroll.w;
    window->work.h    = window->scroll.h;
    window->mousenum  = ARROW;
    window->mouseform = NULL;
    window->milli     = MILLI;
    window->special   = 2 * 256 + 1;            /* Stil = 2, Muster = 1 */
    window->object    = obj;
    window->menu      = menu;
    window->hndl_menu = NULL;
    window->updt_menu = NULL;
    window->test      = NULL;
    window->open      = wi_open;
    window->close     = wi_close;
    window->delete    = NULL;
    window->draw      = wi_draw;
    window->arrow     = NULL;
    window->snap      = NULL;
    window->objop     = NULL;
    window->drag      = NULL;
    window->click     = NULL;
    window->unclick   = NULL;
    window->key       = wi_key;
    window->timer     = wi_timer;
    window->top       = NULL;
    window->untop     = NULL;
    window->edit      = NULL;
    window->showinfo  = info_graf;
    window->showhelp  = help_graf;

    strcpy (window->name, (BYTE *)freetext [FGRAFNAM].ob_spec);
    sprintf (window->info, (BYTE *)freetext [FGRAFINF].ob_spec, 2, 1);
  } /* if */

  return (window);                      /* Fenster zurÅckgeben */
} /* crt_graf */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN open_graf (icon)
WORD icon;

{
  BOOLEAN ok;
  WINDOWP window;

  window = search_window (CLASS_GRAF, SRCH_CLOSED, icon);
  if (window == NULL) window = crt_graf (NULL, NULL, icon);

  ok = window != NULL;

  if (ok) ok = open_window (window);

  return (ok);
} /* open_graf */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

GLOBAL BOOLEAN info_graf (window, icon)
WINDOWP window;
WORD    icon;

{
  return (FALSE);
} /* info_graf */

/*****************************************************************************/
/* Hilfe des Objekts                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN help_graf (window, icon)
WINDOWP window;
WORD    icon;

{
  return (FALSE);
} /* help_graf */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_graf ()

{
  return (TRUE);
} /* init_graf */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_graf ()

{
  return (TRUE);
} /* term_graf */

