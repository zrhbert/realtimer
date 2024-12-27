/*****************************************************************************/
/*                                                                           */
/* Modul: GLOBAL.C                                                           */
/* Datum: 13.11.2024                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
27.11.2024
- dialog.h included for hndl_alert usage
13.11.2024
- logging for "big memory chunk removed"
09.01.95
- DEFAULTRATE auf 0 geÑndert, 09.01.95
- Mxalloc in mem_alloc fÅr TOS030 eingebaut, 19.12.94
05.11.94
- Backslash lîschen bei scrap_read rausgenommen
16.11.93
- app_path wird nun immer von act_path uebernommen
03.06.93
- is_menu_key                                                      
- form_alert Aufruf in note() mit (BYTE*)                                   
*****************************************************************************/

#include <ctype.h>

#include "import.h"

#if UNIX
#if PCC
#include <malloc.h>
#endif
#endif
/* #include "dialog.h" */

#include "export.h"
#include "errors.h"
#include "global.h"


/****** DEFINES **************************************************************/

#define MAX_COLORS  16                  /* GEM Standard-Farben */
#define DEFAULTRATE 0                   /* Default Blinkrate */

#define CTRL_CHAR   '^'                 /* MenÅ-Control-Buchstabe */
#define ALT_CHAR    0x07                /* MenÅ-Alternate-Buchstabe */
#define SHIFT_CHAR  0x01                /* MenÅ-Shifttaste */
#define FUNC_CHAR   'F'                 /* MenÅ-Funktionstaste */

#define FMD_FORWARD  0
#define FMD_BACKWARD 1
#define FMD_DEFLT    2

/****** TYPES ****************************************************************/

typedef struct
{
  UWORD red;
  UWORD green;
  UWORD blue;
} RGB_LIST;

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

/****** VARIABLES ************************************************************/

#if GEMDOS
#if DR_C | LASER_C | TURBO_C | MW_C
EXTERN WORD _app;                         /* Applikation oder Accessory */
#endif
#endif

LOCAL WORD     last_mousenumber = 0;
LOCAL MFORM    *last_mouseform  = NULL;

#if GEM & (GEM2 | GEM3 | XGEM)
LOCAL RGB_LIST gem_colors [MAX_COLORS] =
{
  1000, 1000, 1000,  /* white        */
     0,    0,    0,  /* black        */
  1000,    0,    0,  /* red          */
     0, 1000,    0,  /* green        */
     0,    0, 1000,  /* blue         */
     0, 1000, 1000,  /* cyan         */
  1000, 1000,    0,  /* yellow       */
  1000,    0, 1000,  /* magenta      */
   666,  666,  666,  /* light grey   */
   333,  333,  333,  /* dark grey    */
   666,    0,    0,  /* dark red     */
     0,  666,    0,  /* dark green   */
     0,    0,  666,  /* dark blue    */
     0,  666,  666,  /* dark cyan    */
   666,  666,    0,  /* dark yellow  */
   666,    0,  666   /* dark magenta */
};
#endif

#if GEMDOS

LOCAL UWORD ctrl_keys [] =
{
  0x3900, 0x0201, 0x0302, 0x2903, 0x0504, 0x0605, 0x0706, 0x0d07, 0x0908, 0x0a09,
  0x1b0a, 0x1b0b, 0x330c, 0x351f, 0x340e, 0x080f, 0x0b10, 0x0211, 0x0300, 0x0413,
  0x0514, 0x0615, 0x071e, 0x0817, 0x0918, 0x0a19, 0x341a, 0x331b, 0x601c, 0x0b1d,
  0x601e, 0x0c1f, 0x1a01, 0x1e01, 0x3002, 0x2e03, 0x2004, 0x1205, 0x2106, 0x2207,
  0x2308, 0x1709, 0x240a, 0x250b, 0x260c, 0x320d, 0x310e, 0x180f, 0x1910, 0x1011,
  0x1312, 0x1f13, 0x1414, 0x1615, 0x2f16, 0x1117, 0x2d18, 0x2c19, 0x151a, 0x2714,
  0x1a1a, 0x2804, 0x291e, 0x351f, 0x0d00, 0x1e01, 0x3002, 0x2e03, 0x2004, 0x1205,
  0x2106, 0x2207, 0x2308, 0x1709, 0x240a, 0x250b, 0x260c, 0x320d, 0x310e, 0x180f,
  0x1910, 0x1011, 0x1312, 0x1f13, 0x1414, 0x1615, 0x2f16, 0x1117, 0x2d18, 0x2c19,
  0x151a, 0x2719, 0x2b1c, 0x280e, 0x2b1e
}; /* ctrl_keys */

LOCAL UWORD alt_keys [] =
{
  0x3920, 0x7800, 0x7900, 0x2923, 0x7b00, 0x7c00, 0x7d00, 0x8300, 0x7f00, 0x8000,
  0x1b2a, 0x1b2b, 0x332c, 0x352d, 0x342e, 0x7e00, 0x8100, 0x7800, 0x7900, 0x7a00,
  0x7b00, 0x7c00, 0x7d00, 0x7e00, 0x7f00, 0x8000, 0x343a, 0x333b, 0x603c, 0x8100,
  0x603e, 0x8200, 0x1a40, 0x1e00, 0x3000, 0x2e00, 0x2000, 0x1200, 0x2100, 0x2200,
  0x2300, 0x1700, 0x2400, 0x2500, 0x2600, 0x3200, 0x3100, 0x1800, 0x1900, 0x1000,
  0x1300, 0x1f00, 0x1400, 0x1600, 0x2f00, 0x1100, 0x2d00, 0x2c00, 0x1500, 0x275b,
  0x1a5c, 0x285d, 0x295e, 0x355f, 0x8300, 0x1e00, 0x3000, 0x2e00, 0x2000, 0x1200,
  0x2100, 0x2200, 0x2300, 0x1700, 0x2400, 0x2500, 0x2600, 0x3200, 0x3100, 0x1800,
  0x1900, 0x1000, 0x1300, 0x1f00, 0x1400, 0x1600, 0x2f00, 0x1100, 0x2d00, 0x2c00,
  0x1500, 0x277b, 0x2b7c, 0x287d, 0x2b7e
}; /* alt_keys */

LOCAL UWORD func_keys [] =
{
  0x3b00, 0x3c00, 0x3d00, 0x3e00, 0x3f00, 0x4000, 0x4100, 0x4200, 0x4300, 0x4400,
  0x5400, 0x5500, 0x5600, 0x5700, 0x5800, 0x5900, 0x5A00, 0x5B00, 0x5C00, 0x5D00,
  0x3b00, 0x3c00, 0x3d00, 0x3e00, 0x3f00, 0x4000, 0x4100, 0x4200, 0x4300, 0x4400,
  0x3b00, 0x3c00, 0x3d00, 0x3e00, 0x3f00, 0x4000, 0x4100, 0x4200, 0x4300, 0x4400
}; /* func_keys */

#endif

#if MSDOS | FLEXOS

LOCAL UWORD ctrl_keys [] =
{
  0x3920, 0x0000, 0x0300, 0x0000, 0x0000, 0x0000, 0x071e, 0x0000, 0x0000, 0x0000,
  0x1b1d, 0x1b1d, 0x0000, 0x351f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0300, 0x0000,
  0x0000, 0x0000, 0x071e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0c1c, 0x1011, 0x1e01, 0x3002, 0x2e03, 0x2004, 0x1205, 0x2106, 0x2207,
  0x2308, 0x1709, 0x240a, 0x250b, 0x260c, 0x320d, 0x310e, 0x180f, 0x1910, 0x1011,
  0x1312, 0x1f13, 0x1414, 0x1615, 0x2f16, 0x1117, 0x2d18, 0x2c19, 0x151a, 0x0000,
  0x0c1f, 0x0000, 0x0000, 0x351f, 0x0000, 0x1e01, 0x3002, 0x2e03, 0x2004, 0x1205,
  0x2106, 0x2207, 0x2308, 0x1709, 0x240a, 0x250b, 0x260c, 0x320d, 0x310e, 0x180f,
  0x1910, 0x1011, 0x1312, 0x1f13, 0x1414, 0x1615, 0x2f16, 0x1117, 0x2d18, 0x2c19,
  0x151a, 0x0000, 0x0000, 0x0000, 0x1b1d
}; /* ctrl_keys */

LOCAL UWORD alt_keys [] =
{
  0x3920, 0x7800, 0x7900, 0x0000, 0x7b00, 0x7c00, 0x7d00, 0x0000, 0x7f00, 0x8000,
  0x0000, 0x0000, 0x0000, 0x8200, 0x0000, 0x7e00, 0x8100, 0x7800, 0x7900, 0x7a00,
  0x7b00, 0x7c00, 0x7d00, 0x7e00, 0x7f00, 0x8000, 0x0000, 0x0000, 0x0000, 0x8100,
  0x0000, 0x0000, 0x1040, 0x1e00, 0x3000, 0x2e00, 0x2000, 0x1200, 0x2100, 0x2200,
  0x2300, 0x1700, 0x2400, 0x2500, 0x2600, 0x3200, 0x3100, 0x1800, 0x1900, 0x1000,
  0x1300, 0x1f00, 0x1400, 0x1600, 0x2f00, 0x1100, 0x2d00, 0x1500, 0x2c00, 0x095b,
  0x0c5c, 0x0a5d, 0x0000, 0x8200, 0x0000, 0x1e00, 0x3000, 0x2e00, 0x2000, 0x1200,
  0x2100, 0x2200, 0x2300, 0x1700, 0x2400, 0x2500, 0x2600, 0x3200, 0x3100, 0x1800,
  0x1900, 0x1000, 0x1300, 0x1f00, 0x1400, 0x1600, 0x2f00, 0x1100, 0x2d00, 0x1500,
  0x2c00, 0x087b, 0x567c, 0x0b7d, 0x1b7e
}; /* alt_keys */

LOCAL UWORD func_keys [] =
{
  0x3b00, 0x3c00, 0x3d00, 0x3e00, 0x3f00, 0x4000, 0x4100, 0x4200, 0x4300, 0x4400,
  0x5400, 0x5500, 0x5600, 0x5700, 0x5800, 0x5900, 0x5A00, 0x5B00, 0x5C00, 0x5D00,
  0x5e00, 0x5f00, 0x6000, 0x6100, 0x6200, 0x6300, 0x6400, 0x6500, 0x6600, 0x6700,
  0x6800, 0x6900, 0x6a00, 0x6b00, 0x6c00, 0x6d00, 0x6e00, 0x6f00, 0x7000, 0x7100
}; /* func_keys */

