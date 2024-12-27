/*****************************************************************************/
/*                                                                           */
/* Modul: DIALOG.C                                                           */
/* Datum: 20/06/93                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
20.06.93
- alert_mode auf FALSE

03.06.93
- alert_msgs in DIALOG.H ausgelagert, als GLOBAL
*****************************************************************************/
#include "import.h"
#include "global.h"
#include "windows.h"

#include "event.h"

#include "export.h"
#include "dialog.h"

/****** DEFINES **************************************************************/

#define KIND     (NAME|CLOSER|MOVER)
#define FLAGS    (WI_RESIDENT)
#define XFAC     gl_wbox                /* X-Faktor */
#define YFAC     2                      /* Y-Faktor */
#define XUNITS   1                      /* X-Einheiten fr Scrolling */
#define YUNITS   (gl_hbox / YFAC)       /* Y-Einheiten fr Scrolling */
#define MILLI    0                      /* Millisekunden fr Zeitablauf */

#define MINBUT   6                      /* Mindestens 6 Buchstaben fr Buttons */
#define ALT_CHAR '~'                    /* Zeichen fr Alternate-Bedienung */

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

LOCAL HELPFUNC helpfunc = NULL; /* Zeiger auf Help-Funktion */
LOCAL OBJECT   *alert_tree;     /* Objekt-Baum fr alerts */
LOCAL WORD     alert_index;     /* Resource-Index des Objekt-Baums */
LOCAL BYTE     *alert_title;    /* Titel fr Fehlermeldungsfenster */
LOCAL WORD     alert_exit;      /* Angew„hlter Exit-Button der Alert-Box */
LOCAL STRING   alert_help;      /* Hilfsstring fr Alert */
LOCAL BOOLEAN  alert_mode = FALSE;      /* TRUE, wenn Fehler als Dialogbox ausgegeben werden soll */

/****** FUNCTIONS ************************************************************/

