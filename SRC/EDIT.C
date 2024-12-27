/*****************************************************************************/
/*                                                                           */
/* Modul: EDIT.C                                                             */
/* Datum: 18/11/90                                                           */
/*                                                                           */
/*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"

#include "realtim4.h"
#include "errors.h"

#include "clipbrd.h"
#include "desktop.h"
#include "dialog.h"
#include "printer.h"

#include "export.h"
#include "edit.h"

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|INFO|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_MOUSE|WI_CURSKEYS)
#define XFAC   gl_wbox                  /* X-Faktor */
#define YFAC   gl_hbox                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  (1 * gl_wbox)            /* X-Anfangsposition */
#define INITY  (8 * gl_hbox)            /* Y-Anfangsposition */
#define INITW  initw                    /* Anfangsbreite in Pixel */
#define INITH  inith                    /* Anfangshîhe in Pixel */
#define MILLI  0                        /* Millisekunden fÅr Zeitablauf */

#define MINW      14                    /* Minimale Fensterbreite */
#define MINH       4                    /* Minimale Fensterhîhe */
#define MAX_WDOC 255                    /* 255 Zeichen/Zeile */

/****** TYPES ****************************************************************/

typedef struct
{
  STRING filename;                      /* Name der Datei zum Editieren */
  LONG   size;                          /* aktuelle Textgrî·e */
  LONG   cols;                          /* Anzahl Spalten */
  LONG   lines;                         /* Anzahl Zeilen */
  WORD   x_cursor;                      /* x-Position des Cursors im Fenster */
  WORD   y_cursor;                      /* y-Position des Cursors im Fenster */
  WORD   fontheight;                    /* Zeichensatzhîhe */
  WORD   fontid;                        /* Zeichensatznummer (1 = System) */
  BYTE   *text;                         /* zeigt auf aktuellen Textpuffer */
  BYTE   **line_ptr;                    /* zeigt auf ZeilenanfÑnge */
} EDIT_INF;

typedef BYTE DOCLINE [MAX_WDOC + 1]; /* Zeichenkette fÅr Dokumentzeile */

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

/*****************************************************************************/
/* VorwÑrtsreferenzen                                                        */
/*****************************************************************************/

LOCAL VOID    get_line    _((EDIT_INF *edit_inf, LONG line, BYTE *buffer));
LOCAL VOID    mtextinfo   _((WINDOWP window));
LOCAL BOOLEAN read_edit   _((EDIT_INF *edit_inf));

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
/* Textpuffer-Funktionen                                                     */
/*****************************************************************************/

LOCAL VOID get_line (edit_inf, line, buffer)
EDIT_INF *edit_inf;
LONG     line;
BYTE     *buffer;

{
  REG WORD i, j;
  REG BYTE c;
  REG BYTE *text;

  i    = j = 0;
  c    = EOS;
  text = edit_inf->line_ptr [line];

  while ((j < MAX_WDOC) && (c != LF))
  {
    c = text [i];

    if ((c == CR) || (c == LF))
      buffer [j] = EOS;
    else
    {
      if (c == '\t')
      {
        while ((j < MAX_WDOC) && (j % 8 != 7)) buffer [j++] = SP;
        c = SP;
      } /* if */

      buffer [j] = c;
    } /* else */

    i++;
    j++;
  } /* while */

  buffer [MAX_WDOC] = EOS;
} /* get_line */

/*****************************************************************************/
/* Info-MenÅ                                                                 */
/*****************************************************************************/

LOCAL VOID mtextinfo (window)
WINDOWP window;

{
  LONGSTR  s;
  EDIT_INF *edit_inf;

  edit_inf = (EDIT_INF *)window->special;

  sprintf (s, alerts [ERR_INFEDIT], window->doc.h, edit_inf->size); /* Zeilen, Bytes */
  open_alert (s);
} /* mtextinfo */

/*****************************************************************************/

LOCAL BOOLEAN read_edit (edit_inf)
EDIT_INF *edit_inf;