#endif

LOCAL  WORD tos = 0;	/* EnthÑlt TOS-Version nach init_global */
/****** FUNCTIONS ************************************************************/

LOCAL VOID vdi_fix   _((MFDB *pfd, VOID *theaddr, WORD wb, WORD h));
LOCAL VOID vdi_trans _((WORD *saddr, WORD swb, WORD *daddr, WORD dwb, WORD h));

LOCAL WORD find_obj  _((OBJECT *tree, WORD start_obj, WORD which));
LOCAL WORD fm_inifld _((OBJECT *tree, WORD start_fld));

#if GEM & (GEM2 | GEM3 | XGEM)

#define graf_growbox grafgrowbox        /* for compatibility with header file */

LOCAL VOID graf_growbox   _((WORD orgx, WORD orgy, WORD orgw, WORD orgh, WORD x, WORD y, WORD w, WORD h));
LOCAL VOID graf_shrinkbox _((WORD orgx, WORD orgy, WORD orgw, WORD orgh, WORD x, WORD y, WORD w, WORD h));
#endif

#if GEMDOS
LOCAL LONG *get_actpd _((VOID));
#endif /* GEMDOS */

/*****************************************************************************/
/* ôffne virtuelle Workstation                                               */
/*****************************************************************************/

GLOBAL VOID open_vwork ()

{
  WORD i;
  WORD work_in [11];
  WORD work_out [57];

  for (i = 0; i < 10; work_in [i++] = 1);
  work_in [10] = RC;                           /* Raster Koordinaten */
  vdi_handle = phys_handle;
  v_opnvwk (work_in, &vdi_handle, work_out);   /* ôffne virtuelle Workstation */
  colors = work_out [13];                      /* Anzahl der Farben */

  if (vdi_handle == 0) vdi_handle = phys_handle;

#if GEM & XGEM
  vst_point (vdi_handle, gl_point, &i, &i, &i, &i);
#endif

  vqt_attributes (vdi_handle, work_out);       /* Globale Zeichensatzgrîûen */

  gl_wchar = work_out [6];                     /* Werte von Zeichen holen */
  gl_hchar = work_out [7];
} /* open_vwork */

/*****************************************************************************/
/* Schlieûe virtuelle Workstation                                            */
/*****************************************************************************/

GLOBAL VOID close_vwork ()

{
  if (vdi_handle != phys_handle)                /* Virtuelle Workstation ist offen */
  {
    v_clsvwk (vdi_handle);                      /* Workstation freigeben */
    vdi_handle = phys_handle;                   /* Physikalischen Bildschirm benutzen */
  } /* if */
} /* close_vwork */

/*****************************************************************************/
/* ôffne Workstation                                                         */
/*****************************************************************************/

GLOBAL WORD open_work (device, dev_info)
WORD    device;
DEVINFO *dev_info;

{
  WORD i;
  WORD handle;
  WORD work_in [103];
  WORD work_out [57];

  if (! gdos_ok ()) return (0);

  for (i = 0; i < 103; i++) work_in [i] = 1;

  work_in [0]  = device;                   /* Device handle */
  work_in [10] = RC;                       /* Raster Koordinaten */

  if (device == SCREEN)
  {
    handle = phys_handle;
    v_opnvwk (work_in, &handle, work_out); /* Virtuell îffnen */
  } /* if */
  else                                     /* Nicht Bildschirm */
  {
    work_in [11] = OW_NOCHANGE;            /* Paralleler oder serieller port */
    v_opnwk (work_in, &handle, work_out);  /* Physikalisch îffnen */
  } /* else */

  dev_info->dev_w = work_out [0] + 1L;
  dev_info->dev_h = work_out [1] + 1L;
  dev_info->pix_w = work_out [3];
  dev_info->pix_h = work_out [4];

  return (handle);
} /* open_work */

/*****************************************************************************/
/* Schlieûe Workstation                                                      */
/*****************************************************************************/

GLOBAL VOID close_work (device, out_handle)
WORD device, out_handle;

{
  if (device >= PRINTER)
    v_clswk (out_handle);
  else
    v_clsvwk (out_handle);
} /* close_work */

/*****************************************************************************/

GLOBAL BOOLEAN gdos_ok ()

{
#if GEMDOS
#if LASER_C | MW_C | TURBO_C
  return (vq_gdos () != 0);
#else
  return (TRUE);
#endif /* TURBO_C */
#else
  return (TRUE);
#endif /* GEMDOS */
} /* gdos_ok */

/*****************************************************************************/
/* Maus-Routinen                                                             */
/*****************************************************************************/

GLOBAL VOID set_mouse (number, addr)
WORD  number;
MFORM *addr;

{
  last_mousenumber = mousenumber;
  last_mouseform   = mouseform;
  mousenumber      = number;
  mouseform        = addr;
  graf_mouse (number, addr);
} /* set_mouse */

/*****************************************************************************/

GLOBAL VOID last_mouse ()

{
  set_mouse (last_mousenumber, last_mouseform);
} /* last_mouse */

/*****************************************************************************/

GLOBAL VOID hide_mouse ()

{
  if (hidden == 0) graf_mouse (M_OFF, NULL);
  hidden++;
} /* hide_mouse */

/*****************************************************************************/

GLOBAL VOID show_mouse ()

{
  hidden--;
  if (hidden == 0) graf_mouse (M_ON, NULL);
} /* show_mouse */

/*****************************************************************************/

GLOBAL VOID busy_mouse ()

{
  if (busy == 0) set_mouse (HOURGLASS, NULL);
  busy++;
} /* busy_mouse */

/*****************************************************************************/

GLOBAL VOID arrow_mouse ()

{
  busy--;
  if (busy == 0) set_mouse (ARROW, NULL);
} /* arrow_mouse */

/*****************************************************************************/
/* Objekt-Routinen                                                           */
/*****************************************************************************/

GLOBAL VOID do_state (tree, obj, state)
OBJECT *tree;
WORD   obj;
UWORD  state;

{
  tree [obj].ob_state |= state;         /* Status im Objekt setzen */
} /* do_state */

/*****************************************************************************/

GLOBAL VOID undo_state (tree, obj, state)
OBJECT *tree;
WORD   obj;
UWORD  state;

{
  tree [obj].ob_state &= ~ state;       /* Status im Objekt lîschen */
} /* undo_state */

/*****************************************************************************/

GLOBAL VOID flip_state (tree, obj, state)
OBJECT *tree;
WORD   obj;
UWORD  state;

{
  tree [obj].ob_state ^= state;         /* Status im Objekt invertieren */
} /* flip_state */

/*****************************************************************************/

GLOBAL WORD find_state (tree, obj, state)
OBJECT *tree;
WORD   obj;
UWORD  state;

{
  do
  {
    if (is_state (tree, obj, state)) return (obj);
  } while (! is_flags (tree, obj++, LASTOB));

  return (NIL);
} /* find_state */

/*****************************************************************************/

GLOBAL BOOLEAN is_state (tree, obj, state)
OBJECT *tree;
WORD   obj;
UWORD  state;

{
  return ((tree [obj].ob_state & state) != 0);
} /* is_state */

/*****************************************************************************/

GLOBAL VOID do_flags (tree, obj, flags)
OBJECT *tree;
WORD   obj;
UWORD  flags;

{
  tree [obj].ob_flags |= flags;         /* Flags im Objekt setzen */
} /* do_flags */

/*****************************************************************************/

GLOBAL VOID undo_flags (tree, obj, flags)
OBJECT *tree;
WORD   obj;
UWORD  flags;

{
  tree [obj].ob_flags &= ~ flags;       /* Flags im Objekt lîschen */
} /* undo_flags */

/*****************************************************************************/

GLOBAL VOID flip_flags (tree, obj, flags)
OBJECT *tree;
WORD   obj;
UWORD  flags;

{
  tree [obj].ob_flags ^= flags;         /* Flags im Objekt invertieren */
} /* flip_flags */

/*****************************************************************************/

GLOBAL WORD find_flags (tree, obj, flags)
OBJECT *tree;
WORD   obj;
UWORD  flags;

{
  do
  {
    if (is_flags (tree, obj, flags)) return (obj);
  } while (! is_flags (tree, obj++, LASTOB));

  return (NIL);
} /* find_flags */

/*****************************************************************************/

GLOBAL BOOLEAN is_flags (tree, obj, flags)
OBJECT *tree;
WORD   obj;
UWORD  flags;

{
  return ((tree [obj].ob_flags & flags) != 0);
} /* is_flags */

/*****************************************************************************/

GLOBAL WORD find_type (tree, obj, type)
OBJECT *tree;
WORD   obj;
UWORD  type;

{
  do
  {
    if (OB_TYPE (tree, obj) == type) return (obj);
  } while (! is_flags (tree, obj++, LASTOB));

  return (NIL);
} /* find_type */

/*****************************************************************************/

GLOBAL VOID set_checkbox (tree, obj, selected)
OBJECT  *tree;
WORD    obj;
BOOLEAN selected;

{
  if (selected)
    do_state (tree, obj, SELECTED);
  else
    undo_state (tree, obj, SELECTED);
} /* set_checkbox */

/*****************************************************************************/

GLOBAL BOOLEAN get_checkbox (tree, obj)
OBJECT *tree;
WORD   obj;

{
  return (is_state (tree, obj, SELECTED));
} /* get_checkbox */

/*****************************************************************************/

GLOBAL VOID set_rbutton (tree, obj, lower, upper)
OBJECT *tree;
WORD   obj, lower, upper;

{
  REG WORD i;

  for (i = lower; i <= upper; i++)
    if (is_flags (tree, i, RBUTTON))
    {
      undo_state (tree, i, SELECTED);
      if (obj == i) do_state (tree, i, SELECTED);
    } /* if, for */
} /* set_rbutton */

/*****************************************************************************/

GLOBAL WORD get_rbutton (tree, obj)
OBJECT *tree;
WORD   obj;

{
  return (find_state (tree, obj, SELECTED));
} /* get_rbutton */

/*****************************************************************************/

