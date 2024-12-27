/*****************************************************************************/
/*                                                                           */
/* Modul: WINDOWS.C                                                          */
/* Datum: 05.02.95                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
05.11.2024
- max_reswind als LONG
05.02.95
- close verhindern bei nicht exit-objekten, 05.02.95
- +/- fÅr Zahlenfelder herausgenommen, 03.02.95
- Vorrang fÅr TRA vor anderen Fenstern in key_all, 04.01.95
- Vorrang fÅr dispobj vor window in key_window, 04.01.05
15.07.94
- save_info_obj nun auch bei geschlossenem Fenster
- dispobj->key wird nur angesprungen wenn vorhanden
- wi_timer nur noch bei offenem Fenster
18.05.94
- move_window mit dispobj->moved
- bug bei dispobj->click beseitigt
- save_info_obj eingebaut
- dispobj eingebaut
- window->start eingebaut
- click_window: selektierbare Texte auch fÅr G_TEXT
- window->finished eingebaut
- TRA-Vorfahrt wieder eingebaut gebaut
- angepaût fÅr TRA-Vorfahrt bei den Tasten                                 
- Erweiterung in click_window fÅr selektierbare Texte                          
- Erweiterung in draw_object fÅr ob_lines                                  
*****************************************************************************/

#include <ctype.h>

#include "import.h"
#include "global.h"
#include "lists.h"
#include "realtspc.h"			/* for objects.h */
#include "objects.h"		/* for save_info_obj */
#include "dispobj.h"

#include "export.h"
#include "windows.h"

/****** DEFINES **************************************************************/

#define MIN_WIDTH   (8 * gl_wbox)         /* Kleinste Breite */
#define MIN_HEIGHT  (4 * gl_hbox)         /* Kleinste Hîhe */

#define M_LTARROW   1                     /* MenÅpfeil links */
#define M_RTARROW   2                     /* MenÅpfeil rechts */

#define DLG_CNTRL(mk) ((mk->ascii_code == ESC      ) || \
                       (mk->ascii_code == CR       ) || \
                       (mk->scan_code  == DELETE   ) || \
                       (mk->scan_code  == BACKSPACE) || \
                       (mk->scan_code  == TAB      ) || \
                       (mk->scan_code  == LEFT     ) || \
                       (mk->scan_code  == RIGHT    ) || \
                       (mk->scan_code  == UP       ) || \
                       (mk->scan_code  == DOWN     ) || \
                       (mk->scan_code  == POS1     ) || \
                       (mk->scan_code  == ENDKEY   ) || \
                       (mk->scan_code  == PGUP     ) || \
                       (mk->scan_code  == PGDOWN   ))

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

LOCAL WORD    top;                        /* Anzahl der Fenster */
LOCAL WINDOWP *windows;                   /* Fensterkeller */
LOCAL WINDOWP windrec;                    /* Speicher fÅr Fensterzeiger */
LOCAL WORD    max_windows;                /* Maximale Anzahl Fenster */
LOCAL WORD    nowindow;                   /* Fehler fÅr "Kein Fenster mehr" */
LOCAL WORD    menu_offset;                /* X-Offset MenÅzeile */
LOCAL SET     used_windows;               /* Slots der belegten Fenster */
LOCAL WORD    helpclass;                  /* Klasse des nicht-modalen Hilfefensters */
LOCAL BOOLEAN graph_proc;                 /* Graphik Prozessor aktiv */

#if GEM & GEM1
LOCAL WORD    width;                      /* Breite von THEACTIVE */
#endif

LOCAL OBJECT arrows [3] =
{
  -1,  1,  2, G_BOX,     NONE,   NORMAL, 0x1100L,      0, 0, 772,513,
   2, -1, -1, G_BOXCHAR, NONE,   NORMAL, 0x4FF1100L,   0, 0, 258,513,
   0, -1, -1, G_BOXCHAR, LASTOB, NORMAL, 0x3FF1100L, 514, 0, 258,513
}; /* arrows */

/****** FUNCTIONS ************************************************************/

LOCAL WORD    find_slot    _((WORD wh));
LOCAL WORD    find_wslot   _((WINDOWP window));
LOCAL VOID    sort_order   _((VOID));
LOCAL VOID    move_order   _((WORD newtop, WINDOWP window));
LOCAL VOID    get_work     _((WINDOWP window, BOOLEAN work));
LOCAL VOID    edit_object  _((WINDOWP window, WORD key, WORD kind));
LOCAL WORD    find_first   _((OBJECT *object));
LOCAL WORD    find_last    _((OBJECT *object));
LOCAL VOID    arrow_object _((WINDOWP window, WORD dir, LONG oldpos, LONG newpos));
LOCAL BOOLEAN in_icon      _((WORD mox, WORD moy, WINDOWP window, WORD obj));
LOCAL VOID    set_style    _((WORD expr));
LOCAL VOID    draw_box     _((CONST RECT *box, WORD x_offset, WORD y_offset));
LOCAL VOID    draw_all     _((WORD num_boxes, CONST RECT *boxes, WORD x_offset, WORD y_offset, CONST RECT *bound, CONST RECT *inner, RECT *diff));
LOCAL WORD    calc_pos     _((LISTBOX *list));
LOCAL WORD    calc_size    _((LISTBOX *list));
LOCAL BOOLEAN in_listbox   _((LISTBOX *list));
LOCAL WORD    item2obj     _((LISTBOX *list, WORD item));
LOCAL WORD    obj2item     _((LISTBOX *list, WORD obj));
LOCAL VOID    snap_object  _((WINDOWP window, RECT *new, WORD mode));
LOCAL VOID    hndl_mscroll _((WINDOWP window, WORD obj, WORD breturn));
LOCAL VOID    set_menu     _((BOOLEAN enable));
LOCAL VOID    fix_menu     _((WINDOWP window));

/*****************************************************************************/
/* Suche nach Fenster mit Fensterklasse                                      */
/*****************************************************************************/

GLOBAL WINDOWP search_window (class, mode, icon)
WORD class, mode, icon;

{
  REG WORD    slot;
  REG WINDOWP window;
  REG BOOLEAN sub;

  sub   = (mode & SRCH_SUB) != 0;
  mode &= ~ SRCH_SUB;

  for (slot = 0; slot < top; slot++)
  {
    window = windows [slot];

    if ((class == NIL) || (class == window->class) || sub && (class == window->subclass))
      if (((window->opened == 0) && (mode & SRCH_CLOSED)) ||
          ((window->opened > 0) && (mode & SRCH_OPENED)))
        if ((icon == NIL) || (icon == window->icon)) return (window);
  } /* for */

  return (NULL);
} /* search_window */

/*****************************************************************************/
/* Suche Fenster von Fenster-Handle                                          */
/*****************************************************************************/

GLOBAL WINDOWP find_window (wh)
WORD wh;

{
  REG WORD slot;

  for (slot = 0; slot < top; slot++)
    if (windows [slot]->handle == wh) return (windows [slot]);

  return (NULL);
} /* find_window */

/*****************************************************************************/
/* Suche oberstes Fenster                                                    */
/*****************************************************************************/

GLOBAL WINDOWP find_top ()

{
  WINDOWP topwin;

  topwin = (top == 0) ? NULL : windows [0];

  if (topwin != NULL) get_work (topwin, FALSE); /* Falls ObjektbÑume gehandhabt werden */

  return (topwin);
} /* find_top */

/*****************************************************************************/
/* Teste, ob Fenster ganz oben liegt                                         */
/*****************************************************************************/

GLOBAL BOOLEAN is_top (window)
WINDOWP window;

{
  WORD wh, ret;

  wind_get (DESK, WF_TOP, &wh, &ret, &ret, &ret);

  return ((window != NULL) && (window->handle == wh));
} /* is_top */

/*****************************************************************************/
/* Teste, ob Fenster offen sind                                              */
/*****************************************************************************/

GLOBAL BOOLEAN any_open (incl_desk, incl_closer, incl_modal)
BOOLEAN incl_desk, incl_closer, incl_modal;

{
  REG BOOLEAN is_open;

  is_open = FALSE;

  if (top > 0)
    is_open = (windows [0]->opened > 0) &&
              (incl_desk || (windows [0]->class != class_desk) &&
              (! incl_closer || windows [0]->kind & CLOSER) &&
              (incl_modal || ! (windows [0]->flags & WI_MODAL)));

  return (is_open);
} /* any_open */

/*****************************************************************************/
/* ZÑhle Anzahl vorhandener Fenster einer Klasse                             */
/*****************************************************************************/

GLOBAL WORD num_windows (class, mode, winds)
WORD    class, mode;
WINDOWP winds [];

{
  REG WORD    slot, num;
  REG WINDOWP window;
  REG BOOLEAN sub;

  sub   = (mode & SRCH_SUB) != 0;
  mode &= ~ SRCH_SUB;

  for (slot = 0, num = 0; slot < top; slot++)
  {
    window = windows [slot];

    if ((class == NIL) || (class == window->class) || sub && (class == window->subclass))
      if (((window->opened == 0) && (mode & SRCH_CLOSED)) ||
          ((window->opened > 0) && (mode & SRCH_OPENED)))
      {
        if (winds != NULL) winds [num] = window;
        num++;
      } /* if, if */
  } /* for */

  return (num);
} /* num_windows */

/*****************************************************************************/

GLOBAL WORD num_locked ()

{
  WORD slot, num;

  for (slot = 0, num = 0; slot < top; slot++)
    if (windows [slot]->flags & WI_LOCKED) num++;

  return (num);
} /* num_locked */

/*****************************************************************************/
/* Suche Slot von Fenster-Handle                                             */
/*****************************************************************************/

LOCAL WORD find_slot (wh)
WORD wh;

{
  REG WORD slot;

  for (slot = 0; slot < top; slot++)
    if (windows [slot]->handle == wh) return (slot);

  return (FAILURE);
} /* find_slot */

/*****************************************************************************/
/* Suche Slot von Fenster                                                    */
/*****************************************************************************/

LOCAL WORD find_wslot (window)
WINDOWP window;

{
  REG WORD slot;

  for (slot = 0; slot < top; slot++)
    if (windows [slot] == window) return (slot);

  return (FAILURE);
} /* find_wslot */

/*****************************************************************************/
/* VerÑndere Reihenfolge                                                     */
/*****************************************************************************/

LOCAL VOID sort_order ()

{
  WORD    i, newtop;
  WINDOWP save [SETMAX + 1];

  for (i = 0; i < top; i++) save [i] = windows [i];

  newtop = 0;

  for (i = 0; i < top; i++)
    if (save [i]->opened > 0) windows [newtop++] = save [i];

  for (i = 0; i < top; i++)
    if (save [i]->opened == 0) windows [newtop++] = save [i];
} /* sort_order */

/*****************************************************************************/

LOCAL VOID move_order (newtop, window)
WORD    newtop;
WINDOWP window;

{
  REG WORD i;

  for (i = newtop; i > 0; i--) windows [i] = windows [i - 1];

  windows [0] = window;

  sort_order ();
} /* move_order */

/*****************************************************************************/
/* Hole Arbeitsbereich des Fensters                                          */
/*****************************************************************************/

LOCAL VOID get_work (window, work)
WINDOWP window;
BOOLEAN work;

{
  RECT   diff;
  WORD   x_offset, y_offset;
  OBJECT *object, *menu;

  if (work)
  {
    diff.x = window->work.x - window->scroll.x; /* Berechne Differenz */
    diff.y = window->work.y - window->scroll.y;
    diff.w = window->work.w - window->scroll.w;
    diff.h = window->work.h - window->scroll.h;

    wind_get (window->handle, WF_WXYWH,
              &window->work.x, &window->work.y, &window->work.w, &window->work.h);

    window->scroll.x = window->work.x - diff.x; /* Korrigiere auch Scrollbereich */
    window->scroll.y = window->work.y - diff.y;
    window->scroll.w = window->work.w - diff.w;
    window->scroll.h = window->work.h - diff.h;
  } /* if */

  menu = window->menu;

  if (menu != NULL)                          /* Menu vorhanden */
  {
    x_offset   = (window->flags & WI_MNSCROLL) ? menu_offset : 0;
    menu->ob_x = window->work.x + x_offset - menu [window->first_menu].ob_x;
    menu->ob_y = window->work.y;
  } /* if */

  object = window->object;

  if (object != NULL)                        /* Objekt vorhanden */
  {
    x_offset = y_offset = 0;

    if (window->class == class_desk)         /* Bewege Objektbaum Desktop */
    {
      x_offset = desk.x;
      y_offset = desk.y;
    } /* if */

    object->ob_x = window->scroll.x - x_offset - (WORD)window->doc.x * window->xfac;
    object->ob_y = window->scroll.y - y_offset - (WORD)window->doc.y * window->yfac;
  } /* if */
} /* get_work */

/*****************************************************************************/
/* Editiere Objekt im Fenster                                                */
/*****************************************************************************/

LOCAL VOID edit_object (window, key, kind)
WINDOWP window;
WORD    key, kind;

{
  BOOLEAN init;
  TEDINFO *ptedinfo;
  BYTE    *p;
  WORD    ch;

  if (window->edit_obj == NIL) window->edit_obj = 0;

  if (window->flags & (WI_MODAL | WI_MODELESS))
    if (window->edit_obj != 0)
    {
      init = (kind == EDINIT) && (window->edit_inx >= 0);

      if (init)
      {
        ptedinfo = (TEDINFO *)window->object [window->edit_obj].ob_spec;
        p        = &ptedinfo->te_ptext [window->edit_inx];
        ch       = *p;
        *p       = EOS;
      } /* if */

      if (window->edit_inx < 0) window->edit_inx = 0;   /* Sonst spinnt GEM */

      objc_edit (window->object, window->edit_obj, key, &window->edit_inx, kind);

      if (init) *p = ch;
    } /* if, if */
} /* edit_object */

/*****************************************************************************/

LOCAL WORD find_first (object)
OBJECT *object;

{
  REG WORD obj;


  obj = ROOT;

  do
  {
    if (! is_state (object, obj, DISABLED) &&
        ! is_flags (object, obj, HIDETREE) &&
          is_flags (object, obj, EDITABLE)) return (obj);
  } while (! is_flags (object, obj++, LASTOB));

  return (NIL);
} /* find_first */

/*****************************************************************************/

LOCAL WORD find_last (object)
OBJECT *object;

{
  REG WORD found, obj;


  found = NIL;
  obj   = ROOT;

  do
  {
    if (! is_state (object, obj, DISABLED) &&
        ! is_flags (object, obj, HIDETREE) &&
          is_flags (object, obj, EDITABLE)) found = obj;
  } while (! is_flags (object, obj++, LASTOB));

  return (found);
} /* find_last */

/*****************************************************************************/
/* Kreiere Fenster                                                           */
/*****************************************************************************/

GLOBAL WINDOWP create_window (kind, class)
UWORD kind;
WORD  class;

