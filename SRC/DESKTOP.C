/*****************************************************************************/
/*                                                                           */
/* Modul: DESKTOP.C                                                          */
/* Datum: 02.02.95                                                           */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************
- drag_objs: alle Funktionen abgeschaltet, 02.02.95
- printer, edit, meta, clipboard, image und trash rausgenommen, 18.11.94
- wi_drag: alle Funktionen abgeschaltet
13.07.93
- help_desktop: ACC-Aufruf
- wi_objop: Hilfe auch fÅr nicht aktive Module per Icon-Text
- show_desktop: mabout
- Umbau von wi_objop fÅr inderekte window öbergabe
- Umstellung auf neue RTMCLASS Struktur
- Dummy Definitionen fÅr setup und status
*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"

#include "realtim4.h"
#include "realtspc.h"
#include "errors.h"

#include "clipbrd.h"
#include "dialog.h"
#include "disk.h"
#include "event.h"
#include "menu.h"
#include "printer.h"
#include "trash.h"

#include "objects.h"

#include "export.h"
#include "desktop.h"

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|INFO|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_RESIDENT)
#define XFAC   2                        /* X-Faktor */
#define YFAC   2                        /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  (2 * gl_wbox)            /* X-Anfangsposition */
#define INITY  (6 * gl_hbox)            /* Y-Anfangsposition */
#define INITW  (desk.x + desk.w -  6 * gl_wbox) /* Anfangsbreite in Pixel */
#define INITH  (desk.y + desk.h - 10 * gl_hbox) /* Anfangshîhe in Pixel */
#define MILLI  0                        /* Millisekunden fÅr Zeitablauf */

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

/****** FUNCTIONS ************************************************************/

LOCAL VOID    draw_dobj   _((WORD obj));
LOCAL BOOLEAN drag_react  _((WINDOWP src_window, WORD src_obj, WINDOWP dest_window, WORD dest_obj));
LOCAL VOID    drag_objs   _((WINDOWP window, WORD obj, SET objs));
LOCAL VOID    fill_select _((WINDOWP window, SET objs, RECT *area));
LOCAL VOID    invert_objs _((WINDOWP window, SET objs));
LOCAL VOID    rubber_objs _((WINDOWP window, MKINFO *mk));
LOCAL BOOLEAN in_icon     _((WORD mox, WORD moy, ICONBLK *icon, RECT *r));

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

GLOBAL WINDOWP find_desk ()
{
  return (search_window (class_desk, SRCH_ANY, NIL));
} /* find_desk */

/*****************************************************************************/

GLOBAL VOID get_dxywh (obj, border)
WORD obj;
RECT *border;

{
  get_border (find_desk (), obj, border);
} /* get_dxywh */

/*****************************************************************************/

GLOBAL VOID set_func (keys)
CONST BYTE *keys;

{
  WORD    num, p, obj;
  STRING  key;
  WINDOWP desk;
  OBJECT  *desktop;

  desk = find_desk ();

  if (desk != NULL)
  {
    desktop = desk->object;

    if (desktop != NULL)
      for (num = 1; num <= MAX_FUNC; num++)
      {
        obj = FKEYS + 3 * num;

        for (p = 0; (keys [p]) && (keys [p] != ','); p++);

        strncpy (key, keys, p);
        key [p] = EOS;
        if (strcmp (key, "$") == 0) get_ptext (desktop, obj, key); /* Alter Wert */
        strcat (key, "        ");
        key [8] = EOS;               /* Maximal 8 Zeichen */
        keys += p;
        if (*keys) keys++;

        set_ptext (desktop, obj, key);
        if (p != 0)
          undo_state (desktop, obj, DISABLED);
        else
          do_state (desktop, obj, DISABLED);
      } /* for, if */
  } /* if */
} /* set_func */

/*****************************************************************************/

GLOBAL VOID draw_func ()

{
  draw_dobj (FKEYS);
} /* draw_func */

/*****************************************************************************/

GLOBAL VOID draw_key (key)
WORD key;