GLOBAL VOID set_ptext (tree, obj, s)
OBJECT *tree;
WORD   obj;
BYTE   *s;

{
  TEDINFO *ptedinfo;

  ptedinfo = (TEDINFO *)tree [obj].ob_spec;
  strncpy (ptedinfo->te_ptext, s, ptedinfo->te_txtlen - 1);
  ptedinfo->te_ptext [ptedinfo->te_txtlen - 1] = EOS;
} /* set_ptext */

/*****************************************************************************/

GLOBAL VOID get_ptext (tree, obj, s)
OBJECT *tree;
WORD   obj;
BYTE   *s;

{
  TEDINFO *ptedinfo;

  ptedinfo = (TEDINFO *)tree [obj].ob_spec;
  strcpy (s, ptedinfo->te_ptext);
} /* get_ptext */

/*****************************************************************************/

GLOBAL VOID menu_check (tree, obj, checkit)
OBJECT  *tree;
WORD    obj;
BOOLEAN checkit;

{
#if GEM & XGEM
  menu_icheck (tree, obj, checkit);
#else
  if (checkit)
    do_state (tree, obj, CHECKED);
  else
    undo_state (tree, obj, CHECKED);
#endif
} /* menu_check */

/*****************************************************************************/

GLOBAL VOID menu_enable (tree, obj, enableit)
OBJECT  *tree;
WORD    obj;
BOOLEAN enableit;

{
#if GEM & XGEM
  menu_ienable (tree, obj, checkit);
#else
  if (enableit)
    undo_state (tree, obj, DISABLED);
  else
    do_state (tree, obj, DISABLED);
#endif
} /* menu_enable */

/*****************************************************************************/

GLOBAL VOID objc_rect (tree, obj, rect, calc_border)
OBJECT  *tree;
WORD     obj;
RECT    *rect;
BOOLEAN calc_border;

{
  WORD border, diff;

  objc_offset (tree, obj, &rect->x, &rect->y);
  rect->w = tree [obj].ob_width;
  rect->h = tree [obj].ob_height;

  if (calc_border &&
      ((OB_TYPE (tree, obj) == G_BOX) ||
       (OB_TYPE (tree, obj) == G_IBOX) ||
       (OB_TYPE (tree, obj) == G_BOXCHAR)))
  {
    border = (WORD)(((LONG)tree [obj].ob_spec >> 16) & 0x00FFL);

    if (border & 0x0080) border |= 0xFF00;      /* Rand negativ */

    if (border < 0)
    {
      rect->x += border;                        /* Wegen Rand */
      rect->y += border;
      rect->w -= 2 * border;
      rect->h -= 2 * border;
    } /* if */

    if (is_state (tree, obj, SHADOWED))         /* Schatten berÅcksichtigen */
    {
      rect->w += 2 * abs (border);
      rect->h += 2 * abs (border);
    } /* if */

    if (is_state (tree, obj, OUTLINED))         /* Outlined berÅcksichtigen */
    {
      if (border >= 0)
        diff = 3;
      else
        if (border > -3)
          diff = 3 + border;
        else
          diff = 0;

      rect->x -= diff;                          /* Wegen Rand */
      rect->y -= diff;        
      rect->w += 2 * diff;
      rect->h += 2 * diff;
    } /* if */
  } /* if */
} /* objc_rect */

/*****************************************************************************/

LOCAL VOID vdi_fix (pfd, theaddr, wb, h)
MFDB *pfd;
VOID *theaddr;
WORD wb, h;

{
  pfd->mp  = theaddr;
  pfd->fwp = wb << 3;
  pfd->fh  = h;
  pfd->fww = wb >> 1;
  pfd->np  = 1;
} /* vdi_fix */

/*****************************************************************************/

LOCAL VOID vdi_trans (saddr, swb, daddr, dwb, h)
WORD *saddr;
WORD swb;
WORD *daddr;
WORD dwb;
WORD h;

{
  MFDB src, dst;

  vdi_fix (&src, saddr, swb, h);
  src.ff = TRUE;

  vdi_fix (&dst, daddr, dwb, h);
  dst.ff = FALSE;

  vr_trnfm (vdi_handle, &src, &dst);
} /* vdi_trans */

/*****************************************************************************/

GLOBAL VOID trans_gimage (tree, obj)
OBJECT *tree;
WORD   obj;

{
  ICONBLK *piconblk;
  BITBLK  *pbitblk;
  WORD    *taddr;
  WORD    wb, hl, type;

  type = tree [obj].ob_type;

  if (type == G_ICON)
  {
    piconblk = (ICONBLK *)tree [obj].ob_spec;
    taddr    = piconblk->ib_pmask;
    wb       = piconblk->ib_wicon;
    wb       = wb >> 3;
    hl       = piconblk->ib_hicon;

    vdi_trans (taddr, wb, taddr, wb, hl);

    taddr = piconblk->ib_pdata;
  } /* if */
  else
  {
    pbitblk = (BITBLK *)tree [obj].ob_spec;
    taddr   = pbitblk->bi_pdata;
    wb      = pbitblk->bi_wb;
    hl      = pbitblk->bi_hl;
  } /* else */

  vdi_trans (taddr, wb, taddr, wb, hl);
} /* trans_gimage */

/*****************************************************************************/
/* Default-Attribute fÅr Linie setzen                                        */
/*****************************************************************************/

GLOBAL VOID line_default (vdi_handle)
WORD vdi_handle;

{
  vswr_mode (vdi_handle, MD_REPLACE);
  vsl_color (vdi_handle, BLACK);
  vsl_ends (vdi_handle, SQUARED, SQUARED);
  vsl_type (vdi_handle, SOLID);
  vsl_width (vdi_handle, 1);
} /* line_default */

/*****************************************************************************/
/* Default-Attribute fÅr Text setzen                                         */
/*****************************************************************************/

GLOBAL VOID text_default (vdi_handle)
WORD vdi_handle;

{
  WORD ret;

  vswr_mode (vdi_handle, MD_REPLACE);
  vst_font (vdi_handle, FONT_SYSTEM);
  vst_color (vdi_handle, BLACK);
  vst_height (vdi_handle, gl_hchar, &ret, &ret, &ret, &ret);
  vst_effects (vdi_handle, TXT_NORMAL);
  vst_alignment (vdi_handle, ALI_LEFT, ALI_TOP, &ret, &ret);
  vst_rotation (vdi_handle, 0);
} /* text_default */

/*****************************************************************************/
/* Text ausgeben (> 127 Zeichen)                                             */
/*****************************************************************************/

GLOBAL VOID v_text (vdi_handle, x, y, string, charwidth)
WORD vdi_handle, x, y;
BYTE *string;
WORD charwidth;

{
  WORD len, minlen;
  BYTE s [128];
  BYTE *p;

  for (len = strlen (string), p = string; len > 0; len -= 127, p += 127, x += 127 * charwidth)
  {
    minlen = min (127, len);
    strncpy (s, p, minlen);
    s [minlen] = EOS;
    v_gtext (vdi_handle, x, y, s);
  } /* for */
} /* v_text */

/*****************************************************************************/
/* Dialog-Verarbeitung                                                       */
/*****************************************************************************/

GLOBAL BOOLEAN background (tree, obj, get, screen, buffer)
OBJECT  *tree;
WORD    obj;
BOOLEAN get;
MFDB    *screen, *buffer;

{
  RECT  box;
  LONG  size;
  WORD  diff;
  WORD  xy [8];
  WORD  work_out [57];

  objc_rect (tree, obj, &box, TRUE);

  if (rc_intersect (&desk, &box))               /* Nur auf sichtbarem Schirm zeichnen */
  {
    diff = box.x & 0x000F;                      /* Auf Wortgrenze ? */

    if (diff != 0)
    {
       box.x -= diff;
       box.w += diff;
    } /* if */

    vq_extnd (vdi_handle, TRUE, work_out);      /* Hole Anzahl Planes */

    screen->mp  = NULL;                         /* Bildschirm */
    buffer->fwp = box.w;
    buffer->fh  = box.h;
    buffer->fww = box.w / 16 + (box.w % 16 != 0);
    buffer->ff  = FALSE;
    buffer->np  = work_out [4];

    if (get)
    {
      size       = (LONG)buffer->fww * 2L * (LONG)buffer->fh * (LONG)buffer->np;
      buffer->mp = mem_alloc (size);

      rect2array (&box, xy);
      xywh2array (0, 0, box.w, box.h, &xy [4]);
    } /* if */
    else
    {
      xywh2array (0, 0, box.w, box.h, xy);
      rect2array (&box, &xy [4]);
    } /* if */

    if (buffer->mp != NULL)
    {
      set_clip (FALSE, &desk);
      hide_mouse ();

      if (get)
        vro_cpyfm (vdi_handle, S_ONLY, xy, screen, buffer);
      else
      {
        vro_cpyfm (vdi_handle, S_ONLY, xy, buffer, screen);
        mem_free (buffer->mp);
      } /* else */

      show_mouse ();
    } /* if */
    else                        /* Wenigstens Update */
      if (! get) form_dial (FMD_FINISH, 0, 0, 0, 0, box.x, box.y, box.w, box.h);
  } /* if */

  return (buffer->mp != NULL);
} /* background */

/*****************************************************************************/

GLOBAL BOOLEAN opendial (tree, grow, size, screen, buffer)
OBJECT  *tree;
BOOLEAN grow;
RECT    *size;
MFDB    *screen, *buffer;

{
  BOOLEAN ok;
  RECT    r, d;

  ok = FALSE;

#if GEM & XGEM
  screen = NULL;
#endif

  if (size == NULL)
    xywh2rect (0, 0, 0, 0, &r);
  else
    rc_copy (size, &r);

  form_center (tree, &d.x, &d.y, &d.w, &d.h);

  if ((screen == NULL) || (buffer == NULL))
    form_dial (FMD_START, r.x, r.y, r.w, r.h, d.x, d.y, d.w, d.h);
  else
    ok = background (tree, ROOT, TRUE, screen, buffer);

  if (grow) growbox (&r, &d);

  return (ok);
} /* opendial */

/*****************************************************************************/

GLOBAL BOOLEAN closedial (tree, shrink, size, screen, buffer)
OBJECT  *tree;
BOOLEAN shrink;
RECT    *size;
MFDB    *screen, *buffer;

