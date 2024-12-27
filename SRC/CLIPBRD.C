/*****************************************************************************/
/*                                                                           */
/* Modul: CLIPBRD.C                                                          */
/* Datum: 31/10/90                                                           */
/*                                                                           */
/*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"

#include "realtim4.h"
#include "errors.h"

#include "desktop.h"
#include "dialog.h"
#include "edit.h"
#include "image.h"
#include "meta.h"
#include "resource.h"
#include "trash.h"

#include "export.h"
#include "clipbrd.h"

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|INFO|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_RESIDENT|WI_CURSKEYS)
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 4 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 8 * gl_hbox)           /* Y-Anfangsposition */
#define MILLI  0                        /* Millisekunden fÅr Zeitablauf */

#define IXFAC  (72 + gl_wbox)           /* X-Faktor */
#define IYFAC  (40 + gl_hbox / 2)       /* Y-Faktor */
#define IINITW (desk.w - IXFAC)         /* Anfangsbreite in Pixel */
#define IINITH ( 2 * IYFAC)             /* Anfangshîhe in Pixel */
#define IWDOC  (desk.w / IXFAC - 1)     /* Maximale Breite der Ausgabe */

#define TXFAC  gl_wbox                  /* X-Faktor */
#define TYFAC  (gl_hbox + 2)            /* Y-Faktor */
#define TINITW (34 * TXFAC)             /* Anfangsbreite in Pixel */
#define TINITH ( 8 * TYFAC)             /* Anfangshîhe in Pixel */
#define TWDOC  44                       /* Maximale Breite der Ausgabe */

#define XOFFSET     (2 * gl_wbox)       /* X-Offset Work/Scrollbereich */
#define YOFFSET     2                   /* Y-Offset Work/Scrollbereich */

#define MAXTYPE     7                   /* Maximal erkannte Typen */
#define MAX_FILES   100                 /* Maximale Anzahl Dateien im Clipboard */
#define SCRAPSPEC   "SCRAP.*"           /* Scrap-Spezifikation */
#define SCRAP_DIF   0x0020              /* DIF-Dateien (nicht offiziell) */

#if GEMDOS
#define FIRST(path, spec)  (Fsfirst (path, spec) == 0)
#define NEXT               (Fsnext () == 0)
#else
#define FIRST(path, spec)  (Fsfirst (path, spec) > 0)
#define NEXT               (Fsnext () > 0)
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

typedef struct
{
  UWORD type;
  WORD  icon;
  BYTE  *suff;
} SCRAP_TYPE;

typedef struct
{
  unsigned second : 5;
  unsigned minute : 6;
  unsigned hour   : 5;
} TIME;

typedef struct
{
  unsigned day   : 5;
  unsigned month : 4;
  unsigned year  : 7;
} DATE;

typedef struct
{
  UWORD type;
  WORD  icon;
  TIME  time;
  DATE  date;
  ULONG filesize;
  BYTE  filename [14];
} SCRAP_FILE;

/****** VARIABLES ************************************************************/

LOCAL SCRAP_TYPE scrap_type [] =
{
  {SCRAP_CSV, ICSV, "CSV"},
  {SCRAP_TXT, ITXT, "TXT"},
  {SCRAP_GEM, IGEM, "GEM"},
  {SCRAP_IMG, IIMG, "IMG"},
  {SCRAP_DCA, IDCA, "DCA"},
  {SCRAP_DIF, IDIF, "DIF"},
  {SCRAP_USR, IUSR, "USR"}
}; /* scrap_type */

LOCAL SCRAP_FILE scrap_files [MAX_FILES];
LOCAL WORD       num_files;
LOCAL LONG       len_files;
LOCAL BOOLEAN    as_icons;
LOCAL WORD       show_which;

/****** FUNCTIONS ************************************************************/

