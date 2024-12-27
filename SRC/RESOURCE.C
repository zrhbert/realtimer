/*****************************************************************************/
/*                                                                           */
/* Modul: RESOURCE.C                                                         */
/* Datum: 14.01.94                                                           */
/* Umgestellt auf einzelne "groûe" (>64 KB) Resource-Dateien                 */
/*                                                                           */
/*****************************************************************************/

#ifndef RSC_CREATE
#define RSC_CREATE 0                    /* Resource-File im Code */
#endif

#ifndef XRSC_CREATE
#define XRSC_CREATE 1                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "xrsrc.h"

#include "realtim4.h"

#if RSC_CREATE
#include "rcm.h"
#include "realtim4.rsh"
                                /* In RSH-Dateien fehlt das letzte CR/LF */
BYTE      **rs_strings;			/* BD */
RS_IMDOPE *rs_imdope;			/* BD */
#endif

#if XRSC_CREATE
#include "realtim4.rsh"
#include "realtim4.rh"
#endif

#include "export.h"
#include "resource.h"

/****** DEFINES **************************************************************/

#ifndef RSC_NAME
#define RSC_NAME   "REALTIM4.RSC"          /* Name der Resource-Datei */
#endif

#define ALT_CHAR   '~'

/****** TYPES ****************************************************************/

typedef struct
{
  WORD bgc;
  WORD style;
  WORD interior;
  WORD bdc;
  WORD width;
  WORD chc;
} OBINFO;

/****** VARIABLES ************************************************************/

LOCAL USERBLK check_blk;                /* used for Macintosh check boxes */
LOCAL USERBLK radio_blk;                /* used for Macintosh radio buttons */

LOCAL WORD	rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
LOCAL WORD	*rsc_ptr = &rsc_hdr;
/****** FUNCTIONS ************************************************************/

#if MSDOS
EXTERN PARMBLK   *fardr_start  _((VOID));
EXTERN VOID      fardr_end     _((WORD state));
#endif

LOCAL VOID       get_obinfo    _((LONG obspec, OBINFO *obinfo));

#if MSDOS
LOCAL WORD       draw_checkbox _((VOID));
LOCAL WORD       draw_rbutton  _((VOID));
#else
LOCAL WORD CDECL draw_checkbox _((FAR PARMBLK *pb));
LOCAL WORD CDECL draw_rbutton  _((FAR PARMBLK *pb));
#endif

/*****************************************************************************/

LOCAL VOID get_obinfo (obspec, obinfo)
LONG   obspec;
OBINFO *obinfo;

{
  WORD colorwd;

  colorwd       = (WORD)obspec;
  obinfo->bgc   = colorwd & 0x0F;
  obinfo->style = (colorwd & 0x70) >> 4;

  if (obinfo->style == 0)
    obinfo->interior = 0;
  else
    if (obinfo->style == 7)
      obinfo->interior = 1;
    else
      obinfo->interior = (colorwd & 0x80) ? 3 : 2;

  obinfo->bdc   = (colorwd & 0xF000) >> 12;
  obinfo->width = (WORD)(obspec >> 16) & 0xFF;

  if (obinfo->width > 127) obinfo->width = 256 - obinfo->width;

  obinfo->chc = (colorwd & 0x0F00) >> 8;
} /* get_obinfo */

/*****************************************************************************/
/* Zeichnet ankreuzbare Buttons                                              */
/*****************************************************************************/