{
  BOOLEAN ok;
  RECT    r, d;

  ok = FALSE;

#if GEM & XGEM
  screen = NULL;
#endif

  if (size == NULL)
    xywh2rect (0, 0, 0, 0, &r);
  else
    rc_copy (size, &r);

  form_center (tree, &d.x, &d.y, &d.w, &d.h);

  if ((screen == NULL) || (buffer == NULL))
    form_dial (FMD_FINISH, r.x, r.y, r.w, r.h, d.x, d.y, d.w, d.h);
  else
    ok = background (tree, ROOT, FALSE, screen, buffer);

  if (shrink) shrinkbox (&r, &d);

  return (ok);
} /* closedial */

/*****************************************************************************/

GLOBAL WORD hndl_dial (tree, def, grow_shrink, save_back, size, ok)
OBJECT  *tree;
WORD    def;
BOOLEAN grow_shrink, save_back;
RECT    *size;
BOOLEAN *ok;

{
  RECT r;
  WORD exit_obj;
  MFDB screen, buffer;
  MFDB *screenp, *bufferp;

  if (save_back)                                        /* Hintergrund retten */
  {
    screenp = &screen;
    bufferp = &buffer;
  } /* if */
  else
    screenp = bufferp = NULL;                           /* Keinen Hintergrund retten */

  *ok = opendial (tree, grow_shrink, size, screenp, bufferp);
  form_center (tree, &r.x, &r.y, &r.w, &r.h);           /* Mitte berechnen */

  objc_draw (tree, ROOT, MAX_DEPTH, r.x, r.y, r.w, r.h); /* Zeichnen */
  draw_oblines (tree);

  show_mouse ();                                        /* Maus ist beim Scrolling zweimal versteckt */
  show_mouse ();
  set_mouse (ARROW, NULL);
  exit_obj = formdo (tree, def) & 0x7FFF;               /* Dialog */
  last_mouse ();
  hide_mouse ();
  hide_mouse ();

  *ok = closedial (tree, grow_shrink, size, screenp, bufferp);
  undo_state (tree, exit_obj, SELECTED);                /* Objekt wieder weiû machen */

  return (exit_obj);           /* Objekt, mit dem Dialogbox verlassen wurde */
} /* hndl_dial */

/*****************************************************************************/

LOCAL WORD find_obj (tree, start_obj, which)
OBJECT *tree;
WORD   start_obj, which;

{
  WORD obj, theflag, thestate, flag, inc;

  obj  = 0;
  flag = EDITABLE;
  inc  = 1;

  switch (which)
  {
    case FMD_BACKWARD : inc = -1;               /* fall thru */
    case FMD_FORWARD  : obj = start_obj + inc;
                        break;
    case FMD_DEFLT    : flag = DEFAULT;
                        break;
  } /* switch */

  while (obj >= 0)
  {
    theflag  = tree [obj].ob_flags;
    thestate = tree [obj].ob_state;

    if (theflag & flag)
     if (! (theflag & HIDETREE))
       if (! (thestate & DISABLED)) return (obj);

    if (theflag & LASTOB)
      obj = NIL;
    else
      obj += inc;
  } /* while */

  return (obj);
} /* find_obj */

/*****************************************************************************/

LOCAL WORD fm_inifld (tree, start_fld)
OBJECT *tree;
WORD   start_fld;

{
  if (start_fld == 0) start_fld = find_obj (tree, 0, FMD_FORWARD);
  return (start_fld);
} /* fm_inifld */

/*****************************************************************************/

GLOBAL WORD formdo (tree, start)
OBJECT *tree;
WORD   start;

{
  WORD    edit_obj;
  WORD    next_obj, next, prev;
  WORD    which, cont;
  WORD    idx;
  WORD    mx, my, mb, ks, br;
  UWORD   kr;
  MKINFO  mk;
  BOOLEAN check;

  wind_update (BEG_UPDATE);
  wind_update (BEG_MCTRL);

  next_obj = fm_inifld (tree, start);
  edit_obj = 0;
  cont     = TRUE;

  while (cont)
  {
    if ((next_obj != NIL) && (next_obj != 0) && (edit_obj != next_obj))
    {
      edit_obj = next_obj;
      next_obj = 0;
      objc_edit (tree, edit_obj, 0, &idx, EDINIT);
    } /* if */

    which = evnt_multi (MU_KEYBD | MU_BUTTON,
                        0x0002, 0x0001, 0x0001,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        NULL,
                        0, 0,
                        &mx, &my, &mb, &ks, &kr, &br);

    if (which & MU_KEYBD)
    {
      mk.alt       = (ks & K_ALT) != 0;
      mk.scan_code = kr >> 8;
      next         = NIL;

      if (mk.scan_code == UNDO) next = find_flags (tree, ROOT, UNDO_FLAG);
      if (next == NIL) next = check_alt (tree, &mk);

      if (next != NIL)
      {
        objc_offset (tree, next, &mx, &my);
        br    = 1;
        which = MU_BUTTON;
      } /* if */
      else
      {
        cont = form_keybd (tree, edit_obj, next_obj, kr, &next_obj, &kr);
        if (kr) objc_edit (tree, edit_obj, kr, &idx, EDCHAR);
      } /* else */
    } /* if */

    if (which & MU_BUTTON)
    {
      next_obj = objc_find (tree, ROOT, MAX_DEPTH, mx, my);

      if (next_obj == NIL)
      {
        beep ();
        next_obj = 0;
      } /* if */
      else
      {
        prev  = next_obj - 1;
        check = FALSE;

        if (prev != NIL)
          if ((OB_TYPE (tree, prev) == G_USERDEF) &&
              (OB_FLAGS (tree, prev) & SELECTABLE) &&
              (OB_TYPE (tree, next_obj) == G_STRING))
          {
            next_obj = prev;
            check    = ! (OB_FLAGS (tree, next_obj) & RBUTTON);

            if (check) undo_flags (tree, next_obj, SELECTABLE);
          } /* if, if */

        if (! is_flags (tree, next_obj, LASTOB) && (br == 1))
          if ((OB_TYPE (tree, next_obj) == G_STRING) &&
              ((OB_TYPE (tree, next_obj + 1) == G_FTEXT) ||
               (OB_TYPE (tree, next_obj + 1) == G_FBOXTEXT) ||
               (OB_TYPE (tree, next_obj + 1) == G_BOXTEXT))) next_obj++;

        cont = form_button (tree, next_obj, br, &next_obj);

        if (check)              /* In benutzerdefinierte Checkbox geklickt */
        {
          do_flags (tree, prev, SELECTABLE);
          flip_state (tree, prev, SELECTED);
          objc_draw (tree, prev, MAX_DEPTH, desk.x, desk.y, desk.w, desk.h);
          evnt_button (0x0001, 0x0001, 0x0000, &mx, &my, &mb, &ks);
        } /* if */
      } /* else */
    } /* if */

    if ((! cont) || ((next_obj != 0) && (next_obj != edit_obj)))
      if (edit_obj != 0) objc_edit (tree, edit_obj, 0, &idx, EDEND);
  } /* while */

  wind_update (END_MCTRL);
  wind_update (END_UPDATE);

  return (next_obj);
} /* formdo */

/*****************************************************************************/

GLOBAL VOID blink (tree, obj, blinkrate)
OBJECT *tree;
WORD   obj, blinkrate;

{
  REG WORD i;

  if ((tree != NULL) && (obj != NIL))           /* Blinken mîglich */
    for (i = 0; i < 2 * blinkrate; i++)
    {
      objc_change (tree, obj, 0, desk.x, desk.y, desk.w, desk.h, tree [obj].ob_state ^ SELECTED, TRUE);
#if GEM & XGEM
      evnt_timer (10, 0);
#else
      evnt_timer (50, 0);
#endif
    } /* for */
} /* blink */

/*****************************************************************************/

GLOBAL WORD popup_menu (tree, obj, x, y, center_obj, relative, bmsk)
OBJECT  *tree;
WORD    obj, x, y, center_obj;
BOOLEAN relative;
WORD    bmsk;