LOCAL BYTE    *get_spec   _((WORD which, BYTE *s));
LOCAL WORD    compare     _((SCRAP_FILE *arg1, SCRAP_FILE *arg2));
LOCAL WORD    read_scrap  _((SCRAP_FILE *files, LONG *len));
LOCAL VOID    show_scrap  _((WINDOWP window));
LOCAL VOID    get_rect    _((WINDOWP window, WORD obj, RECT *rect));
LOCAL VOID    drag_objs   _((WINDOWP window, WORD obj, SET objs));
LOCAL VOID    fill_select _((WINDOWP window, SET objs, RECT *area));
LOCAL VOID    invert_objs _((WINDOWP window, SET objs));
LOCAL VOID    rubber_objs _((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN in_icon     _((WORD mox, WORD moy, ICONBLK *icon, RECT *r));

LOCAL VOID    mclipinfo   _((WINDOWP window));
LOCAL VOID    masicon     _((WINDOWP window));
LOCAL VOID    mastext     _((WINDOWP window));
LOCAL VOID    mshow       _((WINDOWP window, WORD item));
LOCAL VOID    mcut        _((WINDOWP window, SET objs, BOOLEAN ext));
LOCAL VOID    mcopy       _((WINDOWP window, SET objs, BOOLEAN ext));
LOCAL VOID    mclear      _((WINDOWP window, SET objs));
LOCAL VOID    mselall     _((WINDOWP window));

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
LOCAL VOID    wi_objop    _((WINDOWP window, SET objs, WORD ACTION));
LOCAL WORD    wi_drag     _((WINDOWP src_window, WORD src_obj, WINDOWP dest_window, WORD dest_obj));
LOCAL VOID    wi_click    _((WINDOWP window, MKINFO *mk));
LOCAL VOID    wi_unclick  _((WINDOWP window));
LOCAL BOOLEAN wi_key      _((WINDOWP window, MKINFO *mk));
LOCAL VOID    wi_timer    _((WINDOWP window));
LOCAL VOID    wi_top      _((WINDOWP window));
LOCAL VOID    wi_untop    _((WINDOWP window));
LOCAL VOID    wi_edit     _((WINDOWP window, WORD action));

/*****************************************************************************/
/* Angepaûte Scrap-Routinen                                                  */
/*****************************************************************************/

GLOBAL WORD scrap_read (pscrap)
BYTE *pscrap;

{
  WORD    result;
#if GEM & (GEM1 | GEM2)
  STR128  path;
  BYTE    *p;
  DTA     dta, *old_dta;
  BOOLEAN ok;
  WORD    i;

  scrp_read (pscrap);

  if (*pscrap)
  {
    result = 0;

    old_dta = (DTA *)Fgetdta ();
    Fsetdta (&dta);
    strcpy (path, pscrap);

    i = strlen (path);

    if (path [i - 1] != PATHSEP)
    {
      path [i]     = PATHSEP;
      path [i + 1] = EOS;
    } /* if */

    strcat (path, SCRAPSPEC);

    ok = FIRST (path, 0x00);

    while (ok)
    {
      p = strrchr (dta.d_fname, SUFFSEP);

      if (p != NULL)
        for (i = 0, p++; i < MAXTYPE; i++)
          if (strcmp (p, scrap_type [i].suff) == 0)
            result |= scrap_type [i].type;

      ok = NEXT;
    } /* while */

    Fsetdta (old_dta);
  } /* if */
  else
    result = -1;
#else
  result = scrp_read (pscrap);
#endif

  return (result);
} /* scrap_read */

/*****************************************************************************/

GLOBAL WORD scrap_write (pscrap)
BYTE *pscrap;

{
  return (scrp_write (pscrap));
} /* scrap_write */

/*****************************************************************************/

GLOBAL WORD scrap_clear ()

{
  WORD    result;
  STR128  scrap;
#if GEM & (GEM1 | GEM2)
  STR128  path, s;
  DTA     dta, *old_dta;
  BOOLEAN ok;
  WORD    i;
#endif

  result = 0;

  scrp_read (scrap);

  if (*scrap)
  {
#if GEM & (GEM1 | GEM2)
    result = 1;

    old_dta = (DTA *)Fgetdta ();
    Fsetdta (&dta);
    strcpy (path, scrap);

    i = strlen (path);

    if (path [i - 1] != PATHSEP)
    {
      path [i]     = PATHSEP;
      path [i + 1] = EOS;
    } /* if */

    strcat (path, SCRAPSPEC);

    ok = FIRST (path, 0x00);

    while (ok)
    {
      strcpy (s, scrap);
      strcat (s, dta.d_fname);

#if GEMDOS
      if (Fdelete (s) < 0) result = 0;
#else
      if (! Fdelete (s)) result = 0;
#endif

      ok = NEXT;
    } /* while */

    Fsetdta (old_dta);
#else
    result = scrp_clear ();
#endif
  } /* if */

  return (result);
} /* scrap_clear */

/*****************************************************************************/

GLOBAL VOID get_clipxywh (obj, border)
WORD obj;
RECT *border;

{
  WINDOWP window;

  xywh2rect (0, 0, 0, 0, border);

  window = search_window (CLASS_CLIPBRD, SRCH_ANY, NIL);

  if (window != NULL)
    if (window->opened > 0) get_rect (window, obj, border);
} /* get_clipxywh */

/*****************************************************************************/

GLOBAL VOID print_clipfiles (window, objs)
WINDOWP window;
SET     objs;

{
  WORD       i;
  STRING     filename;
  SCRAP_FILE *fp;

  if (window != NULL)
  {
    fp = (SCRAP_FILE *)window->special;

    for (i = 0; i < num_files; i++, fp++)
      if (setin (objs, i))
      {  
        strcpy (filename, scrapdir);
        strcat (filename, fp->filename);

        switch (fp->type)
        {
          case SCRAP_GEM : print_meta  (filename); break;
          case SCRAP_IMG : print_image (filename); break;
          default        : print_edit  (filename); break;
        } /* switch */
      } /* if, for */
  } /* if */
} /* print_clipfiles */

/*****************************************************************************/

LOCAL BYTE *get_spec (which, s)
WORD which;
BYTE *s;

{
  BYTE *p;

  p = (BYTE *)clipmenu [which].ob_spec;
  strcpy (s, p + 2);                            /* 2 fÅhrende Leerzeichen */

  p = strchr (s, SP);
  if (p != NULL) *p = EOS;                      /* Letzte Leerzeichen */

  return (s);
} /* get_spec */

/*****************************************************************************/

LOCAL WORD compare (arg1, arg2)
SCRAP_FILE *arg1, *arg2;

{
  STRING s1, s2;
  BYTE   *p1, *p2;

  p1 = strchr (arg1->filename, SUFFSEP);
  p2 = strchr (arg2->filename, SUFFSEP);

  sprintf (s1, "%-3s%s", (p1 != NULL) ? p1 + 1 : "", arg1->filename);
  sprintf (s2, "%-3s%s", (p2 != NULL) ? p2 + 1 : "", arg2->filename);

  p1 = strchr (s1, SUFFSEP);
  p2 = strchr (s2, SUFFSEP);

  if (p1 != NULL) *p1 = EOS;
  if (p2 != NULL) *p2 = EOS;

  return (strcmp (s1, s2));
} /* compare */

/*****************************************************************************/

LOCAL WORD read_scrap (files, len)
SCRAP_FILE *files;
LONG       *len;

{
  WORD       result, i;
  STR128     path;
  STRING     s;
  BYTE       *p;
  DTA        dta, *old_dta;
  BOOLEAN    ok;
  UWORD      date, time;
  SCRAP_FILE *fp;

  if (*scrapdir)
  {
    busy_mouse ();

    result = 0;
    *len   = 0;

    old_dta = (DTA *)Fgetdta ();
    Fsetdta (&dta);
    strcpy (path, scrapdir);
    strcat (path, get_spec (show_which, s));

    ok = FIRST (path, 0x00);
    fp = files;

    while (ok && (result < MAX_FILES))
    {
      *len             += dta.d_length;
      date              = dta.d_date;
      time              = dta.d_time;
      fp->type          = SCRAP_USR;
      fp->icon          = IUSR;
      fp->date.day      = date & 0x1F;
      date            >>= 5;
      fp->date.month    = date & 0x0F;
      date            >>= 4;
      fp->date.year     = date & 0x7F;
      fp->time.second   = time & 0x1F;
      time            >>= 5;
      fp->time.minute   = time & 0x3F;
      time            >>= 6;
      fp->time.hour     = time & 0x1F;
      fp->filesize      = dta.d_length;

      strcpy (fp->filename, dta.d_fname);

      p = strrchr (dta.d_fname, SUFFSEP);

      if (p != NULL)
        for (i = 0, p++; i < MAXTYPE; i++)
          if (strcmp (p, scrap_type [i].suff) == 0)
          {
            fp->type = scrap_type [i].type;
            fp->icon = scrap_type [i].icon;
          } /* if, for, if */

      fp++;
      result++;

      ok = NEXT;
    } /* while */

#if ANSI | MW_C
  qsort ((VOID *)files, (SIZE_T)result, sizeof (SCRAP_FILE), compare);
#endif

    Fsetdta (old_dta);
    arrow_mouse ();
  } /* if */

  return (result);
} /* read_scrap */

/*****************************************************************************/

LOCAL VOID show_scrap (window)
WINDOWP window;

{
  if (window != NULL)
  {
    if (window == sel_window) unclick_window (window);

    num_files     = read_scrap (scrap_files, &len_files);
    window->doc.x = 0;
    window->doc.y = 0;
    window->doc.w = (num_files == 0) ? 0 : as_icons ? ((num_files < IWDOC) ? num_files : IWDOC) : TWDOC;
    window->doc.h = (num_files == 0) ? 0 : as_icons ? (num_files + window->doc.w - 1) / window->doc.w : num_files;

    sprintf (window->info, (BYTE *)freetext [FCLIPINF].ob_spec, len_files, num_files); /* Infozeile */
    wind_set (window->handle, WF_INFO, ADR (window->info), 0, 0); /* Infozeile neu setzen */
    set_sliders (window, HORIZONTAL + VERTICAL, SLPOS + SLSIZE);
    redraw_window (window, &window->scroll);
  } /* if */
} /* show_scrap */

/*****************************************************************************/

LOCAL VOID get_rect (window, obj, rect)
WINDOWP window;
WORD    obj;
RECT    *rect;

{
  xywh2rect (0, 0, 0, 0, rect);

  if ((0 <= obj) && (obj < num_files))
    if (as_icons)
    {
      rect->x = window->scroll.x + (obj % window->doc.w - window->doc.x) * window->xfac;
      rect->y = window->scroll.y + (obj / window->doc.w - window->doc.y) * window->yfac;
      rect->w = icons [ICSV].ob_width;
      rect->h = icons [ICSV].ob_height;
    } /* if */
    else
    {
      rect->x = window->scroll.x - window->doc.x * window->xfac;
      rect->y = window->scroll.y + (obj - window->doc.y) * window->yfac;
      rect->w = window->doc.w * window->xfac;
      rect->h = window->yfac - 2;
    } /* if */
} /* get_rect */

/*****************************************************************************/

LOCAL VOID drag_objs (window, obj, objs)
WINDOWP window;
WORD    obj;
SET     objs;

{
  RECT    r, bound;
  WORD    mox, moy;
  WORD    i, result, num_objs;
  WORD    dest_obj;
  WINDOWP dest_window;
  SET     inv_objs;
  RECT    all [MAX_FILES];

  if (window != NULL)
  {
    rc_copy (&desk, &bound);

    setclr (inv_objs);
    for (i = ITRASH; i < FKEYS; i++) setincl (inv_objs, i);

    for (i = 0, num_objs = 0; i < num_files; i++)
      if (setin (objs, i)) get_rect (window, i, &all [num_objs++]);

    set_mouse (FLAT_HAND, NULL);
    drag_boxes (num_objs, all, find_desk (), inv_objs, &r, &bound, 1, 1);
    last_mouse ();
    graf_mkstate (&mox, &moy, &result, &result);

    result = drag_to_window (mox, moy, window, 0, &dest_window, &dest_obj);

    if (dest_window != NULL)
      if (dest_window->class == class_desk)
      {
        if (result == DRAG_OK)
          switch (dest_obj)
          {
            case ITRASH   : mclear (window, objs);          break;
            case IPRINTER : print_clipfiles (window, objs); break;
            case IDISK    : break;
            case ICLIPBRD : break;
          } /* switch, if */
      } /* if */
      else
        if (dest_window->class == CLASS_TRASH)
          if (result == DRAG_OK) mclear (window, objs);
  } /* if */
} /* drag_objs */

/*****************************************************************************/

LOCAL VOID fill_select (window, objs, area)
WINDOWP window;
SET     objs;
RECT    *area;

{
  REG WORD i;
      RECT r;

  setclr (objs);

  for (i = 0; i < num_files; i++)
  {
    get_rect (window, i, &r);

    if (rc_intersect (area, &r))                                 /* Im Rechteck */
      if (rc_intersect (&window->scroll, &r)) setincl (objs, i); /* Im Scrollbereich */
  } /* for */
} /* fill_select */

/*****************************************************************************/

LOCAL VOID invert_objs (window, objs)
WINDOWP window;
SET     objs;

{
  REG WORD       i, obj;
      RECT       r, inv;
      WORD       xy [4];
      SCRAP_FILE *fp;
      ICONBLK    *iconblk;

  fp = (SCRAP_FILE *)window->special;

  wind_update (BEG_UPDATE);
  hide_mouse ();

  wind_get (window->handle, WF_FIRSTXYWH, &r.x, &r.y, &r.w, &r.h);

  while ((r.w != 0) && (r.h != 0))
  {
    if (rc_intersect (&window->scroll, &r))
    {
      for (i = 0; i < num_files; i++)
        if (setin (objs, i))
        {
          get_rect (window, i, &inv);

          obj = fp [i].icon;

          icons [obj].ob_x = inv.x;     /* Bevor "inv" verÑndert wird */
          icons [obj].ob_y = inv.y;

          if (rc_intersect (&r, &inv))
            if (as_icons)
            {
              iconblk = (ICONBLK *)(icons [obj].ob_spec);
              strcpy (iconblk->ib_ptext, fp [i].filename);

              if ((window == sel_window) && (setin (sel_objs, i)))
                undo_state (icons, obj, SELECTED);      /* Normal-Darstellung */
              else
                do_state (icons, obj, SELECTED);        /* Invers-Darstellung */

              objc_draw (icons, obj, MAX_DEPTH, inv.x, inv.y, inv.w, inv.h);
            } /* if */
            else
            {
              set_clip (TRUE, &inv);            /* sichtbarer Bereich */
              rect2array (&inv, xy);
              vswr_mode (vdi_handle, MD_XOR);
              vsf_interior (vdi_handle, FIS_SOLID);
              vsf_color (vdi_handle, BLACK);
              vr_recfl (vdi_handle, xy);
            } /* else, if */
        } /* if, for */
    } /* if */

    wind_get (window->handle, WF_NEXTXYWH, &r.x, &r.y, &r.w, &r.h);
  } /* while */

  show_mouse ();
  wind_update (END_UPDATE);
} /* invert_objs */

/*****************************************************************************/

LOCAL VOID rubber_objs (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  RECT r;
  SET  new_objs;

  r.x = mk->mox;
  r.y = mk->moy;

  set_mouse (POINT_HAND, NULL);
  graf_rubbox (r.x, r.y, -r.x, -r.y, &r.w, &r.h);
  last_mouse ();

  if (r.w < 0)
  {
    r.x += r.w;
    r.w  = - r.w;
  } /* if */

  if (r.h < 0)
  {
    r.y += r.h;
    r.h  = - r.h;
  } /* if */

  if (mk->shift)                                /* Auschlieûlich odernd auswÑhlen */
  {
    fill_select (window, new_objs, &r);
    invert_objs (window, new_objs);
    setxor (sel_objs, new_objs);
  } /* if */
  else
    if (mk->ctrl)                               /* ZusÑtzlich auswÑhlen */
    {
      fill_select (window, new_objs, &r);
      setnot (sel_objs);
      setand (new_objs, sel_objs);
      setnot (sel_objs);
      invert_objs (window, new_objs);
      setor (sel_objs, new_objs);
    } /* if */
    else                                        /* AuswÑhlen */
    {
      fill_select (window, sel_objs, &r);
      invert_objs (window, sel_objs);
    } /* else */

  sel_window = setcmp (sel_objs, NULL) ? NULL : window;
} /* rubber_objs */

/*****************************************************************************/

LOCAL BOOLEAN in_icon (mox, moy, icon, r)
WORD    mox, moy;
ICONBLK *icon;
RECT    *r;

{
  BOOLEAN ok;
  RECT    r1;

  ok = FALSE;

  if (inside (mox, moy, r))         /* Im gesamten Rechteck */
  {
    rc_copy (r, &r1);
    r1.x += icon->ib_xicon;
    r1.y += icon->ib_yicon;
    r1.w  = icon->ib_wicon;
    r1.h  = icon->ib_ytext;         /* Bis zum Text, falls Icon kÅrzer */

    ok = inside (mox, moy, &r1);    /* Im Icon */

    if (! ok)                       /* Vielleicht im Text */
    {
      rc_copy (r, &r1);
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
/* MenÅbehandlung                                                            */
/*****************************************************************************/

LOCAL VOID mclipinfo (window)
WINDOWP window;

{
  LONGSTR s;

  if (window == sel_window)
    wi_objop (window, sel_objs, OBJ_INFO);
  else
  {
    sprintf (s, alerts [ERR_INFCLIP], len_files, num_files);
    open_alert (s);
  } /* else */
} /* mclipinfo */

/*****************************************************************************/

LOCAL VOID masicon (window)
WINDOWP window;

{
  as_icons      = TRUE;
  window->doc.x = 0;
  window->doc.y = 0;
  window->doc.w = (num_files == 0) ? 0 : (num_files < IWDOC) ? num_files : IWDOC;
  window->doc.h = (num_files == 0) ? 0 : (num_files + window->doc.w - 1) / window->doc.w;
  window->xfac  = IXFAC;
  window->yfac  = IYFAC;

  menu_text (window->menu, MCSHOWAS, (BYTE *)freetext [FASTEXT].ob_spec);
  set_sliders (window, HORIZONTAL + VERTICAL, SLPOS + SLSIZE);
  redraw_window (window, &window->scroll);
} /* masicon */

/*****************************************************************************/

LOCAL VOID mastext (window)
WINDOWP window;

{
  as_icons      = FALSE;
  window->doc.x = 0;
  window->doc.y = 0;
  window->doc.w = (num_files == 0) ? 0 : TWDOC;
  window->doc.h = num_files;
  window->xfac  = TXFAC;
  window->yfac  = TYFAC;

  menu_text (window->menu, MCSHOWAS, (BYTE *)freetext [FASICONS].ob_spec);
  set_sliders (window, HORIZONTAL + VERTICAL, SLPOS + SLSIZE);
  redraw_window (window, &window->scroll);
} /* mastext */

/*****************************************************************************/

LOCAL VOID mshow (window, item)
WINDOWP window;
WORD    item;

{
  STRING s;

  if (item != show_which)
  {
    menu_icheck (clipmenu, show_which, FALSE);
    show_which = item;
    menu_icheck (clipmenu, show_which, TRUE);
    sprintf (window->name, " %s%s ", scrapdir, get_spec (show_which, s));
    wind_set (window->handle, WF_NAME, ADR (window->name), 0, 0); /* Name neu setzen */
    show_scrap (window);
  } /* if */
} /* mshow */

/*****************************************************************************/

LOCAL VOID mcut (window, objs, ext)
WINDOWP window;
SET     objs;
BOOLEAN ext;

{
  WORD       i;
  STRING     oldname, newname;
  BYTE       suffix [4];
  BYTE       *p;
  LONGSTR    alert;
  BOOLEAN    exist, changed;
  SCRAP_FILE *fp;

  changed = FALSE;
  fp      = (SCRAP_FILE *)window->special;

  for (i = 0; i < num_files; i++, fp++)
    if (setin (objs, i))
    {
      p = strchr (fp->filename, SUFFSEP);

      if (p == NULL)
        suffix [0] = EOS;
      else
        strcpy (suffix, p + 1);

      strcpy (oldname, scrapdir);
      strcat (oldname, fp->filename);

      strcpy (newname, scrapdir);
      strcat (newname, SCRAPSPEC);
      strcpy (strrchr (newname, SUFFSEP) + 1, suffix);

      if (strcmp (oldname, newname) != 0)
      {
        sprintf (alert, alerts [ERR_ASKCUT], oldname, newname);

        exist = file_exist (newname);

        if (! exist || (open_alert (alert) == 1))
        {
          changed = TRUE;

          if (exist) Fdelete (newname);
          Frename (0, oldname, newname);
        } /* if */
      } /* if */
    } /* if, for */

  if (changed) show_scrap (window);
} /* mcut */

/*****************************************************************************/

LOCAL VOID mcopy (window, objs, ext)
WINDOWP window;
SET     objs;
BOOLEAN ext;

{
  WORD       i;
  STRING     oldname, newname;
  BYTE       suffix [4];
  BYTE       *p;
  LONGSTR    alert;
  BOOLEAN    exist, changed, ok;
  SCRAP_FILE *fp;
  WORD       src_handle, dst_handle;
  VOID       *buffer;
  LONG       mem, size;

  changed = FALSE;
  fp      = (SCRAP_FILE *)window->special;

  for (i = 0; i < num_files; i++, fp++)
    if (setin (objs, i))
    {
      p = strchr (fp->filename, SUFFSEP);

      if (p == NULL)
        suffix [0] = EOS;
      else
        strcpy (suffix, p + 1);

      strcpy (oldname, scrapdir);
      strcat (oldname, fp->filename);

      strcpy (newname, scrapdir);
      strcat (newname, SCRAPSPEC);
      strcpy (strrchr (newname, SUFFSEP) + 1, suffix);

      if (strcmp (oldname, newname) != 0)
      {
        sprintf (alert, alerts [ERR_ASKCOPY], oldname, newname);

        exist = file_exist (newname);

        if (! exist || (open_alert (alert) == 1))
        {
          mem = min (mem_avail () - 1024, fp->filesize);

          if (mem > 0)
          {
            busy_mouse ();

            buffer     = mem_alloc (mem);
            src_handle = Fopen (oldname, 0);

#if MSDOS | FLEXOS
            if (DOS_ERR) src_handle = -32 - src_handle;
#endif

            if (src_handle >= 0)
            {
              dst_handle = Fcreate (newname, 0);

#if MSDOS | FLEXOS
              if (DOS_ERR) dst_handle = -32 - dst_handle;
#endif

              if (dst_handle >= 0)
              {
                changed = TRUE;
                ok      = TRUE;

                while (ok && ((size = Fread (src_handle, mem, buffer)) > 0))
                  if (Fwrite (dst_handle, size, buffer) < size) ok = FALSE;

                Fclose (dst_handle);

                if (size < 0) ok = FALSE;
                if (! ok) Fdelete (newname);
              } /* if */

              Fclose (src_handle);
            } /* if */

            mem_free (buffer);
            arrow_mouse ();
          } /* if */
          else
            hndl_alert (ERR_NOMEMORY);
        } /* if */
      } /* if */
    } /* if, for */

  if (changed) show_scrap (window);
} /* mcopy */

/*****************************************************************************/

LOCAL VOID mclear (window, objs)
WINDOWP window;
SET     objs;

{
  WORD       i;
  STRING     filename;
  LONGSTR    alert;
  BOOLEAN    changed;
  SCRAP_FILE *fp;

  changed = FALSE;
  fp      = (SCRAP_FILE *)window->special;

  for (i = 0; i < num_files; i++, fp++)
    if (setin (objs, i))
    {
      strcpy (filename, scrapdir);
      strcat (filename, fp->filename);
      sprintf (alert, alerts [ERR_ASKCLEAR], filename);

      if (open_alert (alert) == 1)
      {
        changed = TRUE;

        Fdelete (filename);
      } /* if */
    } /* if, for */

  if (changed) show_scrap (window);
} /* mclear */

/*****************************************************************************/

LOCAL VOID mselall (window)
WINDOWP window;

{
  WORD i;

  if (window != sel_window) unclick_window (sel_window);
  sel_window = window;
  for (i = 0; i < num_files; i++) setincl (sel_objs, i);
  redraw_window (window, &window->scroll);
} /* mselall */

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

  switch (title)
  {
    case MCINFO   : switch (item)
                    {
                      case MCCLPINF : mclipinfo (window); break;
                    } /* switch */
                    break;
    case MCSHOW   : switch (item)
                    {
                      case MCSHOWAS : if (as_icons)
                                        mastext (window);
                                      else
                                        masicon (window);
                                      break;
                      default       : mshow (window, item); break;
                    } /* switch */
                    break;
  } /* switch */

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
    case DO_UNDO   : ret = FALSE;                                   break;
    case DO_CUT    : ret = ext && (window == sel_window);           break;
    case DO_COPY   : ret = ext && (window == sel_window);           break;
    case DO_PASTE  : ret = FALSE;                                   break;
    case DO_CLEAR  : ret = window == sel_window;                    break;
    case DO_SELALL : ret = (num_files > 0) && (window->opened > 0); break;
    case DO_CLOSE  :                                                break;
    case DO_DELETE :                                                break;
  } /* switch */

  return (ret);
} /* wi_test */

/*****************************************************************************/
/* ôffne Fenster                                                             */
/*****************************************************************************/

LOCAL VOID wi_open (window)
WINDOWP window;

{
  show_scrap (window);      /* Beim ôffnen Inhalt einlesen */
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
  WORD       x, y, w, h, i, obj;
  RECT       r, new;
  BYTE       *p;
  BYTE       suff [4];
  STRING     s, filename;
  SCRAP_FILE *fp;
  ICONBLK    *iconblk;

  clr_work (window);

  fp = (SCRAP_FILE *)window->special;

  rc_copy (&window->scroll, &r);

  if (as_icons)
  {
    for (y = window->doc.y; y < window->doc.h; y++)
      for (x = window->doc.x; x < window->doc.w; x++)
      {
        i = y * window->doc.w + x;

        if (i < num_files)
        {
          obj = fp [i].icon;

          icons [obj].ob_x = r.x + (x - window->doc.x) * window->xfac;
          icons [obj].ob_y = r.y + (y - window->doc.y) * window->yfac;

          iconblk = (ICONBLK *)(icons [obj].ob_spec);
          strcpy (iconblk->ib_ptext, fp [i].filename);

          if ((window == sel_window) && (setin (sel_objs, i)))
            do_state (icons, obj, SELECTED);            /* Invers-Darstellung */
          else
            undo_state (icons, obj, SELECTED);          /* Normal-Darstellung */

          objc_draw (icons, obj, MAX_DEPTH, clip.x, clip.y, clip.w, clip.h);
        } /* if */
      } /* for, for */
  } /* if */
  else
  {
    text_default (vdi_handle);

    w  = r.w / window->xfac;
    h  = (r.h + window->yfac - 1) / window->yfac;
    fp = (SCRAP_FILE *)window->special;

    if (window->scroll.x + window->scroll.w == clip.x + clip.w)
    {
      rc_copy (&clip, &new);
      new.w += window->xfac;
      set_clip (TRUE, &new);                            /* schnellere Textausgabe */
    } /* if */

    for (i = window->doc.y, fp += i; (i < window->doc.h) && (i < window->doc.y + h); i++, fp++, r.y += window->yfac)
      if (r.y + window->yfac > clip.y)                  /* Irgendwas im Schirm */
      {
        strcpy (filename, fp->filename);
        p = strrchr (filename, SUFFSEP);

        if (p != NULL)
        {
          *p = EOS;
          strcpy (suff, p + 1);
        } /* if */
        else
          suff [0] = EOS;

        sprintf (s, " %-8s %-3s %9ld  %02d-%02d-%02d  %02d:%02d:%02d ",
                 filename, suff, fp->filesize,
                 fp->date.day, fp->date.month, fp->date.year + 80,
                 fp->time.hour, fp->time.minute, fp->time.second * 2);

        s [window->doc.x + w] = EOS;

        if (strlen (s) > window->doc.x)
        {
          if ((window == sel_window) && (setin (sel_objs, i)))
            vswr_mode (vdi_handle, MD_ERASE);           /* Invers-Darstellung */
          else
            vswr_mode (vdi_handle, MD_REPLACE);         /* Normal-Darstellung */

          v_gtext (vdi_handle, r.x, r.y, s + window->doc.x); /* Textzeile ausgeben */
        } /* if */
      } /* if, for */
  } /* else */
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

  if (dir & HORIZONTAL)         /* Horizontale Pfeile und Schieber */
  {
    if (delta != 0)                             /* Scrolling nîtig */
    {
      window->doc.x = newpos;                   /* Neue Position */

      set_sliders (window, HORIZONTAL, SLPOS);  /* Schieber setzen */
      scroll_window (window, HORIZONTAL, delta * window->xfac);
    } /* if */
  } /* if */
  else                          /* Vertikale Pfeile und Schieber */
  {
    if (delta != 0)                             /* Scrolling nîtig */
    {
      window->doc.y = newpos;                   /* Neue Position */

      set_sliders (window, VERTICAL, SLPOS);    /* Schieber setzen */
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
  diff.x = (new->x - r.x) / 8 * 8;              /* Differenz berechnen */
  diff.y = (new->y - r.y) & 0xFFFE;
  diff.w = (new->w - r.w) / 8 * 8;
  diff.h = (new->h - r.h) / hbox * hbox;

  new->x = r.x + diff.x;                        /* Schnelle Position */
  new->y = r.y + diff.y;                        /* Y immer gerade */
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
/* Objektoperationen von Fenster                                             */
/*****************************************************************************/

LOCAL VOID wi_objop (window, objs, action)
WINDOWP window;
SET     objs;
WORD    action;

{
  WORD       i;
  BOOLEAN    ok;
  STR128     filename;
  LONGSTR    s;
  SCRAP_FILE *fp;

  fp = (SCRAP_FILE *)window->special;

  for (i = 0; i < num_files; i++, fp++)
    if (setin (objs, i))
      switch (action)
      {
        case OBJ_OPEN : strcpy (filename, scrapdir);
                        strcat (filename, fp->filename);

                        switch (fp->type)
                        {
                          case SCRAP_GEM : ok = open_meta  (i, filename); break;
                          case SCRAP_IMG : ok = open_image (i, filename); break;
                          default        : ok = open_edit  (i, filename); break;
                        } /* switch */

                        if (! ok) hndl_alert (ERR_NOOPEN);
                        break;
        case OBJ_INFO : sprintf (s, alerts [ERR_FILEINFO], fp->filename, fp->filesize,
                                 fp->date.day, fp->date.month, fp->date.year + 80,
                                 fp->time.hour, fp->time.minute, fp->time.second * 2);
                        open_alert (s);
                        break;
        case OBJ_HELP : switch (fp->type)
                        {
                          case SCRAP_GEM : ok = help_meta  (NULL, i); break;
                          case SCRAP_IMG : ok = help_image (NULL, i); break;
                          default        : ok = help_edit  (NULL, i); break;
                        } /* switch */

                        if (! ok) hndl_alert (ERR_NOHELP);
                        break;
      } /* switch, if, for */
  if ((window == sel_window) && (action == OBJ_OPEN)) unclick_window (window);
} /* wi_objop */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

LOCAL VOID wi_click (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  WORD       obj, i, maxlen;
  SET        new_objs;
  RECT       r;
  SCRAP_FILE *fp;
  STR128     parms;

  if (sel_window != window) unclick_window (sel_window); /* Deselektieren */

  obj = NIL;

  if (inside (mk->mox, mk->moy, &window->scroll))        /* Innerhalb Scrollbereich ? */
  {
    if (as_icons)
      obj = window->doc.x + (mk->mox - window->scroll.x) / window->xfac +
           (window->doc.y + (mk->moy - window->scroll.y) / window->yfac) * window->doc.w; 
    else
      obj = window->doc.y + (mk->moy - window->scroll.y) / window->yfac;

    get_rect (window, obj, &r);
    if (! inside (mk->mox, mk->moy, &r)) obj = NIL;

    if (as_icons && (obj != NIL))
      if (! in_icon (mk->mox, mk->moy, (ICONBLK *)icons [ICSV].ob_spec, &r)) obj = NIL;
  } /* if */

  if (obj != NIL)
  {
    setclr (new_objs);
    setincl (new_objs, obj);                    /* Aktuelles Objekt */

    if (mk->shift)
    {
      invert_objs (window, new_objs);
      setxor (sel_objs, new_objs);
      if (! setin (sel_objs, obj)) obj = NIL;   /* Wieder deselektiert */
    } /* if */
    else
    {
      if (! setin (sel_objs, obj))
      {
        unclick_window (window);                /* Alte Objekte lîschen */
        invert_objs (window, new_objs);
      } /* if */

      setor (sel_objs, new_objs);
    } /* else */

    sel_window = setcmp (sel_objs, NULL) ? NULL : window;

    if ((sel_window != NULL) && (obj != NIL))
    {
      if ((mk->breturn == 1) && (mk->mobutton & 1)) /* Zieh-Operation */
        drag_objs (window, obj, sel_objs);

      if (mk->breturn == 2)                 /* Doppelklick auf Icon */
        if (window->objop != NULL)
          (*window->objop) (sel_window, sel_objs, OBJ_OPEN);
    } /* if */
  } /* if */
  else
    if (inside (mk->mox, mk->moy, &window->work))       /* Innerhalb Workbereich ? */
    {
      if (! (mk->shift || mk->ctrl)) unclick_window (window); /* Deselektieren */
      if ((mk->breturn == 1) && (mk->mobutton & 1))     /* Gummiband-Operation */
        rubber_objs (window, mk);
    } /* if, else */

  parms [0] = EOS;
  fp        = (SCRAP_FILE *)window->special;
  maxlen    = HALFSTRLEN - strlen (scrapdir) - 1 - strlen (app_path) - strlen (app_name);

  for (i = 0; i < num_files; i++, fp++)                 /* Sammle Dateiname */
    if (setin (sel_objs, i))
      if (strlen (parms) + strlen (fp->filename) + 1 < maxlen)
      {
        strcat (parms, fp->filename);
        strcat (parms, ",");
      } /* if, if, for */

  if (*parms)                                           /* Eventuelle öbergabe an OUTPUT */
  {
    parms [strlen (parms) - 1] = EOS;
    strcpy (tail, scrapdir);
    strcat (tail, parms);
  } /* if */
} /* wi_click */

/*****************************************************************************/

LOCAL VOID wi_unclick (window)
WINDOWP window;

{
  if (! done) tail [0] = EOS;

  invert_objs (window, sel_objs);
} /* wi_unclick */

/*****************************************************************************/
/* Taste fÅr Fenster                                                         */
/*****************************************************************************/

LOCAL BOOLEAN wi_key (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  if (menu_key (window, mk)) return (TRUE);

  if (mk->ascii_code == ESC)
  {
    show_scrap (window);
    return (TRUE);
  } /* if */

  return (FALSE);
} /* wi_key */

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
    case DO_CUT    : mcut (window, sel_objs, ext);  break;
    case DO_COPY   : mcopy (window, sel_objs, ext); break;
    case DO_PASTE  : break;
    case DO_CLEAR  : mclear (window, sel_objs);     break;
    case DO_SELALL : mselall (window);              break;
  } /* switch */
} /* wi_edit */

/*****************************************************************************/
/* Iconbehandlung                                                            */
/*****************************************************************************/

GLOBAL BOOLEAN icons_clipbrd (src_obj, dest_obj)
WORD src_obj, dest_obj;

{
  BOOLEAN result;
  WINDOWP window;
  SET     all;

  result = FALSE;

  switch (src_obj)
  {
    case ICLIPBRD : window = search_window (CLASS_CLIPBRD, SRCH_ANY, src_obj);
                    switch (dest_obj)
                    {
                      case ITRASH   : scrap_clear ();
                                      show_scrap (window);
                                      break;
                      case IDISK    : /* Keine Aktion */
                                      break;
                      case IPRINTER : setall (all);
                                      print_clipfiles (window, all);
                                      break;
                      case ICLIPBRD : /* Keine Aktion */
                                      break;
                    } /* switch */

                    result = TRUE;
                    break;
  } /* switch */

  return (result);
} /* icons_clipbrd */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

GLOBAL WINDOWP crt_clipbrd (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
  WINDOWP window;
  WORD    menu_height, inx;
  STRING  s;

  inx    = num_windows (CLASS_CLIPBRD, SRCH_ANY, NULL);
  window = create_window (KIND, CLASS_CLIPBRD);

  if (window != NULL)
  {
    menu_height = (menu != NULL) ? gl_hattr : 0;
    num_files   = read_scrap (scrap_files, &len_files);

    window->flags     = FLAGS;
    window->icon      = icon;
    window->doc.x     = 0;
    window->doc.y     = 0;
    window->doc.w     = (num_files == 0) ? 0 : as_icons ? ((num_files < IWDOC) ? num_files : IWDOC) : TWDOC;
    window->doc.h     = (num_files == 0) ? 0 : as_icons ? (num_files + window->doc.w - 1) / window->doc.w : num_files;
    window->xfac      = as_icons ? IXFAC : TXFAC;
    window->yfac      = as_icons ? IYFAC : TYFAC;
    window->xunits    = XUNITS;
    window->yunits    = YUNITS;
    window->scroll.x  = INITX + inx * gl_wbox;
    window->scroll.y  = INITY + inx * gl_hbox + odd (menu_height);
    window->scroll.w  = as_icons ? IINITW : TINITW;
    window->scroll.h  = as_icons ? IINITH : IINITH;
    window->work.x    = window->scroll.x - XOFFSET;
    window->work.y    = window->scroll.y - YOFFSET - menu_height;
    window->work.w    = window->scroll.w + XOFFSET;
    window->work.h    = window->scroll.h + YOFFSET + menu_height;
    window->mousenum  = ARROW;
    window->mouseform = NULL;
    window->milli     = MILLI;
    window->special   = (LONG)scrap_files;
    window->object    = obj;
    window->menu      = menu;
    window->hndl_menu = handle_menu;
    window->updt_menu = update_menu;
    window->test      = wi_test;
    window->open      = wi_open;
    window->close     = wi_close;
    window->delete    = NULL;
    window->draw      = wi_draw;
    window->arrow     = wi_arrow;
    window->snap      = wi_snap;
    window->objop     = wi_objop;
    window->drag      = NULL;
    window->click     = wi_click;
    window->unclick   = wi_unclick;
    window->key       = wi_key;
    window->timer     = NULL;
    window->top       = NULL;
    window->untop     = NULL;
    window->edit      = wi_edit;
    window->showinfo  = info_clipbrd;
    window->showhelp  = help_clipbrd;

    sprintf (window->name, " %s%s ", scrapdir, get_spec (show_which, s));
    sprintf (window->info, (BYTE *)freetext [FCLIPINF].ob_spec, len_files, num_files);
  } /* if */

  return (window);                      /* Fenster zurÅckgeben */
} /* crt_clipbrd */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN open_clipbrd (icon)
WORD icon;

{
  BOOLEAN ok;
  WINDOWP window;

  if ((window = search_window (CLASS_CLIPBRD, SRCH_OPENED, icon)) != NULL)
  {
    ok = TRUE;
    top_window (window);
  } /* if */
  else
  {
    if ((window = search_window (CLASS_CLIPBRD, SRCH_CLOSED, icon)) == NULL)
      window = crt_clipbrd (NULL, clipmenu, icon);

    ok = window != NULL;

    if (ok) ok = open_window (window);
  } /* else */

  return (ok);
} /* open_clipbrd */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

GLOBAL BOOLEAN info_clipbrd (window, icon)
WINDOWP window;
WORD    icon;

{
  if (icon != NIL)
    window = search_window (CLASS_CLIPBRD, SRCH_ANY, icon);

  if (window != NULL) mclipinfo (window);
  return (window != NULL);
} /* info_clipbrd */

/*****************************************************************************/
/* Hilfe des Objekts                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN help_clipbrd (window, icon)
WINDOWP window;
WORD    icon;

{
  hndl_alert (ERR_HELPCLIP);
  return (TRUE);
} /* help_clipbrd */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_clipbrd ()

{
  WORD i;

  as_icons = strcmp ((BYTE *)clipmenu [MCSHOWAS].ob_spec, (BYTE *)freetext [FASTEXT].ob_spec) == 0;

  for (i = show_which = MSHOWSCR; i <= MSHOWDIF; i++)
    if (clipmenu [i].ob_state & CHECKED) show_which = i;

  return (TRUE);
} /* init_clipbrd */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_clipbrd ()

{
  return (TRUE);
} /* term_clipbrd */