{
  WINDOWP desk;
  OBJECT  *desktop;

  desk = find_desk ();

  if (desk != NULL)
  {
    desktop = desk->object;

    if (desktop != NULL)
      if (! (desktop [FKEYS].ob_flags & HIDETREE))
      {
        hide_mouse ();
        draw_dobj (FKEYS + key * 3 - 2);        /* Objektnummer der Box */
        show_mouse ();
      } /* if, if */
  } /* if */
} /* draw_key */

/*****************************************************************************/

GLOBAL VOID set_deskinfo (info, center)
CONST BYTE *info;
BOOLEAN    center;

{
  WINDOWP window;
  OBJECT  *desktop;
  LONGSTR infostr, s;
  WORD    width, inx;

  window = find_desk ();

  if (window != NULL)
  {
    width = desk.w / gl_wbox;

    if (width > STRLEN) width = STRLEN;         /* Infozeile hat 80 Zeichen */

    strcpy (infostr, info);
    infostr [width] = EOS;

    if (window->handle == DESK)                 /* RegulÑrer Desktop */
    {
      desktop = window->object;

      if (desktop != NULL)
      {
        mem_set (s, SP, width);
        inx = center ? ((width - strlen (infostr)) / 2) : 0;

        mem_move (s + inx, infostr, strlen (infostr));
        s [width] = EOS;
        set_ptext (desktop, DESKINFO, s);
        draw_dobj (DESKINFO);
      } /* if */
    } /* if */
    else                                        /* Desktop im Fenster */
    {
      infostr [STRLEN - 1] = EOS;               /* STRLEN - 1 wegen Leerzeichen */
      strcpy (window->info, " ");
      strcat (window->info, infostr);
      wind_set (window->handle, WF_INFO, ADR (window->info), 0, 0); /* Infozeile neu setzen */
    } /* else */
  } /* if */
} /* set_deskinfo */

/*****************************************************************************/

GLOBAL VOID set_meminfo ()

{
  WINDOWP desk;
  OBJECT  *desktop;
  STRING  s, t;
  LONG    avail;

#if GEMDOS
  WORD i;
  LONG size;
  VOID *adr [100];

  avail = 0;

  for (i = 0; (i < 100) && ((size = Mavail ()) > 0); i++)
  {
    adr [i]  = (VOID *)Malloc (size);
    avail   += size;
  }/* for */

  while (i > 0) Mfree (adr [--i]);
#else
  avail = mem_avail ();
#endif

  desk = find_desk ();

  if (desk != NULL)
  {
    desktop = desk->object;

    if ((desktop != NULL) && ! is_flags (desktop, FKEYS, HIDETREE))
    {
      sprintf (s, (BYTE *)freetext [FMEMORY].ob_spec, (WORD)(avail >> 10));
      get_ptext (desktop, IMEMORY, t);

      if (strcmp (s, t) != 0)             /* Es hat sich was getan */
      {
        set_ptext (desktop, IMEMORY, s);
        draw_dobj (INFOBOX);
      } /* if */
    } /* if */
  } /* if */
} /* set_meminfo */

/*****************************************************************************/

LOCAL VOID draw_dobj (obj)
WORD obj;

{
  draw_object (find_desk (), obj);
} /* draw_dobj */

/*****************************************************************************/

LOCAL BOOLEAN drag_react (src_window, src_obj, dest_window, dest_obj)
WINDOWP src_window;
WORD    src_obj;
WINDOWP dest_window;
WORD    dest_obj;

{
/*
  if ((src_obj == ICLIPBRD) || (dest_obj == ICLIPBRD))
    return (icons_clipbrd (src_obj, dest_obj));
  else
*/
    return (FALSE);
} /* drag_react */

/*****************************************************************************/

LOCAL VOID drag_objs (window, obj, objs)
WINDOWP window;
WORD    obj;
SET     objs;