{
  REG WINDOWP window;
  REG WORD i;

  window = NULL;                /* ZunÑchst kein Fenster zur VerfÅgung */

  for (i = 0; (i < max_windows) && setin (used_windows, i); i++);

  if (i == max_windows)
  {
    if (nowindow >= 0) error (1, nowindow, NIL, NULL);
  } /* if */
  else
  {
    setincl (used_windows, i);  /* Setze Fenster als benutzt */
    window = &windrec [i];      /* Merke Adresse */

    mem_set (window, 0, sizeof (WINDOW));
    window->handle     = NO_HANDLE;
    window->opened     = 0;
    window->kind       = kind;
    window->class      = class;
    window->subclass   = class;
    window->first_menu = THEFIRST;
    windows [top++]    = window; /* Neues Fenster in Keller */
	 window->dispobjs   = list_create();
  } /* else */

  return (window);              /* Gib kreiertes Fenster zurÅck */
} /* create_window */

/*****************************************************************************/
/* Lîsche Fenster                                                            */
/*****************************************************************************/

GLOBAL VOID delete_window (window)
WINDOWP window;

{
	WORD    slot, i, opened;
	UWORD   flags;
	BOOLEAN cont;
	DISPOBJP	dispobj;
	LIST_P	header, element;
	
  if (window != NULL)
  {
    cont = (window->test != NULL) ? (*window->test) (window, DO_DELETE) : TRUE;

    if (cont)
    {
      flags          = window->flags;
      window->flags |= WI_RESIDENT;             /* Lîsche nicht beim Schlieûen */

      opened = window->opened;
      while (window->opened > 0)
      {
        close_window (window);                  /* Schlieûe Fenster */
        if (opened == window->opened)
        {
          window->flags = flags;
          return;                               /* Fenster darf nicht geschlossen werden */
        } /* if */

        opened = window->opened;
      } /* while */

		/* Liste der Display-Objekte durchgehen und delete aufrufen */
		header = window->dispobjs;
		element = list_next(header);
		while (element != header) {
			dispobj = (DISPOBJP) element->key;
			if (dispobj->delete) (*dispobj->delete) (dispobj);
			element = list_next(element);
		} /* while */
		
      if (window->delete != NULL) (*window->delete) (window); /* Lîschaktion durchfÅhren */

      slot = find_wslot (window);               /* Nicht nur oberstes Fenster kann gelîscht werden */

      for (i = slot + 1; i < top; i++) windows [i - 1] = windows [i];

      slot = (WORD)(window - windrec);          /* Zeiger Arithmetik */
      setexcl (used_windows, slot);             /* Fenster freigeben */
      top--;                                    /* Slot freigeben */
    } /* if */
  } /* if */
} /* delete_window */

/*****************************************************************************/
/* ôffne Fenster                                                             */
/*****************************************************************************/

GLOBAL BOOLEAN open_window (window)
WINDOWP window;

{
  WORD    wh, i;
  RECT    r;
  WORD    xy [4];
  WORD    newtop;
  WINDOWP untopped;
  BOOLEAN modal;

  wh = NO_HANDLE;

  if (window != NULL)
  {
    wh = window->handle;

    if (window->opened == 0)
    {
      if (window->class == DESK)
        wh = DESK;
      else
        wh = wind_create (window->kind, desk.x, desk.y, desk.w, desk.h);

      if (wh < 0)
      {
        wh = NO_HANDLE;                   /* Kein Fenster mehr */
        if (nowindow >= 0) error (1, nowindow, NIL, NULL);
      } /* if */

      window->handle = wh;
    } /* if */

    if (wh != NO_HANDLE)
    {
      if (top > 0)
      {
        untopped = windows [0];                   /* Altes aktives Fenster */

        if (window != untopped) untop_window (untopped);
      } /* if */

      window->opened++;                           /* Fenster einmal mehr offen */
      window->flags &= ~ WI_FULLED;               /* Fenster hat nicht volle Grîûe */
      window->flags |= WI_ONTOP;                  /* Fenster ist oben */

      if (window->open != NULL) (*window->open) (window); /* Aktion durchfÅhren */

      if (wh == DESK)                             /* GEM Desktop */
      {
        wind_set (DESK, WF_NEWDESK, ADR (window->object), 0, 0);
        form_dial (FMD_FINISH, 0, 0, 0, 0, desk.x, desk.y, desk.w, desk.h);
      } /* if */
      else
      {
        wind_calc (WC_BORDER, window->kind,       /* Rand berechnen */
                   window->work.x, window->work.y, window->work.w, window->work.h,
                   &r.x, &r.y, &r.w, &r.h);
        wind_set (wh, WF_NAME, ADR (window->name), 0, 0); /* Name setzen */
        wind_set (wh, WF_INFO, ADR (window->info), 0, 0); /* Infozeile setzen */
        set_sliders (window, HORIZONTAL + VERTICAL, SLPOS + SLSIZE);
        if (window->opened == 1) wind_open (wh, r.x, r.y, r.w, r.h); /* Fenster îffnen */
      } /* else */

      get_work (window, FALSE);                   /* Arbeitsbereich holen */

      if (window->opened == 1)
      {
        newtop = find_slot (wh);                  /* Neu geîffnetes Fenster oben */

        if (wh == DESK)                           /* Desktop darf nie nach oben */
        {
          window->opened = 0;
          move_order (newtop, window);
          window->opened = 1;
        } /* if */
        else
        {
          move_order (newtop, window);

          if (window->bg_color >= WHITE)
          {
            hide_mouse ();
            set_clip (TRUE, &window->work);
            vswr_mode (vdi_handle, MD_REPLACE);
            vsf_interior (vdi_handle, FIS_SOLID);
            vsf_color (vdi_handle, window->bg_color);
            rect2array (&window->work, xy);
            vr_recfl (vdi_handle, xy);
            show_mouse ();
          } /* if */
        } /* else */

        window->flags    &= ~ WI_FIRSTDRW;        /* Erstes Redraw vor Taste und Klick */
        window->exit_obj  = 0;
      } /* if */

      if (window->flags & WI_MODAL)
      {
        for (i = 1; i < top; i++)
          if ((windows [i]->opened > 0) && (windows [i]->class != helpclass)) windows [i]->flags |= WI_LOCKED;

        set_menu (FALSE);
      } /* if */
      else                                      /* Fenster wurde geîffnet, wÑhrend modaler Zustand */
      {
        for (i = 0, modal = FALSE; i < top; i++)
          if ((windows [i]->opened > 0) && (windows [i]->flags & WI_MODAL)) modal = TRUE;

        if (modal && (window->class != helpclass)) window->flags |= WI_LOCKED;
      } /* if */
    } /* if */
    else
      if (! (window->flags & WI_RESIDENT)) delete_window (window); /* Speicherstrukturen freigeben */
  } /* if */

  return (wh != NO_HANDLE);
} /* open_window */

/*****************************************************************************/
/* Schlieûe Fenster                                                          */
/*****************************************************************************/

GLOBAL VOID close_window (window)
WINDOWP window;