{
  MFDB    screen, buffer;
  OBJECT  *objp;
  WORD    item, founditem, olditem;
  WORD    mox, moy, mobutton, mokstate;
  BOOLEAN leave;
  WORD    xold, yold;
  WORD    xdiff, ydiff;
  RECT    r, box;
  WORD    event, ret;
  UWORD   uret;

  graf_mkstate (&mox, &moy, &mobutton, &mokstate);
  objc_rect (tree, obj, &box, FALSE);

  objp  = &tree [obj];
  xold  = objp->ob_x;                           /* Alte Werte retten */
  yold  = objp->ob_y;
  xdiff = (relative ? mox : 0) - box.x;
  ydiff = (relative ? moy : 0) - box.y;

  if (center_obj != NIL)                        /* Zentrum von Objekt berÅcksichtigen */
  {
    objc_rect (tree, center_obj, &r, FALSE);

    if (relative)
    {
      x -= r.w / 2 + r.x - box.x;
      y -= r.h / 2 + r.y - box.y;
    } /* if */
    else
    {
      x -= r.x - box.x;
      y -= r.y - box.y;
    } /* else */
  } /* if */

  objp->ob_x += xdiff + x;                      /* X/Y neu setzen */
  objp->ob_y += ydiff + y;

  objc_rect (tree, obj, &box, FALSE);           /* RÑnder berÅcksichtigen */

  xdiff = box.x + box.w - (desk.x + desk.w);    /* Rechts heraushÑngend ? */
  if (xdiff > 0) objp->ob_x -= xdiff;

  ydiff = box.y + box.h - (desk.y + desk.h);    /* Unten heraushÑngend ? */
  if (ydiff > 0) objp->ob_y -= ydiff;

  objc_rect (tree, obj, &box, FALSE);           /* RÑnder berÅcksichtigen */

  xdiff = box.x - desk.x;                       /* Links heraushÑngend ? */
  if (xdiff < 0) objp->ob_x -= xdiff;

  ydiff = box.y - desk.y;                       /* Oben heraushÑngend ? */
  if (ydiff < 0) objp->ob_y -= ydiff;

  if (relative)
  {
    objp->ob_x &= 0xFFFE;                       /* X immer gerade */
    objp->ob_y -= ! odd (objp->ob_y);           /* Y immer ungerade */
  } /* if */

  olditem   = NIL;
  founditem = item = objc_find (tree, obj, MAX_DEPTH, mox, moy); /* In MenÅ ? */

  if (item != NIL)
    if (is_state (tree, item, DISABLED) || ! is_flags (tree, item, SELECTABLE)) item = NIL;

  if (item != NIL) do_state (tree, item, SELECTED);

  background (tree, obj, TRUE, &screen, &buffer);
  objc_draw (tree, obj, MAX_DEPTH, desk.x, desk.y, desk.w, desk.h);

  set_mouse (ARROW, NULL);
  wind_update (BEG_MCTRL);                      /* Mauskontrolle Åbernehmen */

  do
  {
    if (founditem != NIL)                       /* In MenÅeintrag */
    {
      leave = TRUE;
      objc_rect (tree, founditem, &r, FALSE);
    } /* if */
    else                                        /* Auûerhalb Pop-Up-MenÅ */
    {
      leave = FALSE;
      objc_rect (tree, obj, &r, FALSE);
    } /* else */

    event = evnt_multi (MU_BUTTON | MU_M1,
                        1, bmsk, ~ mobutton & bmsk,
                        leave, r.x, r.y, r.w, r.h,
                        0, 0, 0, 0, 0,
                        NULL, 0, 0,
                        &mox, &moy, &ret, &ret, &uret, &ret);

	if (objc_find (tree, obj, MAX_DEPTH, mox, moy))
	{ /* geÑndert: Nix tun, wenn Maus auf Hintergrundobjekt zeigt */
	    olditem   = item;
		 founditem = item = objc_find (tree, obj, MAX_DEPTH, mox, moy);
		
		 if (item != NIL)
		   if (is_state (tree, item, DISABLED) || ! is_flags (tree, item, SELECTABLE)) item = NIL;
		
		 if (olditem != item)
		 {
		   if (olditem != NIL)
		     objc_change (tree, olditem, 0, desk.x, desk.y, desk.w, desk.h, tree [olditem].ob_state ^ SELECTED, TRUE);
		
		   if (item != NIL)
		     objc_change (tree, item, 0, desk.x, desk.y, desk.w, desk.h, tree [item].ob_state ^ SELECTED, TRUE);
		 } /* if */
    } /* if */
  } while (! (event & MU_BUTTON));

  wind_update (END_MCTRL);                      /* Mauskontrolle wieder abgeben */
  blink (tree, item, blinkrate);
  last_mouse ();

  background (tree, obj, FALSE, &screen, &buffer);

  if (item != NIL) undo_state (tree, item, SELECTED);
  if (~ mobutton & bmsk) evnt_button (1, bmsk, 0x0000, &ret, &ret, &ret, &ret); /* Warte auf Mausknopf */

  objp->ob_x = xold;                            /* Alte Werte restaurieren */
  objp->ob_y = yold;

  return (item);
} /* popup_menu */

/*****************************************************************************/

GLOBAL BOOLEAN is_menu_key (menu, mk, title, item)
OBJECT *menu;
MKINFO *mk;
WORD   *title, *item;

{
  REG WORD  ltitle, litem;
  REG BYTE  *s;
  REG UWORD key;
      WORD  i, x, func;
      WORD  sign, c;
      WORD  menubox;

  *title = NIL;
  *item  = NIL;

  if (menu != NULL)
  {
    key     = 0;
    menubox = menu [ROOT].ob_tail;
    menubox = menu [menubox].ob_head;
    ltitle  = THEFIRST;

    do
    {
      litem = menu [menubox].ob_head;                   /* Erster Eintrag */

      do
      {
        if (((menu [litem].ob_type & 0xFF) == G_STRING) && ! is_state (menu, litem, DISABLED))
        {
          s = (BYTE *)menu [litem].ob_spec;             /* MenÅ */

          /* énderung: rechts angehÑngte Leerzeichen ignorieren. BD */
          for (x = strlen (s)-1; (x >= 0) && (s [x] == SP); x--);
			 
			 /*if(x>0) x--;*/
				          
          for (i = x; (i >= 0) && (s [i] != SP); i--);

          if ((i >= 0) && (strlen (s + i) >= 2))        /* Leerzeichen und ein Buchstabe */
          {
            sign = s [++i];                             /* "Vorzeichen" */
            c    = ((sign == FUNC_CHAR) && (strlen (s + i + 1) > 0)) ? FUNC_CHAR : s [++i]; /* Eigentliches Zeichen */
            func = 0;

            if (c == EOS)                               /* Genau 1 Zeichen */
            {
              if (! (mk->ctrl || mk->alt))
                if (toupper (sign) == toupper (mk->ascii_code)) key = mk->kreturn;
            } /* if */
            else
            {
              if (c == FUNC_CHAR) sscanf (s + i + 1, "%d", &func); /* Funktionstaste */

              switch (sign)
              {
                case CTRL_CHAR  : if (mk->ctrl)         /* Control-Zeichen */
                                    if (func != 0)
                                      key = func_keys [func - 1 + 2 * MAX_FUNC];
                                    else
                                      if (mk->ascii_code == (ctrl_keys [c - SP] & 0x00FF))
                                        key = mk->kreturn;
                                  break;
                case ALT_CHAR   : if (mk->alt)          /* Alternate-Zeichen */
                                    if (func != 0)
                                      key = func_keys [func - 1 + 3 * MAX_FUNC];
                                    else
                                      key = alt_keys [c - SP];
                                  break;
                case SHIFT_CHAR : if (mk->shift)        /* Shift-Zeichen */
                                    if (func != 0) key = func_keys [func - 1 + MAX_FUNC];
                                  break;
                case FUNC_CHAR  : if (! (mk->ctrl || mk->alt || mk->shift))
                                    if (func != 0) key = func_keys [func - 1];
                                  break;
              } /* switch */
            } /* else */
          } /* if */
  
          if ((key != 0) && (key == mk->kreturn))       /* Zeichen erkannt */
          {
           *title = ltitle;
           *item  = litem;

           return (TRUE);                               /* Fertig */
          } /* if */
        } /* if */

        litem = menu [litem].ob_next;                   /* NÑchster Eintrag */
      } while (litem != menubox);

      menubox = menu [menubox].ob_next;                 /* NÑchstes Drop-Down-MenÅ */
      ltitle  = menu [ltitle].ob_next;                  /* NÑchster Titel */
    } while (ltitle != THEACTIVE);
  } /* if */

  return (FALSE);
} /* is_menu_key */

/*****************************************************************************/

GLOBAL WORD check_alt (tree, mk)
OBJECT *tree;
MKINFO *mk;

{
  WORD  obj, pos;
  BYTE  *p, ch;
  UWORD code;

  if (mk->alt)
  {
    obj = ROOT;

    do
    {
      if (! is_flags (tree, obj, HIDETREE) &&
          ! is_state (tree, obj, DISABLED) &&
          (OB_EXTYPE (tree, obj) != 0) &&
          ((OB_TYPE (tree, obj) == G_BUTTON) || (OB_TYPE (tree, obj) == G_STRING)))
      {
        pos  = OB_EXTYPE (tree, obj);
        p    = (BYTE *)OB_SPEC (tree, obj);
        ch   = toupper (p [pos - 1]) & 0xFF;

        if (isprint (ch))
        {
          code = alt_keys [ch - SP];
          if ((code >> 8) == mk->scan_code) return (obj);
        } /* if */
      } /* if */
    } while (! is_flags (tree, obj++, LASTOB));
  } /* if */

  return (NIL);
} /* check_alt */

/*****************************************************************************/

GLOBAL VOID draw_oblines (tree)
OBJECT *tree;

{
  WORD obj;

  obj = ROOT;

  do
  {
    draw_obline (tree, obj);
  } while (! is_flags (tree, obj++, LASTOB));
} /* draw_oblines */

/*****************************************************************************/

GLOBAL VOID draw_obline (tree, obj)
OBJECT *tree;
WORD   obj;

{
  WORD pos;
  WORD xy [4];
  RECT rect;
  BYTE *p;

  if (! is_flags (tree, obj, HIDETREE) && (OB_EXTYPE (tree, obj) != 0))
  {
    line_default (vdi_handle);
    vsl_udsty (vdi_handle, 0xAAAA);

    objc_rect (tree, obj, &rect, FALSE);

    if (OB_TYPE (tree, obj) == G_BUTTON)
    {
      p       = (BYTE *)OB_SPEC (tree, obj);
      rect.x += (rect.w - strlen (p) * gl_wbox) / 2;
      rect.y += (rect.h - gl_hbox) / 2;
    } /* if */

    rect2array (&rect, xy);

    pos     = OB_EXTYPE (tree, obj) - 1;
    xy [0] += pos * gl_wbox;
    xy [1] += gl_hbox - 1;
    xy [2]  = xy [0] + gl_wbox - 1;
    xy [3]  = xy [1];

    vsl_type (vdi_handle, is_state (tree, obj, DISABLED) ? USERLINE : SOLID);
    vsl_color (vdi_handle, is_state (tree, obj, SELECTED) ? WHITE : BLACK);
    v_pline (vdi_handle, 2, xy);
  } /* if */
} /* draw_obline */

/*****************************************************************************/
/* Rechteck-Routinen                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN rc_equal (p1, p2)
CONST RECT *p1, *p2;

{
  return ((p1->x == p2->x) && (p1->y == p2->y) &&
          (p1->w == p2->w) && (p1->h == p2->h));
} /* rc_equal */

/*****************************************************************************/

GLOBAL VOID rc_copy (ps, pd)
CONST RECT *ps;
RECT       *pd;

{
  pd->x = ps->x;
  pd->y = ps->y;
  pd->w = ps->w;
  pd->h = ps->h;
} /* rc_copy */

/*****************************************************************************/

GLOBAL VOID rc_union (p1, p2)
CONST RECT *p1;
RECT       *p2;

{
  RECT r;

  if ((p2->w == 0) || (p2->h == 0))
    rc_copy (p1, p2);
  else
  {
    r.x = min (p1->x, p2->x);
    r.y = min (p1->y, p2->y);
    r.w = max (p1->x + p1->w, p2->x + p2->w) - r.x;
    r.h = max (p1->y + p1->h, p2->y + p2->h) - r.y;

    rc_copy (&r, p2);
  } /* else */
} /* rc_union */