LOCAL VOID    click_alert _((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN key_alert   _((WINDOWP window, MKINFO *mk));

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
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_dialog (alerts, tree, index, title)
BYTE     **alerts;
OBJECT   *tree;
WORD     index;
BYTE     *title;

{
  WORD obj, y;

  alert_msgs  = alerts;
  alert_tree  = tree;
  alert_index = index;
  alert_title = title;

  if (alert_tree != NULL)
  {
    obj = find_type (alert_tree, ROOT, G_IMAGE);

    while (OB_TYPE (alert_tree, obj) == G_IMAGE)
    {
      alert_tree [obj].ob_x = gl_wbox;
      alert_tree [obj].ob_y = gl_hbox;
      obj++;
    } /* while */

    obj = find_type (alert_tree, ROOT, G_BUTTON);
    y   = alert_tree [obj - 1].ob_y + 2 * gl_hbox;

    do
    {
      alert_tree [obj].ob_y = y;
    } while (! is_flags (alert_tree, obj++, LASTOB));

    obj--;
    alert_tree->ob_height = alert_tree [obj].ob_y + alert_tree [obj].ob_height + gl_hbox;
  } /* if */

  return (TRUE);
} /* init_dialog */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_dialog ()

{
  return (TRUE);
} /* term_dialog */

/*****************************************************************************/

GLOBAL VOID set_helpfunc (help)
HELPFUNC help;

{
  helpfunc = help;
} /* set_helpfunc */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

GLOBAL WINDOWP crt_dialog (obj, menu, icon, title, flags)
OBJECT *obj, *menu;
WORD   icon;
BYTE   *title;
UWORD  flags;

{
  WINDOWP window;
  WORD    menu_height, help_button, diff;
  UWORD   kind;
  RECT    b;

  kind = KIND;

  if (flags & WI_MODAL) kind &= ~ CLOSER;

  window = create_window (kind, CLASS_DIALOG);

  if (window != NULL)
  {
    menu_height = (menu != NULL) ? gl_hattr : 0;

#if 0 /* GEM & GEM1 */
    {
      WORD diff;

      diff = desk.y + desk.h - 400;

      if (diff > 0) obj->ob_y -= diff;          /* Wie in GEM/3 */
    } /* GEM & GEM1 */
#endif

    obj->ob_x = max (0, (WORD)(obj->ob_x & 0xFFF8));
    obj->ob_y = max (2 * gl_hattr, obj->ob_y);
    obj->ob_y = (obj->ob_y & 0xFFFE) + 1;       /* Immer ungerade */

    window->flags     = FLAGS | flags;
    window->icon      = icon;
    window->doc.x     = 0;
    window->doc.y     = 0;
    window->doc.w     = 0;
    window->doc.h     = 0;
    window->xfac      = XFAC;
    window->yfac      = YFAC;
    window->xunits    = XUNITS;
    window->yunits    = YUNITS;
    window->scroll.x  = obj->ob_x;
    window->scroll.y  = obj->ob_y;
    window->scroll.w  = obj->ob_width;
    window->scroll.h  = obj->ob_height;
    window->work.x    = window->scroll.x;
    window->work.y    = window->scroll.y - menu_height;
    window->work.w    = window->scroll.w;
    window->work.h    = window->scroll.h + menu_height;
    window->bg_color  = -1;
    window->mousenum  = ARROW;
    window->mouseform = NULL;
    window->milli     = MILLI;
    window->special   = 0;
    window->edit_obj  = 0;
    window->edit_inx  = 0;
    window->exit_obj  = 0;
    window->object    = obj;
    window->menu      = menu;
    window->hndl_menu = NULL;
    window->updt_menu = NULL;
    window->test      = NULL;
    window->open      = wi_open;
    window->close     = wi_close;
    window->delete    = NULL;
    window->draw      = NULL;
    window->arrow     = NULL;
    window->snap      = NULL;
    window->objop     = NULL;
    window->drag      = NULL;
    window->click     = NULL;
    window->unclick   = NULL;
    window->key       = NULL;
    window->timer     = NULL;
    window->top       = NULL;
    window->untop     = NULL;
    window->edit      = NULL;
    window->showinfo  = NULL;
    window->showhelp  = NULL;

    strcpy (window->name, title);

    help_button = find_flags (window->object, ROOT, HELP_FLAG);

    if (help_button != NIL)
      if (helpfunc == NULL)
        do_state (window->object, help_button, DISABLED);
      else
        undo_state (window->object, help_button, DISABLED);

    wind_calc (WC_BORDER, window->kind,
             window->work.x, window->work.y, window->work.w, window->work.h,
             &b.x, &b.y, &b.w, &b.h);

    diff = desk.y - b.y;
    if (diff > 0)
    {
      window->scroll.y += (diff + 1) & 0xFFFE;
      window->work.y   += (diff + 1) & 0xFFFE;
    } /* if */
  } /* if */

  return (window);                      /* Fenster zurckgeben */
} /* crt_dialog */

/*****************************************************************************/
/* ™ffnen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN open_dialog (icon)
WORD icon;

{
  BOOLEAN ok;
  WINDOWP window;

  if ((window = search_window (CLASS_DIALOG, SRCH_OPENED, icon)) != NULL)
  {
    ok = TRUE;
    top_window (window);
  } /* if */
  else
  {
    window = search_window (CLASS_DIALOG, SRCH_CLOSED, icon);
    ok     = window != NULL;

    if (ok)
    {
      ok = open_window (window);

      if (ok)
        if (window->flags & WI_MODAL) hndl_modal (icon != alert_index);
    } /* if */
  } /* else */

  return (ok);
} /* open_dialog */

/*****************************************************************************/

GLOBAL WORD hndl_alert (alert_id)
WORD alert_id;

{
  WORD button;

  button = NIL;

  if (alert_msgs != NULL) button = open_alert (alert_msgs [alert_id]);

  return (button);
} /* hndl_alert */

/*****************************************************************************/

GLOBAL WORD open_alert (alertmsg)
BYTE *alertmsg;

{
  WORD    ret, i, j, x, w, w1, w2;
  WINDOWP window;
  BOOLEAN ok;
  INT     image, bell, def, cancel, help;
  WORD    image1, string1, button1;
  WORD    maxstr, maxbut, num_buttons;
  BYTE    *p, *s;
  STRING  str;

  image1  = find_type (alert_tree, ROOT, G_IMAGE);
  string1 = find_type (alert_tree, ROOT, G_STRING);
  button1 = find_type (alert_tree, ROOT, G_BUTTON);
  w       = alert_tree [string1].ob_width / gl_wbox;

  p = strchr (alertmsg, SEP_OPEN) + 1;
  sscanf (p, "%d", &image);                             /* Image einlesen */

  for (i = image1; OB_TYPE (alert_tree, i) == G_IMAGE; i++) /* Image vorbereiten */
  {
    if (i - image1 == image)
      undo_flags (alert_tree, i, HIDETREE);
    else
      do_flags (alert_tree, i, HIDETREE);
  } /* for */

  maxstr = 0;
  p      = strchr (p, SEP_OPEN) + 1;
  i      = string1;

  while (*p != SEP_CLOSE)                       /* Strings verarbeiten */
  {
    s = str;
    undo_flags (alert_tree, i, HIDETREE);

    while (((*p != SEP_CLOSE) || (p [1] != SEP_OPEN)) && (*p != SEP_LINE)) *s++ = *p++;

    *s      = EOS;
    str [w] = EOS;
    strcpy ((BYTE *)alert_tree [i].ob_spec, str);
    maxstr = max (maxstr, strlen (str));
    i++;
    if (*p != SEP_CLOSE) p++;
  } /* while */

  while (OB_TYPE (alert_tree, i) == G_STRING) do_flags (alert_tree, i++, HIDETREE);

  p = strchr (p, SEP_OPEN) + 1;
  i = button1;

  do                                            /* Buttons verstecken */
  {
    do_flags (alert_tree, i, HIDETREE | EXIT);
    undo_flags (alert_tree, i, DEFAULT | UNDO_FLAG | HELP_FLAG);
    undo_state (alert_tree, i, DISABLED);
  } while (! is_flags (alert_tree, i++, LASTOB));

  maxbut      = 0;
  i           = button1;
  num_buttons = 0;

  while (*p != SEP_CLOSE)                       /* Buttons verarbeiten */
  {
    s = (BYTE *)alert_tree [i].ob_spec;
    undo_flags (alert_tree, i, HIDETREE);

    while ((*p != SEP_CLOSE) && (*p != SEP_LINE)) *s++ = *p++;

    *s = EOS;
    s  = (BYTE *)alert_tree [i].ob_spec;

    if (strchr (s, ALT_CHAR) != NULL)           /* alternate control char */
    {
      for (j = 0; s [j] != ALT_CHAR; j++);

      alert_tree [i].ob_type &= 0xFF;
      alert_tree [i].ob_type |= ((j + 1) << 8); /* Position merken */
      strcpy (s + j, s + j + 1);                /* Zeichen rausl”schen */
    } /* if */

    maxbut = max (maxbut, strlen ((BYTE *)alert_tree [i].ob_spec));
    i++;
    num_buttons++;
    if (*p != SEP_CLOSE) p++;
  } /* while */

  p = strchr (p, SEP_OPEN) + 1;                /* Button-Definitionen holen */
  sscanf (p, "%d|%d|%d|%d", &bell, &def, &cancel, &help);

  if (def != 0)
  {
    def += button1 - 1;
    do_flags (alert_tree, def, DEFAULT);
  } /* if */

  if (cancel != 0)
  {
    cancel += button1 - 1;
    do_flags (alert_tree, cancel, UNDO_FLAG);
  } /* if */

  if (help != 0)
  {
    help += button1 - 1;
    do_flags (alert_tree, help, HELP_FLAG);
    undo_flags (alert_tree, help, EXIT);
  } /* if */

  p = strchr (p, SEP_OPEN) + 1;                /* Hilfstext-Definitionen holen */
  strcpy (alert_help, p);
  p  = strchr (alert_help, SEP_CLOSE);
  *p = EOS;

  maxbut = max (maxbut, MINBUT);               /* Minimale Breite der Kn”pfe */

  for (i = button1, x = gl_wbox, maxbut += 2; i - button1 < num_buttons; i++)
  {
    alert_tree [i].ob_x     = x;
    alert_tree [i].ob_width = maxbut * gl_wbox;
    x += (maxbut + 2) * gl_wbox;
  } /* for */

  w1                   = alert_tree [string1].ob_x + maxstr * gl_wbox;
  w2                   = alert_tree [button1 + num_buttons - 1].ob_x + maxbut * gl_wbox;
  alert_tree->ob_width = gl_wbox + max (w1, w2);

  if (w1 > w2)                                          /* Buttons zentrieren */
    for (i = button1; i - button1 < num_buttons; i++)
      alert_tree [i].ob_x += (w1 - w2) / 2;

  form_center (alert_tree, &ret, &ret, &ret, &ret);
  window = alert_mode ? NULL : crt_dialog (alert_tree, NULL, alert_index, alert_title, WI_MODAL);

  if (window != NULL)
  {
    window->flags &= ~ WI_RESIDENT;
    window->open   = NULL;
    window->close  = NULL;
    window->click  = click_alert;
    window->key    = key_alert;
  } /* if */

  if ((help != 0) && (helpfunc == NULL)) do_state (alert_tree, help, DISABLED);

  if (bell) beep ();

  if ((window == NULL) || ! open_dialog (alert_index))
  {
    if (help != 0) do_state (alert_tree, help, DISABLED);       /* Keine Hilfe, da kein weiteres Fenster */
    alert_tree->ob_spec = 0x00021100L;                          /* Rand innen = 2 Pixel */
    alert_exit = hndl_dial (alert_tree, 0, FALSE, TRUE, NULL, &ok) - find_type (alert_tree, ROOT, G_BUTTON) + 1;
    alert_tree->ob_spec = 0x00001100L;
    if (help != 0) undo_state (alert_tree, help, DISABLED);
  } /* if */

  return (alert_exit);
} /* open_alert */

/*****************************************************************************/

GLOBAL BOOLEAN set_alert (as_dialog)
BOOLEAN as_dialog;

{
  BOOLEAN old_mode;

  old_mode   = alert_mode;
  alert_mode = as_dialog;

  return (old_mode);
} /* set_alert */

/*****************************************************************************/

GLOBAL VOID hndl_modal (use_timer)
BOOLEAN use_timer;

{
  BOOLEAN modal_done;                   /* Modale Dialogbox beendet */
  WORD    devents;                      /* Einzutretende Dialog-Ereignisse */
  WORD    event;                        /* Eingetretenes Ereignis */
  WORD    msgbuff [8];                  /* Event Message Puffer */
  WORD    locked;                       /* Anzahl gelockter Fenster */
  WORD    act_hidden;                   /* Anzahl versteckter M„use */
  WORD    act_busy;                     /* Aktuelle Anzahl Busy-Calls */
  WORD    act_number;                   /* Aktuelle Mausform-Nummer */
  MFORM   *act_form;                    /* Aktuelle Mausform */
  WORD    i;                            /* Hilfsvariable */
  WORD    ret;                          /* Dummy Return-Wert */
  MKINFO  mk;                           /* Maus und Tastatur Info */
  WINDOWP window;                       /* Fr Top-Window */
  WORD    tophandle;                    /* Handle von Top-Window */

  locked    = num_locked ();
  window    = find_top ();
  tophandle = window->handle;

  if ((window == NULL) || (window->class == CLASS_DIALOG))
  {
    act_number = mousenumber;
    act_form   = mouseform;
    act_hidden = hidden;
    act_busy   = busy;
    busy       = 0;

    for (i = 0; i < act_hidden; i++) show_mouse ();
    graf_mouse (ARROW, NULL);
  } /* if */

  wind_update (END_UPDATE);             /* BEG_UPDATE in event.c */

  devents = MU_KEYBD | MU_BUTTON  | MU_MESAG;

  if (events & MU_TIMER) devents |= MU_TIMER;

  if (! use_timer) devents &= ~ MU_TIMER;

  do
  {
    event = evnt_multi (devents,
                        bclicks, bmask, bstate,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        msgbuff,
                        (UWORD)(millisecs & 0xFFFF), (UWORD)(millisecs >> 16),
                        &mk.mox, &mk.moy,
                        &mk.momask, &mk.kstate,
                        &mk.kreturn, &mk.breturn);

    graf_mkstate (&ret, &ret, &mk.mobutton, &mk.kstate); /* Werte nach Ereignis */

#if MSDOS
    if (mk.momask == 0x0000) mk.momask = 0x0001;         /* Irgendein Knopf ist linker Knopf */
#endif

    wind_update (BEG_UPDATE);           /* Keine Interaktion zulassen */

    mk.shift  = (mk.kstate & (K_RSHIFT | K_LSHIFT)) != 0;
    mk.ctrl   = (mk.kstate & K_CTRL) != 0;
    mk.alt    = (mk.kstate & K_ALT) != 0;
    acc_close = FALSE;

    if (event & MU_KEYBD ) hndl_keybd (&mk);
    if (event & MU_BUTTON) hndl_button (&mk);
    if (event & MU_MESAG ) hndl_mesag (msgbuff);
    if (event & MU_TIMER ) hndl_timer (millisecs);

    if (deskacc && done)
      close_all (FALSE, TRUE);          /* Schlieže alle Fenster */

    wind_update (END_UPDATE);           /* Benutzer darf wieder agieren */
    modal_done = (find_window (tophandle) == NULL) || (num_locked () < locked);
  } while (! (modal_done || acc_close));

  wind_update (BEG_UPDATE);             /* END_UPDATE in event.c */

  if ((window == NULL) || (window->class == CLASS_DIALOG))
  {
    busy = act_busy;
    for (i = 0; i < act_hidden; i++) hide_mouse ();
    graf_mouse (act_number, act_form);
  } /* if */
} /* hndl_modal */

/*****************************************************************************/
/* Box zeichnen                                                              */
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
/* ™ffne Fenster                                                             */
/*****************************************************************************/

LOCAL VOID wi_open (window)
WINDOWP window;

{
  box (window, TRUE);
} /* wi_open */

/*****************************************************************************/
/* Schlieže Fenster                                                          */
/*****************************************************************************/

LOCAL VOID wi_close (window)
WINDOWP window;

{
  box (window, FALSE);
} /* wi_close */

/*****************************************************************************/

LOCAL VOID click_alert (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  if (window->exit_obj > 0)
  {
    if (is_flags (window->object, window->exit_obj, HELP_FLAG))
    {
      if (helpfunc != NULL) (*helpfunc) (alert_help);
      undo_state (window->object, window->exit_obj, SELECTED);
      draw_object (window, window->exit_obj);
      window->flags &= ~ WI_DLCLOSE;    /* falls HELP default (und damit EXIT) ist */
    } /* if */

    if (is_flags (window->object, window->exit_obj, EXIT))
      alert_exit = window->exit_obj - find_type (window->object, ROOT, G_BUTTON) + 1;
  } /* if */
} /* click_alert */

/*****************************************************************************/

LOCAL BOOLEAN key_alert (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  WORD def, next;
  RECT r;

  if (mk->scan_code == TAB)
  {
    def = find_flags (window->object, ROOT, DEFAULT);

    if (def != NIL)
    {
      if (mk->shift)
      {
        next = def - 1;

        if ((OB_TYPE (window->object, next) != G_BUTTON))
          next = find_flags (window->object, ROOT, LASTOB);

        while ((OB_FLAGS (window->object, next) & HIDETREE) ||
               (OB_STATE (window->object, next) & DISABLED))
        {
          next--;
          if ((OB_TYPE (window->object, next) != G_BUTTON))
            next = find_flags (window->object, ROOT, LASTOB);
        } /* while */
      } /* if */
      else
      {
        next = window->object [def].ob_next;

        if (next <= def) next = find_type (window->object, ROOT, G_BUTTON);

        while ((OB_FLAGS (window->object, next) & HIDETREE) ||
               (OB_STATE (window->object, next) & DISABLED))
        {
          next = window->object [next].ob_next;
          if (next <= def) next = find_type (window->object, ROOT, G_BUTTON);
        } /* while */
      } /* else */

      if (next != def)
      {
        undo_flags (window->object, def, DEFAULT);
        do_flags (window->object, next, DEFAULT);

        objc_rect (window->object, def, &r, TRUE);
        r.x -= gl_wbox;
        r.y -= gl_hbox;
        r.w += 2 * gl_wbox;
        r.h += 2 * gl_hbox;
        rc_intersect (&desk, &r);
        set_redraw (window, &r);

        objc_rect (window->object, next, &r, TRUE);
        r.x -= gl_wbox;
        r.y -= gl_hbox;
        r.w += 2 * gl_wbox;
        r.h += 2 * gl_hbox;
        rc_intersect (&desk, &r);
        set_redraw (window, &r);
      } /* if */
    } /* if */
  } /* if */

  return (FALSE);
} /* key_alert */