{
  REG LONG    i, file_size, lines;
  REG HPTR    text;
      BOOLEAN ok;
      WORD    f;

  busy_mouse ();

  ok = TRUE;
  f  = Fopen (edit_inf->filename, 0);

#if MSDOS | FLEXOS
  if (DOS_ERR) f = -32 - f;
#endif

  if (f < -3)                                   /* Datei nicht gefunden */
  {
    hndl_alert (ERR_FOPEN);
    ok = FALSE;
  } /* if */
  else
  {
    file_size = Fseek (0L, f, 2);

    edit_inf->text = (BYTE *)mem_alloc (file_size);
    if (edit_inf->text == NULL)
    {
      hndl_alert (ERR_NOMEMORY);
      ok = FALSE;
    } /* if */
    else
    {
      edit_inf->size       = file_size;
      edit_inf->fontheight = gl_hbox;
      edit_inf->fontid     = 1;                 /* system font */

      Fseek (0L, f, 0);
      file_size = Fread (f, edit_inf->size, edit_inf->text);

      if (file_size < edit_inf->size)
      {
        edit_inf->size = file_size;
	hndl_alert (ERR_FREAD);
      } /* if */

      text           = edit_inf->text;
      lines          = 0;
      i              = 1;
      edit_inf->cols = 0;

      while (file_size > 0)                     /* Zeilen zÑhlen */
      {
        if ((*text == EOS) || (i == MAX_WDOC)) *text = LF;
        if (*text == LF)
        {
          if (i > edit_inf->cols) edit_inf->cols = i;
          lines++;
          i = 0;
        } /* if */

        file_size--;
        text++;
        i++;
      } /* while */

      edit_inf->cols     -= 2;                  /* CR, LF */
      edit_inf->lines     = lines;
      edit_inf->line_ptr  = (BYTE **)mem_alloc (lines * sizeof (edit_inf->line_ptr));
      if (edit_inf->line_ptr == NULL)
      {
        hndl_alert (ERR_NOMEMORY);
        mem_free (edit_inf->text);
        ok = FALSE;
      } /* if */
      else
      {
        text = edit_inf->text;
        i    = 0;

        while (i < lines)                       /* ZeilenanfÑnge setzen */
        {
          edit_inf->line_ptr [i] = (FPTR)text;
          while (*text != LF) text++;
          i++;
          text++;
        } /* while */
      } /* else */
    } /* else */

    Fclose (f);
  } /* else */

  arrow_mouse ();
  return (ok);
} /* read_edit */

/*****************************************************************************/
/* Box zeichnen                                                              */
/*****************************************************************************/

LOCAL VOID box (window, grow)
WINDOWP window;
BOOLEAN grow;

