/*****************************************************************************/
/*                                                                           */
/* Modul: IMAGE.C                                                            */
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
#include "image.h"

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_CURSKEYS)
#define XFAC   16                       /* X-Faktor */
#define YFAC   1                        /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 8                        /* Y-Einheiten fÅr Scrolling */
#define INITX  (2 * XFAC)               /* X-Anfangsposition */
#define INITY  (3 * gl_hbox)            /* Y-Anfangsposition */
#define INITW  initw                    /* Anfangsbreite in Pixel */
#define INITH  inith                    /* Anfangshîhe in Pixel */
#define MILLI  0                        /* Millisekunden fÅr Zeitablauf */

#define MAX_PLANES      8
#define MAX_PATTERNS    8
#define MAX_LINEBUF  1024               /* Bildbreite maximal 8192 Pixel */
#define MIN_WIDTH    (8 * gl_wbox)      /* Kleinste Breite */
#define MIN_HEIGHT   (4 * gl_hbox)      /* Kleinste Hîhe */

/****** TYPES ****************************************************************/

typedef UBYTE HUGE *HUPTR;

typedef struct img_header
{
  WORD version;
  WORD headlen;
  WORD planes;
  WORD pat_run;
  WORD pix_width;
  WORD pix_height;
  WORD sl_width;
  WORD sl_height;
} IMG_HEADER;

typedef struct
{
  STRING filename;
  WORD   planes;
  MFDB   s;
  HUPTR  raster_buf;
  WORD   width, height;
} IMG_INF;

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

#if GEMDOS
EXTERN VOID   vdi             _((VOID));
#if TURBO_C
#define v_bit_image do_bit_image
LOCAL  VOID   do_bit_image    _((WORD handle, CONST BYTE *filename,
                                 WORD aspect, WORD x_scale, WORD y_scale,
                                 WORD h_align, WORD v_align, WORD *xy ));
#endif /* TURBO_C */
#endif /* GEMDOS */

LOCAL VOID    new_trnfm       _((WORD source_planes, MFDB *s, MFDB *d));
LOCAL VOID    flip_word       _((HUPTR adr));
LOCAL VOID    conv_bit_image  _((IMG_HEADER *img_header, HUPTR img_buffer,
                                 HUPTR raster_buf, HUPTR raster_ptr,
                                 HUPTR *plane_ptr, WORD max_lines,
                                 WORD screen_planes, WORD fww, IMG_INF *img_inf));
LOCAL BOOLEAN read_bit_image  _((IMG_INF *img_inf));

LOCAL VOID    update_menu     _((WINDOWP window));
LOCAL VOID    handle_menu     _((WINDOWP window, WORD title, WORD item));
LOCAL VOID    box             _((WINDOWP window, BOOLEAN grow));
LOCAL BOOLEAN wi_test         _((WINDOWP window, WORD action));
LOCAL VOID    wi_open         _((WINDOWP window));
LOCAL VOID    wi_close        _((WINDOWP window));
LOCAL VOID    wi_delete       _((WINDOWP window));
LOCAL VOID    wi_draw         _((WINDOWP window));
LOCAL VOID    wi_arrow        _((WINDOWP window, WORD dir, LONG oldpos, LONG newpos));
LOCAL VOID    wi_snap         _((WINDOWP window, RECT *new, WORD mode));
LOCAL VOID    wi_objop        _((WINDOWP window, SET objs, WORD action));
LOCAL WORD    wi_drag         _((WINDOWP src_window, WORD src_obj, WINDOWP dest_window, WORD dest_obj));
LOCAL VOID    wi_click        _((WINDOWP window, MKINFO *mk));
LOCAL VOID    wi_unclick      _((WINDOWP window));
LOCAL BOOLEAN wi_key          _((WINDOWP window, MKINFO *mk));
LOCAL VOID    wi_timer        _((WINDOWP window));
LOCAL VOID    wi_top          _((WINDOWP window));
LOCAL VOID    wi_untop        _((WINDOWP window));
LOCAL VOID    wi_edit         _((WINDOWP window, WORD action));

/*****************************************************************************/

#if GEMDOS /* TURBO-C auf ATARI ST kann v_bit_image nicht direkt aufrufen */
#if TURBO_C
LOCAL VOID do_bit_image (handle, filename, aspect, x_scale, y_scale, h_align, v_align, xy)
      WORD handle, aspect, x_scale, y_scale, h_align, v_align;
      WORD *xy;
CONST BYTE *filename;