/*****************************************************************************/

GLOBAL BOOLEAN rc_intersect (p1, p2)
CONST RECT *p1;
RECT       *p2;

{
  REG WORD tx, ty, tw, th;

  tw = min (p2->x + p2->w, p1->x + p1->w);
  th = min (p2->y + p2->h, p1->y + p1->h);
  tx = max (p2->x, p1->x);
  ty = max (p2->y, p1->y);

  p2->x = tx;
  p2->y = ty;
  p2->w = tw - tx;
  p2->h = th - ty;

  return ((tw > tx) && (th > ty));
} /* rc_intersect */

/*****************************************************************************/

GLOBAL BOOLEAN inside (x, y, r)
WORD       x, y;
CONST RECT *r;

{
  return ((x >= r->x) && (y >= r->y) && (x < r->x + r->w) && (y < r->y + r->h));
} /* inside */

/*****************************************************************************/

GLOBAL VOID rect2array (rect, array)
CONST RECT *rect;
WORD       *array;

{
  *array++ = rect->x;
  *array++ = rect->y;
  *array++ = rect->x + rect->w - 1;
  *array   = rect->y + rect->h - 1;
} /* rect2array */

/*****************************************************************************/

GLOBAL VOID array2rect (array, rect)
CONST WORD *array;
RECT       *rect;

{
  rect->x = min (array [0], array [2]);
  rect->y = min (array [1], array [3]);
  rect->w = max (array [0], array [2]) - rect->x + 1;
  rect->h = max (array [1], array [3]) - rect->y + 1;
} /* array2rect */

/*****************************************************************************/

GLOBAL VOID xywh2array  (x, y, w, h, array)
WORD x, y, w, h;
WORD *array;

{
  *array++ = x;
  *array++ = y;
  *array++ = x + w - 1;
  *array   = y + h - 1;
} /* xywh2array */

/*****************************************************************************/

GLOBAL VOID array2xywh  (array, x, y, w, h)
CONST WORD *array;
WORD       *x, *y, *w, *h;

{
  *x = *array++;
  *y = *array++;
  *w = *array++ - *x + 1;
  *h = *array - *y + 1;
} /* array2xywh */

/*****************************************************************************/

GLOBAL VOID xywh2rect (x, y, w, h, rect)
WORD x, y, w, h;
RECT *rect;

{
  rect->x = x;
  rect->y = y;
  rect->w = w;
  rect->h = h;
} /* xywh2rect */

/*****************************************************************************/

GLOBAL VOID rect2xywh (rect, x, y, w, h)
CONST RECT *rect;
WORD       *x, *y, *w, *h;

{
  *x = rect->x;
  *y = rect->y;
  *w = rect->w;
  *h = rect->h;
} /* rect2xywh */

/*****************************************************************************/

GLOBAL VOID set_clip (clipflag, size)
BOOLEAN     clipflag;
CONST RECT *size;

{
  RECT r;
  WORD xy [4];

  if (size == NULL)
    rc_copy (&desk, &r);                /* Nichts definiert, nimm Desktop */
  else
    rc_copy (size, &r);                 /* Benutze definierte Grîûe */

  rc_copy (&r, &clip);                  /* Rette aktuelle Werte */

  if (rc_intersect (&desk, &r))         /* Nur auf Desktop zeichnen */
    rect2array (&r, xy);
  else
    xywh2array (0 ,0 ,0 ,0, xy);        /* Nichts zeichnen */

  vs_clip (vdi_handle, clipflag, xy);   /* Setze Rechteckausschnitt */
} /* set_clip */

/*****************************************************************************/

#if GEM & (GEM2 | GEM3 | XGEM)
LOCAL VOID graf_growbox (orgx, orgy, orgw, orgh, x, y, w, h)
WORD orgx, orgy, orgw, orgh;
WORD x, y, w, h;

{
  WORD  cx, cy, cnt, xstep, ystep;

  xgrf_stepcalc (orgw, orgh, x, y, w, h, &cx, &cy, &cnt, &xstep, &ystep);
  graf_mbox (orgw, orgh, orgx, orgy, cx, cy);
  xgrf_2box (cx, cy, orgw, orgh, TRUE, cnt, xstep, ystep, TRUE);
} /* graf_growbox */

/*****************************************************************************/

LOCAL VOID graf_shrinkbox (orgx, orgy, orgw, orgh, x, y, w, h)
WORD orgx, orgy, orgw, orgh;
WORD x, y, w, h;

{
  WORD cx, cy, cnt, xstep, ystep;

  xgrf_stepcalc (orgw, orgh, x, y, w, h, &cx, &cy, &cnt, &xstep, &ystep);
  xgrf_2box (x, y, w, h, TRUE, cnt, -xstep, -ystep, TRUE);
  graf_mbox (orgw, orgh, cx, cy, orgx, orgy);
} /* graf_shrinkbox */
#endif

/*****************************************************************************/

GLOBAL VOID growbox (st, fin)
CONST RECT *st, *fin;

{
  RECT r;

  if (grow_shrink)
  {
    rc_copy (st, &r);

    if ((r.x == 0) && (r.y == 0))
    {
      r.x = fin->x + fin->w / 2;
      r.y = fin->y + fin->h / 2;
    } /* if */

    graf_growbox (r.x, r.y, r.w, r.h, fin->x, fin->y, fin->w, fin->h);
  } /* if */
} /* growbox */

/*****************************************************************************/

GLOBAL VOID shrinkbox (fin, st)
CONST RECT *fin, *st;

{
  RECT r;

  if (grow_shrink && ! acc_close)
  {
    rc_copy (fin, &r);

    if ((r.x == 0) && (r.y == 0))
    {
      r.x = st->x + st->w / 2;
      r.y = st->y + st->h / 2;
    } /* if */

    graf_shrinkbox (r.x, r.y, r.w, r.h, st->x, st->y, st->w, st->h);
  } /* if */
} /* shrinkbox */

/*****************************************************************************/
/* Fehlerbehandlung                                                          */
/*****************************************************************************/

GLOBAL VOID beep ()

{
  if (ring_bell)
  {
#if GEM & (GEM2 | GEM3 | XGEM)
    v_sound (phys_handle, 550, 3);
#else
#if GEMDOS
    Bconout (2, BEL);
#endif
#endif
  } /* if */
} /* beep */

/*****************************************************************************/

GLOBAL WORD note (button, index, helpinx, helptree)
WORD   button, index, helpinx;
OBJECT *helptree;

{
  WORD    ret;
  BOOLEAN ok;

  set_mouse (ARROW, NULL);

  if (alertmsg == NULL)
    ret = 0;
  else
    do
    {
      ret = form_alert (button, (BYTE*)alertmsg [index]);

      if ((ret == helpinx) && (helptree != NULL)) hndl_dial (helptree, 0, FALSE, TRUE, NULL, &ok);
    } while (ret == helpinx);

  last_mouse ();
  return (ret);
} /* note */

/*****************************************************************************/

GLOBAL WORD error (button, index, helpinx, helptree)
WORD   button, index, helpinx;
OBJECT *helptree;

{
  beep ();
  return (note (button, index, helpinx, helptree));
} /* error */

/*****************************************************************************/
/* Speicher-Routinen                                                         */
/*****************************************************************************/

#if GEMDOS
LOCAL LONG *get_actpd ()

{
  LONG *ret;
  WORD tos;
  LONG stack;

  stack = Super (NULL);
  tos   = *(UWORD *)(*(LONG *)0x4F2 + 0x02);            /* get TOS version */

  if (tos >= 0x0102)
    ret = (LONG *)(*(LONG *)(*(LONG *)0x4F2 + 0x28));   /* get pointer to basepage */
  else
    ret = (LONG *)0x602C;

  Super ((VOID *)stack);
  return (ret);
} /* get_actpd */
#endif

/*****************************************************************************/

GLOBAL VOID *mem_alloc (mem)
LONG mem;

{
    VOID *ret = NULL;
    LONG memavail = mem_avail();

/*  
    if (mem > 1500L)
        fprintf (stdout, "Big memory chunk!\n");
*/
    if (memavail > mem + 100000L)
    {
        
        
        if (tos >= 0x0300)
            ret = (VOID *)Mxalloc (mem, 3);
        else
            ret = (VOID *)Malloc (mem);
        
        
    } /* if */
	else {
   	hndl_alert (ERR_NOMEMORY);
	}
    
    
    if (ret == NULL) {
   	hndl_alert (ERR_NOMEMORY);
    }
	
	
    return (ret);
} /* mem_alloc */

/*****************************************************************************/

GLOBAL VOID mem_free (memptr)
VOID *memptr;

{
  BOOLEAN ok;

#if GEMDOS
  LONG *pdp, pd;

  ok = memptr != NULL;

  if (ok)
  {
    pdp = get_actpd ();
    pd  = *pdp;

#if TURBO_C
    *pdp = (LONG)_BasPag;
#endif

    Mfree (memptr);
    *pdp = pd;
  } /* if */
#endif

#if MSDOS | FLEXOS
  ok = memptr != NULL;

  if (ok) Mfree (memptr);
#endif

#if UNIX
  ok = memptr != NULL;

  if (ok) free (memptr);
#endif
} /* mem_free */

/*****************************************************************************/

GLOBAL LONG mem_avail ()

{
    LONG ret;
    
    if (tos >= 0x0300)
        ret = (VOID *)Mxalloc (-1L, 3);
    else
        ret = (VOID *)Malloc (-1L);

    if (ret < 100000L)
        hndl_alert (ERR_NOMEMORY);
    
    return (ret);
} /* mem_avail */

/*****************************************************************************/

GLOBAL VOID *mem_set (dest, val, len)
VOID  *dest;
WORD  val;
UWORD len;

{
#if DR_C | LASER_C | LATTICE_C
  REG UBYTE *d;

  for (d = (UBYTE *)dest; len > 0; len--) *d++ = (UBYTE)val;

  return (dest);
#else
  return (memset (dest, val, len));
#endif
} /* mem_set */

/*****************************************************************************/

GLOBAL VOID *mem_move (dest, src, len)
VOID       *dest;
CONST VOID *src;
UWORD      len;