{
  RECT l, b;

  get_clipxywh (window->icon, &l);

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
/* Lîsche Fenster                                                            */
/*****************************************************************************/

LOCAL VOID wi_delete (window)
WINDOWP window;

{
  EDIT_INF *edit_inf;

  edit_inf = (EDIT_INF *)window->special;
  mem_free (edit_inf->text);
  mem_free (edit_inf->line_ptr);
  mem_free (edit_inf);
  set_meminfo ();
} /* wi_delete */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

LOCAL VOID wi_draw (window)
WINDOWP window;

{
  REG LONG     i;
  REG EDIT_INF *edit_inf;
      WORD     x, y, w, h;
      RECT     new;
      DOCLINE  line;

  edit_inf = (EDIT_INF *)window->special;

  clr_scroll (window);
  text_default (vdi_handle);

  x = window->scroll.x;                 /* x-Koordinate fÅr alle Zeilen */
  y = window->scroll.y;                 /* y-Koordinate fÅr erste Zeile */
  w = window->scroll.w / window->xfac;  /* Breite des Fensters in Zeichen */
  h = window->scroll.h;                 /* Hîhe in Pixeln */

  if (window->scroll.x + window->scroll.w == clip.x + clip.w)
  {
    rc_copy (&clip, &new);
    new.w += window->xfac;
    set_clip (TRUE, &new);  /* schnellere Textausgabe */
  } /* if */

  for (i = window->doc.y; (i < window->doc.h) && (y - window->scroll.y < h); i++, y += window->yfac)
  {
    get_line (edit_inf, i, line);
    line [window->doc.x + w] = EOS;
    if (y + edit_inf->fontheight > clip.y)
      if (strlen (line) > window->doc.x)
        v_text (vdi_handle, x, y, line + window->doc.x, window->xfac);  /* Textzeile ausgeben */
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

  if (dir & HORIZONTAL)             /* Horizontale Pfeile und Schieber */
  {
    if (delta != 0)                                    /* Scrolling nîtig */
    {
      window->doc.x = newpos;                          /* Neue Position */

      set_sliders (window, HORIZONTAL, SLPOS);         /* Schieber setzen */
      scroll_window (window, HORIZONTAL, delta * window->xfac);
    } /* if */
  } /* if */
  else                              /* Vertikale Pfeile und Schieber */
  {
    if (delta != 0)                                    /* Scrolling nîtig */
    {
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

  wind_get (window->handle, WF_CXYWH, &r.x, &r.y, &r.w, &r.h);

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

GLOBAL VOID print_edit (filename)
BYTE *filename;

{
  STRING outname;
  WORD   in_handle, out_handle;
  LONG   size;
  BYTE   buffer [1024];
  WORD   port;

  port = 0;
  if (! prn_check (port)) return;

  busy_mouse ();

#if MSDOS
  strcpy (outname, "PRN");
#else
  strcpy (outname, "PRN:");
#endif

  out_handle = Fcreate (outname, 0);

#if MSDOS | FLEXOS
  if (DOS_ERR) out_handle = -32 - out_handle;
#endif

  if (out_handle < -3)
    hndl_alert (ERR_FOPEN);
  else
  {
    in_handle = Fopen (filename, 0);

#if MSDOS | FLEXOS
    if (DOS_ERR) in_handle = -32 - in_handle;
#endif

    if (in_handle < -3)
      hndl_alert (ERR_FOPEN);
    else
    {
      while ((size = Fread (in_handle, (LONG)sizeof (buffer), buffer)) > 0)
        Fwrite (out_handle, size, buffer);

      Fclose (in_handle);
    } /* else */

    Fclose (out_handle);
  } /* else */

  arrow_mouse ();
} /* print_edit */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

GLOBAL WINDOWP crt_edit (obj, menu, icon, filename)
OBJECT *obj, *menu;
WORD   icon;
BYTE   *filename;

{
  WINDOWP  window;
  WORD     menu_height, inx;
  STR128   s;
  EDIT_INF *edit_inf;
  WORD     initw, inith;

  edit_inf = (EDIT_INF *)mem_alloc ((LONG)sizeof (EDIT_INF));
  if (edit_inf == NULL)
  {
    hndl_alert (ERR_NOMEMORY);
    return (NULL);
  } /* if */

  mem_set (edit_inf, 0, sizeof (EDIT_INF));
  strcpy (edit_inf->filename, filename);

  if (! read_edit (edit_inf))
  {
    mem_free (edit_inf);
    return (NULL);
  } /* if */

  initw = min (desk.w - 5 * XFAC, edit_inf->cols * XFAC);
  inith = min (10 * YFAC, edit_inf->lines * YFAC);
  initw = max (initw, MINW * XFAC);
  inith = max (inith, MINH * YFAC);

  inx    = num_windows (CLASS_EDIT, SRCH_ANY, NULL);
  window = create_window (KIND, CLASS_EDIT);

  if (window != NULL)
  {
    menu_height = (menu != NULL) ? gl_hattr : 0;

    window->flags     = FLAGS;
    window->icon      = icon;
    window->doc.x     = 0;
    window->doc.y     = 0;
    window->doc.w     = edit_inf->cols;
    window->doc.h     = edit_inf->lines;
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
    window->mousenum  = TEXT_CRSR;
    window->mouseform = NULL;
    window->milli     = MILLI;
    window->special   = (LONG)edit_inf;
    window->object    = obj;
    window->menu      = menu;
    window->hndl_menu = NULL;
    window->updt_menu = NULL;
    window->test      = NULL;
    window->open      = wi_open;
    window->close     = wi_close;
    window->delete    = wi_delete;
    window->draw      = wi_draw;
    window->arrow     = wi_arrow;
    window->snap      = wi_snap;
    window->objop     = NULL;
    window->drag      = NULL;
    window->click     = NULL;
    window->unclick   = NULL;
    window->key       = NULL;
    window->timer     = NULL;
    window->top       = NULL;
    window->untop     = NULL;
    window->edit      = NULL;
    window->showinfo  = info_edit;
    window->showhelp  = help_edit;

    if ((strchr (filename, DRIVESEP) == NULL) && (strchr (filename, PATHSEP) == NULL))
      strcpy (s, act_path);
    else
      s [0] = EOS;

    strcat (s, filename);
    sprintf (window->name, " %s ", s);
    sprintf (window->info, (BYTE *)freetext [FEDITINF].ob_spec, window->doc.h);
  } /* if */
  else
  {
    mem_free (edit_inf->text);
    mem_free (edit_inf);
  } /* else */

  set_meminfo ();
  return (window);                                      /* Fenster zurÅckgeben */
} /* crt_edit */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN open_edit (icon, filename)
WORD icon;
BYTE *filename;

{
  BOOLEAN ok;
  WINDOWP window;

  if ((icon != NIL) && (window = search_window (CLASS_EDIT, SRCH_OPENED, icon)) != NULL)
  {
    ok = TRUE;
    top_window (window);
  } /* if */
  else
  {
    if ((window = search_window (CLASS_EDIT, SRCH_CLOSED, icon)) == NULL)
      window = crt_edit (NULL, NULL, icon, filename);

    ok = window != NULL;

    if (ok) ok = open_window (window);
  } /* else */

  return (ok);
} /* open_edit */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

GLOBAL BOOLEAN info_edit (window, icon)
WINDOWP window;
WORD    icon;

{
  if (icon != NIL)
    window = search_window (CLASS_EDIT, SRCH_ANY, icon);

  if (window != NULL) mtextinfo (window);
  return (window != NULL);
} /* info_edit */

/*****************************************************************************/
/* Hilfe des Objekts                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN help_edit (window, icon)
WINDOWP window;
WORD    icon;

{
  hndl_alert (ERR_HELPEDIT);
  return (TRUE);
} /* help_edit */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_edit ()

{
  return (TRUE);
} /* init_edit */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_edit ()

{
  return (TRUE);
} /* term_edit */