#if MSDOS
LOCAL WORD draw_checkbox ()
{
  PARMBLK *pb = fardr_start();
#else
LOCAL WORD CDECL draw_checkbox (pb)
FAR PARMBLK *pb;

{
#endif

  LONG    ob_spec;
  WORD    ob_x, ob_y;
  BOOLEAN selected;
  MFDB    s, d;
  BITBLK  *bitblk;
  WORD    obj;
  WORD    pxy [8];
  WORD    index [2];
  RECT    r;
  OBINFO  obinfo;

  ob_spec   = pb->pb_parm;
  ob_x      = pb->pb_x;
  ob_y      = pb->pb_y;
  selected  = pb->pb_currstate & SELECTED;

  get_obinfo (ob_spec, &obinfo);
  xywh2rect (pb->pb_xc, pb->pb_yc, pb->pb_wc, pb->pb_hc, &r);
  set_clip (TRUE, &r);

  if (selected) /* it was an objc_change */
    obj = (gl_hbox > 8) ? CBHSEL : CBLSEL;   /* high resolution : low resolution */
  else
    obj = (gl_hbox > 8) ? CBHNORM : CBLNORM; /* high resolution : low resolution */

  bitblk = (BITBLK *)userimg [obj].ob_spec;

  d.mp  = NULL; /* screen */
  s.mp  = (VOID *)bitblk->bi_pdata;
  s.fwp = bitblk->bi_wb << 3;
  s.fh  = bitblk->bi_hl;
  s.fww = bitblk->bi_wb >> 1;
  s.ff  = FALSE;
  s.np  = 1;

  pxy [0] = 0;
  pxy [1] = 0;
  pxy [2] = s.fwp - 1;
  pxy [3] = s.fh - 1;
  pxy [4] = ob_x;
  pxy [5] = ob_y;
  pxy [6] = ob_x + pxy [2];
  pxy [7] = ob_y + pxy [3];

  index [0] = obinfo.bgc;
  index [1] = WHITE;

  vrt_cpyfm (vdi_handle, MD_REPLACE, pxy, &s, &d, index);    /* copy it */

#if MSDOS
  fardr_end (pb->pb_currstate & ~ SELECTED);
#endif

  return (pb->pb_currstate & ~ SELECTED);
} /* draw_checkbox */

/*****************************************************************************/
/* Zeichnet runde Radiobuttons                                               */
/*****************************************************************************/

#if MSDOS
LOCAL WORD draw_rbutton ()
{
  PARMBLK *pb = fardr_start();
#else
LOCAL WORD CDECL draw_rbutton (pb)
FAR PARMBLK *pb;

{
#endif

  LONG    ob_spec;
  WORD    ob_x, ob_y;
  BOOLEAN selected;
  MFDB    s, d;
  BITBLK  *bitblk;
  WORD    obj;
  WORD    pxy [8];
  WORD    index [2];
  RECT    r;
  OBINFO  obinfo;

  ob_spec   = pb->pb_parm;
  ob_x      = pb->pb_x;
  ob_y      = pb->pb_y;
  selected  = pb->pb_currstate & SELECTED;

  get_obinfo (ob_spec, &obinfo);
  xywh2rect (pb->pb_xc, pb->pb_yc, pb->pb_wc, pb->pb_hc, &r);
  set_clip (TRUE, &r);

  if (selected) /* it was an objc_change */
    obj = (gl_hbox > 8) ? RBHSEL : RBLSEL; /* high resolution : low resolution */
  else
    obj = (gl_hbox > 8) ? RBHNORM : RBLNORM; /* high resolution : low resolution */

  bitblk = (BITBLK *)userimg [obj].ob_spec;

  d.mp  = NULL; /* screen */
  s.mp  = (VOID *)bitblk->bi_pdata;
  s.fwp = bitblk->bi_wb << 3;
  s.fh  = bitblk->bi_hl;
  s.fww = bitblk->bi_wb >> 1;
  s.ff  = FALSE;
  s.np  = 1;

  pxy [0] = 0;
  pxy [1] = 0;
  pxy [2] = s.fwp - 1;
  pxy [3] = s.fh - 1;
  pxy [4] = ob_x;
  pxy [5] = ob_y;
  pxy [6] = ob_x + pxy [2];
  pxy [7] = ob_y + pxy [3];

  index [0] = obinfo.bgc;
  index [1] = WHITE;

  vrt_cpyfm (vdi_handle, MD_REPLACE, pxy, &s, &d, index);    /* copy it */

#if MSDOS
  fardr_end (pb->pb_currstate & ~ SELECTED);
#endif

  return (pb->pb_currstate & ~ SELECTED);
} /* draw_rbutton */

/*****************************************************************************/

GLOBAL VOID fix_objs (tree, is_dialog)
OBJECT  *tree;
BOOLEAN is_dialog;

{
  WORD    obj;
  OBJECT  *ob;
  ICONBLK *ib;
  BITBLK  *bi;
  WORD    y_radio, h_radio;
  WORD    y_check, h_check;
  BOOLEAN hires;
  UWORD   type, i;
  OBINFO  obinfo;
  BYTE    *p;
#if GEM & (GEM2 | GEM3 | XGEM)
  BYTE    *s;
#endif

  hires   = (gl_hbox > 8) ? TRUE : FALSE;

  obj     = hires ? RBHNORM : RBLNORM;
  bi      = (BITBLK *)userimg [obj].ob_spec;
  h_radio = bi->bi_hl;
  y_radio = (gl_hbox - h_radio) / 2;

  obj     = hires ? CBHNORM : CBLNORM;
  bi      = (BITBLK *)userimg [obj].ob_spec;
  h_check = bi->bi_hl;
  y_check = (gl_hbox - h_check) / 2;

  check_blk.ub_code = draw_checkbox;
  radio_blk.ub_code = draw_rbutton;

  if (tree != NULL)
  {
#if GEM & (GEM2 | GEM3 | XGEM)
    if (is_dialog)
    {
      undo_state (tree, ROOT, SHADOWED);
      do_state (tree, ROOT, OUTLINED);
    } /* if */
#endif

    obj = NIL;

    do
    {
      ob   = &tree [++obj];
      type = ob->ob_type & 0xFF;

      get_obinfo (ob->ob_spec, &obinfo);

#if GEM & (GEM2 | GEM3 | XGEM)
      if ((type == G_STRING) && (ob->ob_state & DISABLED))
        for (s = (BYTE *)ob->ob_spec; *s; s++)
          if (*s == 0x13) *s = '-';
#endif

      if (is_dialog)
        if ((type == G_BUTTON) || (type == G_STRING))
        {
          p = (BYTE *)ob->ob_spec;

          if (strchr (p, ALT_CHAR) != NULL) /* alternate control char */
          {
            for (i = 0; p [i] != ALT_CHAR; i++);

            ob->ob_type |= (i + 1) << 8;    /* Position merken */
            if (type == G_STRING) ob->ob_width -= gl_wbox;
            strcpy (p + i, p + i + 1);      /* Zeichen rauslîschen */
          } /* if */
        } /* if, if */

      if (type == G_ICON)
      {
        ib = (ICONBLK *)ob->ob_spec;
        /*
        ob->ob_height = ib->ib_ytext + ib->ib_htext; /* Objekthîhe = Iconhîhe */
        */
        trans_gimage (tree, obj);         /* Icons an Bildschirm anpassen */
      } /* if */

      if (type == G_IMAGE)
      {
        bi = (BITBLK *)ob->ob_spec;
        ob->ob_height = bi->bi_hl;        /* Objekthîhe = Imagehîhe */
        trans_gimage (tree, obj);         /* Bit Images an Bildschirm anpassen */
      } /* if */

#if MSDOS | FLEXOS
      if (type == G_FBOXTEXT) ob->ob_height += 2;
#endif

      if (OB_STATE (tree, obj) & DRAW3D) ob->ob_y -= gl_hbox / 2;     /* group box */

      if (OB_STATE (tree, obj) & WHITEBAK)              /* half height box */
      {
        ob->ob_height -= gl_hbox / 2;
        if (! hires) ob->ob_height--;
      } /* if */

      if ((type == G_BOX) && (ob->ob_state & CROSSED))
      {
        ob->ob_state &= ~ CROSSED;

        if (ob->ob_flags & RBUTTON)                     /* radio button */
        {
          radio_blk.ub_parm  = (LONG)ob->ob_spec;
          ob->ob_y          += y_radio;
          ob->ob_height      = h_radio;
#if MSDOS | FLEXOS | DR_C | LATTICE_C | MW_C | TURBO_C
          ob->ob_type        = G_USERDEF;
          ob->ob_spec        = (LONG)&radio_blk;
#endif
        } /* if */
        else                                            /* checkbox */
        {
          check_blk.ub_parm  = (LONG)ob->ob_spec;
          ob->ob_y          += y_check;
          ob->ob_height      = h_check;
#if MSDOS | FLEXOS | DR_C | LATTICE_C | MW_C | TURBO_C
          ob->ob_type        = G_USERDEF;
          ob->ob_spec        = (LONG)&check_blk;
#endif
        } /* else */
      } /* if */
    } while (! (ob->ob_flags & LASTOB));
  } /* if */
} /* fix_objs */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_resource ()

{
  WORD   ret, i;
  STRING s, rsc_name;
#if GEM & (GEM2 | GEM3 | XGEM)
  BYTE   *p;
#endif

#if RSC_CREATE || XRSC_CREATE
#if RSC_CREATE
  rsc_create (gl_wbox, gl_hbox, NUM_TREE, NUM_OBS, NUM_FRSTR, NUM_FRIMG,
              rs_strings, rs_frstr, rs_bitblk, rs_frimg, rs_iconblk,
              rs_tedinfo, rs_object, (OBJECT **)rs_trindex, (RS_IMDOPE *)rs_imdope);
#endif
/*
  alertmsg = &rs_strings [FREESTR];             /* Adresse der Fehlermeldungen */
*/
  desktop  = (OBJECT *)rs_trindex [DESKTOP];    /* Adresse der MenÅzeile */
  menu     = (OBJECT *)rs_trindex [MENU];       /* Adresse der MenÅzeile */
  about    = (OBJECT *)rs_trindex [ABOUT];      /* Adresse der "About"-Box */
  freetext = (OBJECT *)rs_trindex [FREETEXT];   /* Adresse der Freestrings */
  popup    = (OBJECT *)rs_trindex [POPUP];      /* Adresse des Pop-Up-MenÅs */
  userimg  = (OBJECT *)rs_trindex [USERIMG];    /* Adresse der Mac-Images */
  icons    = (OBJECT *)rs_trindex [ICONS];      /* Adresse der Icons */
  settings = (OBJECT *)rs_trindex [SETTINGS];   /* Adresse der Einstellungen */
  settinghelp  = (OBJECT *)rs_trindex [SETTINGHELP];    /* Adresse der Einstellungshilfe */
  selfont  = (OBJECT *)rs_trindex [SELFONT];    /* Adresse der Schriftauswahl */
  fonthelp = (OBJECT *)rs_trindex [FONTHELP];   /* Adresse der Schriftauswahlhilfe */
  clipmenu = (OBJECT *)rs_trindex [CLIPMENU];   /* Adresse der Clipboard-MenÅzeile */
  infmeta  = (OBJECT *)rs_trindex [INFMETA];    /* Adresse der Meta-Infobox */
  alert    = (OBJECT *)rs_trindex [ALERT];      /* Adresse der Alertbox */
#else

#if GEM & GEM1
  strcpy (rsc_name, RSC_NAME);                  /* Programm wurde vielleicht mit Pexec gestartet */
#else
  strcpy (rsc_name, app_name);
  p = strrchr (rsc_name, SUFFSEP);              /* Programmname hat immer Suffix */
  strcpy (p + 1, "RSC");
#endif

  if (! rs_load (rsc_ptr, rsc_name))
  {
    strcpy (s, "[3][Resource-File|");
    strcat (s, rsc_name);
    strcat (s, "?][ EXIT ]");
    beep ();
    form_alert (1, s);
    if (! deskacc) return (FALSE);
    menu_unregister (gl_apid);                  /* Wieder abmelden */
    while (TRUE) evnt_timer (0, 1);             /* Lasse andere Prozesse ran */
  } /* if */

  rs_gaddr (rsc_ptr, R_FRSTR, ROOT,     &alertmsg);    /* Adresse der Fehlermeldungen */

  rs_gaddr (rsc_ptr, R_TREE,  MENU,     &menu);        /* Adresse der MenÅzeile */
  rs_gaddr (rsc_ptr, R_TREE,  ABOUT,    &about);       /* Adresse der "About"-Box */
  rs_gaddr (rsc_ptr, R_TREE,  DESKTOP,  &desktop);     /* Adresse des Desktop */
  rs_gaddr (rsc_ptr, R_TREE,  FREETEXT, &freetext);    /* Adresse der Freestrings */
  rs_gaddr (rsc_ptr, R_TREE,  POPUP,    &popup);       /* Adresse des Pop-Up-MenÅs */
  rs_gaddr (rsc_ptr, R_TREE,  USERIMG,  &userimg);     /* Adresse der Mac-Images */
  rs_gaddr (rsc_ptr, R_TREE,  ICONS,    &icons);       /* Adresse der Icons */
  rs_gaddr (rsc_ptr, R_TREE,  SETTINGS, &settings);    /* Adresse der Einstellungen */
  rs_gaddr (rsc_ptr, R_TREE,  SETTINGHELP,  &settinghelp);     /* Adresse der Einstellungshilfe */
  rs_gaddr (rsc_ptr, R_TREE,  SELFONT,  &selfont);     /* Adresse der Schriftauswahl */
  rs_gaddr (rsc_ptr, R_TREE,  FONTHELP, &fonthelp);    /* Adresse der Schriftauswahlhilfe */
  rs_gaddr (rsc_ptr, R_TREE,  CLIPMENU, &clipmenu);    /* Adresse der Clipboard-MenÅzeile */
  rs_gaddr (rsc_ptr, R_TREE,  INFMETA,  &infmeta);     /* Adresse der Meta-Infobox */
  rs_gaddr (rsc_ptr, R_TREE,  ALERT,    &alert);       /* Adresse der Alertbox */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);

#endif

  fix_objs (menu,     FALSE);
  fix_objs (about,    TRUE);
  fix_objs (desktop,  FALSE);
  fix_objs (freetext, FALSE);
  fix_objs (popup,    FALSE);
  fix_objs (userimg,  FALSE);
  fix_objs (icons,    FALSE);
  fix_objs (settings, TRUE);
  fix_objs (settinghelp,  TRUE);
  fix_objs (selfont,  TRUE);
  fix_objs (fonthelp, TRUE);
  fix_objs (clipmenu, FALSE);
  fix_objs (infmeta,  TRUE);
  fix_objs (alert,    TRUE);
  
  do_flags (settings, SETPASSWD, NOECHO_FLAG);
  do_flags (settings, SETCANCEL, UNDO_FLAG);
  do_flags (settings, SETHELP,   HELP_FLAG);

  do_flags (selfont, SFCANCEL, UNDO_FLAG);
  do_flags (selfont, SFHELP,   HELP_FLAG);


  if (desktop != NULL)                          /* Eigener Desktop */
  {
    mem_set (s, SP, STRLEN);                    /* Info-Zeile mit Leerzeichen fÅllen */
    s [STRLEN] = EOS;
    set_ptext (desktop, DESKINFO, s);

    desktop [DESKINFO].ob_y = desk.y;
    do_flags (desktop, DESKINFO, HIDETREE);

    if (desktop->ob_width < MIN_WDESK) desktop->ob_width = MIN_WDESK; /* FÅr niedrige Auflîsung */

    if (class_desk == DESK)
    {
      if ((desktop->ob_width > desk.x + desk.w) ||
          (desktop->ob_height > desk.y + desk.h)) class_desk = DESKWINDOW; /* Desktop im Fenster */
    } /* if */

#if GEMDOS
    if (class_desk == DESK)                     /* Desktop nicht im Fenster */
      if (colors == 4) desktop->ob_spec = 0x173L; /* GrÅner Desktop */
#endif

    if (desk.x + desk.w > desktop->ob_width)    /* Falls Desktop breiter Objektbreite */
      desktop->ob_width = desk.x + desk.w;      /* Groûe Bildschirme */
        
    if (desk.y + desk.h > desktop->ob_height)   /* Falls Desktop hîher Objekthîhe */
      desktop->ob_height = desk.y + desk.h;     /* Groûe Bildschirme */

    desktop [FKEYS].ob_y = desk.y + desk.h - desktop [FKEYS].ob_height - 5;

    if (desk.w > MIN_WDESK)                     /* FÅr groûe Bildschirme */
      desktop [FKEYS].ob_x += (desk.w - MIN_WDESK) / 2; /* Zentrieren */

	/* Schriftzug auf halbe Hîhe zw. Icons und MenÅleiste setzen */
	form_center (&desktop[RTMSCHRIFTZUG], &ret, &ret, &ret, &ret);
	/* desktop[RTMSCHRIFTZUG].ob_y = (desktop[ITRASH].ob_y - desktop[RTMSCHRIFTZUG].ob_height)/2; */
  } /* if */

  return (TRUE);
} /* init_resource */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_resource ()

{
  BOOLEAN ok;

  ok = TRUE;

#if ((XRSC_CREATE|RSC_CREATE) == 0)
  ok = rs_free (rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_resource */