{
  WORD i;

  for (i = 0; i < 4; i++) ptsin [i] = xy [i];

  intin [0] = aspect;
  intin [1] = x_scale;
  intin [2] = y_scale;
  intin [3] = h_align;
  intin [4] = v_align;

  i = 5;
  while ((intin [i++] = (WORD)(UBYTE)*filename++) != 0);

  contrl [0] = 5;
  contrl [1] = 2;
  contrl [3] = --i;
  contrl [5] = 23;
  contrl [6] = handle;

  vdi ();
} /* do_bit_image */
#endif /* TURBO_C */
#endif /* GEMDOS */

/*****************************************************************************/

GLOBAL VOID print_image (filename)
BYTE *filename;

{
  WORD       xy [4];
  WORD       out_handle;
  DEVINFO    dev_info;
  STRING     filespec;
  STRING     act_path, img_path;
  WORD       act_drive, img_drive;
  WORD       handle;
  LONG       size_header;
  IMG_HEADER *img_header;
  IMG_HEADER header;
  WORD       port;

  port = 0;
  if (! prn_check (port)) return;

  out_handle = open_work (PRINTER, &dev_info);

  if (out_handle > 0)
  {
    handle = Fopen (filename, 0);

#if MSDOS | FLEXOS
    if (DOS_ERR) handle = -32 - handle;
#endif

    if (handle < -3)                         /* Datei nicht gefunden */
    {
      hndl_alert (ERR_FOPEN);
      return;
    } /* if */
    else
    {
      size_header = sizeof (IMG_HEADER);
      Fread (handle, size_header, &header);
      img_header = &header;
      Fclose (handle);

#if I8086
      {
        UBYTE *img_buffer;
        WORD  i, headlen;

        img_buffer = (UBYTE *)&header;
        headlen    = img_header->headlen;
        flip_word ((HUPTR)&headlen);
        for (i = 0; i < headlen; i++) flip_word ((HUPTR)&img_buffer [i * 2]);
      } /* #if */
#endif

      busy_mouse ();
      set_meminfo ();
      get_path (act_path, 0);
      act_drive = Dgetdrv ();

      file_split (filename, &img_drive, img_path, filespec, NULL);
      Dsetdrv (img_drive);
      set_path (img_path);

      xy [0] = 0;
      xy [1] = 0;
      xy [2] = xy [0] + img_header->sl_width - 1;
      xy [3] = xy [1] + img_header->sl_height - 1;

      v_bit_image (out_handle, filespec, 0, 1, 0, 1, 1, xy);

      v_updwk (out_handle);
      close_work (PRINTER, out_handle);
      Dsetdrv (act_drive);
      set_path (act_path);
      set_meminfo ();
      arrow_mouse ();
    } /* else */
  } /* if */
} /* print_image */

/*****************************************************************************/

LOCAL VOID new_trnfm (source_planes, s, d)
WORD source_planes;
MFDB *s;
MFDB *d;

{
#if GEMDOS
  REG WORD *src_ptr;
  REG LONG size;
  REG LONG planewords;

  if ((source_planes == 1) || (s->np == 1))
    vr_trnfm (vdi_handle, s, d);
  else
  {
    planewords = s->fww * s->fh;
    size       = 2 * planewords * s->np;
    src_ptr    = (WORD *)mem_alloc (size);
    if (src_ptr == NULL) return;

    mem_lmove (src_ptr, s->mp, size);
    s->mp = (VOID *)src_ptr;
    vr_trnfm (vdi_handle, s, d);

    mem_free (src_ptr);
  } /* else */
#else
  vr_trnfm (vdi_handle, s, d);
#endif
} /* new_trnfm */

/*****************************************************************************/

LOCAL VOID flip_word (adr)
HUPTR adr;

{
  REG UBYTE c;

  c       = adr [0];
  adr [0] = adr [1];
  adr [1] = c;
} /* flip_word */

/*****************************************************************************/

LOCAL VOID conv_bit_image (img_header, img_buffer, raster_buf, raster_ptr,
                           plane_ptr, max_lines, screen_planes, fww, img_inf)
IMG_HEADER *img_header;
HUPTR      img_buffer;
HUPTR      raster_buf;
HUPTR      raster_ptr;
HUPTR      *plane_ptr;
WORD       max_lines;
WORD       screen_planes;
WORD       fww;
IMG_INF    *img_inf;