{
#if TRUE
#else
  RECT    ob, r, bound;
  WORD    mox, moy;
  WORD    bytex, bytey;
  WORD    i, f, fh, result, num_objs;
  WORD    dest_obj, d_obj;
  WINDOWP dest_window;
  OBJECT  *desktop;
  SET     inv_objs, not_objs;
  BOOLEAN calcd;
  RECT    all [FKEYS - ITRASH];

  if (window != NULL)
  {
    desktop = window->object;

    if (desktop != NULL)
    {
      rc_copy (&window->scroll, &bound);

/*    f  = (window->doc.h - window->doc.y) * window->yfac - window->scroll.h;*/ /* doc.h gerade ! */
      f  = desk.h - window->doc.y * window->yfac - window->scroll.h;  /* Von unten fehlend */

      fh = 0;                                   /* Hîhe der Funktionstasten */
      if (! (desktop [FKEYS].ob_flags & HIDETREE)) fh = desktop [FKEYS].ob_height + 5;

      bound.h += min (f - fh - 4, 0);

      if (! (desktop [DESKINFO].ob_flags & HIDETREE))
      {
        bound.y += gl_hbox + 1;
        bound.h -= gl_hbox + 1;
      } /* if */

      setclr (inv_objs);
      for (i = ITRASH, num_objs = 0; i < FKEYS; i++)
      {
        setincl (inv_objs, i);
        if (setin (objs, i)) objc_rect (desktop, i, &all [num_objs++], FALSE);
      } /* for */

      setcpy (not_objs, objs);
      setnot (not_objs);
      setand (inv_objs, not_objs);

      set_mouse (FLAT_HAND, NULL);
      drag_boxes (num_objs, all, window, inv_objs, &r, &bound, 8, 8);
      last_mouse ();
      graf_mkstate (&mox, &moy, &result, &result);

      bytex = r.w / 8;                              /* X einrasten */
      if (r.w % 8 >= 4) bytex++;
      if (r.w % 8 <= -4) bytex--;
      r.w = bytex * 8;

      bytey = r.h / 8;                              /* Y einrasten */
      if (r.h % 8 >= 4) bytey++;
      if (r.h % 8 <= -4) bytey--;
      r.h = bytey * 8;

      calcd = FALSE;

      for (i = 0; i <= SETMAX; i++)
        if (setin (objs, i))
        {
          objc_rect (desktop, i, &ob, TRUE);

          result = drag_to_window (mox, moy, window, i, &dest_window, &d_obj);

          if (! calcd) dest_obj = d_obj;
          calcd = TRUE;                         /* Nur einmal ausrechnen */

          switch (result)
          {
            case DRAG_SWIND  : if (dest_obj == obj) dest_obj = ROOT; /* Auf gleiches Objekt */

                               if (dest_obj == ROOT)    /* Verschieben */
                               {
                                 if ((r.w != 0) || (r.h != 0))
                                 {
                                   desktop [i].ob_x += r.w;
                                   desktop [i].ob_y += r.h;

                                   do_flags (desktop, i, HIDETREE); /* Altes Objekt nicht zeichnen */
                                   redraw_window (window, &ob);
                                   undo_flags (desktop, i, HIDETREE);
                                   draw_dobj (i);
                                 } /* if */
                               } /* if */
                               else
                                 if (! drag_react (window, i, dest_window, dest_obj))
                                   graf_mbox (ob.w, ob.h, ob.x + r.w, ob.y + r.h, ob.x, ob.y); /* ZurÅckschnalzen */
                               break;
            case DRAG_SCLASS :
            case DRAG_NOWIND :
            case DRAG_NORCVR :
            case DRAG_NOACTN : graf_mbox (ob.w, ob.h, ob.x + r.w, ob.y + r.h, ob.x, ob.y); /* ZurÅckschnalzen */
                               break;
          } /* switch */
        } /* if, for */
    } /* if */
  } /* if */
#endif
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

  for (i = ITRASH; i < FKEYS; i++)
    if (! (window->object [i].ob_flags & HIDETREE))
    {
      get_border (window, i, &r);

      if (rc_intersect (area, &r))                                 /* Im Rechteck */
        if (rc_intersect (&window->scroll, &r)) setincl (objs, i); /* Im Workbereich */
    } /* if, for */
} /* fill_select */

/*****************************************************************************/