{
  WORD    wh, i;
  BOOLEAN cont, to_close, is_modal;
  WINDOWP topped;

  if (window != NULL)
  {
    cont = (window->test != NULL) ? (*window->test) (window, DO_CLOSE) : TRUE;

    if (cont)                                       /* Weitermachen */
    {
      topped =  windows [0];                        /* Aktuelles aktives Fenster */
      wh     = window->handle;

      if (sel_window == window) unclick_window (sel_window); /* Deselektieren */

      if (window->opened > 0)                       /* Fenster offen? */
      {
        is_modal = (window->flags & WI_MODAL) != 0;

        if (! acc_close && is_top (window)) edit_object (window, 0, EDEND);

        if (wh == DESK)                             /* GEM Desktop */
        {
          wind_set (DESK, WF_NEWDESK, ADR (NULL), 0, 0); /* Original Desktop */
          if (! done) form_dial (FMD_FINISH, 0, 0, 0, 0, desk.x, desk.y, desk.w, desk.h);
        } /* if */
        else
        {
#if GEM & GEM1
          to_close = ! acc_close;                   /* Sonst érger mit GEM */
#else
          to_close = TRUE;
#endif
          if (to_close && (window->opened == 1))
          {
            wind_close (wh);                        /* Schlieûe Fenster */
            wind_delete (wh);                       /* Window-Handle freigeben */
          } /* if */
        } /* else */

        if (window->close != NULL) (*window->close) (window); /* Schlieûaktion durchfÅhren */

        window->opened--;                           /* Fenster einmal geschlossen */

        if (window->opened == 0)                    /* Fenster ganz schlieûen */
        {
          window->handle  = NO_HANDLE;              /* Kein Handle mehr verfÅgbar */
          window->flags  &= ~ (WI_ONTOP | WI_FIRSTDRW); /* Fenster ist nicht mehr oben */

          sort_order ();
          if (! (window->flags & WI_RESIDENT)) delete_window (window);

          if (top > 0)
            if (topped != windows [0])              /* Neues oberstes Fenster ? */
            {
              topped = windows [0];

              if (topped->opened > 0)
                if (! (topped->flags & WI_ONTOP))
                {
                  if (topped->top != NULL) (*topped->top) (topped); /* Topaktion durchfÅhren */
                  topped->flags ^= WI_ONTOP;
                } /* if, if */
            } /* if, if */

          if (is_modal)
          {
            for (i = 0; (i < top) && cont; i++)    /* Teste, ob noch ein modales Fenster vorhanden */
              if ((windows [i]->flags & WI_MODAL) && (windows [i]->opened > 0))
              {
                cont = FALSE;
                windows [i]->flags &= ~ WI_LOCKED; /* Gib oberstes modales Fenster frei */
              } /* if, if */

            if (cont)
            {
              for (i = 0; i < top; i++) windows [i]->flags &= ~ WI_LOCKED;
              set_menu (TRUE);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* if */
} /* close_window */

/*****************************************************************************/
/* Schlieûe oberstes Fenster                                                 */
/*****************************************************************************/

GLOBAL VOID close_top ()

{
  if (top > 0) close_window (windows [0]);
} /* close_top */

/*****************************************************************************/
/* Schlieûe/Lîsche alle Fenster                                              */
/*****************************************************************************/

GLOBAL VOID close_all (delete, close_desk)
BOOLEAN delete, close_desk;

{
  WINDOWP 	window;
  WORD    	x, bound, newtop;
  RTMCLASSP module;

  if (top > 0)
  {
    if (! close_desk)
    {
      window = search_window (class_desk, SRCH_ANY, NIL);  /* Suche Desktop */
      if (window == NULL) close_desk = TRUE;               /* Kein Sonderfall */
    } /* if */

    if (close_desk)
      bound = 0;                                           /* Schlieûe auch Desktop */
    else
    {
      bound  = 1;                                          /* Lasse Desktop offen */
      newtop = find_slot (window->handle);                 /* Suche Slot von Desktop */

      move_order (newtop, window);                         /* Bringe Desktop nach oben */
    } /* else */

    wind_update (BEG_UPDATE);                              /* Keine Aktion zulassen */

		x = top;
		for (x = top; x >= 0; x--)
		{
			module = Module(windows[x]);
			/* Einstellungen sichern */
			if (module)
				if (module->info_name)
					save_info_obj (module->info_name, module->file_name, module->actual->number, &windows[x]->scroll, windows[x]->opened);
		} /* for */			

    while ((top > bound) && (windows [bound]->opened > 0))
    {
	      close_window (windows [bound]);                      /* Schlieûe Fenster */
	 } /* while */	

    if (delete)
      while (top > bound) delete_window (windows [bound]); /* Lîsche Fenster */

    wind_update (END_UPDATE);                              /* Aktionen zulassen */
  } /* if */
} /* close_all */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

GLOBAL VOID draw_window (window)
WINDOWP window;

{
	LIST_P	header, element;
	DISPOBJP	dispobj;
  RECT r;

  if ((window != NULL) && (window->opened > 0))
  {
    get_work (window, FALSE);           /* Eventuell Objekte korrigieren */

    if (window->object != NULL && (window->flags & (WI_MODAL | WI_MODELESS)))
             /* Objekte im Fenster zeichnen, ABER nur wenn DBox, BD*/
    {
      r = window->scroll;
      if (rc_intersect (&clip, &r))     /* Objekt nur auf Scrollbereich zeichnen */
      {
        objc_draw (window->object, ROOT, MAX_DEPTH, r.x, r.y, r.w, r.h);
        draw_oblines (window->object);
      } /* if */
    } /* if */

    if (window->draw != NULL) (*window->draw) (window); /* Zeichenroutine ausfÅhren */
		/* Liste der Display-Objekte durchgehen und delete aufrufen */
		header = window->dispobjs;
		element = list_next(header);
		while (element != header) {
			dispobj = (DISPOBJP) element->key;
			if (dispobj->draw) (*dispobj->draw) (dispobj);
			element = list_next(element);
		} /* while */
  } /* if */
} /* draw_window */

/*****************************************************************************/
/* Suche und zeichne Åberlagernde Rechtecke von Fenstern                     */
/*****************************************************************************/

GLOBAL VOID redraw_window (window, area)
WINDOWP    window;
CONST RECT *area;

{
  WORD    wh, ret;
  RECT    r1, r2;
  BOOLEAN cursor;
	LIST_P	header, element;
	DISPOBJP	dispobj;

  if ((window != NULL) && (window->opened > 0))
  {
    wind_update (BEG_UPDATE);           /* Benutzer darf nicht mehr agieren */
    hide_mouse ();                      /* Maus verstecken, da etwas gezeichnet wird */

    cursor = FALSE;

    if (window->flags & (WI_MODAL | WI_MODELESS))
      if (window->edit_obj != 0)
      {
        wind_get (DESK, WF_TOP, &wh, &ret, &ret, &ret);

        if (wh == window->handle) cursor = TRUE;
      } /* if, if */

    if (cursor && (window->flags & WI_FIRSTDRW)) edit_object (window, 0, EDEND);

	 /* Neue Status-Informationen festlegen etc., BD */
	 if (window->start)	window->start (window);
		/* Liste der Display-Objekte durchgehen */
		header = window->dispobjs;
		element = list_next(header);
		while (element != header) {
			dispobj = (DISPOBJP) element->key;
			if (dispobj->start) (*dispobj->start) (dispobj);
			element = list_next(element);
		} /* while */

    set_clip (TRUE, area);              /* Setze clipping */
    draw_mbar (window);                 /* Zeichne MenÅzeile */

    r2 = *area;                         /* Schnitt-Rechteck */

    wh = window->handle;
    wind_get (wh, WF_FIRSTXYWH, &r1.x, &r1.y, &r1.w, &r1.h);
                                        /* Hole erstes Rechteck der Rechteckliste */

    while ((r1.w != 0) && (r1.h != 0))  /* Breite und Hîhe noch nicht null */
    {
      if (rc_intersect (&r2, &r1))      /* Schneide Rechtecke */
      {	
        	set_clip (TRUE, &r1);           /* Setze clipping */
        	draw_window (window);           /* Zeichne Fensterausschnitt */
      } /* if */

      wind_get (wh, WF_NEXTXYWH, &r1.x, &r1.y, &r1.w, &r1.h);
    } /* while */                       /* Hole nÑchstes Rechteck */

	 /* Neue Status-Informationen festlegen etc., BD */
	 if (window->finished)	window->finished (window);
		/* Liste der Display-Objekte durchgehen */
		header = window->dispobjs;
		element = list_next(header);
		while (element != header) {
			dispobj = (DISPOBJP) element->key;
			if (dispobj->finished) (*dispobj->finished) (dispobj);
			element = list_next(element);
		} /* while */

    if (cursor) edit_object (window, 0, EDINIT);

    window->flags |= WI_FIRSTDRW;       /* Fenster wurde erstes Mal gezeichnet */

    show_mouse ();                      /* Zeige Maus wieder */
    wind_update (END_UPDATE);           /* Benutzer darf wieder agieren */
  } /* if */
} /* redraw_window */

/*****************************************************************************/
/* Bringe Fenster nach oben                                                  */
/*****************************************************************************/

GLOBAL VOID top_window (window)
WINDOWP window;

{
  WORD    wh;
  WORD    newtop, oldtop, ret;
  WINDOWP topwin;
#if GEMDOS
  RECT    r;
#endif

  wind_get (DESK, WF_TOP, &oldtop, &ret, &ret, &ret);

  if (window != NULL)
    if (window->handle != oldtop)
    {
      if (window->flags & WI_LOCKED)
      {
        beep ();

#if GEM & (GEM2 | GEM3 | XGEM)
        topwin = find_window (oldtop);

        if (topwin != NULL)
        {
          edit_object (topwin, 0, EDINIT);
          if (topwin->flags & WI_MODAL) topwin->flags |= WI_ONTOP; /* untop rÅckgÑngig machen */
        } /* if */
#endif
      } /* if */

      if (! (window->flags & (WI_NOTOP | WI_LOCKED)))
      {
        topwin = (top > 0) ? windows [0] : NULL;        /* Altes aktives Fenster */

        if (topwin != NULL)
          if (window != topwin)
            if (topwin->flags & WI_ONTOP) untop_window (topwin);

        wh = window->handle;
        wind_set (wh, WF_TOP, 0, 0, 0, 0);

        newtop = find_slot (wh);        /* Neues Fenster oben im Keller */
        move_order (newtop, window);

        if (! (window->flags & WI_ONTOP))
        {
          if (window->top != NULL) (*window->top) (window); /* Topaktion durchfÅhren */
          window->flags ^= WI_ONTOP;
        } /* if */

        edit_object (window, 0, EDINIT);       /* Fenster wird nicht gezeichnet */
        window->flags |= WI_FIRSTDRW;

#if GEMDOS
        if (window->flags & (WI_MODAL | WI_MODELESS))
          if (window->edit_obj > 0)
          {
            get_border (window, window->edit_obj, &r);
            r.y -= gl_hbox / 2;
            r.h += gl_hbox;
            set_redraw (window, &r);
          } /* if */
#endif
      } /* if */
    } /* if, if */
} /* top_window */

/*****************************************************************************/
/* Behandle Fenster, welches nach unten gebracht wurde                       */
/*****************************************************************************/

GLOBAL VOID untop_window (window)
WINDOWP window;

{
  if (window != NULL)
    if (window->flags & WI_ONTOP)
    {
      if (window->opened > 0)
      {
        edit_object (window, 0, EDEND);
        if (window->untop != NULL) (*window->untop) (window); /* Untopaktion durchfÅhren */
      } /* if */

      window->flags &= ~ (WI_ONTOP | WI_FIRSTDRW);
    } /* if, if */
} /* untop_window */

/*****************************************************************************/
/* Bringe unterstes Fenster nach oben                                        */
/*****************************************************************************/

GLOBAL VOID cycle_window ()

{
  REG WORD    i;
  REG WINDOWP window;
  REG WINDOWP found;

  found = NULL;

  for (i = 0; i < top; i++)
  {
    window = windows [i];

    if ((window->opened > 0) && (window->class != DESK) && ! (window->flags & WI_LOCKED))
      found = window;
  } /* for */

  if (found != NULL) top_window (found);
} /* cycle_window */

/*****************************************************************************/
/* Scrolle den Fensterinhalt                                                 */
/*****************************************************************************/

GLOBAL VOID scroll_window (window, dir, delta)
WINDOWP window;
WORD    dir;
LONG    delta;

{
  WORD  wh;
  WORD  wdiff, hdiff;
  MFDB  s, d;
  WORD  xy [8];
  RECT  r, r1, r2;

  wind_update (BEG_UPDATE);             /* Benutzer darf nicht mehr agieren */
  hide_mouse ();                        /* Maus verstecken, da gescrollt wird */

  r = window->scroll;                   /* Hole Scrollbereich */

  wh    = window->handle;               /* Window Handle wird noch gebraucht */
  wdiff = r.x + r.w - (desk.x + desk.w); /* wdiff > 0 oder hdiff > 0 =>...*/
  hdiff = r.y + r.h - (desk.y + desk.h); /* ...Fenster nicht ganz in Desktop !!! */

  s.mp = d.mp = NULL;                   /* Erzwinge Bildschirmadresse */

  set_clip (TRUE, &r);                  /* Setze clipping auf Scrollbereich */

  if (! ((dir & HORIZONTAL) && (labs (delta) >= r.w) ||
         (dir & VERTICAL) && (labs (delta) >= r.h) ||
         (window->flags & WI_NOSCROLL)))
  {
    rect2array (&r, xy);                /* Anfangswerte fÅr vro_cpyfm */
    rect2array (&r, &xy [4]);           /* Anfangswerte fÅr vro_cpyfm */

    if (dir & HORIZONTAL)               /* Horizontales Scrolling */
    {
      if (delta > 0)                    /* Links Scrolling */
      {
        xy [0] += delta;                /* Werte fÅr vro_cpyfm */
        xy [6] -= delta;

        r.x += r.w - delta;             /* Rechter Bereich nicht gescrollt,... */
        r.w  = delta;                   /* ...muû neu gezeichnet werden */

        if (wdiff > 0) r.x -= wdiff;    /* Fenster war nicht ganz in Desktop */
      } /* if */                        /* Es muû mehr gezeichnet werden */
      else                              /* Rechts Scrolling */
      {
        xy [2] += delta;                /* Werte fÅr vro_cpyfm */
        xy [4] -= delta;

        r.w = -delta;                   /* Linken Bereich noch neu zeichnen */
      } /* else */
    } /* if */
    else                                /* Vertikales Scrolling */
    {
      if (delta > 0)                    /* AufwÑrts Scrolling */
      {
        xy [1] += delta;                /* Werte fÅr vro_cpyfm */
        xy [7] -= delta;

        r.y += r.h - delta;             /* Unterer Bereich nicht gescrollt,... */
        r.h  = delta;                   /* ...muû neu gezeichnet werden */

        if (hdiff > 0) r.y -= hdiff;    /* Fenster war nicht ganz in Desktop */
      } /* if */                        /* Es muû mehr gezeichnet werden */
      else                              /* AbwÑrts Scrolling */
      {
        xy [3] += delta;                /* Werte fÅr vro_cpyfm */
        xy [5] -= delta;

        r.h = -delta;                   /* Oberen Bereich noch neu zeichnen */
      } /* else */
    } /* else */

    if (wdiff > 0)              /* Rechter Rand des Fensters auûerhalb Desktop */
    {
      xy [2] -= wdiff;          /* Korrigiere rechten Rand von Quelle... */
      xy [6] -= wdiff;          /* ...und Ziel */
    } /* if */

    if (hdiff > 0)              /* Unterer Rand des Fensters auûerhalb Desktop */
    {
      xy [3] -= hdiff;          /* Korrigiere unteren Rand von Quelle... */
      xy [7] -= hdiff;          /* ...und Ziel */
    } /* if */

    array2rect (&xy [4], &r2);  /* Nur Zielraster zeichnen */

    wind_get (wh, WF_FIRSTXYWH, &r1.x, &r1.y, &r1.w, &r1.h);
                                /* Hole erstes Rechteck der Rechteckliste */

    while ((r1.w != 0) && (r1.h != 0))                  /* Breite und Hîhe noch nicht null */
    {
      if ((r1.w == window->work.w) &&                   /* Fenster ganz sichtbar */
          (r1.h == window->work.h))
        vro_cpyfm (vdi_handle, S_ONLY, xy, &s, &d);     /* Eigentliches Scrolling */
      else
        if (rc_intersect (&r2, &r1))                    /* Schneide Rechtecke */
        {
          set_clip (TRUE, &r1);                         /* Setze Clipping */
          draw_window (window);                         /* Fenster zeichnen */
        } /* if, else */

      wind_get (wh, WF_NEXTXYWH, &r1.x, &r1.y, &r1.w, &r1.h);
    } /* while */                       /* Hole nÑchstes Rechteck */
  } /* if */

  redraw_window (window, &r);           /* Fenster zeichnen */

  show_mouse ();                        /* Zeige Maus wieder */
  wind_update (END_UPDATE);             /* Benutzer darf wieder agieren */
} /* scroll_window */

/*****************************************************************************/
/* Reagiere auf Anklicken eines Pfeils                                       */
/*****************************************************************************/

LOCAL VOID arrow_object (window, dir, oldpos, newpos)
WINDOWP window;
WORD    dir;
LONG    oldpos, newpos;

{
  LONG delta;

  if (window->object != NULL)
  {
    delta = newpos - oldpos;

    if (delta != 0)
    {
      if (dir & HORIZONTAL)     /* Horizontale Pfeile und Schieber */
      {
        window->doc.x = newpos;                         /* Neue Position */

        set_sliders (window, HORIZONTAL, SLPOS);        /* Schieber setzen */
        scroll_window (window, HORIZONTAL, delta * window->xfac);
      } /* if */
      else                      /* Vertikale Pfeile und Schieber */
      {
        window->doc.y = newpos;                         /* Neue Position */

        set_sliders (window, VERTICAL, SLPOS);          /* Schieber setzen */
        scroll_window (window, VERTICAL, delta * window->yfac);
      } /* else */
    } /* if */
  } /* if */
} /* arrow_object */

/*****************************************************************************/

GLOBAL VOID arrow_window (window, arrow, amount)
WINDOWP window;
WORD    arrow, amount;

{
  WORD w, h, wbox, hbox, dir;
  LONG oldpos, newpos, max_slide;

  if (window != NULL)
  {
    updtmenu = FALSE;                   /* MenÅs mÅssen sich nicht verÑndern */
    wbox     = window->xfac;
    hbox     = window->yfac;
    w        = window->scroll.w / wbox; /* Breite in Zeichen */
    h        = window->scroll.h / hbox; /* Hîhe in Zeichen */

    if (arrow <= WA_DNLINE)
    {
      oldpos  = newpos = window->doc.y;
      dir     = VERTICAL;
      if ((arrow == WA_UPLINE) || (arrow == WA_DNLINE)) amount *= window->yunits;
    } /* if */
    else
    {
      oldpos = newpos = window->doc.x;
      dir    = HORIZONTAL;
      if ((arrow == WA_LFLINE) || (arrow == WA_RTLINE)) amount *= window->xunits;
    } /* else */

    switch (arrow)
    {
      case WA_UPPAGE : newpos -= h * amount; break;
      case WA_DNPAGE : newpos += h * amount; break;
      case WA_UPLINE : newpos -= amount;     break;
      case WA_DNLINE : newpos += amount;     break;
      case WA_LFPAGE : newpos -= w * amount; break;
      case WA_RTPAGE : newpos += w * amount; break;
      case WA_LFLINE : newpos -= amount;     break;
      case WA_RTLINE : newpos += amount;     break;
    } /* switch */

    if (dir == VERTICAL)
      max_slide = window->doc.h - h;            /* Maximale Position */
    else
      max_slide = window->doc.w - w;            /* Maximale Position */

    if (newpos > max_slide) newpos = max_slide; /* Jenseits des Randes */
    if (newpos < 0) newpos = 0;                 /* Jenseits des Randes */

    if (window->arrow != NULL) (*window->arrow) (window, dir, oldpos, newpos);
    arrow_object (window, dir, oldpos, newpos);
  } /* if */
} /* arrow_window */

/*****************************************************************************/
/* Reagiere auf Bewegen der Schieber                                         */
/*****************************************************************************/

GLOBAL VOID h_slider (window, new_value)
WINDOWP window;
WORD    new_value;

{
  WORD w, wbox;
  LONG help;
  LONG oldpos, newpos, max_slide;

  if (window != NULL)
  {
    updtmenu  = FALSE;                                  /* MenÅs mÅssen sich nicht verÑndern */
    wbox      = window->xfac;
    w         = window->scroll.w / wbox;                /* Breite in Zeichen */
    max_slide = window->doc.w - w;                      /* Maximale Position */
    oldpos    = window->doc.x;
    help      = (LONG)new_value * (window->doc.w - w);  /* Erzwinge Langzahl */
    newpos    = help / 1000;
    if (help % 1000 >= 500) newpos++;                   /* Runden */

    if (newpos > max_slide) newpos = max_slide;         /* Jenseits des Randes */
    if (newpos < 0) newpos = 0;                         /* Jenseits des Randes */

    if (window->arrow != NULL) (*window->arrow) (window, HORIZONTAL, oldpos, newpos);
    arrow_object (window, HORIZONTAL, oldpos, newpos);
  } /* if */
} /* h_slider */

/*****************************************************************************/

GLOBAL VOID v_slider (window, new_value)
WINDOWP window;
WORD    new_value;

{
  WORD h, hbox;
  LONG help;
  LONG oldpos, newpos, max_slide;

  if (window != NULL)
  {
    updtmenu  = FALSE;                                  /* MenÅs mÅssen sich nicht verÑndern */
    hbox      = window->yfac;
    h         = window->scroll.h / hbox;                /* Hîhe in Zeichen */
    max_slide = window->doc.h - h;                      /* Maximale Position */
    oldpos    = window->doc.y;
    help      = (LONG)new_value * (window->doc.h - h);  /* Erzwinge Langzahl */
    newpos    = help / 1000;
    if (help % 1000 >= 500) newpos++;                   /* Runden */

    if (newpos > max_slide) newpos = max_slide;         /* Jenseits des Randes */
    if (newpos < 0) newpos = 0;                         /* Jenseits des Randes */

    if (window->arrow != NULL) (*window->arrow) (window, VERTICAL, oldpos, newpos);
    arrow_object (window, VERTICAL, oldpos, newpos);
  } /* if */
} /* v_slider */

/*****************************************************************************/
/* Setze Schieberpositionen und Schiebergrîûe                                */
/*****************************************************************************/

GLOBAL VOID set_sliders (window, which, mode)
WINDOWP window;
WORD    which, mode;

{
  WORD wh;
  WORD w, h, newval, oldval, ret;
  LONG max_doc;

  if (window != NULL)
  {
    wh = window->handle;

    if ((window->kind & HSLIDE) && (which & HORIZONTAL))
    {
      w       = window->scroll.w / window->xfac;
      max_doc = window->doc.w - w;

      if (mode & SLPOS)
      {
        if (max_doc <= 0)                       /* Fenster zu groû oder passend */
          newval = 0;
        else
          newval = (1000L * window->doc.x) / max_doc;

        wind_get (wh, WF_HSLIDE, &oldval, &ret, &ret, &ret);
        if (newval != oldval) wind_set (wh, WF_HSLIDE, newval, 0, 0, 0);
      } /* if */

      if (mode & SLSIZE)
      {
        if (window->doc.w <= 0)                 /* Fenster zu groû oder passend */
          newval = 1000;
        else
          newval = (1000L * w) / window->doc.w;

        wind_get (wh, WF_HSLSIZE, &oldval, &ret, &ret, &ret);
        if (newval != oldval) wind_set (wh, WF_HSLSIZE, newval, 0, 0, 0);
      } /* if */
    } /* if */

    if ((window->kind & VSLIDE) && (which & VERTICAL))
    {
      h       = window->scroll.h / window->yfac;
      max_doc = window->doc.h - h;

      if (mode & SLPOS)
      {
        if (max_doc <= 0)                       /* Fenster zu groû oder passend */
          newval = 0;
        else
          newval = (1000L * window->doc.y) / max_doc;

        wind_get (wh, WF_VSLIDE, &oldval, &ret, &ret, &ret);
        if (newval != oldval) wind_set (wh, WF_VSLIDE, newval, 0, 0, 0);
      } /* if */

      if (mode & SLSIZE)
      {
        if (window->doc.h <= 0)                 /* Fenster zu groû oder passend */
          newval = 1000;
        else
          newval = (1000L * h) / window->doc.h;

        wind_get (wh, WF_VSLSIZE, &oldval, &ret, &ret, &ret);
        if (newval != oldval) wind_set (wh, WF_VSLSIZE, newval, 0, 0, 0);
      } /* if */
    } /* if */
  } /* if */
} /* set_sliders */

/*****************************************************************************/
/* Einschnappen des Fensterinneren auf bestimmte Grenzen                     */
/*****************************************************************************/

LOCAL VOID snap_object (window, new, mode)
WINDOWP window;
RECT    *new;
WORD    mode;

{
  RECT r;
  RECT diff;
  WORD wbox, hbox;
  LONG max_xdoc, max_ydoc;

  if (window->object != NULL)
  {
    wind_get (window->handle, WF_CXYWH, &r.x, &r.y, &r.w, &r.h);

    wbox   = window->xfac;
    hbox   = window->yfac;
    diff.x = (new->x - r.x) / wbox * wbox;      /* Differenz berechnen */
    diff.y = (new->y - r.y) / hbox * hbox;
    diff.w = (new->w - r.w) / wbox * wbox;
    diff.h = (new->h - r.h) / hbox * hbox;

    new->x = r.x + diff.x;                      /* Gerade Position */
    new->y = r.y + diff.y; 
    new->w = r.w + diff.w;                      /* Arbeitsbereich einrasten */
    new->h = r.h + diff.h;

    if (mode & SIZED)
    {
      r.w      = (window->scroll.w + diff.w) / wbox;    /* Neuer Scrollbereich */
      max_xdoc = window->doc.w - r.w;
      r.h      = (window->scroll.h + diff.h) / hbox;
      max_ydoc = window->doc.h - r.h;

      if (max_xdoc < 0) max_xdoc = 0;
      if (max_ydoc < 0) max_ydoc = 0;

      if (window->doc.x > max_xdoc)             /* Jenseits rechter Bereich */
        window->doc.x = max_xdoc;

      if (window->doc.y > max_ydoc)             /* Jenseits unterer Bereich */
        window->doc.y = max_ydoc;
    } /* if */
  } /* if */
} /* snap_object */

/*****************************************************************************/

GLOBAL VOID snap_window (window, new, mode)
WINDOWP window;
RECT    *new;
WORD    mode;

{
  if (window != NULL)
  {
    if (new->x == 0) new->x = -1;               /* Spezialfall linker Rand */
    if (new->w < MIN_WIDTH) new->w = MIN_WIDTH;
    if (new->h < MIN_HEIGHT) new->h = MIN_HEIGHT;

    if (window->snap != NULL) (*window->snap) (window, new, mode);
    snap_object (window, new, mode);
  } /* if */
} /* snap_window */

/*****************************************************************************/
/* Vergîûere Fenster auf volle Grîûe bzw. verkleinere Fenster                */
/*****************************************************************************/

GLOBAL VOID full_window (window)
WINDOWP window;

{
  WORD wh;
  RECT r, new;

  if (window != NULL)
    if (window->kind & FULLER)
    {
      updtmenu = FALSE;                   /* MenÅs mÅssen sich nicht verÑndern */
      wh       = window->handle;

      wind_get (wh, WF_CXYWH, &r.x, &r.y, &r.w, &r.h); /* Hole momentane Grîûe */

      if (window->flags & WI_FULLED)                      /* Mache Fenster klein */
      {
        wind_get (wh, WF_PXYWH, &new.x, &new.y, &new.w, &new.h);
        snap_window (window, &new, MOVED | SIZED);
        shrinkbox (&new, &r);
        wind_set (wh, WF_CXYWH, new.x, new.y, new.w, new.h);
      } /* if */
      else                                                /* Mache Fenster groû */
      {
        wind_get (wh, WF_FXYWH, &new.x, &new.y, &new.w, &new.h);
        snap_window (window, &new, MOVED | SIZED);
        growbox (&r, &new);
        wind_set (wh, WF_CXYWH, new.x, new.y, new.w, new.h);
      } /* else */

      get_work (window, TRUE);
      set_sliders (window, HORIZONTAL + VERTICAL, SLPOS + SLSIZE);
      window->flags ^= WI_FULLED;
    } /* if, if */
} /* full_window */

/*****************************************************************************/
/* éndere Fenstergrîûe                                                       */
/*****************************************************************************/

GLOBAL VOID size_window (window, new)
WINDOWP    window;
CONST RECT *new;

{
  RECT r;

  if (window != NULL)
    if (window->kind & SIZER)
    {
      updtmenu = FALSE;                   /* MenÅs mÅssen sich nicht verÑndern */
      r        = *new;
      snap_window (window, &r, SIZED);
      wind_set (window->handle, WF_CXYWH, r.x, r.y, r.w, r.h);
      get_work (window, TRUE);
      set_sliders (window, HORIZONTAL + VERTICAL, SLPOS + SLSIZE);
      window->flags &= ~ WI_FULLED;
    } /* if, if */
} /* size_window */

/*****************************************************************************/
/* Bewege Fenster                                                            */
/*****************************************************************************/

GLOBAL VOID move_window (window, new)
WINDOWP    window;
CONST RECT *new;

{
	LIST_P	header, element;
	DISPOBJP	dispobj;
	RECT r;
	
	if (window != NULL)
		if (window->kind & MOVER)
		{
			updtmenu = FALSE;                   /* MenÅs mÅssen sich nicht verÑndern */
			r        = *new;
			snap_window (window, &r, MOVED);
			wind_set (window->handle, WF_CXYWH, r.x, r.y, r.w, r.h);
			get_work (window, TRUE);
			window->flags &= ~ WI_FULLED;

			/* in allen dispobj die Koordinaten Åbernehmen */
			header = window->dispobjs;
			element = list_next(header);
			while (element != header) {
				dispobj = (DISPOBJP) element->key;
				if (dispobj->moved) (*dispobj->moved) (dispobj);
				element = list_next(element);
			} /* while */
		} /* if, if */
} /* move_window */

/*****************************************************************************/
/* Ziehen von Objekten in ein Fenster                                        */
/*****************************************************************************/

GLOBAL WORD drag_to_window (mox, moy, src_window, src_obj, dest_window, dest_obj)
WORD    mox, moy;
WINDOWP src_window;
WORD    src_obj;
WINDOWP *dest_window;
WORD    *dest_obj;

{
  WORD    wh;
  WINDOWP dest;
  OBJECT  *object;

  *dest_window = NULL;
  *dest_obj    = NIL;

  wh   = wind_find (mox, moy);
  dest = find_window (wh);

  if (dest == NULL) return (DRAG_NOWIND);       /* Reagiere nicht auf fremden Prozess */

  *dest_window = dest;
  object       = dest->object;

  if (object != NULL) *dest_obj = objc_find (object, ROOT, MAX_DEPTH, mox, moy);

  if (dest->drag == NULL) return (DRAG_NORCVR); /* EmpfÑnger desinteressiert */

  return ((*dest->drag) (src_window, src_obj, *dest_window, *dest_obj));
} /* drag_to_window */

/*****************************************************************************/
/* Klicken in das Fensterinnere                                              */
/*****************************************************************************/

GLOBAL VOID click_window (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  BOOLEAN cont, changed, check;
  WORD    next, prev;
  UWORD   slctd1, slctd2;
  OBJECT  *obj;
  WORD    pos, i, wbox;
  RECT    r;
  BYTE    *p;
  TEDINFO *ptedinfo;
	LIST_P	header, element;
	DISPOBJP	dispobj;

  cont    = TRUE;
  changed = TRUE;

  if (window != NULL)
  {
    if (window->flags & WI_LOCKED)
      beep ();
    else
    {
      window->exit_obj = 0;

      if (is_top (window) && (window->flags & (WI_MODAL | WI_MODELESS)))
      {
        get_work (window, FALSE);       /* Objektbaum fÅr jeweiliges Fenster korrigieren */
        if ((window->class != DESK) && ! (window->flags & WI_FIRSTDRW)) redraw_window (window, &window->work);

        window->exit_obj = next = objc_find (window->object, ROOT, MAX_DEPTH, mk->mox, mk->moy);

		/* Buttons mit selektierbaren Strings als Folgeobjekt */
		/* Erweiterung BD */

			/* Ist Objekt ein String oder Text und Selectable? */
			if (((OB_TYPE(window->object, window->exit_obj-1) == G_STRING)
					|| (OB_TYPE(window->object, window->exit_obj-1) == G_TEXT))
					&& is_flags(window->object, window->exit_obj, SELECTABLE))
			{
				/* Ist linker Nachbar ein Grafik-Button und Selectable ? */
				if ((OB_TYPE(window->object, window->exit_obj-1) == G_BOX)
						&& is_flags(window->object-1, window->exit_obj, SELECTABLE))
				{
					/* String wieder auf deselected und Grafikbutton auf selected */
					do_state(window->object, window->exit_obj-1, SELECTED);
					undo_state(window->object, window->exit_obj-1, SELECTED);
					/* String neuzeichnen */
					draw_object(window, window->exit_obj);
					/* Neues Exitobj */
					window->exit_obj--;
					/* Nachbar neuzeichnen */
					draw_object(window, window->exit_obj);
				} /* if */
			} /* if */

        if (next != NIL)
        {
          prev  = next - 1;
          check = FALSE;

          if (prev != NIL)
            if ((OB_TYPE (window->object, prev) == G_USERDEF) &&
                (OB_FLAGS (window->object, prev) & SELECTABLE) &&
                (OB_TYPE (window->object, next) == G_STRING))
            {
              window->exit_obj = next = prev;
              check            = ! (OB_FLAGS (window->object, next) & RBUTTON);

              if (check) undo_flags (window->object, next, SELECTABLE);
            } /* if, if */

          if (! is_flags (window->object, next, LASTOB) && (mk->breturn == 1))
            if ((OB_TYPE (window->object, next) == G_STRING) &&
                ((OB_TYPE (window->object, next + 1) == G_FTEXT) ||
                 (OB_TYPE (window->object, next + 1) == G_FBOXTEXT) ||
                 (OB_TYPE (window->object, next + 1) == G_BOXTEXT)))
            {
              window->exit_obj = ++next;
              objc_offset (window->object, next, &mk->mox, &mk->moy);
              mk->mox += window->object [next].ob_width - 1;
            } /* if, if */

          obj     = &window->object [next];
          slctd1  = obj->ob_state & SELECTED;
          cont    = form_button (window->object, next, mk->breturn, &next);
          next   &= 0x7FFF;

          if (check)           /* In benutzerdefinierte Checkbox geklickt */
          {
            do_flags (window->object, prev, SELECTABLE);
            flip_state (window->object, prev, SELECTED);
            draw_object (window, prev);
          } /* if */

          slctd2  = obj->ob_state & SELECTED;
          changed = ! (obj->ob_state & DISABLED) &&
                    ((obj->ob_flags & SELECTABLE) && ((slctd1 ^ slctd2) || (mk->breturn == 2)) ||
                     ((obj->ob_type & 0xFF) != G_BUTTON) && ((obj->ob_type & 0xFF) != G_USERDEF));

          if (! cont || (next != 0))
          {
            if (cont)
            {
              edit_object (window, 0, EDEND);
              objc_rect (window->object, next, &r, FALSE);
              window->edit_obj = next;
              window->edit_inx = NIL;
              ptedinfo         = (TEDINFO *)window->object [window->edit_obj].ob_spec;
              wbox             = (ptedinfo->te_font == SMALL) ? 6 : gl_wbox;
              p                = ptedinfo->te_ptmplt;

              if (ptedinfo->te_just == TE_CNTR)
                r.x += (r.w - strlen (p) * wbox) / 2;

              pos = (mk->mox - r.x) / wbox;
              if ((mk->mox - r.x) % wbox >= wbox / 2) pos++;

              if ((ptedinfo->te_just == TE_LEFT) || (ptedinfo->te_just == TE_CNTR))
                for (i = window->edit_inx = 0; i < pos; i++)
                  if (p [i] == '_') window->edit_inx++;

              edit_object (window, 0, EDINIT);
            } /* if */
            else
              if ((window->flags & WI_MODAL) || (! mk->alt && ! (mk->momask & 2))) window->flags |= WI_DLCLOSE;
          } /* if */
        } /* if */
      } /* if */

      if (window->click != NULL)
      {
        if (changed) {
	         (*window->click) (window, mk);
				/* Liste der Display-Objekte durchgehen */
				header = window->dispobjs;
				element = list_next(header);
				while (element != header) {
					dispobj = (DISPOBJP) element->key;
					/* Check if click was inside the display object */
					/* if (inside (mk->mox - window->work.x, mk->moy - window->work.y, &dispobj->work)) */
					if (inside (mk->mox, mk->moy, &dispobj->work))
						if (dispobj->click) (*dispobj->click) (dispobj, mk);
					element = list_next(element);
				} /* while */
			} /* if */
      } /* if */
      else
        if (! (window->flags & (WI_MODAL | WI_MODELESS)))
          if (sel_window != window) unclick_window (sel_window); /* Deselektieren */

      if (window->flags & WI_DLCLOSE)
      {
        undo_state (window->object, next, SELECTED);
        close_window (window);
      } /* if */
      else
        if (! cont)
        {
          undo_state (window->object, next, SELECTED);

          if (is_flags (window->object, next, UNDO_FLAG))
            redraw_window (window, &window->scroll);
          else
            draw_object (window, next);
        } /* if, else */

      window->flags &= ~ WI_DLCLOSE;    /* Auf jeden Fall zurÅcksetzen */
    } /* else */
  } /* if */
} /* click_window */

/*****************************************************************************/

GLOBAL VOID unclick_window (window)
WINDOWP window;

{
  if (window != NULL)
    if (window->unclick != NULL) (*window->unclick) (window);

  sel_window = NULL;
  setclr (sel_objs);
} /* unclick_window */

/*****************************************************************************/
/* Taste fÅr ein Fenster                                                     */
/*****************************************************************************/

GLOBAL BOOLEAN key_window (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  BOOLEAN res, cont, edit, disabled;
  WORD    next, def, save;
  UWORD   kr;
  STRING  s;
  BYTE    *p;
  LONG    l;
	LIST_P	header, element;
	DISPOBJP	dispobj;
	BOOL		found = FALSE;

  res  = FALSE;
  edit = FALSE;
  cont = TRUE;
  kr   = 0;

  if (window != NULL)
    if (! (window->flags & WI_LOCKED))
      if (window->opened > 0)
      {
        if (mk->scan_code == CNTRL_CLR)
        {
          full_window (window);
          return (TRUE);
        } /* if */

        if (window->flags & WI_CURSKEYS)
        {
          switch (mk->scan_code)
          {
            case UP     : arrow_window (window, mk->shift ? WA_UPPAGE : WA_UPLINE, 1);
                          res = TRUE;
                          break;
            case PGUP   : arrow_window (window, WA_UPPAGE, 1);
                          res = TRUE;
                          break;
            case LEFT   : arrow_window (window, mk->shift ? WA_LFPAGE : WA_LFLINE, 1);
                          res = TRUE;
                          break;
            case RIGHT  : arrow_window (window, mk->shift ? WA_RTPAGE : WA_RTLINE, 1);
                          res = TRUE;
                          break;
            case DOWN   : arrow_window (window, mk->shift ? WA_DNPAGE : WA_DNLINE, 1);
                          res = TRUE;
                          break;
            case PGDOWN : arrow_window (window, WA_DNPAGE, 1);
                          res = TRUE;
                          break;
            case POS1   : v_slider (window, 0);
                          res = TRUE;
                          break;
            case ENDKEY : v_slider (window, 1000);
                          res = TRUE;
                          break;
          } /* switch */

          if (res) return (TRUE);
        } /* if */

        window->exit_obj = 0;

        if (is_top (window) && (window->flags & (WI_MODAL | WI_MODELESS)))
        {
          if ((next = check_alt (window->object, mk)) != NIL)
          {
            objc_offset (window->object, next, &mk->mox, &mk->moy);
            mk->momask  = 0x0001;       /* Linker Mausknopf */
            mk->breturn = 1;            /* Einfachklick */
            mk->alt     = FALSE;        /* sonst gehen modeless Dialog nicht zu */
            click_window (window, mk);
            return (TRUE);
          } /* if */

          if (! (mk->ctrl || iscntrl (mk->ascii_code) && ! DLG_CNTRL (mk) || mk->alt && (mk->ascii_code == 0) ||
             (F1 <= mk->scan_code) && (mk->scan_code <= F10) && (mk->ascii_code == 0) ||
             (F11 <= mk->scan_code) && (mk->scan_code <= F20) && (mk->ascii_code == 0)) ||
             (mk->scan_code == UNDO))
          {
            if (mk->ascii_code == 0)
              switch (mk->scan_code)
              {
                case PGUP   : mk->shift     = TRUE;
                              mk->scan_code = UP;
                              break;
                case PGDOWN : mk->shift     = TRUE;
                              mk->scan_code = DOWN;
                              break;
                case POS1   : mk->shift     = TRUE;
                              mk->scan_code = LEFT;
                              break;
                case ENDKEY : mk->shift     = TRUE;
                              mk->scan_code = RIGHT;
                              break;
              } /* switch, if */

            get_work (window, FALSE);   /* Objektbaum fÅr jeweiliges Fenster korrigieren */
            if ((window->class != DESK) && ! (window->flags & WI_FIRSTDRW)) redraw_window (window, &window->work);

            next = window->edit_obj;                    /* Aktuelles Objekt */
            kr   = mk->kreturn;

            if ((mk->ascii_code == HT) && mk->shift)    /* éndere Shift HT in BackTAB */
            {
              mk->ascii_code = 0;
              mk->scan_code  = TAB;
              mk->kreturn    = kr = TAB << 8;
            } /* if */

            disabled = FALSE;
            def      = find_flags (window->object, ROOT, DEFAULT);

            if (def != NIL)
              if (is_state (window->object, def, DISABLED))
              {
                disabled = TRUE;
                undo_flags (window->object, def, DEFAULT);
              } /* if, if */

            if (mk->scan_code != UNDO)
            {
              cont = form_keybd (window->object, window->edit_obj, next, kr, &next, &kr);
/*
              	/* Close verhindern bei nicht exit-objekten */
					if (is_flags (window->object, def, DEFAULT) && !is_flags (window->object, def, EXIT))
						cont = TRUE;
*/					
              if ((OB_TYPE (window->object, window->edit_obj) == G_FTEXT) ||
                  (OB_TYPE (window->object, window->edit_obj) == G_FBOXTEXT))
                p = ((TEDINFO *)window->object [window->edit_obj].ob_spec)->te_pvalid;
              else
                p = NULL;
/* GefÑhrlich!
              if ((p != NULL) && (*p == '9') && ((mk->ascii_code == '+') || (mk->ascii_code == '-')))
              {
                get_ptext (window->object, window->edit_obj, s);
                l = atol (s);

                if (mk->ascii_code == '+')
                  l += mk->shift ? 10 : 1;
                else
                  if (mk->ascii_code == '-') l -= mk->shift ? 10 : 1;

                if (l >= 0)
                {
                  sprintf (s, "%ld", l);

                  if (strlen (s) <= strlen (p))
                  {
                    set_ptext (window->object, window->edit_obj, s);
                    edit_object (window, 0, EDEND);
                    save = window->edit_obj++;  /* draw_object versteckt Cursor */
                    draw_object (window, save);
                    window->edit_obj = save;
                    window->edit_inx = NIL;
                    edit_object (window, 0, EDINIT);
                  } /* if */
                } /* if */
              } /* if */
*/
            } /* if */
            else
            {
              next = find_flags (window->object, ROOT, UNDO_FLAG);
              cont = next == NIL;

              if (cont)                 /* Kein Abbruch-Knopf */
                next = 0;
              else                      /* Abbruch-Knopf vorhanden */
              {
                do_state (window->object, next, SELECTED);
                draw_object (window, next);
              } /* if */
            } /* else */

            updtmenu = ! cont;          /* MenÅs mÅssen sich nicht verÑndern */
            edit     = ((kr & 0xFF) != 0) || (mk->scan_code == LEFT) || (mk->scan_code == RIGHT) || (mk->scan_code == DELETE);

            if (disabled) do_flags (window->object, def, DEFAULT);

            if (edit)
            {
              if (mk->shift && (mk->scan_code == LEFT))         /* An den Anfang */
              {
                edit_object (window, 0, EDEND);
                window->edit_inx = 0;
                edit_object (window, 0, EDINIT);
              } /* if */
              else
                if (mk->shift && (mk->scan_code == RIGHT))      /* An das Ende */
                {
                  edit_object (window, 0, EDEND);
                  window->edit_inx = NIL;
                  edit_object (window, 0, EDINIT);
                } /* if */
                else
                {
                  if (is_flags (window->object, window->edit_obj, NOECHO_FLAG))
                    if (((kr & 0xFF) != 0) &&
                        ((kr & 0xFF) != ESC) &&
                        ((kr & 0xFF) != BS) &&
                        ((kr & 0xFF) != DEL)) kr = '*';

                  edit_object (window, kr, EDCHAR);
                } /* else, else */
            } /* if */
            else
              if (mk->shift && (mk->scan_code == UP))           /* Zum ersten */
                next = find_first (window->object);
              else
                if (mk->shift && (mk->scan_code == DOWN))       /* Zum letzten */
                  next = find_last (window->object);

            if (! cont || (next != 0) && (next != window->edit_obj))
            {
              if (cont)
              {
                edit_object (window, 0, EDEND);
                window->edit_obj = next;
                window->edit_inx = NIL;         /* Bewege Cursor nach hinten */
                edit_object (window, 0, EDINIT);
              } /* if */
              else
              {
                window->exit_obj = next;
                if ((window->flags & WI_MODAL) || (! mk->alt && ! (mk->momask & 2))) window->flags |= WI_DLCLOSE;
              } /* else */
            } /* if */

            if (window->click != NULL) (*window->click) (window, mk);
/*
				/* Liste der Display-Objekte durchgehen */
				header = window->dispobjs;
				element = list_next(header);
				while (element != header) {
					dispobj = (DISPOBJP) element->key;
					/* if (dispobj->click) (*dispobj->click) (dispobj, mk); */
					element = list_next(element);
				} /* while */
*/
          } /* if */
        } /* if */

			/* Liste der Display-Objekte durchgehen */
			header = window->dispobjs;
			element = list_next(header);
			found = FALSE;
			while (element != header && !found) {
				/* Checken ob auf diese Taste reagiert wird */
				dispobj = (DISPOBJP) element->key;
				/* Tastenroutine anspringen */
				if (dispobj->key) 
					found = (*dispobj->key) (dispobj, mk);
				element = list_next(element);
			} /* while */
			res = found;
        	if (!found && window->key != NULL) res = (*window->key) (window, mk);

        if (edit || (window->flags & WI_MODAL)) res = TRUE;

        if (window->flags & WI_DLCLOSE)
        {
          undo_state (window->object, next, SELECTED);
          close_window (window);
        } /* if */
        else
          if (! cont)
          {
            undo_state (window->object, next, SELECTED);

            if (is_flags (window->object, next, UNDO_FLAG))
              redraw_window (window, &window->scroll);
            else
              draw_object (window, next);
          } /* if, else */

        window->flags &= ~ WI_DLCLOSE;  /* Auf jeden Fall zurÅcksetzen */
      } /* if, if, if */

  return (res);
} /* key_window */

/*****************************************************************************/
/* Taste fÅr alle Fenster                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN key_all (mk)
MKINFO *mk;

{
		
	REG WINDOWP window;
	REG WORD i;
	BOOLEAN ok;
	
	ok = FALSE;
	
	/* TRA-Vorfahrt */
	for (i = 0; i < top; i++)                     /* Untersuche alle Fenster */
	{
		window = windows [i];
		/* CLASS_TRA ist nicht erreichbar ohne objects.h */
		if (strncmp (window->name, " TRA", 4) == 0)
			/* if (window->key) window->key (window, mk); */
			ok = key_window (window, mk);
	} /* for */

	if (!ok)
		if (top > 0)
			{
			ok = key_window (windows [0], mk);
			
			if (! ok && (windows [0]->class != class_desk))
			{
			window = search_window (class_desk, SRCH_OPENED, NIL); /* Wegen HauptmenÅ */
			
			if (window != NULL)
				ok = (window->flags & WI_LOCKED) || key_window (window, mk);
			} /* if */
		} /* if */
		
	return (ok);
} /* key_all */

/*****************************************************************************/
/* Zeitablauf fÅr ein Fenster                                                */
/*****************************************************************************/

GLOBAL VOID timer_window (window)
WINDOWP window;

{
	LIST_P	header, element;
	DISPOBJP	dispobj;

	if (window != NULL)
		if (window->timer != NULL) (*window->timer) (window);
	/* Liste der Display-Objekte durchgehen */
	header = window->dispobjs;
	element = list_next(header);
	while (element != header) {
		dispobj = (DISPOBJP) element->key;
		if (dispobj->timer) (*dispobj->timer) (dispobj);
		element = list_next(element);
	} /* while */
} /* timer_window */

/*****************************************************************************/
/* Zeitablauf fÅr alle Fenster                                               */
/*****************************************************************************/

GLOBAL VOID timer_all (milli)
LONG milli;

{
  REG WORD i;
  REG WINDOWP window;
	
  for (i = 0; i < top; i++)                     /* Untersuche alle Fenster */
  {
    window = windows [i];

    /* Zeitablauf berÅcksichtigen,
    	 Timer aktivieren nur wenn Fenster offen oder 
    	 PUF Fenster */
    if (window->milli != 0 && 
    		((window->opened > 0) || (window->class == CLASS_PUF)))
    {
      window->count += milli;
      
		/* énderung: wi_timer-Routine kann wi->milli auf 0 setzen um abzubrechen. BD */
      while ((window->count >= window->milli)&& (window->milli!=0))    /* Zeitereignis eingetreten */
      {
        window->count -= window->milli;
        timer_window (window);
      } /* while */
    } /* if */
  } /* for */
} /* timer_all */

/*****************************************************************************/
/* Fenster-Objektroutinen                                                    */
/*****************************************************************************/

GLOBAL VOID get_border (window, obj, border)
WINDOWP window;
WORD    obj;
RECT    *border;

{
  xywh2rect (0, 0, 0, 0, border);

  if (window != NULL)
    if (window->opened > 0)
      if ((window->object != NULL) && (obj != NIL)) objc_rect (window->object, obj, border, TRUE);
} /* get_border */

/*****************************************************************************/

GLOBAL VOID draw_object (window, obj)
WINDOWP window;
WORD    obj;

{
  WORD    wh, ret;
  RECT    r, border, save;
  BOOLEAN cursor;

  if (window != NULL)
    if ((window->opened > 0) && (window->object != NULL))
    {
      cursor = FALSE;

      if (window->flags & (WI_MODAL | WI_MODELESS))
        if ((window->edit_obj != 0) && (window->edit_obj == obj))
        {
          wind_get (DESK, WF_TOP, &wh, &ret, &ret, &ret);

          if (wh == window->handle) cursor = TRUE;
        } /* if, if */

      if (cursor) edit_object (window, 0, EDEND);

      wh = window->handle;

      get_border (window, obj, &border);
      wind_get (wh, WF_FIRSTXYWH, &r.x, &r.y, &r.w, &r.h);

      while ((r.w != 0) && (r.h != 0))
      {
        if (rc_intersect (&border, &r))
          if (rc_intersect (&window->scroll, &r))
          {
            save = clip;
            rc_intersect (&desk, &r);	/* geÑndert: Damit Åbergroûe Boxen gez. wd. kînnnen. BD */
            set_clip (TRUE, &r);
            objc_draw (window->object, obj, MAX_DEPTH, r.x, r.y, r.w, r.h);
            if (obj == ROOT)
	            /* énderung BD */
	            draw_oblines (window->object);
	         else
            	draw_obline (window->object, obj);
            set_clip (TRUE, &save);
          } /* if, if */

        wind_get (wh, WF_NEXTXYWH, &r.x, &r.y, &r.w, &r.h);
      } /* while */

      if (cursor) edit_object (window, 0, EDINIT);
    } /* if, if */
} /* draw_object */

/*****************************************************************************/

GLOBAL VOID set_cursor (window, obj, inx)
WINDOWP window;
WORD    obj, inx;

{
  if ((window != NULL) && (window->object != NULL) && (window->opened > 0))
    if (window->flags & (WI_MODAL | WI_MODELESS))
    {
      edit_object (window, 0, EDEND);
      window->edit_obj = obj;
      window->edit_inx = inx;
      edit_object (window, 0, EDINIT);
    } /* if, if */
} /* set_cursor */

/*****************************************************************************/

LOCAL BOOLEAN in_icon (mox, moy, window, obj)
WORD    mox, moy;
WINDOWP window;
WORD    obj;

{
  BOOLEAN ok;
  ICONBLK *icon;
  RECT    r, r1;

  ok = FALSE;

  get_border (window, obj, &r);

  if (inside (mox, moy, &r))        /* Im gesamten Rechteck */
  {
    icon  = (ICONBLK *)window->object [obj].ob_spec;
    r1    = r;
    r1.x += icon->ib_xicon;
    r1.y += icon->ib_yicon;
    r1.w  = icon->ib_wicon;
    r1.h  = icon->ib_ytext;         /* Bis zum Text, falls Icon kÅrzer */

    ok = inside (mox, moy, &r1);    /* Im Icon */

    if (! ok)                       /* Vielleicht im Text */
    {
      r1    = r;
      r1.x += icon->ib_xtext;
      r1.y += icon->ib_ytext;
      r1.w  = icon->ib_wtext;
      r1.h  = icon->ib_htext;

      ok = inside (mox, moy, &r1);  /* Im Text */
    } /* if */
  } /* if */

  return (ok);
} /* in_icon */

/*****************************************************************************/

LOCAL VOID set_style (expr)
WORD expr;

{
  vsl_udsty (vdi_handle, odd (expr) ? 0xAAAA : 0x5555);
} /* set_style */

/*****************************************************************************/

LOCAL VOID draw_box (box, x_offset, y_offset)
CONST RECT *box;
WORD       x_offset, y_offset;

{
  WORD xy [10];

  xy [0] = x_offset + box->x;
  xy [1] = y_offset + box->y;
  xy [2] = xy [0];
  xy [3] = xy [1] + box->h - 1;
  xy [4] = xy [0] + box->w - 1;
  xy [5] = xy [3];
  xy [6] = xy [4];
  xy [7] = xy [1];
  xy [8] = xy [0];
  xy [9] = xy [1];

  set_clip (TRUE, &desk);

  if (graph_proc)
  {
    set_style (xy [0] + xy [1] + 1);
    v_pline (vdi_handle, 2, &xy [0]);
    set_style (xy [2] + xy [3] + 1);
    v_pline (vdi_handle, 2, &xy [2]);
    set_style (xy [4] + xy [5] + 1);
    v_pline (vdi_handle, 2, &xy [4]);
    set_style (xy [6] + xy [7] + 1);
    v_pline (vdi_handle, 2, &xy [6]);
  } /* if */
  else
  {
    set_style (xy [0] + xy [1]);
    v_pline (vdi_handle, 2, &xy [0]);
    set_style (xy [3]);
    v_pline (vdi_handle, 2, &xy [2]);
    set_style (xy [4] + xy [5]);
    v_pline (vdi_handle, 2, &xy [4]);
    set_style (xy [7]);
    v_pline (vdi_handle, 2, &xy [6]);
  } /* else */
} /* draw_box */

/*****************************************************************************/

LOCAL VOID draw_all (num_boxes, boxes, x_offset, y_offset, bound, inner, diff)
WORD       num_boxes;
CONST RECT *boxes;
WORD       x_offset, y_offset;
CONST RECT *bound, *inner;
RECT       *diff;

{
  REG WORD i, delta;

  delta = 0;
  i     = bound->x - inner->x;
  if (i > 0) delta = i;                         /* Links heraushÑngend */

  i = bound->x + bound->w - (inner->x + inner->w + delta);
  if (i < 0) delta += i;                        /* Rechts heraushÑngend */

  x_offset += delta;

  delta = 0;
  i     = bound->y - inner->y;
  if (i > 0) delta = i;                         /* Oben heraushÑngend */

  i = bound->y + bound->h - (inner->y + inner->h + delta);
  if (i < 0) delta += i;                        /* Unten heraushÑngend */

  y_offset += delta;

  diff->w = x_offset;
  diff->h = y_offset;

  hide_mouse ();
  for (i = 0; i < num_boxes; i++) draw_box (&boxes [i], x_offset, y_offset);
  show_mouse ();
} /* draw_all */

/*****************************************************************************/

GLOBAL VOID drag_boxes (num_objs, boxes, inv_window, inv_objs, diff, bound, x_raster, y_raster)
WORD       num_objs;
CONST RECT *boxes;
WINDOWP    inv_window;
SET        inv_objs;
RECT       *diff;
CONST RECT *bound;
WORD       x_raster, y_raster;

{
  WORD    event, i, obj, last_obj, ret;
  WORD    x_offset, y_offset, x_last, y_last;
  RECT    startbox, box;
  MKINFO  mk, start;
  OBJECT  *object;

  graf_mkstate (&start.mox, &start.moy, &start.mobutton, &start.kstate);
  mk = start;
  xywh2rect (0, 0, 0, 0, &startbox);

  for (i = 0; i < num_objs; i++)                /* Vereinigungsbox */
    rc_union (&boxes [i], &startbox);

  box      = startbox;
  x_offset = y_offset = 0;
  obj      = last_obj = NIL;
  object   = NULL;

  if (inv_window != NULL) object = inv_window->object;
  if (bound == NULL) bound = &desk;

  line_default (vdi_handle);
  vswr_mode (vdi_handle, MD_XOR);               /* Modi einstellen */
  vsl_type (vdi_handle, USERLINE);

  draw_all (num_objs, boxes, x_offset, y_offset, bound, &box, diff);

  x_last = start.mox;
  y_last = start.moy;

  do
  {
    event = evnt_multi (MU_BUTTON | MU_M1,
                        1, start.mobutton, 0x0000,
                        TRUE, mk.mox, mk.moy, 1, 1,
                        0, 0, 0, 0, 0,
                        NULL,
                        0, 0,
                        &mk.mox, &mk.moy,
                        &mk.momask, &mk.kstate,
                        &mk.kreturn, &mk.breturn);

    graf_mkstate (&ret, &ret, &mk.mobutton, &mk.kstate); /* Werte nach Ereignis */

    if (event & MU_M1)
      if ((x_raster != 0) && ((abs (mk.mox - x_last)) >= x_raster) ||
          (y_raster != 0) && ((abs (mk.moy - y_last)) >= y_raster))
      {
        x_last = mk.mox;
        y_last = mk.moy;

        draw_all (num_objs, boxes, x_offset, y_offset, bound, &box, diff);

        if (object != NULL)
        {
          last_obj = obj;

          if (wind_find (mk.mox, mk.moy) == inv_window->handle)
          {
            obj = objc_find (object, ROOT, MAX_DEPTH, mk.mox, mk.moy);

            if (obj != NIL)
              if (OB_TYPE (object, obj) == G_ICON)
                if (! in_icon (mk.mox, mk.moy, inv_window, obj)) obj = NIL;
          } /* if */
          else
            obj = NIL;

          if (obj != last_obj)
          {
            if (setin (inv_objs, last_obj))
            {
              flip_state (object, last_obj, SELECTED);
              draw_object (inv_window, last_obj);
            } /* if */

            if (setin (inv_objs, obj))
            {
              flip_state (object, obj, SELECTED);
              draw_object (inv_window, obj);
            } /* if */
          } /* if */
        } /* if */

        x_offset = mk.mox - start.mox;
        y_offset = mk.moy - start.moy;

        if (x_raster != 0) x_offset = x_offset / x_raster * x_raster;
        if (y_raster != 0) y_offset = y_offset / y_raster * y_raster;

        box.x = startbox.x + x_offset;
        box.y = startbox.y + y_offset;

        draw_all (num_objs, boxes, x_offset, y_offset, bound, &box, diff);
      } /* if, if */
  } while (! (event & MU_BUTTON));

  draw_all (num_objs, boxes, x_offset, y_offset, bound, &box, diff);

  if (obj != NIL)
    if (setin (inv_objs, obj) && is_state (object, obj, SELECTED))
    {
      undo_state (object, obj, SELECTED);
      draw_object (inv_window, obj);
    } /* if, if */

  if (last_obj != NIL)
    if (setin (inv_objs, last_obj) && is_state (object, last_obj, SELECTED))
    {
      undo_state (object, last_obj, SELECTED);
      draw_object (inv_window, last_obj);
    } /* if, if */

  diff->x = mk.mox;
  diff->y = mk.moy;
} /* drag_boxes */

/*****************************************************************************/

LOCAL WORD calc_pos (list)
LISTBOX *list;

{
  WORD pos, max_pos;
  LONG value;

  max_pos = list->num_items - list->vis_items;

  if (max_pos <= 0)
    pos = 0;
  else
  {
    value = 1000L * list->first_item * (list->tree [list->parent].ob_height - list->tree [list->slider].ob_height) / max_pos;
    pos   = (WORD)(value / 1000);
    if (value % 1000 >= 500) pos++;
  } /* else */

  return (pos);
} /* calc_pos */

/*****************************************************************************/

LOCAL WORD calc_size (list)
LISTBOX *list;

{
  WORD size;

  size = list->tree [list->parent].ob_height;

  if ((list->num_items > 0) && (list->vis_items <= list->num_items))
    size = (size * list->vis_items) / list->num_items;

  if (size < gl_hbox) size = gl_hbox;

  return (size);
} /* calc_size */

/*****************************************************************************/

LOCAL BOOLEAN in_listbox (list)
LISTBOX *list;

{
  WORD item;

  item = list->active - list->first_item;
  return ((0 <= item) && (item < min (list->vis_items, list->num_items)));
} /* in_listbox */

/*****************************************************************************/

LOCAL WORD item2obj (list, item)
LISTBOX *list;
WORD    item;

{
  return (item - list->first_item + list->items + 1);
} /* item2obj */

/*****************************************************************************/

LOCAL WORD obj2item (list, obj)
LISTBOX *list;
WORD    obj;

{
  return (list->first_item + obj - list->items - 1);
} /* obj2item */

/*****************************************************************************/

GLOBAL VOID draw_listobj (list, obj, flip)
LISTBOX *list;
WORD    obj;
BOOLEAN flip;

{
  RECT    r, scroll;
  OBJECT  *tree;
  WINDOWP window;

  objc_rect (list->tree, obj, &r, obj == list->slider);
  window = list->window;

  if (flip && (list->sel_state == SELECTED) && (window == NULL))
    objc_change (list->tree, obj, 0, r.x, r.y, r.w, r.h, list->tree [obj].ob_state ^ SELECTED, TRUE);
  else
  {
    if (flip)
    {
      flip_state (list->tree, obj, list->sel_state);
      if (list->sel_state == CHECKED) r.w = 2 * gl_wbox;
    } /* if */

    if (window == NULL)
      if ((OB_TYPE (list->tree, obj) == G_STRING) || (obj == list->items))
        objc_draw (list->tree, list->items, MAX_DEPTH, r.x, r.y, r.w, r.h);
      else
        objc_draw (list->tree, obj, MAX_DEPTH, desk.x, desk.y, desk.w, desk.h);
    else
    {
      tree           = window->object;  /* Falls nicht der Objektbaum... */
      window->object = list->tree;      /* ...des Fensters benutzt wird */

      if ((OB_TYPE (list->tree, obj) == G_STRING) || (obj == list->items))
      {
        scroll         = window->scroll;
        window->scroll = r;
        rc_intersect (&scroll, &window->scroll);
        draw_object (window, list->items);
        window->scroll = scroll;
      } /* if */
      else
        draw_object (window, obj);

      window->object = tree;
    } /* else */
  } /* else */
} /* draw_listobj */

/*****************************************************************************/

GLOBAL BOOLEAN listbox (list, flags, mk)
LISTBOX *list;
UWORD   flags;
MKINFO  *mk;

{
  BOOLEAN    dbl, to_draw;
  REG OBJECT *tree;
  BYTE       *p, *l;
  WORD       i, obj, old;
  WORD       max_pos, old_pos, new_pos, height;
  LONG       value, delta;
  RECT       r, rect;

  dbl  = FALSE;
  tree = list->tree;

  if (flags & LIST_INIT)
  {
    list->vis_items = tree [list->items].ob_tail - tree [list->items].ob_head + 1;
    list->width     = (tree [list->items].ob_width + 1) / gl_wbox;      /* 1 Pixel wird unten abgezogen */
    max_pos         = list->num_items - list->vis_items;

    if (list->first_item > max_pos) list->first_item = max_pos;
    if (list->first_item < 0) list->first_item = 0;

    if (tree [list->items].ob_width % gl_wbox == 0) tree [list->items].ob_width--;

    if (tree [list->parent].ob_height % gl_hbox == 0)   /* Wegen Umrissen der Pfeile */
    {
      tree [list->parent].ob_y++;
      tree [list->parent].ob_height -= 2;
    } /* if */

    l = (BYTE *)list->itemlist;

    for (i = 0; i < list->vis_items; i++)
    {
      obj = i + list->items + 1;
      undo_state (tree, obj, list->sel_state);

      if (tree [obj].ob_width % gl_wbox == 0) tree [obj].ob_width--;

      switch (OB_TYPE (tree, obj))
      {
        case G_STRING   :
        case G_BUTTON   : p = (BYTE *)tree [obj].ob_spec;
                          break;
        case G_TEXT     :
        case G_BOXTEXT  :
        case G_FTEXT    :
        case G_FBOXTEXT : p = ((TEDINFO *)tree [obj].ob_spec)->te_ptext;
                          break;
      } /* switch */

      if (list->first_item + i < list->num_items)       /* GenÅgend EintrÑge vorhanden */
      {
        if (list->indirect)
          strncpy (p, *(BYTE **)&l [(list->first_item + i) * list->itemsize], list->width);
        else
          strncpy (p, (BYTE *)&l [(list->first_item + i) * list->itemsize], list->width);
        p [list->width] = EOS;
      } /* if */
      else
        *p = EOS;
    } /* for */

    if (in_listbox (list)) do_state (tree, item2obj (list, list->active), list->sel_state);

    tree [list->slider].ob_height = calc_size (list);
    tree [list->slider].ob_y      = calc_pos (list);

    if (flags & LIST_DRAW) draw_listobj (list, list->root, FALSE);
  } /* if */

  if (flags & LIST_CLICK)
  {
    obj = objc_find (tree, list->root, MAX_DEPTH, mk->mox, mk->moy);

    do
    {
      if ((list->sel_state != NORMAL) &&
          ! is_state (tree, obj, DISABLED) &&
          (list->items < obj) && (obj <= list->items + min (list->vis_items, list->num_items)))
      {
        dbl = mk->breturn == 2;

        if (list->active != obj2item (list, obj))
        {
          if (list->active != NIL)              /* Bereits einmal selektiert */
            if (in_listbox (list))
            {
              old = item2obj (list, list->active);
              draw_listobj (list, old, TRUE);
            } /* if, if */

          list->active = obj2item (list, obj);
          draw_listobj (list, obj, TRUE);
        } /* if */
      } /* if */

      if ((list->parent <= obj) && (obj <= list->down))
      {
        old_pos = new_pos = list->first_item;
        max_pos = list->num_items - list->vis_items;

        if (max_pos < 0) max_pos = 0;

        if (obj == list->up) new_pos--;
        if (obj == list->down) new_pos++;

        if (obj == list->parent)
        {
          objc_rect (tree, list->slider, &r, FALSE);
          if (mk->moy < r.y) 
            new_pos -= list->vis_items;         /* Seite nach oben */
          else
            new_pos += list->vis_items;         /* Seite nach unten */
        } /* if */

        if (obj == list->slider)
        {
          value   = (LONG)max_pos * graf_slidebox (tree, list->parent, obj, 1);
          new_pos = (WORD)(value / 1000);
          if (value % 1000 >= 500) new_pos++;
        } /* if */

        if (new_pos > max_pos) new_pos = max_pos;
        if (new_pos < 0) new_pos = 0;

        delta = new_pos - old_pos;

        if (delta != 0)
        {
          if (list->active != NIL)
            if (in_listbox (list))
            {
              old = item2obj (list, list->active);
              undo_state (tree, old, list->sel_state);
            } /* if, if */

          list->first_item = new_pos;

          if (list->active != NIL)
            if (in_listbox (list))
            {
              old = item2obj (list, list->active);
              do_state (tree, old, list->sel_state);
            } /* if, if */

          l = (BYTE *)list->itemlist;

          for (i = 0; i < list->vis_items; i++)
          {
            old = i + list->items + 1;

            switch (OB_TYPE (tree, old))
            {
              case G_STRING   :
              case G_BUTTON   : p = (BYTE *)tree [old].ob_spec;
                                break;
              case G_TEXT     :
              case G_BOXTEXT  :
              case G_FTEXT    :
              case G_FBOXTEXT : p = ((TEDINFO *)tree [old].ob_spec)->te_ptext;
                                break;
            } /* switch */

            if (list->first_item + i < list->num_items)         /* GenÅgend EintrÑge vorhanden */
            {
              if (list->indirect)
                strncpy (p, *(BYTE **)&l [(list->first_item + i) * list->itemsize], list->width);
              else
                strncpy (p, (BYTE *)&l [(list->first_item + i) * list->itemsize], list->width);
              p [list->width] = EOS;
            } /* if */
          } /* for */

          old = tree [list->slider].ob_y;
          tree [list->slider].ob_y = calc_pos (list);
          if (old != tree [list->slider].ob_y) draw_listobj (list, list->parent, FALSE);

          to_draw = labs (delta) != 1;

          if (! to_draw && (list->window != NULL))
          {
            to_draw = ! is_top (list->window);          /* Fenster nicht ganz oben */

            if (! to_draw)
            {
              objc_rect (tree, list->items, &r, FALSE);
              rect = r;
              rc_intersect (&desk, &rect);
              to_draw = ! rc_equal (&r, &rect);         /* Objekt nicht ganz in Desktop */

              if (! to_draw)
              {
                rect = r;
                rc_intersect (&list->window->scroll, &rect);
                to_draw = ! rc_equal (&r, &rect);       /* Objekt nicht ganz im Scrollbereich */
              } /* if */
            } /* if */
          } /* if */

          if (to_draw)
            draw_listobj (list, list->items, FALSE);
          else
          {
            height = tree [list->items + 1].ob_height;
            objc_rect (tree, list->items, &r, FALSE);

            if (delta > 0)
            {
              r.y += height;
              r.h -= height;
              scroll_area (&r, VERTICAL, (WORD)(delta * height));
              draw_listobj (list, list->items + list->vis_items, FALSE);
            } /* if */
            else
            {
              r.h -= height;
              scroll_area (&r, VERTICAL, (WORD)(delta * height));
              draw_listobj (list, list->items + 1, FALSE);
            } /* else */
          } /* else */
        } /* if */
      } /* if */

      graf_mkstate (&mk->mox, &mk->moy, &mk->mobutton, &mk->kstate);
      obj = objc_find (tree, list->root, MAX_DEPTH, mk->mox, mk->moy);
    } while ((mk->momask == mk->mobutton) && ! is_flags (tree, obj, TOUCHEXIT));
  } /* if */

  return (dbl);
} /* listbox */

/*****************************************************************************/

GLOBAL VOID edit_noecho (mk, cursor, s, maxlen)
MKINFO *mk;
WORD   cursor;
BYTE   *s;
WORD   maxlen;

{
  WORD len;

  len = strlen (s);

  if ((mk->ascii_code != 0) && (mk->ascii_code != CR) && (mk->scan_code != TAB) && ! mk->ctrl && (! iscntrl (mk->ascii_code) || DLG_CNTRL (mk)))
    switch (mk->ascii_code)
    {
      case ESC : *s = EOS;
                 break;
      case BS  : cursor--;      /* durchfallen */
      case DEL : if ((0 <= cursor) && (cursor < len))
                   mem_move (s + cursor, s + cursor + 1, len - cursor);
                 break;
      default  : if (cursor < maxlen)
                 {
                   mem_move (s + cursor + 1, s + cursor, len - cursor);
                   if (len < maxlen) len++;
                 } /* if */
                 else
                   cursor--;

                 s [cursor] = mk->ascii_code;
                 s [len]    = EOS;
                 break;
    } /* switch, if */
} /* edit_noecho */

/*****************************************************************************/
/* Scroll- und Lîschroutinen                                                 */
/*****************************************************************************/

GLOBAL VOID scroll_area (area, dir, delta)
CONST RECT *area;
WORD       dir, delta;

{
  MFDB  s, d;
  WORD  xy [8];
  RECT  dst;

  wind_update (BEG_UPDATE);             /* Benutzer darf nicht mehr agieren */
  hide_mouse ();                        /* Maus verstecken, da gescrollt wird */

  s.mp = d.mp = NULL;                   /* Erzwinge Bildschirmadresse */

  rect2array (area, xy);
  rect2array (area, &xy [4]);

  if (dir & HORIZONTAL)                 /* Horizontales Scrolling */
  {
    xy [4] -= delta;
    xy [6] -= delta;
  } /* else */
  else                                  /* Vertikales Scrolling */
  {
    xy [5] -= delta;
    xy [7] -= delta;
  } /* if */

  array2rect (&xy [4], &dst);           /* Nur Zielraster zeichnen */

  if (rc_intersect (&desk, &dst))
  {
    set_clip (TRUE, &dst);              /* Setze Clipping auf Schirm */
    vro_cpyfm (vdi_handle, S_ONLY, xy, &s, &d); /* Eigentliches Scrolling */
  } /* if */

  show_mouse ();                        /* Zeige Maus wieder */
  wind_update (END_UPDATE);             /* Benutzer darf wieder agieren */
} /* scroll_area */

/*****************************************************************************/

GLOBAL VOID clr_area (area)
CONST RECT *area;

{
  WORD xy [4];
  RECT r;

  r = *area;

  if ((r.w > 0) && (r.h > 0))
  {
    vswr_mode (vdi_handle, MD_REPLACE);         /* Modus = replace */
    vsf_interior (vdi_handle, FIS_SOLID);       /* Muster */
    vsf_color (vdi_handle, WHITE);              /* Farbe weiû */
    rect2array (&r, xy);                        /* Bereich setzen */
    vr_recfl (vdi_handle, xy);                  /* Bereich lîschen */
  } /* if */
} /* clr_area */

/*****************************************************************************/

GLOBAL VOID clr_work (window)
WINDOWP window;

{
  RECT r;
  WORD menu_height;

  r            = window->work;
  menu_height  = (window->menu != NULL) ? gl_hattr : 0;
  r.y         += menu_height;
  r.h         -= menu_height;

  clr_area (&r);
} /* clr_work */

/*****************************************************************************/

GLOBAL VOID clr_scroll (window)
WINDOWP window;

{
  clr_area (&window->scroll);
} /* clr_scroll */

/*****************************************************************************/

GLOBAL VOID clr_left (window)
WINDOWP window;

{
  RECT r;

  r.x = window->work.x;
  r.y = window->scroll.y;
  r.w = window->scroll.x - window->work.x;
  r.h = window->scroll.h;

  clr_area (&r);
} /* clr_left */

/*****************************************************************************/

GLOBAL VOID clr_top (window)
WINDOWP window;

{
  RECT r;
  WORD menu_height;

  menu_height = (window->menu != NULL) ? gl_hattr : 0;

  r.x = window->work.x;
  r.y = window->work.y + menu_height;
  r.w = window->work.w;
  r.h = window->scroll.y - window->work.y - menu_height;

  clr_area (&r);
} /* clr_top */

/*****************************************************************************/

GLOBAL VOID clr_right (window)
WINDOWP window;

{
  RECT r;

  r.x = window->scroll.x + window->scroll.w;
  r.y = window->scroll.y;
  r.w = window->work.x + window->work.w - window->scroll.x - window->scroll.w;
  r.h = window->scroll.h;

  clr_area (&r);
} /* clr_right */

/*****************************************************************************/

GLOBAL VOID clr_bottom (window)
WINDOWP window;

{
  RECT r;

  r.x = window->work.x;
  r.y = window->scroll.y + window->scroll.h;
  r.w = window->work.w;
  r.h = window->work.y + window->work.h - window->scroll.y - window->scroll.h;

  clr_area (&r);
} /* clr_bottom */

/*****************************************************************************/

GLOBAL VOID set_redraw (window, area)
WINDOWP    window;
CONST RECT *area;

{
  WORD msgbuff [8];

  msgbuff [0] = WM_REDRAW;
  msgbuff [1] = gl_apid;
  msgbuff [2] = 0;
  msgbuff [3] = window->handle;
  msgbuff [4] = area->x;
  msgbuff [5] = area->y;
  msgbuff [6] = area->w;
  msgbuff [7] = area->h;

  appl_write (gl_apid, sizeof (msgbuff), msgbuff);
} /* set_redraw */

/*****************************************************************************/
/* Fenster-MenÅroutinen                                                      */
/*****************************************************************************/

GLOBAL VOID draw_mtitle (window, title)
WINDOWP window;
WORD    title;

{
  WORD wh;
  RECT r1, r2;

  if ((window->opened > 0) && (window->menu != NULL))
  {
    wh = window->handle;

    get_work (window, FALSE);   /* MenÅzeile fÅr jeweiliges Fenster korrigieren */
    objc_rect (window->menu, title, &r2, TRUE);

    wind_get (wh, WF_FIRSTXYWH, &r1.x, &r1.y, &r1.w, &r1.h);

    while ((r1.w != 0) && (r1.h != 0))
    {
      if (rc_intersect (&r2, &r1))
        objc_draw (window->menu, THEBAR, 2, r1.x, r1.y, r1.w, r1.h);

      wind_get (wh, WF_NEXTXYWH, &r1.x, &r1.y, &r1.w, &r1.h);
    } /* while */
  } /* if */
} /* draw_mtitle */

/*****************************************************************************/

GLOBAL VOID draw_mbar (window)
WINDOWP window;

{
  WORD wh;
  RECT r, r1, r2, r3;
  WORD x_offset;
  WORD xy [4];

  if (window->menu != NULL)
  {
    wh = window->handle;

    get_work (window, FALSE);

    if (window->updt_menu != NULL) (*window->updt_menu) (window); /* MenÅs einstellen */

    x_offset = 0;

    if (window->flags & WI_MNSCROLL)
    {
      x_offset = menu_offset;

      arrows [ROOT].ob_x = window->work.x;
      arrows [ROOT].ob_y = window->work.y;

      if (window->first_menu == THEFIRST)       /* Auf erstem MenÅ */
        do_state (arrows, M_LTARROW, DISABLED);
      else
        undo_state (arrows, M_LTARROW, DISABLED); 

      if (window->first_menu == window->menu [THEACTIVE].ob_tail) /* Auf letzem MenÅ */
        do_state (arrows, M_RTARROW, DISABLED);
      else
        undo_state (arrows, M_RTARROW, DISABLED);
    } /* if */

    line_default (vdi_handle);

    xywh2array (window->work.x, window->work.y + gl_hattr - 1, window->work.w, 1, xy);
    r2 = clip;                          /* Aktuelles Clipping berÅcksichtigen */
    xywh2rect (window->work.x + x_offset + window->menu [THEACTIVE].ob_x, window->work.y, window->work.w, gl_hattr - 1, &r3);

    r   = window->work;                 /* MenÅzeile lîschen */
    r.h = gl_hattr - 1;

    wind_get (wh, WF_FIRSTXYWH, &r1.x, &r1.y, &r1.w, &r1.h);

    while ((r1.w != 0) && (r1.h != 0))
    {
      if (rc_intersect (&r2, &r1))      /* Objekt nur auf Arbeitsbereich zeichnen */
      {
        set_clip (TRUE, &r1);
        clr_area (&r);
        if (window->flags & WI_MNSCROLL) objc_draw (arrows, ROOT, 1, r1.x, r1.y, r1.w, r1.h);

        if (rc_intersect (&r3, &r1))
          objc_draw (window->menu, THEBAR, 2, r1.x, r1.y, r1.w, r1.h);

        v_pline (vdi_handle, 2, xy);    /* Line unter MenÅzeile */
      } /* if */

      wind_get (wh, WF_NEXTXYWH, &r1.x, &r1.y, &r1.w, &r1.h);
    } /* while */
  } /* if */
} /* draw_mbar */

/*****************************************************************************/

GLOBAL VOID menu_normal (window, title, normal)
WINDOWP window;
WORD    title;
BOOLEAN normal;

{
  WORD state;

  if (window == NULL)
  {
    if (menu != NULL) menu_tnormal (menu, title, normal);  /* Benutze globales MenÅ */
  } /* if */
  else
    if (window->menu != NULL)
    {
      state = window->menu [title].ob_state;

      if (normal)
        undo_state (window->menu, title, SELECTED);
      else
        do_state (window->menu, title, SELECTED);

      if (window->first_menu <= title)     /* Im Fenster */
        if (state != window->menu [title].ob_state) draw_mtitle (window, title);
    } /* if, else */
} /* menu_normal */

/*****************************************************************************/

LOCAL VOID hndl_mscroll (window, obj, breturn)
WINDOWP window;
WORD    obj, breturn;

{
  WORD old, new, thelast, offset;
  WORD mox, moy, mobutton, mokstate;
  RECT r;

  do
  {
    new     = old = window->first_menu;
    thelast = window->menu [THEACTIVE].ob_tail;

    switch (obj)
    {
      case M_LTARROW : if (breturn == 2)
                         new = THEFIRST;
                       else
                         new--;
                       break;
      case M_RTARROW : if (breturn == 2) 
                         new = thelast;
                       else
                         new++;
                       break;
    } /* switch */

    if (new < THEFIRST) new = THEFIRST;
    if (new > thelast) new = thelast;

    window->first_menu = new;

    if (old != new)
    {
      offset = ((old == THEFIRST) || (new == THEFIRST) ||
                (old == thelast)  || (new == thelast)) ? 0 : menu_offset;
      r.x    = window->work.x + offset;
      r.y    = window->work.y;
      r.w    = window->work.w - offset;
      r.h    = gl_hattr - 1;

      set_clip (TRUE, &r);
      draw_mbar (window);
    } /* if */

    graf_mkstate (&mox, &moy, &mobutton, &mokstate);
    obj = objc_find (arrows, ROOT, 1, mox, moy);  /* In MenÅpfeilen ? */
  } while (mobutton & 1);       /* Bis kein Knopf mehr gedrÅckt ist */
} /* hndl_mscroll */

/*****************************************************************************/

LOCAL VOID set_menu (enable)
BOOLEAN enable;

{
  REG BOOLEAN menu_in_bar;
  REG WINDOWP window;

  window      = search_window (class_desk, SRCH_ANY, NIL);
  menu_in_bar = (window == NULL) || (window->menu == NULL);

  wind_update (BEG_UPDATE);

  if ((menu != NULL) && ! deskacc && menu_in_bar)
  {
#if GEM & GEM1
    if (enable)
      menu [THEACTIVE].ob_width = width;
    else
      if (menu [THEACTIVE].ob_width != 0)
      {
        width = menu [THEACTIVE].ob_width;
        menu [THEACTIVE].ob_width = 0;
      } /* if, else */
#else
    {
      REG WORD title;

      title = THEFIRST;

      do
      {
        if (enable)
          undo_state (menu, title, DISABLED);
        else
          do_state (menu, title, DISABLED);

        title = menu [title].ob_next;
      } while (title != THEACTIVE);
    } /* #else */
#endif
  } /* if */

  wind_update (END_UPDATE);
} /* set_menu */

/*****************************************************************************/

LOCAL VOID fix_menu (window)
WINDOWP window;

{
  OBJECT *menu;
  WORD   title, menubox;
  WORD   tdiff, ddiff, border;
  RECT   bbox, mbox;

  menu    = window->menu;
  menubox = menu [ROOT].ob_tail;
  menubox = menu [menubox].ob_head;
  title   = THEFIRST;

  objc_rect (menu, THEBAR, &bbox, FALSE);        /* Rechteck der MenÅzeile */

  if (window->handle != DESK)                    /* Keine Accessories */
  {                                              /* MenÅbaum umhÑngen */
    menu [menubox].ob_height   = menu [menubox + 1].ob_height;
    menu [menubox].ob_tail     = menu [menubox].ob_head;
    menu [menubox + 1].ob_next = menubox;
  } /* if */

  do
  {
    menu [menubox].ob_x = menu [THEACTIVE].ob_x + menu [title].ob_x; /* Normalstellung */
    menu [menubox].ob_y = 0;                                         /* Normalstellung */

    objc_rect (menu, menubox, &mbox, FALSE);

    if ((mbox.y + mbox.h > desk.y + desk.h) &&          /* Muû nach oben klappen */
        (bbox.y > (desk.y + desk.h) / 2))               /* Oben mehr Platz als unten */
    {
      tdiff  = 0;
      border = (WORD)(((LONG)menu [menubox].ob_spec >> 16) & 0x00FFL);

      if (border & 0x0080) border |= 0xFF00;            /* Rand negativ */

      if (border < 0) tdiff = - border;                 /* Wegen Rand */

      if (is_state (menu, menubox, SHADOWED))           /* Schatten berÅcksichtigen */
        tdiff += 2 * abs (border);

      if (is_state (menu, menubox, OUTLINED))           /* Outlined berÅcksichtigen */
        if (border >= 0)
          tdiff += 3;                                   /* Wegen Rand */
        else
          if (border > -3) tdiff += 3 + border;

      menu [menubox].ob_y = - (menu [menubox].ob_height + menu [title].ob_height + tdiff);
    } /* if */

    ddiff = mbox.x + mbox.w - (desk.x + desk.w);        /* Rechts heraushÑngend ? */

    if (ddiff > 0) menu [menubox].ob_x -= ddiff;
    objc_rect (menu, menubox, &mbox, FALSE);
    if (mbox.x < 0) menu [menubox].ob_x -= mbox.x;      /* Links heraushÑngend */

    menubox = menu [menubox].ob_next;                   /* NÑchstes Drop-Down-MenÅ */
    title   = menu [title].ob_next;                     /* NÑchster Titel */
  } while (title != THEACTIVE);
} /* fix_menu */

/*****************************************************************************/

GLOBAL BOOLEAN menu_manager (window, mox, moy, mobutton, breturn)
WINDOWP window;
WORD    mox, moy, mobutton, breturn;

{
  MFDB    screen, buffer;
  WORD    title, item, oldtitle, olditem;
  WORD    menubox, obj;
  RECT    r;
  WORD    i;
  WORD    event, ret;
  UWORD   uret;
  BOOLEAN in_title;
  OBJECT  *menu;

  get_work (window, FALSE);     /* MenÅzeile fÅr jeweiliges Fenster korrigieren */

  menu = window->menu;

  if (moy >= window->work.y + gl_hattr) return (FALSE);  /* Nicht in MenÅbereich */
  if (moy < window->work.y) return (FALSE);              /* z.B. in Infozeile */
  if (! is_top (window) && (window->flags & WI_TOPMENU)) return (FALSE);

  if (window->flags & WI_LOCKED) return (FALSE);

  if (window->flags & WI_MNSCROLL)
  {
    arrows [ROOT].ob_x = window->work.x;
    arrows [ROOT].ob_y = window->work.y;

    obj = objc_find (arrows, ROOT, 1, mox, moy); /* In MenÅpfeilen ? */

    if (obj != NIL)                             /* MenÅpfeile behandeln */
    {
      hndl_mscroll (window, obj, breturn);
      return (TRUE);
    } /* if */
  } /* if */

  if (desk.w < MIN_WDESK)                       /* MenÅzeile angleichen */
  {
    menu [ROOT].ob_width                = MIN_WDESK;
    menu [THEBAR].ob_width              = MIN_WDESK;
    menu [menu [ROOT].ob_tail].ob_width = MIN_WDESK;
  } /* if */

  if (window->updt_menu != NULL) (*window->updt_menu) (window); /* MenÅs einstellen */

  oldtitle = NIL;
  olditem  = NIL;
  title    = objc_find (menu, THEACTIVE, MAX_DEPTH, mox, moy); /* In MenÅ ? */
  item     = NIL;
  menubox  = NIL;

  if (title != NIL)
    if (is_state (menu, title, DISABLED) || (title < window->first_menu)) title = NIL;

  if (title == NIL) return (TRUE);              /* Kein korrektes MenÅ */

  fix_menu (window);

  set_mouse (ARROW, NULL);
  wind_update (BEG_MCTRL);                      /* Mauskontrolle Åbernehmen */

  do
  {
    if (oldtitle != title)
    {
      if (title != NIL)
      {
        menu_normal (window, title, FALSE);

        menubox = menu [ROOT].ob_tail;
        menubox = menu [menubox].ob_head;

        for (i = THEFIRST; i < title; i++)
          menubox = menu [menubox].ob_next;             /* Durchhangeln */

        background (menu, menubox, TRUE, &screen, &buffer);
        objc_draw (menu, menubox, 1, desk.x, desk.y, desk.w, desk.h);
      } /* if */
    } /* if */

    if (item != NIL)                    /* In MenÅeintrag */
      objc_rect (menu, item, &r, FALSE);
    else
      if (title != NIL)                 /* In MenÅtitel */
        objc_rect (menu, title, &r, FALSE);
      else
        xywh2rect (mox, moy, 1, 1, &r); /* Sonstwo */

    event = evnt_multi (MU_BUTTON | MU_M1,
                        1, 0x0001, ~ mobutton & 0x0001,
                        TRUE, r.x, r.y, r.w, r.h,
                        0, 0, 0, 0, 0,
                        NULL, 0, 0,
                        &mox, &moy, &ret, &ret, &uret, &ret);

    oldtitle = title;
    olditem  = item;

    title = objc_find (menu, THEACTIVE, MAX_DEPTH, mox, moy);
    item  = NIL;

    if ((title < window->first_menu) || ! inside (mox, moy, &window->work)) title = NIL;

    if (menubox != NIL) item = objc_find (menu, menubox, MAX_DEPTH, mox, moy);

    if (title != NIL)
      if (is_state (menu, title, DISABLED)) title = NIL;

    in_title = title != NIL;

    if (item != NIL)
      if (is_state (menu, item, DISABLED)) item = NIL;

    if (title == NIL) title = oldtitle;             /* Auûerhalb bei gewÑhltem MenÅ */

    if (olditem != item)
    {
      if (olditem != NIL)
        objc_change (menu, olditem, 0, desk.x, desk.y, desk.w, desk.h, menu [olditem].ob_state ^ SELECTED, TRUE);

      if (item != NIL)
        objc_change (menu, item, 0, desk.x, desk.y, desk.w, desk.h, menu [item].ob_state ^ SELECTED, TRUE);
    } /* if */

    if (oldtitle != title)
      if (oldtitle != NIL)
      {
        background (menu, menubox, FALSE, &screen, &buffer);
        menu_normal (window, oldtitle, TRUE);

        menubox = NIL;
      } /* if, if */
  } while (! (event & MU_BUTTON) || (~ mobutton & 0x0001) && in_title);

  wind_update (END_MCTRL);                      /* Mauskontrolle wieder abgeben */
  blink (menu, item, blinkrate);
  last_mouse ();

  if (menubox != NIL) background (menu, menubox, FALSE, &screen, &buffer);

  if (item == NIL)
    menu_normal (window, title, TRUE);          /* Titel wieder normal */
  else
  {
    undo_state (menu, item, SELECTED);
    if (~ mobutton & 0x0001) evnt_button (1, 0x0001, 0x0000, &ret, &ret, &ret, &ret); /* Warte auf Mausknopf */

    if (window->hndl_menu != NULL) (*window->hndl_menu) (window, title, item);
  } /* else */

  return (TRUE);
} /* menu_manager */

/*****************************************************************************/

GLOBAL BOOLEAN menu_key (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  OBJECT  *lmenu;
  WORD    title, item;
  WINDOWP wind;

  lmenu = window->menu;

  if (lmenu == NULL)
    if (window->class == class_desk) lmenu = menu;      /* Globales MenÅ */

  if ((lmenu != NULL) && (window->hndl_menu != NULL))
  {
    if (window->updt_menu != NULL) (*window->updt_menu) (window); /* MenÅs einstellen */

    if (is_menu_key (lmenu, mk, &title, &item))
    {
      wind = (window->menu == NULL) ? NULL : window;    /* Globaler MenÅ-Handler */

      (*window->hndl_menu) (wind, title, item);         /* MenÅ aufrufen */

      return (TRUE);                                    /* fertig */
    } /* if */
  } /* if */

  return (FALSE);
} /* menu_key */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_windows (err_nowindow, max_reswind, class_help)
WORD err_nowindow, max_reswind, class_help;

{
  BOOLEAN ok;
  WORD    wattr, hattr;
  LONG    max_res;
#if 0
  WORD    pel, index;

  v_get_pixel (vdi_handle, desk.x, desk.y + 1, &pel, &index);
#endif

#if GEM & XGEM
  graph_proc = TRUE;
#else
/*graph_proc = pel == 0;*/
  graph_proc = FALSE;
#endif

  sel_window  = NULL;                     /* Kein Fenster angewÑhlt */
  top         = 0;                        /* Oberstes Fenster Index 0 */
  windows     = NULL;                     /* Kein Fensterkeller */
  windrec     = NULL;                     /* Keine Fenster */
  nowindow    = err_nowindow;             /* Fehler kein Fenster Åbrig */
  wattr       = gl_wattr - 2;             /* RÑnder abziehen */
  hattr       = gl_hattr - 2;
  menu_offset = (2 * wattr + 8) & 0xFFF8; /* Offset MenÅzeile */
  helpclass   = class_help;

  setclr (used_windows);                  /* Benutzte Fenster */
  setclr (sel_objs);                      /* Keine Objekte selektiert */

  arrows [ROOT].ob_width       = 2 * wattr + 1; /* Pfeile anpassen */
  arrows [ROOT].ob_height      = hattr + 1;
  arrows [M_LTARROW].ob_width  = wattr;
  arrows [M_LTARROW].ob_height = hattr + 1;
  arrows [M_RTARROW].ob_x      = wattr + 1;
  arrows [M_RTARROW].ob_width  = wattr;
  arrows [M_RTARROW].ob_height = hattr + 1;

  if (max_reswind > SETMAX) max_reswind = SETMAX;
  if (max_reswind < 0) max_reswind = 0;

  max_res = (mem_avail () / (sizeof (WINDOWP) + sizeof (WINDOW)));

  if (max_reswind > max_res) max_reswind = max_res;

  max_windows = max_reswind;
  ok          = (max_windows > 0);

  if (ok)
  {
    windows = (WINDOWP *)mem_alloc ((LONG)max_windows * sizeof (WINDOWP));
    windrec = (WINDOWP)mem_alloc ((LONG)max_windows * sizeof (WINDOW));
    ok      = (windows != NULL) && (windrec != NULL);
  } /* if */

  return (ok);
} /* init_windows */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_windows ()

{
  close_all (TRUE, TRUE);                  /* Schlieûe und lîsche alle Fenster */

  if (windrec != NULL) mem_free (windrec); /* Speicher freigeben */
  if (windows != NULL) mem_free (windows); /* Speicher freigeben */

  return (TRUE);
} /* term_windows */

