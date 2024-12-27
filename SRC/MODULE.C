/*****************************************************************************/
/*                                                                           */
/* Modul: MODULE.C                                                           */
/* Datum: 02/08/90                                                           */
/*                                                                           */
/*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"

#include "realtim4.h"

#include "desktop.h"

#include "export.h"
#include "module.h"

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|INFO|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_RESIDENT)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (36 * gl_wbox)           /* Anfangsbreite in Pixel */
#define INITH  ( 8 * gl_hbox)           /* Anfangshîhe in Pixel */
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
/* MenÅbehandlung                                                            */
/*****************************************************************************/

LOCAL VOID update_menu (window)
WINDOWP window;

{
} /* update_menu */

/*****************************************************************************/

LOCAL VOID handle_menu (window, title, item)
WINDOWP window;
WORD    title, item;

{
  if (window != NULL)
    menu_normal (window, title, FALSE);         /* Titel invers darstellen */

  if (window != NULL)
    menu_normal (window, title, TRUE);          /* Titel wieder normal darstellen */
} /* handle_menu */

/*****************************************************************************/
/* Box zeichnen                                                              */
/*****************************************************************************/

LOCAL VOID box (window, grow)
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

LOCAL BOOLEAN wi_test (window, action)
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
/* Lîsche Fenster                                                            */
/*****************************************************************************/

LOCAL VOID wi_delete (window)
WINDOWP window;

{
} /* wi_delete */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

LOCAL VOID wi_draw (window)
WINDOWP window;

{
  clr_scroll (window);
} /* wi_draw */

/*****************************************************************************/
/* Reagiere auf Pfeile                                                       */
/*****************************************************************************/

LOCAL VOID wi_arrow (window, dir, oldpos, newpos)
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

LOCAL VOID wi_snap (window, new, mode)
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

LOCAL VOID wi_objop (window, objs, action)
WINDOWP window;
SET     objs;
WORD    action;

{
} /* wi_objopen */

/*****************************************************************************/
/* Ziehen in das Fenster                                                     */
/*****************************************************************************/

LOCAL WORD wi_drag (src_window, src_obj, dest_window, dest_obj)
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

LOCAL VOID wi_click (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
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
  if (menu_key (window, mk)) return (TRUE);

  return (FALSE);
} /* wi_key */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

LOCAL VOID wi_timer (window)
WINDOWP window;

{
  if (is_top (window))
  {
  } /* if */
} /* wi_timer */

/*****************************************************************************/
/* Fenster nach oben gebracht                                                */
/*****************************************************************************/

LOCAL VOID wi_top (window)
WINDOWP window;

{
} /* wi_top */

/*****************************************************************************/
/* Fenster nach unten gebracht                                               */
/*****************************************************************************/

LOCAL VOID wi_untop (window)
WINDOWP window;

{
} /* wi_untop */

/*****************************************************************************/
/* Cut/Copy/Paste fÅr Fenster                                                */
/*****************************************************************************/

LOCAL VOID wi_edit (window, action)
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

GLOBAL BOOLEAN icons_module (src_obj, dest_obj)
WORD src_obj, dest_obj;

{
  BOOLEAN result;
  WINDOWP window;

  result = FALSE;

  switch (src_obj)
  {
    case IMODULE : window = search_window (CLASS_MODULE, SRCH_ANY, src_obj);
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

GLOBAL WINDOWP crt_module (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
  WINDOWP window;
  WORD    menu_height, inx;

  inx    = num_windows (CLASS_MODULE, SRCH_ANY, NULL);
  window = create_window (KIND, CLASS_MODULE);

  if (window != NULL)
  {
    menu_height = (menu != NULL) ? gl_hattr : 0;

    window->flags     = FLAGS;
    window->icon      = icon;
    window->doc.x     = 0;
    window->doc.y     = 0;
    window->doc.w     = INITW / XFAC;
    window->doc.h     = 0;
    window->xfac      = XFAC;
    window->yfac      = YFAC;
    window->xunits    = XUNITS;
    window->yunits    = YUNITS;
    window->scroll.x  = INITX + inx * gl_wbox;
    window->scroll.y  = INITY + inx * gl_hbox + odd (menu_height);
    window->scroll.w  = INITW;
    window->scroll.h  = INITH;
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
    window->showinfo  = info_module;
    window->showhelp  = help_module;

    sprintf (window->name, (BYTE *)freetext [FMODULEN].ob_spec);
    sprintf (window->info, (BYTE *)freetext [FMODULEI].ob_spec, 0);
  } /* if */

  return (window);                      /* Fenster zurÅckgeben */
} /* crt_module */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN open_module (icon)
WORD icon;

{
  BOOLEAN ok;
  WINDOWP window;

  window = search_window (CLASS_MODULE, SRCH_CLOSED, icon);
  if (window == NULL) window = crt_module (NULL, NULL, icon);

  ok = window != NULL;

  if (ok) ok = open_window (window);

  return (ok);
} /* open_module */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

GLOBAL BOOLEAN info_module (window, icon)
WINDOWP window;
WORD    icon;

{
  if (icon != NIL)
    window = search_window (CLASS_MODULE, SRCH_ANY, icon);

  if (window != NULL) note (1, INFMODULE, NIL, NULL);
  return (window != NULL);
} /* info_module */

/*****************************************************************************/
/* Hilfe des Objekts                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN help_module (window, icon)
WINDOWP window;
WORD    icon;

{
  note (1, HELPMODULE, NIL, NULL);
  return (TRUE);
} /* help_module */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_module ()

{
  return (TRUE);
} /* init_module */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_module ()

{
  return (TRUE);
} /* term_module */