LOCAL VOID invert_objs (window, objs)
WINDOWP window;
SET     objs;

{
  REG WORD i;

  for (i = 0; i <= SETMAX; i++)
    if (setin (objs, i))
    {
      flip_state (window->object, i, SELECTED);
      draw_object (window, i);
    } /* if, for */
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
/* Schlieûe Fenster                                                          */
/*****************************************************************************/

LOCAL VOID wi_close (window)
WINDOWP window;

{
  if (! deskacc && (window->menu != NULL))      /* MenÅzeile im Desktop-Fenster */
  {
    done          = TRUE;                       /* Sonst keine MenÅzeile mehr */
    called_by [0] = EOS;                        /* Programm ganz beenden */
  } /* if */
} /* wi_close */

/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

LOCAL VOID wi_draw (window)
WINDOWP window;

{
  WORD pxy [4];

  if (window->object == NULL)
  {
    vswr_mode (vdi_handle, MD_REPLACE);         /* Modus = replace */
    vsf_interior (vdi_handle, FIS_PATTERN);     /* Muster */
    vsf_style (vdi_handle, 4);                  /* Desktop-Muster */
    vsf_color (vdi_handle, BLUE);               /* Farbe blau */
    vsf_perimeter (vdi_handle, FALSE);          /* Keine Umrandung */

    rect2array (&window->scroll, pxy);
    v_bar (vdi_handle, pxy);                    /* Scrollbereich mit Muster fÅllen */
  } /* if */
} /* wi_draw */

/*****************************************************************************/
/* Einrasten des Fensters                                                    */
/*****************************************************************************/

LOCAL VOID wi_snap (window, new, mode)
WINDOWP window;
RECT    *new;
WORD    mode;

{
  RECT r, diff;

  wind_get (window->handle, WF_CXYWH, &r.x, &r.y, &r.w, &r.h);

  diff.x = (new->x - r.x) & 0xFFF8;             /* Differenz berechnen */
  diff.y = (new->y - r.y) & 0xFFFE;

  new->x = r.x + diff.x;                        /* Byteposition */
  new->y = r.y + diff.y;                        /* Y immer gerade */
} /* wi_snap */

/*****************************************************************************/
/* Objektoperationen von Fenster                                             */
/*****************************************************************************/

LOCAL VOID wi_objop (window, objs, action)
WINDOWP window;
SET     objs;
WORD    action;

{
  WORD    i;
  BOOLEAN ok = FALSE;
  REG WORD rtmi;
  REG RTMCLASSP rtmmodule;

	/* Untersuche alle RTM-Module auf passendes Icon */
	 	
	for (rtmi = 0; rtmi < rtmtop && !ok; rtmi++)         	/* Untersuche alle Module */
	{
	 rtmmodule = rtmmodules [rtmi];
	
	 	if (setin (objs, rtmmodule->icon_number)) /* Icon angewÑhlt? */
		{
		   switch (action)
		   {
		   	/* rtmmodule->window, weil evtl. Desktop=window ist */
				case OBJ_OPEN :
					ok = (rtmmodule->open) (rtmmodule->icon_number);
					break;
				case OBJ_INFO :
					ok = (rtmmodule->info) (NULL, NIL);
					break;
				case OBJ_HELP :
					/* ok = (rtmmodule->help) (rtmmodule); */
					break;
			} /* switch */
		} /* if */
	} /* for */

	if (ok==FALSE) /* keine Modul-Funktion gefunden, "normale" MenÅabfrage */
	{
		for (i = 0; i < FKEYS; i++)
			if (setin (objs, i))
				switch (action)
				{
					case OBJ_OPEN :
						switch (i)
						{
							case IDISK    : ok = open_disk (i);     break;
							/*
							case ITRASH   : ok = open_trash (i);    break;
							case IPRINTER : ok = open_printer (i);  break;
							case ICLIPBRD : ok = open_clipbrd (i);  break;
							*/
						} /* switch */
						
						if (! ok) hndl_alert (ERR_NOOPEN);
						break;
					case OBJ_INFO :
						switch (i)
						{
							case IDISK    : ok = info_disk (NULL, i);    	break;
							/*
							case ITRASH   : ok = info_trash (NULL, i);   	break;
							case IPRINTER : ok = info_printer (NULL, i); 	break;
							case ICLIPBRD : ok = info_clipbrd (NULL, i); 	break;
							*/
						} /* switch */
						
						if (! ok) hndl_alert (ERR_NOINFO);
						break;
					case OBJ_HELP :
						switch (i)
						{
							case IDISK    : ok = help_disk (NULL, i);    	break;
							/*
							case ITRASH   : ok = help_trash (NULL, i);   	break;
							case IPRINTER : ok = help_printer (NULL, i); 	break;
							case ICLIPBRD : ok = help_clipbrd (NULL, i); 	break;
							*/
							default:
							/* Icon-Text als Hilfe-Referenz */
								ok = help_rtm (((ICONBLK*)desktop[i].ob_spec)->ib_ptext);
								break;
						} /* switch */
						
						if (! ok) hndl_alert (ERR_NOHELP);
					break;
				} /* switch, if, for */
	} /* ok */
	if ((window == sel_window) && (action == OBJ_OPEN)) unclick_window (window);
} /* wi_objop */

/*****************************************************************************/
/* Ziehen in das Fenster                                                     */
/*****************************************************************************/

LOCAL WORD wi_drag (src_window, src_obj, dest_window, dest_obj)
WINDOWP src_window;
WORD    src_obj;
WINDOWP dest_window;
WORD    dest_obj;

{

/*
  if ((dest_obj >= FKEYS) || (dest_obj == DESKINFO)) return (DRAG_NOACTN); /* Kein Ziehen auf Funktionstasten */
  if (src_window->handle == dest_window->handle) return (DRAG_SWIND); /* Im gleichen Fenster */
  if (src_window->class == dest_window->class) return (DRAG_SCLASS);  /* Gleiche Fensterart */
  if (src_window->class == CLASS_CLIPBRD)
    if ((ITRASH <= dest_obj) && (dest_obj < FKEYS)) return (DRAG_OK);
*/
  return (DRAG_NOACTN);
} /* wi_drag */

/*****************************************************************************/
/* Selektieren des Fensterinhalts                                            */
/*****************************************************************************/

LOCAL VOID wi_click (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  WORD   obj, func;
  SET    new_objs;
  OBJECT *desktop;
  RECT   r;

  desktop = window->object;

  if (desktop != NULL)
  {
    obj = objc_find (desktop, ROOT, 2, mk->mox, mk->moy); /* Nur 2 Ebenen */

    if (obj != NIL)
      if ((desktop [obj].ob_type & 0xFF) == G_ICON)
      {
        get_border (window, obj, &r);
        if (! in_icon (mk->mox, mk->moy, (ICONBLK *)desktop [obj].ob_spec, &r)) obj = NIL;
      } /* if */

    if (obj > FKEYS)                            /* Funktionstaste */
    {
      func = (obj - FKEYS - 1) / 3 + 1;         /* 3 Objekte pro Funktionstaste */
      obj  = FKEYS + func * 3;                  /* Inhalt der Funktionstaste */

      if (! (desktop [obj].ob_state & DISABLED))
      {
#if GEM & XGEM
        bstate         = 0x0001;                /* Maustaste kann gedrÅckt gehalten werden */
#else
        bclicks        = 0x0102;                /* Maustaste kann gedrÅckt gehalten werden */
#endif
        mk->ascii_code = 0;                     /* Funktionstasten haben keinen ASCII-Code */
        mk->scan_code  = F1 + func - 1;         /* Funktionstaste gedrÅckt */
        mk->kreturn    = (mk->scan_code << 8) | mk->ascii_code;

        key_window (window, mk);                /* Taste fÅr Fenster */
      } /* if */
    } /* if */
    else
    {
      if (sel_window != window) unclick_window (sel_window); /* Deselektieren */

      if ((desktop [obj].ob_type & 0xFF) == G_ICON)
      {
        setclr (new_objs);
        setincl (new_objs, obj);                /* Aktuelles Objekt */

        if (mk->shift)
        {
          invert_objs (window, new_objs);
          setxor (sel_objs, new_objs);
          if (! setin (sel_objs, obj)) obj = NIL; /* Wieder deselektiert */
        } /* if */
        else
        {
          if (! setin (sel_objs, obj))
          {
            unclick_window (window);            /* Alte Objekte lîschen */
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
          {
            if (window->objop != NULL)
              (*window->objop) (sel_window, sel_objs, OBJ_OPEN);
          } /* if */
        } /* if */
      } /* if */
      else
      {
        if (obj == DESKINFO)
          unclick_window (window);
        else
        {
          if (! (mk->shift || mk->ctrl)) unclick_window (window); /* Deselektieren */
          if ((mk->breturn == 1) && (mk->mobutton & 1))         /* Gummiband-Operation */
            rubber_objs (window, mk);
        } /* else */
      } /* else */
    } /* else */
  } /* if */
  else
    if (sel_window != window) unclick_window (sel_window); /* Deselektieren */
} /* wi_click */

/*****************************************************************************/

LOCAL VOID wi_unclick (window)
WINDOWP window;

{
  if (window->object != NULL) invert_objs (window, sel_objs);
} /* wi_unclick */

/*****************************************************************************/
/* Taste fÅr Fenster                                                         */
/*****************************************************************************/

LOCAL BOOLEAN wi_key (window, mk)
WINDOWP window;
MKINFO  *mk;

{
  WORD    func, obj, i;
  OBJECT  *desktop;
  WINDOWP wind;
  MKINFO  key;

  desktop = window->object;

  if (mk->scan_code == UNDO)            /* UNDO enspricht Control-Z */
  {
    mk->ascii_code = SUB;
    mk->ctrl       = TRUE;
  } /* if */

  if (desktop != NULL)
  {
    mem_move (&key, mk, sizeof (key));

    if (key.scan_code == HELP) key.scan_code = F1;

    if (! (key.shift || key.ctrl || key.alt) && (F1 <= key.scan_code) && (key.scan_code <= F10))
    {
      func = key.scan_code - F1 + 1;
      obj  = FKEYS + func * 3;          /* Objektnummer der Funktionstaste */

      if (! (desktop [obj].ob_state & DISABLED))
      {
        for (i = 0; i < 3; i++) do_state (desktop, obj - i, SELECTED);
        draw_key (func);

        i = key.scan_code - F1;

        if (funcmenus [i].item != 0)
        {
          wind = (window->menu == NULL) ? NULL : window;

          if (window->hndl_menu != NULL)
            (*window->hndl_menu) (wind, funcmenus [i].title, funcmenus [i].item);
        } /* if */

        for (i = 0; i < 3; i++) undo_state (desktop, obj - i, SELECTED);
        draw_key (func);
      } /* if */

      return (TRUE);
    } /* if */
  } /* if */

  if (menu_key (window, mk)) return (TRUE);

  return (FALSE);
} /* wi_key */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer (window)
WINDOWP window;
{

} /* wi_timer */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

GLOBAL WINDOWP crt_desktop (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
  WINDOWP window;
  WORD    kind_desk, menu_height;

  kind_desk = (class_desk == DESK) ? 0 : KIND;
  window    = create_window (kind_desk, class_desk);

  if (window != NULL)
  {
    menu_height = (menu != NULL) ? gl_hattr : 0;

    window->flags     = FLAGS;
    window->icon      = icon;
    window->doc.x     = 0;
    window->doc.y     = 0;
    window->doc.w     = desk.w / XFAC;
    window->doc.h     = desk.h / YFAC;
    window->xfac      = XFAC;
    window->yfac      = YFAC;
    window->xunits    = XUNITS;
    window->yunits    = YUNITS;

    if (window->class == DESK)                  /* RegulÑrer Desktop */
    {
      window->scroll.x = desk.x;
      window->scroll.y = desk.y;
      window->scroll.w = desk.w;
      window->scroll.h = desk.h;
    } /* if */
    else                                        /* Desktop im Fenster */
    {
      window->scroll.x = INITX - odd (desk.x);
      window->scroll.y = INITY - odd (desk.y);
      window->scroll.w = INITW;
      window->scroll.h = INITH;
    } /* else */

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
    window->hndl_menu = hndl_menu;              /* Globaler MenÅ-Handler */
    window->updt_menu = updt_menu;              /* Globaler MenÅ-Updater */
    window->test      = NULL;
    window->open      = NULL;
    window->close     = wi_close;
    window->delete    = NULL;
    window->draw      = wi_draw;
    window->arrow     = NULL;
    window->snap      = wi_snap;
    window->objop     = wi_objop;
    window->drag      = wi_drag;
    window->click     = wi_click;
    window->unclick   = wi_unclick;
    window->key       = wi_key;
    window->timer     = wi_timer;
    window->top       = NULL;
    window->untop     = NULL;
    window->edit      = NULL;
    window->showinfo  = info_desktop;
    window->showhelp  = help_desktop;

    if (class_desk != DESK)                     /* Desktop im Fenster */
      if (obj != NULL)                          /* Dokument angleichen */
      {                                         /* Muû grîûer werden */
        if (obj->ob_width  > desk.x + desk.w) window->doc.w = obj->ob_width / XFAC;
        if (obj->ob_height > desk.y + desk.h) window->doc.h = obj->ob_height / YFAC;
      } /* if, if */

    if (obj == NULL)
    {
      window->doc.w = 0;                        /* Immer groûe Slider zeigen */
      window->doc.h = 0;
    } /* if */
    else
      window->doc.y = window->doc.h - window->scroll.h / window->yfac; /* Unten positionieren */

    strcpy (window->name, (BYTE *)freetext [FDESKNAM].ob_spec); /* Name Deskfenster */
    strcpy (window->info, "");                                  /* Infozeile immer leer */
  } /* if */

  return (window);                              /* Fenster zurÅckgeben */
} /* crt_desktop */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN open_desktop (icon)
WORD icon;

{
  BOOLEAN ok;
  WINDOWP window;
  OBJECT  *w_menu;

  w_menu = NULL;

  if (class_desk == DESK)
  {
    if (desktop != NULL) do_flags (desktop, DESKINFO, HIDETREE);
  } /* if */
  else
    if (! menu_fits || deskacc) w_menu = menu;

  window = find_desk ();                /* Suche Desktop */

  if (window == NULL)
    window = crt_desktop (desktop, w_menu, icon);

  ok = window != NULL;

  if (ok)
    if (window->opened == 0)
    {
      setclr (menus);                   /* Zwinge Funktionstasten einzutragen */
      set_meminfo ();                   /* Speicher anzeigen */
      updt_menu (NULL);                 /* AnfangsmenÅ und Funktionstatsten */
      ok = open_window (window);        /* Desktop îffnen */
    } /* if */
    else
      top_window (window);              /* Bringe Desktop nach oben */

  return (ok);
} /* open_desktop */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

GLOBAL BOOLEAN info_desktop (window, icon)
WINDOWP window;
WORD    icon;

{

  WORD ret;
/*
  window = search_window (CLASS_DIALOG, SRCH_ANY, ABOUT);

  if (window == NULL)
  {
    form_center (about, &ret, &ret, &ret, &ret);
    window = crt_dialog (about, NULL, ABOUT, (BYTE *)freetext [FABOUT].ob_spec, WI_MODAL);
  } /* if */

  if (window != NULL)
    if (! open_dialog (ABOUT)) hndl_alert (ERR_NOOPEN);

*/
	mabout(MDESK);
	return (TRUE);
} /* info_desktop */

/*****************************************************************************/
/* Hilfe des Objekts                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN help_desktop (window, icon)
WINDOWP window;
WORD icon;

{
  return (help_rtm ("RTM4"));
} /* help_desktop */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_desktop ()

{
  return (TRUE);
} /* init_desktop */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_desktop ()

{
  return (TRUE);
} /* term_desktop */