{
#if DR_C | LASER_C | LATTICE_C | HIGH_C
  REG UBYTE *s, *d;
  REG UWORD l;

  s = (UBYTE *)src;
  d = (UBYTE *)dest;
  l = len;

  if ((s < d) && (s + l > d))
    for (d += l, s += l; l > 0; l--) *(--d) = *(--s);
  else 
    for (; l > 0; l--) *d++ = *s++;

  return (dest);
#else
  return (memmove (dest, src, len));
#endif
} /* mem_move */

/*****************************************************************************/

GLOBAL VOID *mem_lset (dest, val, len)
VOID  *dest;
WORD  val;
ULONG len;

{
#if MSDOS | FLEXOS | DR_C | LASER_C | LATTICE_C | MW_C
  REG UBYTE HUGE *d;

  if (len < 0x00010000L)
    mem_set (dest, val, (UWORD)len);
  else
    for (d = (UBYTE HUGE *)dest; len > 0; len--) *d++ = (UBYTE)val;

#else
  if (len != 0) memset (dest, val, len);
#endif

  return (dest);
} /* mem_lset */

/*****************************************************************************/

GLOBAL VOID *mem_lmove (dest, src, len)
VOID       *dest;
CONST VOID *src;
ULONG      len;

{
#if MSDOS | FLEXOS | DR_C | LASER_C | LATTICE_C | MW_C
  REG UBYTE HUGE *s;
  REG UBYTE HUGE *d;
  REG ULONG       l;

  if (len < 0x00010000L)
    mem_move (dest, src, (UWORD)len);
  else
  {
    s = (UBYTE HUGE *)src;
    d = (UBYTE HUGE *)dest;
    l = len;

    if ((s < d) && (s + l > d))
      for (d += l, s += l; l > 0; l--) *(--d) = *(--s);
    else 
      for (; l > 0; l--) *d++ = *s++;
  } /* else */
#else
  if (len != 0) memmove (dest, src, len);
#endif

  return (dest);
} /* mem_lmove */

/*****************************************************************************/
/* Zeichenketten-Routinen                                                    */
/*****************************************************************************/

GLOBAL BYTE *str_upper (s)
BYTE *s;

{
#if MS_C | TURBO_C
  strupr (s);
#else
  while (*s) *s++ = toupper (*s);
#endif

  return (s);
} /* str_upper */

/*****************************************************************************/

GLOBAL BYTE *str_lower (s)
BYTE *s;

{
#if MS_C | TURBO_C
  strlwr (s);
#else
  while (*s) *s++ = tolower (*s);
#endif

  return (s);
} /* str_lower */

/*****************************************************************************/
/* Mengen-Routinen                                                           */
/*****************************************************************************/

GLOBAL VOID setcpy (set1, set2)
SET set1, set2;

{
  mem_move (set1, set2, SETSIZE * sizeof (ULONG));
} /* setcpy */

/*****************************************************************************/

GLOBAL VOID setall (set)
SET set;

{
  mem_set (set, 0xFF, SETSIZE * sizeof (ULONG));
} /* setall */

/*****************************************************************************/

GLOBAL VOID setclr (set)
SET set;

{
  mem_set (set, 0x00, SETSIZE * sizeof (ULONG));
} /* setclr */

/*****************************************************************************/

GLOBAL VOID setnot (set)
SET set;

{
  REG WORD i;

  for (i = 0; i < SETSIZE; i++) set [i] = ~ set [i];
} /* setnot */

/*****************************************************************************/

GLOBAL VOID setand (set1, set2)
SET set1, set2;

{
  REG WORD i;

  for (i = 0; i < SETSIZE; i++) set1 [i] &= set2 [i];
} /* setand */

/*****************************************************************************/

GLOBAL VOID setor (set1, set2)
SET set1, set2;

{
  REG WORD i;

  for(i = 0; i < SETSIZE; i++) set1 [i] |= set2 [i];
} /* setor */

/*****************************************************************************/

GLOBAL VOID setxor (set1, set2)
SET set1, set2;

{
  REG WORD i;

  for (i = 0; i < SETSIZE; i++) set1 [i] ^= set2 [i];
} /* setxor */

/*****************************************************************************/

GLOBAL VOID setincl (set, elt)
SET  set;
WORD elt;

{
  if ((0 <= elt) && (elt <= SETMAX)) set [elt / 32] |= (1L << (elt % 32));
} /* setincl */

/*****************************************************************************/

GLOBAL VOID setexcl (set, elt)
SET  set;
WORD elt;

{
  if ((0 <= elt) && (elt <= SETMAX)) set [elt / 32] &= (~ (1L << (elt % 32)));
} /* setexcl */

/*****************************************************************************/

GLOBAL BOOLEAN setin (set, elt)
SET  set;
WORD elt;

{
  if ((0 <= elt) && (elt <= SETMAX))
    return ((set [elt / 32] & (1L << (elt % 32))) ? TRUE : FALSE);
  else
    return (FALSE);
} /* setin */

/*****************************************************************************/

GLOBAL BOOLEAN setcmp (set1, set2)
SET set1, set2;

{
  REG WORD    i;
  REG BOOLEAN res;

  for (res = TRUE, i = 0; res && (i < SETSIZE); i++)
    res = (set2 != NULL ? set1 [i] == set2 [i] : set1 [i] == 0);

  return (res);
} /* setcmp */

/*****************************************************************************/

GLOBAL WORD setcard (set)
SET set;

{
  REG WORD i, j, card;
  REG ULONG l;

  for (i = card = 0; i < SETSIZE; i++)
    for (j = 0, l = set [i]; j < 32; j++)
    {
      if (l & 1) card++;
      l >>= 1;
    } /* for, for */

  return (card);
} /* setcard */

/*****************************************************************************/
/* Verschiedenes                                                             */
/*****************************************************************************/

GLOBAL VOID file_split (fullname, drive, path, filename, ext)
BYTE *fullname;
WORD *drive;
BYTE *path, *filename, *ext;

{
  STR128 s;
  BYTE   name [13];
  BYTE   *p, *f;
  WORD   drv;

  strcpy (s, fullname);
  p = strchr (s, DRIVESEP);

  if (p == NULL)                                /* Kein Laufwerk gefunden */
  {
    drv = Dgetdrv ();
    p   = s;
  } /* if */
  else
  {
    drv = p [-1] - 'A';
    p++;
  } /* else */

  if (drive != NULL) *drive = drv;

  f = strrchr (p, PATHSEP);

  if (f == NULL)                                /* Kein Pfad */
  {
    strcpy (name, p);                           /* Dateinamen retten */
    if (path != NULL) get_path (path, drv + 1);
  } /* if */
  else
  {
    strcpy (name, f + 1);                       /* Dateinamen retten */
    f [1] = EOS;

    if (path != NULL)
    {
      if (*p != PATHSEP)                        /* Keine Root */
        get_path (path, drv + 1);
      else
        *path = EOS;

      strcat (path, p);

      if (drive == NULL)
      {
        strcpy (s, "A:");
        s [0] += (BYTE)drv;
        strcat (s, path);
        strcpy (path, s);
      } /* if */
    } /* if */
  } /* else */

  if (filename != NULL) strcpy (filename, name);

  if (ext != NULL)
  {
    p = strrchr ((filename != NULL) ? filename : name, SUFFSEP);

    if (p == NULL)
      *ext = EOS;
    else
    {
      strcpy (ext, p + 1);
      if (filename != NULL) *p = EOS;
    } /* else */
  } /* if */
} /* file_split */

/*****************************************************************************/

GLOBAL BOOLEAN get_path (path, drive)
BYTE *path;
WORD drive;

{
  BYTE s [64], sep [2];
  WORD ret;

  path [0] = EOS;

  ret = Dgetpath (s, drive);

  if (*s)
  {
#if GEMDOS
    strcpy (path, s);
#else
    strcpy (path + 1, s);
    path [0] = PATHSEP;
#endif
  } /* if */

  sep [0] = PATHSEP;
  sep [1] = EOS;

  strcat (path, sep);

#if GEMDOS
  return (ret == 0);
#else
  return (ret == 1);
#endif
} /* get_path */

/*****************************************************************************/

GLOBAL BOOLEAN set_path (path)
CONST BYTE *path;

{
  BYTE s [64];
  BYTE *p;
  WORD l;

  strcpy (s, path);

  if (*s)
  {
    p = s;
    if (p [1] == DRIVESEP) p += 2;

    l = strlen (p);
    if ((l > 1) && (p [l - 1] == PATHSEP)) p [l - 1] = EOS;
  } /* if */

#if GEMDOS
  return ((WORD)Dsetpath (s) == 0);
#else
  Dsetpath (s);
  return (! DOS_ERR);
#endif
} /* set_path */

/*****************************************************************************/

GLOBAL BOOLEAN file_exist (filename)
CONST BYTE *filename;

{
  BOOLEAN result;
  STRING  s;
  DTA     dta, *old_dta;

  old_dta = (DTA *)Fgetdta ();
  Fsetdta (&dta);
  strcpy (s, filename);

#if GEMDOS
  result = Fsfirst (s, 0x00) == 0;
#else
  result = Fsfirst (s, 0x00) > 0;
#endif

  Fsetdta (old_dta);

  return (result);
} /* file_exist */

/*****************************************************************************/

GLOBAL BOOLEAN path_exist (pathname)
CONST BYTE *pathname;

{
  BOOLEAN result;
  STRING  s;
  DTA     dta, *old_dta;
  WORD    l;

  old_dta = (DTA *)Fgetdta ();
  Fsetdta (&dta);
  strcpy (s, pathname);

  l = strlen (s);

  if (l > 0)
    if (s [l - 1] == PATHSEP) s [l - 1] = EOS;

#if GEMDOS
  result = Fsfirst (s, 0x10) == 0;
#else
  result = Fsfirst (s, 0x10) > 0;
#endif

  Fsetdta (old_dta);

  return (result);
} /* path_exist */

/*****************************************************************************/

GLOBAL BOOLEAN select_file (name, path, suffix, label, filename)
BYTE *name, *path, *suffix, *label, *filename;