{
  HUPTR img_ptr;
  HUPTR line_ptr;
  UBYTE line_buf [MAX_LINEBUF];
  LONG  l_buflen;
  WORD  vrc;                            /* vertical replication count */
  WORD  bytecols;                       /* counter for planedata */
  UBYTE data;                           /* one byte of pixel data */
  UBYTE pattern [MAX_PATTERNS];
  WORD  max_pattern;
  UWORD length;
  WORD  idx, count;
  WORD  i, line;
  WORD  plane;
  WORD  fwb;
  MFDB  s, d;

  s.mp  = d.mp  = (VOID *)raster_buf;
  s.fwp = d.fwp = fww * 16;
  s.fh  = d.fh  = max_lines;
  s.fww = d.fww = fww;
  s.np  = d.np  = screen_planes;

#if MSDOS
  s.ff  = d.ff  = FALSE;
#else
  s.ff  = d.ff  = TRUE;
#endif

  fwb = fww * 2;

  l_buflen = (img_header->sl_width + 7) / 8;
  if (l_buflen > MAX_LINEBUF) return;

  line        = 0;
  max_pattern = min (img_header->pat_run, MAX_PATTERNS);
  img_ptr     = img_buffer;

  while (line < max_lines)
  {
    vrc = 1;

    for (plane = 0; plane < img_header->planes; plane++)
    {
      bytecols = l_buflen;
      line_ptr = line_buf;

      while (bytecols > 0)
      {
        data = *img_ptr++;

        switch (data)
        {
          case 0    : /* vertical replication count or pattern run */
                      data = *img_ptr++;

                      if (data == 0) /* vertical replication count */
                      {
                        if (*img_ptr++ == 0xFF) vrc = *img_ptr++;
                      } /* if */
                      else /* pattern run */
                      {
                        bytecols -= data * img_header->pat_run;
                        for (i = 0; i < max_pattern; i++)
                          pattern [i] = img_ptr [i];
                        img_ptr  += img_header->pat_run;

                        while (data > 0)
                        {
                          for (i = 0; i < max_pattern; i++)
                            *line_ptr++ = pattern [i];
                          data--;
                        } /* while */
                      } /* else */
                      break;
          case 0x80 : /* bit string */
                      data = *img_ptr++;
                      bytecols -= data;

                      while (data > 0)
                      {
                        *line_ptr++ = *img_ptr++;
                        data--;
                      } /* while */
                      break;
          default   : /* solid run */
                      length    = data & 0x7F;
                      bytecols -= length;
                      data      = (data & 0x80) ? 0xFF : 0;

                      while (length > 0)
                      {
                        *line_ptr++ = data;
                        length--;
                      } /* while */
                      break;
        } /* switch */
      } /* while */

#if I8086 /* flip words */
      for (i = 0; i < l_buflen / 2; i++) flip_word (&line_buf [i * 2]);
#endif

      idx        = plane % screen_planes;
      raster_ptr = plane_ptr [idx];

      for (count = 0; count < vrc; count++)
      {
        if (line + count < max_lines)
        {
          line_ptr = line_buf;
          for (i = 0; i < l_buflen; i++) *raster_ptr++ |= *line_ptr++;
          if ((l_buflen & 0x1) != 0) raster_ptr++;
        } /* if */
      } /* for */
    } /* for */

    for (i = 0; i < screen_planes; i++) plane_ptr [i] += vrc * fwb;
    line += vrc;
  } /* while */

  new_trnfm (img_header->planes, &d, &s);
  mem_move (&img_inf->s, &s, sizeof (s));
} /* conv_bit_image */

/*****************************************************************************/

LOCAL BOOLEAN read_bit_image (img_inf)
IMG_INF *img_inf;