{
  BYTE   *p;
  STRING s;

#if GEMDOS
  WORD tos;
  LONG stack;

  stack = Super (NULL);
  tos   = *(UWORD *)(*(LONG *)0x4F2 + 0x02); /* Hole TOS-Version */

  Super ((VOID *)stack);
#endif

  if ((path != NULL) && (*path))        /* Pfad Ñndern */
    strcpy (fs_path, path);

  if (suffix != NULL)                   /* Suffix Ñndern */
  {
    p = strrchr (fs_path, PATHSEP);
    if (p != NULL) p [1] = EOS;         /* Suffix lîschen */
    strcat (fs_path, suffix);
  } /* if */

  if (name != NULL)                     /* Name Ñndern */
  {
    strncpy (fs_sel, name, 12);         /* Default-Name */
    fs_sel [12] = EOS;
  } /* if */

  set_mouse (ARROW, NULL);

#if GEMDOS
  if (tos >= 0x0104)
    fsel_exinput (fs_path, fs_sel, &fs_button, label);
  else
    fsel_input (fs_path, fs_sel, &fs_button);
#else
  fsel_exinput (fs_path, fs_sel, &fs_button, label);
#endif

  last_mouse ();

  strcpy (s, fs_path);                  /* Path aufbereiten */
  p = strrchr (s, PATHSEP);
  if (p != NULL) p [1] = EOS;

  if (*fs_sel)                          /* Dateinamen gewÑhlt */
  {
    strcpy (filename, s);
    strcat (filename, fs_sel);
  } /* if */
  else
    filename [0] = EOS;                 /* Keinen Dateinamen gewÑhlt */

  if (fs_button != 0)
  {
    if ((path != NULL) && (*path))      /* Pfad Ñndern */
      strcpy (path, fs_path);

    if ((name != NULL) && (*name) && (*fs_sel)) /* Name Ñndern */
      strcpy (name, fs_sel);
  } /* if */

  return (*fs_sel && (fs_button != 0)); /* Dateiname und OK gewÑhlt */
} /* select_file */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_global (argc, argv, acc_menu, class)
INT  argc;
BYTE *argv [];
BYTE *acc_menu;
WORD class;

{
  WORD    i;
  STR128  s;
  BYTE    *p;
  BOOLEAN ok;
  WORD    char_width, char_height, cell_width, cell_height;
#if GEM & (GEM2 | GEM3 | XGEM)
  WORD    max_colors, rgb [3];
#endif
#if GEMDOS
  LONG    drives;
  WORD    drive;
  BOOLEAN ready;
  LONG stack;

  stack = Super (NULL);
  tos   = *(UWORD *)(*(LONG *)0x4F2 + 0x02); /* Hole TOS-Version */

  Super ((VOID *)stack);
#endif

  i = appl_init ();                       /* Applikationsnummer besorgen */

#if (LATTICE_C | TURBO_C) | (GEM & (GEM2 | GEM3 | XGEM))
  gl_apid = i;                            /* gl_apid nicht extern */
#endif

  if (gl_apid < 0) return (FALSE);

  phys_handle = graf_handle (&gl_wbox, &gl_hbox, &gl_wattr, &gl_hattr); /* Handle des Bildschirms */
  vdi_handle  = phys_handle;              /* Benutze physikalischen Bildschirm */

  open_vwork ();                          /* Workstation îffnen */

  vst_font (vdi_handle, FONT_SYSTEM);

  for (gl_point = 8; gl_point <= 10; gl_point++)
  {
    vst_point (vdi_handle, i, &char_width, &char_height, &cell_width, &cell_height);
    if (cell_height == gl_hbox) break;    /* Punktgrîûe des System Fonts */
  } /* for */

#if GEM & (GEM2 | GEM3 | XGEM)            /* wegen TT */
  max_colors = min (colors, MAX_COLORS);

  if (colors > 2)                         /* nicht b/w */
    for (i = 0; i < max_colors; i++)      /* GEM-Farben festlegen */
    {
      rgb [0] = gem_colors [i].red;
      rgb [1] = gem_colors [i].green;
      rgb [2] = gem_colors [i].blue;

      vs_color (vdi_handle, i, rgb);
    } /* for, if */
#endif

  wind_get (DESK, WF_WXYWH, &desk.x, &desk.y, &desk.w, &desk.h);        /* Grîûe des Desktop */

  hidden        = 0;                      /* Maus ist da */
  busy          = 0;                      /* Maus ist nicht geschÑftig */
  mousenumber   = ARROW;                  /* Aktuelle Mausform-Nummer */
  mouseform     = NULL;                   /* Aktuelle Mausform */
  done          = FALSE;                  /* "Ende" wurde nicht gewÑhlt */
  ring_bell     = TRUE;                   /* Glocke eingeschaltet */
  grow_shrink   = TRUE;                   /* Grow/Shrink-Modus eingeschaltet */
  blinkrate     = DEFAULTRATE;            /* Anfangsblinkrate */
  updtmenu      = TRUE;                   /* MenÅs immer auf neuen Stand bringen */
  cmd [0]       = EOS;                    /* Kein Kommando */
  tail [0]      = EOS;                    /* Keine Kommandozeile */
  called_by [0] = EOS;                    /* Eventuell kein aufrufendes Programm */
  menu          = NULL;                   /* Noch keine MenÅzeile */
  about         = NULL;                   /* Noch keine About-Box */
  desktop       = NULL;                   /* Noch kein eigener Desktop */
  freetext      = NULL;                   /* Noch keine freien Texte */
  alertmsg      = NULL;                   /* Noch keine Fehlermeldungen */

  act_drv = Dgetdrv ();                   /* Aktuelles Laufwerk holen */
  strcpy (act_path, "A:");
  act_path [0] += (BYTE)act_drv;          /* Aktuelles Laufwerk hinzufÅgen */
  get_path (act_path + 2, 0);             /* Aktuellen Pfad holen */

#if FLEXOS
  str_upper (act_path);                   /* Aktueller Pfad nicht immer groû */
#endif

  strcpy (fs_path, "A:\\*.*");            /* Standard-Zugriffspfad */
  fs_path [0] += (BYTE)act_drv;           /* Standard-Laufwerk */
  fs_sel [0]   = EOS;                     /* Leerer Dateiname */

  for (i = 1; i < argc; i++)              /* Programm mit Pexec aufgerufen */
  {
    strcat (tail, argv [i]);              /* FÅge Parameter zusammen */
    strcat (tail, " ");
  } /* for */

  if (*tail) tail [strlen (tail) - 1] = EOS; /* Letztes Leerzeichen lîschen */

  shel_read (cmd, s);                     /* Kommando holen */

  s [s [0] + 1] = EOS;                    /* Lîsche '\r' */

  if (*tail == EOS) strcpy (tail, s + 1); /* Programm mit shel_write aufgerufen */

  str_upper (tail);                       /* Parameter immer in Groûschrift */

  p = strrchr (tail, PROGSEP);

  if (p != NULL)
  {
    strcpy (called_by, p + 1);            /* Aufrufendes Programm feststellen */
    *p = EOS;                             /* Programm ausblenden */
  } /* if */

  for (p = tail; *p; p++)
    if (*p == ',') *p = SP;               /* Kommata durch Leerzeichen ersetzen */

  strcpy (app_name, cmd);                 /* Kopiere Programmname */
  strcpy (app_path, cmd);                 /* Kopiere Pfadname */
  p = strrchr (app_path, PATHSEP);

  if (TRUE /* p == NULL */)     /* BD: Immer akt. Pfad verwenden */                     /* Kein Pfad */
    strcpy (app_path, act_path);          /* Pfad durch aktuellen Pfad ersetzen */
  else
  {
    p++;
    strcpy (app_name, p);                 /* Kopiere Programmname */
    *p = EOS;
  } /* if */

#if MSDOS | FLEXOS | LATTICE_C
  p = strrchr (app_name, SUFFSEP);

  deskacc = (p != NULL) &&
            ((strcmp (p + 1, "ACC") == 0) ||
             (strcmp (p + 1, "RSC") == 0)); /* Accessories mit rsrc_load */
#else
  deskacc = ! _app;                       /* _app wird im Initialisierungscode gesetzt */
#endif

  if (deskacc)
  {
    menu_id    = (acc_menu == NULL) ? FAILURE : menu_register (gl_apid, acc_menu); /* Accesory-MenÅ */
    class_desk = DESKWINDOW;                    /* Desktop im Fenster */

    if (menu_id < 0)
      while (TRUE) evnt_timer (0, 1);           /* Lasse andere Prozesse ran */
  } /* if */
  else
  {
    menu_id    = FAILURE;                       /* Kein Accesoory-MenÅ */
    class_desk = class;                         /* Desktop-Klasse */
  } /* else */

  scrp_read (scrapdir);                         /* Scrap-Directory lesen */

  if (*scrapdir == EOS)                         /* Noch keines gesetzt */
  {
    strcpy (scrapdir, SCRAPDIR);

#if GEMDOS
    drives = Dsetdrv (act_drv);                 /* Alle Laufwerke */

    if ((drives & 0xFFFFFFFCL) == 0L)
      drive = 0;                                /* Benutze Laufwerk A */
    else
    {
      drive = 2;                                /* Beginne bei Laufwerk C */
      ready = FALSE;

      while (! ready)
      {
        if (drives & (1L << drive))             /* Laufwerk gefunden */
          ready = TRUE;
        else
          drive++;
      } /* while */
    } /* else */

    scrapdir [0] = (BYTE)('A' + drive);
#endif

    scrp_write (scrapdir);                      /* Scrap-Directory setzen */
  } /* if */

  i = strlen (scrapdir);

  if (scrapdir [i - 1] != PATHSEP)
  {
    scrapdir [i]     = PATHSEP;
    scrapdir [i + 1] = EOS;
  } /* if */

  strcpy (s, scrapdir);

/* FÅhrt zu Problemen bei TOS 030, BD
  s [strlen (s) - 1] = EOS;                     /* Backslash lîschen */
*/

  if (! path_exist (scrapdir))
  {
#if GEMDOS
    ok = Dcreate (s) >= 0;                      /* Erzeuge Scrap-Directory */
#else
    ok = Dcreate (s);                           /* Erzeuge Scrap-Directory */
#endif
    if (! ok)
    {
      *scrapdir = EOS;
      scrp_write (scrapdir);                    /* Kein Scrap-Directory mîglich */
    } /* if */
  } /* if */

  return (TRUE);
} /* init_global */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_global ()

{
  if (gl_apid >= 0)
  {
    close_vwork ();                     /* Workstation schlieûen */
    appl_exit ();                       /* Applikation beenden */
  } /* if */

  return (TRUE);
} /* term_global */