{
  WORD       handle;
  WORD       i, screen_planes;
  WORD       max_lines;
  LONG       max_llines, planesize;
  LONG       max_buffer;
  LONG       size_header;
  IMG_HEADER *img_header;
  IMG_HEADER header;
  LONG       img_len;
  HUPTR      img_buffer;
  HUPTR      raster_buf;
  HUPTR      raster_ptr;
  HUPTR      plane_ptr [MAX_PLANES];
  LONG       rast_buflen;
  WORD       fww;
  WORD       work_out [57];

  handle = Fopen (img_inf->filename, 0);

#if MSDOS | FLEXOS
  if (DOS_ERR) handle = -32 - handle;
#endif

  if (handle < -3)                         /* Datei nicht gefunden */
  {
    hndl_alert (ERR_FOPEN);
    return (FALSE);
  } /* if */
  else
  {
    busy_mouse ();

    size_header = sizeof (IMG_HEADER);
    Fread (handle, size_header, &header);
    img_header = &header;
    img_buffer = (HUPTR)&header;

#if I8086
    {
      WORD headlen;

      headlen = img_header->headlen;
      flip_word ((UBYTE *)&headlen);
      for (i = 0; i < headlen; i++) flip_word (&img_buffer [i * 2]);
    } /* if */
#endif

    vq_extnd (vdi_handle, TRUE, work_out);
    screen_planes = work_out [4];
    if (img_header->planes == 1) screen_planes = 1; /* fÅr Optimierung */

    fww        = (img_header->sl_width + 15) / 16; /* fww = form width in words */
    max_lines  = img_header->sl_height;
    max_buffer = mem_avail ();

    max_llines  = max_buffer / (2L * (LONG)fww * screen_planes);
    max_lines   = min (max_llines, img_header->sl_height);
    planesize   = 2L * (LONG)fww * max_lines;
    rast_buflen = planesize * screen_planes;
    raster_buf  = (HUPTR)mem_alloc (rast_buflen);
    raster_ptr  = raster_buf;

    if (raster_buf == NULL)
    {
      arrow_mouse ();
      hndl_alert (ERR_NOMEMORY);
      return (FALSE);
    } /* if */

    for (i = 0; i < screen_planes; i++) plane_ptr [i] = raster_buf + i * planesize;
    mem_lset ((VOID *)raster_buf, 0, rast_buflen);

    size_header = img_header->headlen * 2; /* in bytes */
    img_len     = Fseek (0L, handle, 2) - size_header;
    img_buffer  = (HUPTR)mem_alloc (img_len);

    if (img_buffer == NULL)
    {
      arrow_mouse ();
      mem_free ((VOID *)raster_buf);
      hndl_alert (ERR_NOMEMORY);
      return (FALSE);
    }
    else
    {
      img_inf->planes     = img_header->planes;
      img_inf->raster_buf = raster_buf;

      Fseek (size_header, handle, 0);
      img_len = Fread (handle, img_len, (VOID FAR *)img_buffer); /* read pixel data */

      conv_bit_image (img_header, img_buffer, raster_buf, raster_ptr,
                      plane_ptr, max_lines, screen_planes, fww, img_inf);

      mem_free ((VOID *)img_buffer);
    } /* else */

    img_inf->width  = 16 * fww;
    img_inf->height = max_lines;

    Fclose (handle);
    arrow_mouse ();
    return (TRUE);
  } /* else */
} /* read_bit_image */

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
  IMG_INF *img_inf;

  img_inf = (IMG_INF *)window->special;
  mem_free ((VOID *)img_inf->raster_buf);
  mem_free (img_inf);
  set_meminfo ();
} /* wi_delete */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

LOCAL VOID wi_draw (window)
WINDOWP window;

{
  IMG_INF *img_inf;
  MFDB    d;
  RECT    r;
  WORD    pxy [8];
  WORD    index [2];
  WORD    width, height;

  img_inf = (IMG_INF *)window->special;

  pxy [0] = window->doc.x * window->xfac + clip.x - window->scroll.x;
  pxy [1] = window->doc.y * window->yfac + clip.y - window->scroll.y;

  width  = min (img_inf->s.fwp - pxy [0], clip.w);
  height = min (img_inf->s.fh - pxy [1], clip.h);

  pxy [2] = pxy [0] + width - 1;
  pxy [3] = pxy [1] + height - 1;
  pxy [4] = clip.x;
  pxy [5] = clip.y;
  pxy [6] = pxy [4] + width - 1;
  pxy [7] = pxy [5] + height - 1;

  index [0] = BLACK;
  index [1] = WHITE;

  d.mp = NULL; /* screen */

  if (window->scroll.w > img_inf->width)
  {
    r   = window->scroll;
    r.x = pxy [6];
    rc_intersect (&window->scroll, &r);
    rc_intersect (&clip, &r);
    clr_area (&r);
  } /* if */

  if (window->scroll.h > img_inf->height)
  {
    r   = window->scroll;
    r.y = pxy [7];
    rc_intersect (&window->scroll, &r);
    rc_intersect (&clip, &r);
    clr_area (&r);
  } /* if */

  if (img_inf->planes == 1)     /* Quellbild in monochrom zeichnen */
    vrt_cpyfm (vdi_handle, MD_REPLACE, pxy, &img_inf->s, &d, index);
  else                          /* Quellbild in Farbe zeichnen */
    vro_cpyfm (vdi_handle, S_ONLY, pxy, &img_inf->s, &d);
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

  diff.x = (new->x - r.x) / wbox * wbox;        /* Differenz berechnen */
  diff.w = (new->w - r.w) / wbox * wbox;
  diff.h = (new->h - r.h);

  new->x = r.x + diff.x;                        /* Schnelle Position */
  new->w = r.w + diff.w;                        /* Arbeitsbereich einrasten */

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
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

GLOBAL WINDOWP crt_image (obj, menu, icon, filename)
OBJECT *obj, *menu;
WORD   icon;
BYTE   *filename;

{
  WINDOWP  window;
  WORD     menu_height, inx;
  STR128   s;
  IMG_INF  *img_inf;
  WORD     initw, inith;

  img_inf = (IMG_INF *)mem_alloc ((LONG)sizeof (IMG_INF));
  if (img_inf == NULL)
  {
    hndl_alert (ERR_NOMEMORY);
    return (NULL); /* zuwenig Speicher */
  } /* if */

  mem_set (img_inf, 0, sizeof (IMG_INF));
  strcpy (img_inf->filename, filename);

  if (! read_bit_image (img_inf))
  {
    mem_free (img_inf);
    return (NULL);
  } /* if */

  initw = min (desk.w - 2 * gl_wattr, img_inf->width) / 16 * 16;
  inith = min (desk.h - 2 * gl_hattr, img_inf->height);
  initw = max (initw, MIN_WIDTH);
  inith = max (inith, MIN_HEIGHT);

  inx    = num_windows (CLASS_IMAGE, SRCH_ANY, NULL);
  window = create_window (KIND, CLASS_IMAGE);

  if (window != NULL)
  {
    menu_height = (menu != NULL) ? gl_hattr : 0;

    window->flags     = FLAGS;
    window->icon      = icon;
    window->doc.x     = 0;
    window->doc.y     = 0;
    window->doc.w     = img_inf->width / XFAC;
    window->doc.h     = img_inf->height / YFAC;
    window->xfac      = XFAC;
    window->yfac      = YFAC;
    window->xunits    = XUNITS;
    window->yunits    = YUNITS;
    window->scroll.x  = INITX + inx * XFAC;
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
    window->special   = (LONG)img_inf;
    window->object    = obj;
    window->menu      = menu;
    window->hndl_menu = handle_menu;
    window->updt_menu = update_menu;
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
    window->showinfo  = info_image;
    window->showhelp  = help_image;

    if ((strchr (filename, DRIVESEP) == NULL) && (strchr (filename, PATHSEP) == NULL))
      strcpy (s, act_path);
    else
      s [0] = EOS;

    strcat (s, filename);
    sprintf (window->name, " %s ", s);
  } /* if */
  else
  {
    mem_free ((VOID *)img_inf->raster_buf);
    mem_free (img_inf);
  } /* else */

  set_meminfo ();
  return (window);                      /* Fenster zurÅckgeben */
} /* crt_image */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN open_image (icon, filename)
WORD icon;
BYTE *filename;

{
  BOOLEAN ok;
  WINDOWP window;

  if ((icon != NIL) && (window = search_window (CLASS_IMAGE, SRCH_OPENED, icon)) != NULL)
  {
    ok = TRUE;
    top_window (window);
  } /* if */
  else
  {
    if ((window = search_window (CLASS_IMAGE, SRCH_CLOSED, icon)) == NULL)
      window = crt_image (NULL, NULL, icon, filename);

    ok = window != NULL;

    if (ok) ok = open_window (window);
  } /* else */

  return (ok);
} /* open_image */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

GLOBAL BOOLEAN info_image (window, icon)
WINDOWP window;
WORD    icon;

{
  LONGSTR s, d;
  WORD    colors;
  IMG_INF *img_inf;

  if (window != NULL)
  {
    img_inf = (IMG_INF *)window->special;
    colors  = 1L << img_inf->planes;
    strcpy (s, alerts [ERR_INFIMAGE]);
    sprintf (d, s, img_inf->width, img_inf->height, colors);
    open_alert (d);
  } /* if */

  return (window != NULL);
} /* info_image */

/*****************************************************************************/
/* Hilfe des Objekts                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN help_image (window, icon)
WINDOWP window;
WORD    icon;

{
  hndl_alert (ERR_HELPIMAG);
  return (TRUE);
} /* help_image */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_image ()

{
  return (TRUE);
} /* init_image */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_image ()

{
  return (TRUE);
} /* term_image */

