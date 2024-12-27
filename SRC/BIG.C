/*****************************************************************************/
/*                                                                           */
/* Modul: BIG.C                                                              */
/*                                                                           */
/*****************************************************************************/

#define BIGVERSION "V 0.06"
#define BIGDATE "19.05.94"

/*****************************************************************************
V 0.06
- window->module eingebaut
- Aufruf von crt_mod in module
- Filter wieder eingebaut
*****************************************************************************/

#ifndef XRSC_CREATE
/*#define XRSC_CREATE TRUE*/                    /* X-Resource-File im Code */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "xrsrc.h"

#include "realtim4.h"
#include "big_mod.h"
#include "realtspc.h"
#include "var.h"

#include "errors.h"
#include "desktop.h"
#include "dialog.h"
#include "resource.h"

#include "objects.h"
#include "puf.h"

#include "msh_unit.h"
#include "msh.h"
#include "tra_mod.h"

#include "export.h"
#include "big.h"

#if XRSC_CREATE
#include "big_mod.rsh"
#include "big_mod.rh"
#endif

/****** DEFINES **************************************************************/

#define KIND   (NAME|CLOSER|FULLER|MOVER|INFO|SIZER|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE)
#define FLAGS  (WI_RESIDENT|WI_NOSCROLL)
#define XFAC   (1000/QUANT)                  /* X-Faktor */
#define YFAC   BORDERH                  /* Y-Faktor */
#define XUNITS 1                        /* X-Einheiten fÅr Scrolling */
#define YUNITS 1                        /* Y-Einheiten fÅr Scrolling */
#define INITX  ( 2 * gl_wbox)           /* X-Anfangsposition */
#define INITY  ( 6 * gl_hbox)           /* Y-Anfangsposition */
#define INITW  (60 * XFAC)           /* Anfangsbreite in Pixel */
#define INITH  (9 * YFAC)           /* Anfangshîhe in Pixel */
#define MILLI  100                     /* Millisekunden fÅr Zeitablauf */

#define MOD_RSC_NAME "BIG_MOD.RSC"		/* Name der Resource-Datei */

/* PARTS */

#define	GHOST	0x1L
#define PARENT	0x2L

/* VALUES */

#define	X_KOOR	0X1L
#define Y_KOOR	0X2L
#define	Z_KOOR	0X4L
#define	E_VOL	0X8L
#define	B_VOL	0X16L

#define BOXH	(gl_hbox + 4)
#define	BORDERW	(10 * gl_wbox)
#define	BORDERH	(gl_hbox + 4)
#define	RULERH	(gl_hbox + 4)
#define	MAXTRACKS	64
#define MAXINSTANCES 10

/****** TYPES ****************************************************************/

typedef struct _mod_data
{
	ULONG *next;
	ULONG *prev;
	CHAR data[1024];
} BIG_DATA;


typedef struct _mod_value
{
	CHAR	cookie[6];	/* VALU */
	ULONG	hsize;			/* Grîûe Header */
	ULONG	flags;			/* Div. Flags z. B. ob x,y,z, ... Wert */
	ULONG	datasize;		/* LÑnge Speicherblock */
	VOID	*data;			/* Ptr auf Speicherblock */
	UWORD	res[16];		/* Res. for future catastrophes */
} BIG_VALUES;


typedef struct _mod_track
{
	CHAR	cookie[6];	/* TRCK */
	ULONG	hsize;			/* Grîûe Header */
	ULONG	flags;			/* Div. Flags */
	BIG_VALUES	*val_x; /* X-K. gepackt */
	BIG_VALUES	*val_y;	/* Y-K. gep.	*/
	BIG_VALUES	*val_z;	/* Z-K. gep.	*/
	BIG_VALUES	*val_zm;	/* Zoom gep.	*/
	BIG_VALUES	*val_vol;	/* Event-Vol gep. */
	UWORD	res[16];				/* Res. for future catastrophes */
}	BIG_TRACK;


typedef struct _mod_part
{
	CHAR	cookie[6];	/* PART */
	ULONG	hsize;			/* Grîûe Header */
	ULONG flags;			/* Div. Flags z. B. GHOST */
	ULONG	smpte;			/* SMPTE-Startzeit */
	ULONG	tracks;			/* Bit-Vektor fÅr Tracks; 0 => Tracks 0 u. 1 */
	BIG_TRACK	*track0;	/* Datenstruktur Track */
	BIG_TRACK	*track1;	/* --- " --- */
	struct _mod_part	*link;		/* Zur Verwaltubg von Ghost-Parts */
	UWORD	res[16];			/* Res. for future catastrophes */
}	BIG_PART;

typedef struct _mod_volume
{
	ULONG	smpte;			/* SMPTE-Zeit */
	UCHAR vol;
} VOLUME;

typedef struct _mod_voldata
{
	ULONG *next;
	ULONG *prev;
	VOLUME data[1024];
} VOLDATA;

typedef struct _mod_volumelist
{
	VOLDATA *track[64];
} VOLUMELIST;

typedef	struct setup
{
	VOID *dummy;
} SETUP;

typedef struct setup *SET_P;

typedef	struct	win_status *WSTAT_P;

typedef	struct	win_status
{
	UINT	drawall		: 1;	/* Fenster komplett zeichnen */
	UINT	border		: 1;
	UINT	ruler			: 1;
	UINT	locon			: 1;
	LONG	leftposit;
	LONG	rightposit;
	LONG	locposit;
	INT		trackoff;
} WIN_STATUS;

typedef	struct status *STAT_P;

typedef	struct status
{
	UINT	play			: 1	;	/* PLAY gedrÅckt */
	UINT	record		: 1	;	/* RTM Record an/aus */
	UINT	puf_record	: 1	;	/* PUF Record an/aus */
	UINT	cycle			: 1	;	/* Cycle-Modus an/aus*/
	UINT	sync			: 1	;	/* synchron mitlaufen */
	UINT	note_off		: 1	;	/* Note-Off Events schicken */
	LONG	leftloc;					/* Linker Locator */
	LONG	rightloc;				/* Rechter Locator */
	LONG	posit;					/* SMPTE Zeit	*/
	BOOLEAN	pause;				/* Pause: Sequencer anhalten */
	WORD	channel1;				/* Ausgabekanal CMI 1*/
	WORD	channel2;				/* Ausgabekanal CMI 2*/
	WORD	port1;					/* Ausgabe-Anschluû CMI 1*/
	WORD	port2;					/* Ausgabe-Anschluû CMI 2*/
	TFilter filter;
	WSTAT_P	winstatus;
} STATUS;


/****** VARIABLES ************************************************************/
PRIVATE WORD	big_rsc_hdr;					/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE WORD	*big_rsc_ptr = &big_rsc_hdr;		/* Zeigerstruktur fÅr RSC-Datei */
PRIVATE OBJECT *big_setup;
PRIVATE OBJECT *big_help;
PRIVATE OBJECT *big_desk;
PRIVATE OBJECT *big_text;
PRIVATE OBJECT *big_info;
PRIVATE OBJECT *big_menu;

PRIVATE RTMCLASSP	modulep[MAXMSAPPLS];			/* Zeiger auf Modul-Strukturen */
PRIVATE WORD		refNum;		/* Referenznummer */

PRIVATE VOLUMELIST *volumelist;

/****** FUNCTIONS ************************************************************/

/* WINDOW-Funktionen */

PRIVATE BOOLEAN init_rsc		_((VOID));
PRIVATE BOOLEAN term_rsc		_((VOID));

PRIVATE VOID		handle_loc 		_((WINDOWP window));
PRIVATE VOID		draw_loc 		_((INT flag, INT *pxyarray));

PUBLIC VOID cdecl receive_evts_big (SHORT refNum);
PUBLIC VOID cdecl play_task_big 	(LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3);
PRIVATE VOID		InstallFilter				_((SHORT refNum));
PRIVATE SHORT		init_midishare (VOID);
PRIVATE VOID hide_loc _((WINDOWP window));
PRIVATE VOID show_loc _((WINDOWP window));

/*****************************************************************************/

PRIVATE BOOLEAN create_volumelist (RTMCLASSP module)
{
	int i;
	
	volumelist = (VOLUMELIST *)mem_alloc(sizeof(VOLUMELIST));
	if(!volumelist)
		return(FALSE);
		
	for(i = 0; i < 64; i++)
	{
		volumelist->track[i] = (VOLDATA *)mem_alloc(sizeof(VOLDATA));
		if(volumelist->track[i] == NULL)
			return(FALSE);
		volumelist->track[i]->data[0].vol = 127;
	}
	return(TRUE);
}


PUBLIC VOID		message	(RTMCLASSP module, WORD type, VOID *msg)
{
	UWORD		variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	STAT_P	status	= module->status;
	SET_P		akt		= module->actual->setup;
	WINDOWP	window	= module->window;

	switch(type)
	{
		case SET_VAR:			/* Record ein/ausschalten */
			switch (variable)
			{
				case VAR_BIG_PLAY:
					status->sync		= (BOOLEAN)value;
					break;
			} /* switch */
			window->milli = 1;
		break;
	} /* switch */
} /* message */

/*****************************************************************************/
/* MenÅbehandlung                                                            */
/*****************************************************************************/

PRIVATE VOID update_menu_mod (window)
WINDOWP window;

{
} /* update_menu */

/*****************************************************************************/

PRIVATE VOID handle_menu_mod (window, title, item)
WINDOWP window;
WORD    title, item;

{
RTMCLASSP	module = (RTMCLASSP)window->module;
STAT_P		status = module->status;
WSTAT_P		winstatus	= module->status->winstatus;

  if (window != NULL)
    menu_normal (window, title, FALSE);         /* Titel invers darstellen */

	switch (title)
	{
		case MBIGINFO:	
			switch(item)
			{
				case MBIGINFOANZEIGE:
					info_mod(window, NIL);
				break;
			}
			break;
		case MBIGANZEIG:
			switch(item)
			{
				case MBIGANZEIGINFO:
					(winstatus->border)?(winstatus->border = FALSE):(winstatus->border = TRUE);
					winstatus->drawall = TRUE;
					window->milli = 1;
				break;
			}
			break;
	}
  if (window != NULL)
    menu_normal (window, title, TRUE);          /* Titel wieder normal darstellen */
} /* handle_menu */


PRIVATE VOID wi_start_mod (window)
WINDOWP window;

{
RTMCLASSP	module = (RTMCLASSP)window->module;
STAT_P		status = module->status;
WSTAT_P		winstatus	= module->status->winstatus;

	if(winstatus->border) {
		window->scroll.x	+= BORDERW;
		window->scroll.w	-= BORDERW;
	}
	window->scroll.y	+= RULERH;
	window->scroll.h	-= RULERH;
}


PRIVATE VOID wi_finished_mod (window)
WINDOWP window;

{
RTMCLASSP	module = (RTMCLASSP)window->module;
STAT_P		status = module->status;
WSTAT_P		winstatus	= module->status->winstatus;

	if(winstatus->border) {
		window->scroll.x	-= BORDERW;
		window->scroll.w	+= BORDERW;
	}
	window->scroll.y	-= RULERH;
	window->scroll.h	+= RULERH;
}


/*****************************************************************************/
/* Zeichne Fensterinhalt                                                     */
/*****************************************************************************/

PRIVATE VOID wi_draw_mod (window)
WINDOWP window;

{
RTMCLASSP	module = (RTMCLASSP)window->module;
STAT_P		status = module->status;
WSTAT_P		winstatus	= module->status->winstatus;
BOOLEAN		new = winstatus->drawall || (window->flags & WI_JUNK);

INT 			pxyarray[4];
INT 			i,j,k;
BYTE			s[6];

	hide_loc(window);
	if(new) {
	  clr_scroll (window);
		line_default(vdi_handle);
		text_default(vdi_handle);

		vsl_color (vdi_handle, WHITE);
		vsf_interior(vdi_handle, FIS_SOLID);

		pxyarray[0]	= window->scroll.x - BORDERW;
		pxyarray[1]	=	window->scroll.y - BORDERH;
		pxyarray[2]	=	pxyarray[0] + window->scroll.w + BORDERW;
		pxyarray[3]	=	pxyarray[1] + RULERH;
		v_bar(vdi_handle, pxyarray);

		if(winstatus->border) {
			pxyarray[2]	=	pxyarray[0] + BORDERW;
			pxyarray[3]	=	pxyarray[1] + window->scroll.h + RULERH;
			v_bar(vdi_handle, pxyarray);
		}
		vsl_color (vdi_handle, BLACK);
		
		pxyarray[0] = window->scroll.x;
		pxyarray[2] = pxyarray[0] + window->scroll.w;
			vsl_type(vdi_handle, DOT);
			for(i = window->scroll.y, k = (window->doc.y * 2 + 1);
			 	((k < MAXTRACKS) && (i <= window->scroll.y + window->scroll.h));
			  	i += BORDERH, k+=2) {
				pxyarray[1] = pxyarray[3] = i;
				sprintf(s, "%2d", k);
				v_gtext(vdi_handle, pxyarray[0] - (2*gl_wbox) + 1, pxyarray[1] + 2, s);
				v_pline( vdi_handle, 2, pxyarray );
			}
		vsl_type(vdi_handle, SOLID);
		pxyarray[0] = pxyarray[2] = window->scroll.x;
		pxyarray[1] = window->scroll.y;
		pxyarray[3] = pxyarray[1] + window->scroll.h;
		v_pline( vdi_handle, 2, pxyarray );

		pxyarray[0] -= 2*gl_wbox;
		pxyarray[2] -= 2*gl_wbox;
		v_pline( vdi_handle, 2, pxyarray );

		vsl_type(vdi_handle, DOT);
		for(i = window->scroll.x + 5 * XFAC; i < window->scroll.x + window->scroll.w; i += 5 * XFAC) {
			pxyarray[0] = pxyarray[2] = i;
			v_pline(vdi_handle, 2, pxyarray);
		}
		
		winstatus->drawall = FALSE;

	} /* if WI_JUNK */

	show_loc(window);
} /* wi_draw_mod */

/*****************************************************************************/
/* Zeitablauf fÅr Fenster                                                    */
/*****************************************************************************/

PRIVATE VOID wi_timer_mod (window)
WINDOWP window;

{
	RTMCLASSP	module = (RTMCLASSP)window->module;
	STAT_P		status = module->status;
	WSTAT_P		winstatus	= module->status->winstatus;
	RECT			scroll;

	if(winstatus->border) {
		scroll.x	= window->scroll.x + BORDERW;
		scroll.w	= window->scroll.w - BORDERW;
	}
	scroll.y	= window->scroll.y - RULERH;
	scroll.h	= window->scroll.h + RULERH;

	scroll.x	= window->scroll.x;
	scroll.w	= window->scroll.w;
	scroll.y	= window->scroll.y;
	scroll.h	= window->scroll.h;

	redraw_window(window, &scroll);
	if(status->play)
		window->milli = MILLI;
	else
		window->milli = 0;
	
} /* wi_timer */

/*****************************************************************************/
/* Reagiere auf Pfeile                                                       */
/*****************************************************************************/

GLOBAL VOID wi_arrow_mod (window, dir, oldpos, newpos)
WINDOWP window;
WORD    dir;
LONG    oldpos, newpos;

{
	RTMCLASSP	module = (RTMCLASSP)window->module;
	WSTAT_P		winstatus	= module->status->winstatus;
  WORD w, h;
  LONG delta;

	winstatus->drawall = TRUE;

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
      	if(winstatus->trackoff > 0)
	      	winstatus->trackoff -= 1;
      } /* if */
      else                                             /* AbwÑrts-Scrolling */
      {
      	if(winstatus->trackoff  < MAXTRACKS)
	      	winstatus->trackoff += 1;
      } /* else */

      window->doc.y = newpos;                          /* Neue Position */

      set_sliders (window, VERTICAL, SLPOS);           /* Schieber setzen */
      scroll_window (window, VERTICAL, delta * window->yfac);
    } /* if */
  } /* else */
} /* wi_arrow_mod */

/*****************************************************************************/
/* Iconbehandlung                                                            */
/*****************************************************************************/

PUBLIC BOOLEAN icons_mod (src_obj, dest_obj)
WORD src_obj, dest_obj;

{
  BOOLEAN result;
  WINDOWP window;

  result = FALSE;

  switch (src_obj)
  {
    case IBIG : window = search_window (CLASS_BIG, SRCH_ANY, src_obj);
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
} /* icons_mod */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

PUBLIC BOOLEAN open_mod (icon)
WORD icon;

{
	BOOLEAN ok;
	WINDOWP window;
	WORD    ret;
	
	window = search_window (CLASS_BIG, SRCH_ANY, icon);
			
	if (window != NULL)
	{
		if (window->opened == 0)
		{	
			if (! open_window (window)) hndl_alert (ERR_NOOPEN);
		} /* if */
		else top_window (window);
	} /* if */
			
	ok= window !=0;
			
	return (ok);
} /* open_mod */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

PUBLIC BOOLEAN info_mod (window, icon)
WINDOWP window;
WORD    icon;

{
	WORD		ret;
	STRING	s;

	window = search_window (CLASS_DIALOG, SRCH_ANY, IBIG);
		
	if (window == NULL)
	{
		 form_center (big_info, &ret, &ret, &ret, &ret);
		 window = crt_dialog (big_info, NULL, IBIG, (BYTE *)big_text [FBIGN].ob_spec, WI_MODAL);
	} /* if */
		
	if (window != NULL)
	{
		window->object = big_info;
		strcpy(s, BIGDATE);
		set_ptext (big_info, BIGIVERDA, s);
		strcpy(s, __DATE__);
		set_ptext (big_info, BIGCOMPILE, s);
		strcpy(s, BIGVERSION);
		set_ptext (big_info, BIGIVERNR, s);
		/*
		sprintf(s, "%d", MAXBIGSETUPS);
		set_ptext (big_info, BIGISETUPS, s);
		sprintf(s, "%d", big_setup_nr);
		set_ptext (big_info, BIGIAKT, s);
		*/

		if (! open_dialog (IBIG)) hndl_alert (ERR_NOOPEN);
	}

	return (window != NULL);
} /* info_mod */


/*****************************************************************************/
/* Einrasten des Fensters                                                    */
/*****************************************************************************/

PRIVATE VOID wi_snap_mod (window, new, mode)
WINDOWP window;
RECT    *new;
WORD    mode;

{
  RECT r, diff;
  WORD wbox, hbox;
  LONG max_xdoc, max_ydoc;
  	RTMCLASSP	module = (RTMCLASSP)window->module;
	WSTAT_P		winstatus	= module->status->winstatus;

	winstatus->drawall = TRUE;
	
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
} /* wi_snap_mod */

/*****************************************************************************/

PRIVATE VOID wi_click_mod (WINDOWP window, MKINFO *mk)
{
	form_alert(1, "[1][Click][OK]");
} /* wi_click_mod */

/*****************************************************************************/

PRIVATE VOID wi_unclick_mod (window)
WINDOWP window;

{
	form_alert(1, "[1][UnClick][OK]");
} /* wi_unclick_mod */

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

PUBLIC WINDOWP crt_mod (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
	WINDOWP window;
	WORD    menu_height, inx;
	
	inx    = num_windows (CLASS_BIG, SRCH_ANY, NULL);
	window = create_window_obj (KIND, CLASS_BIG);
	
	if (window != NULL)
	{
		menu_height = (menu != NULL) ? gl_hattr : 0;
		
		window->flags     = FLAGS;
		window->icon      = icon;
		window->doc.x     = 0;
		window->doc.y     = 0;
		window->doc.w     = 1000;  /*INITW / XFAC;*/
		window->doc.h     = (MAXTRACKS / 2) * BORDERH / YFAC + 1;
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
		window->milli     = 0;
		window->object    = obj;
		window->menu      = menu;
		window->hndl_menu = handle_menu_mod;
		window->updt_menu = update_menu_mod;
		window->draw      = wi_draw_mod;
		window->arrow     = wi_arrow_mod;
		window->snap      = wi_snap_mod;
		window->click     = wi_click_mod;
		window->unclick   = wi_unclick_mod;
		window->timer     = wi_timer_mod;
		window->start			= wi_start_mod;
		window->finished	= wi_finished_mod;
		window->showinfo  = info_mod;
		
		strcpy (window->name, (BYTE *)big_text [FBIGN].ob_spec);
		sprintf (window->info, (BYTE *)big_text [FBIGI].ob_spec, 0);
	} /* if */
	
	return (window);                      /* Fenster zurÅckgeben */
} /* crt_mod */

/*****************************************************************************/
PRIVATE	RTMCLASSP create ()
{
	RTMCLASSP 	module;
	STRING		s;
	WSTAT_P		winstatus;
	WINDOWP		window;
	
	module = create_module ();
	
	if (module != NULL)
	{
		module->class_number		= CLASS_BIG;
		module->icon				= &big_desk[BIGICON];
		module->icon_position 	= IBIG;
		module->icon_number		= IBIG;	/* Soll bei Init vergeben werden */
		module->menu_title		= MWINDOWS;
		module->menu_position	= MBIG;
		module->menu_item			= MBIG;	/* Soll bei Init vergeben werden */
		module->multiple			= FALSE;
		
		module->crt					= crt_mod;

		module->open				= open_mod;
		module->info				= info_mod;
		module->icons				= icons_mod;

		module->init				= init_big;
		module->term				= term_mod;

		module->priority			= 50000L;
		module->object_type		= MODULE_OTHER;
		module->message			= message;

		refNum						= init_midishare ();	
		modulep[refNum]			= module;
		module->special			= (LONG)refNum;
		module->window 			= crt_mod (NULL, big_menu, IBIG);
		window = module->window;
		window->module 			= module;
		module->status				= (STATUS *)mem_alloc (sizeof(STATUS));
		module->status->winstatus  		= mem_alloc(sizeof(WIN_STATUS));
		winstatus					= module->status->winstatus;
		winstatus->drawall	= TRUE;
		winstatus->border		= TRUE;
		winstatus->leftposit		= 0;
		winstatus->rightposit	= (window->scroll.w - BORDERW) * (ULONG)QUANT;
		winstatus->locposit		= 0;
		winstatus->trackoff		= 0;
		winstatus->locon			= 0;
		if (refNum>0)
			InstallFilter(refNum);									/* Midi-Input-Filter einbauen */	
		add_rcv(VAR_SET_BIG,  module);	/* Message einklinken */
		add_rcv(VAR_BIG_PLAY, module);	/* Message einklinken */
		if(!create_volumelist(module))
			form_alert(1,"[1][Volumenliste|konnte nicht angelegt werden|Zuwenig Speicher frei][ Gnii ]");
	} /* if */
	
	return module;
} /* create */

/*****************************************************************************/

PRIVATE BOOLEAN init_rsc ()

{
  WORD   i, y, iconw, iconh, iconr;
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
  /*big_menu  = (OBJECT *)rs_trindex [BIG_SETUP]; /* Adresse des BIG-MenÅs */
  big_setup = (OBJECT *)rs_trindex [BIG_SETUP]; /* Adresse der BIG-Parameter-Box */
	*/
  big_help  = (OBJECT *)rs_trindex [BIG_HELP];	/* Adresse der BIG-Hilfe */
  big_desk  = (OBJECT *)rs_trindex [BIG_DESK];	/* Adresse des BIG-Desktops */
  big_text  = (OBJECT *)rs_trindex [BIG_TEXT];	/* Adresse der BIG-Texte */
  big_info 	= (OBJECT *)rs_trindex [BIG_INFO];	/* Adresse der BIG-Info-Anzeige */
  big_menu 	= (OBJECT *)rs_trindex [BIG_MENU];	/* Adresse des BIG-MenÅs */
#else

  strcpy (rsc_name, MOD_RSC_NAME);                  /* Einsetzen des Modul-Resource-Namens */

  if (! rs_load (big_rsc_ptr, rsc_name))
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

  rs_gaddr (big_rsc_ptr, R_TREE,  BIG_SETUP,	&big_menu);    /* Adresse des BIG-MenÅs */ 
  rs_gaddr (big_rsc_ptr, R_TREE,  BIG_SETUP,	&big_setup);   /* Adresse der BIG-Parameter-Box */
  rs_gaddr (big_rsc_ptr, R_TREE,  BIG_HELP,	&big_help);    /* Adresse der BIG-Hilfe */
  rs_gaddr (big_rsc_ptr, R_TREE,  BIG_DESK,	&big_desk);    /* Adresse des BIG-Desktop */
  rs_gaddr (big_rsc_ptr, R_TREE,  BIG_DESK,	&big_text);    /* Adresse der BIG-Texte */
  rs_gaddr (big_rsc_ptr, R_TREE,  BIG_INFO,	&big_info);    /* Adresse der BIG-Info-Anzeige */
  rs_gaddr (big_rsc_ptr, R_TREE,  BIG_MENU,	&big_menu; 	   /* Adresse der BIG-MenÅzeile */
#endif
#if XRSC_CREATE

for (i = 0; i < NUM_OBS; i++)
   xrsrc_obfix (&rs_object[i], 0);
#endif

	fix_objs (big_menu, TRUE);
	/* fix_objs (big_setup, TRUE); */
	fix_objs (big_help, TRUE);
	fix_objs (big_desk, TRUE);
	fix_objs (big_text, TRUE);
	fix_objs (big_info, TRUE);
	
	/*
	do_flags (big_setup, BIGCANCEL, UNDO_FLAG);
	do_flags (big_setup, BIGHELP, HELP_FLAG);
	*/
	menu_enable(menu, MBIG, TRUE);

	return (TRUE);
} /* init_rsc */

/*****************************************************************************/
/* RSC freigeben                                                      		  */
/*****************************************************************************/

PUBLIC BOOLEAN term_rsc ()

{
  BOOLEAN ok;

  ok = TRUE;

#if ((XRSC_CREATE|RSC_CREATE) == 0)
  ok = rs_free (big_rsc_ptr) != 0;               /* Resourcen freigeben */
#endif

  return (ok);
} /* term_rsc */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_big ()

{
	BOOLEAN		ok = TRUE;
	
	ok &= init_rsc ();
	ok &= (create () > 0);
	return (ok);
} /* init_big */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

PUBLIC BOOLEAN term_mod ()

{
  BOOLEAN ok = TRUE;
	ok &=term_rsc ();
	/* mem_free(big_setups); */
	return (ok);
} /* term_mod */


/*****************************************************************************/
/* MidiShare initialisieren                                                  */
/*****************************************************************************/

PRIVATE SHORT init_midishare ()
{
	/* Meldet ein neues Modul bei MidiShare an und gibt die refNum zurÅck */
	SHORT		ref, refNum = 0;			/* temporÑre Referenznummer */
	STRING	s;
	
	if (MidiShare())
	{
		refNum = MidiGetNamedAppl("RTM BIG");
		if (refNum > 0) MidiClose(refNum);
		refNum = MidiOpen("RTM BIG");				/* Applikation fÅr MidiShare îffnen	*/
	} /* if */

	if (refNum == 0)
		 hndl_alert (ERR_NOMIDISHARE);

	if (refNum == MIDIerrSpace)			/* PrÅfen genug Platz war */
	{
		 hndl_alert (ERR_MIDISHAREFULL);
	} /* if */

	if (refNum > 0)							/* PrÅfen ob alles klar */
	{
		MidiSetRcvAlarm(refNum, receive_evts_big);	/* Interrupt-Handler */		
		try_all_connect (refNum);
	} /* if */
	
	return refNum;
	
} /* init_midishare */


PUBLIC VOID cdecl receive_evts_big (SHORT refNum)
{
	MidiEvPtr	event;
	LONG 			n;
	INT 			r;
	MidiEvPtr	myTask;
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	WINDOWP		window = module->window;
	
	r = refNum;
	for (n = MidiCountEvs(r); n > 0; --n) 	/* Alle empfangenen Events abarbeiten */
	{
		event = MidiGetEv (r);				/*  Information holen */
		switch (EvType(event))
		{
		case typeRTMPosit:
				if (status->sync)
				{
					status->posit 		= get_posit((MidiSTPtr)event);
					/* posit(module, status->posit);    /* find actual position */ */
					window->milli = 1;
				} /* if */
				break;
			case typeRTMCont:
				if (status->sync)
				{
					status->posit 		= get_posit((MidiSTPtr)event);
					status->play = TRUE;
					myTask = MidiTask(play_task_big, MidiGetTime() + QUANT, refNum, 0, 0, 0);
					window->milli = 1;
				} /* if */
				break;
			case typeRTMStop:
				if (status->sync)
				{
					status->posit 		= get_posit((MidiSTPtr)event);
					status->play = FALSE;
					window->milli = 1;
				} /* if */
				break;
			case typeRTMCycleSet:
				status->leftloc 		= get_cycle_start((MidiSTPtr)event);
				status->rightloc		= get_cycle_end((MidiSTPtr)event);
				window->milli = 1;
				break;
			case typeRTMCycleOnOff:
				status->cycle 			= (UWORD) get_cycle_on((MidiSTPtr)event);
				window->milli = 1;
				break;
			case typeRTMRecordOnOff:
				status->record 		= (BOOLEAN) get_record_on((MidiSTPtr)event);
				window->milli = 1;
				break;
		} /* switch */
		MidiFreeEv (event);
	} /* for */
} /* receive_evts_big */

PUBLIC VOID cdecl receive_alarm_mod (SHORT refNum, LONG code)
{
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	WINDOWP		window = module->window;

	switch ((WORD)code)
	{
		case MIDISyncStart:
			status->sync			= TRUE;
			rtm_cont(refNum, MidiGetExtTime());
			window->milli			= 1;
			break;
		case MIDISyncStop:
			status->sync			= FALSE;
			rtm_stop(refNum, MidiGetExtTime());
			window->milli			= 1;
			break;
		case MIDIChangeSync:
			/*
			status->sync			= ~status->sync;
			window->milli			= 1;
			*/
			break;
	} /* switch */
} /* receive_alarm_mod */

/****************************************************************************
* 							InstallFilter						 *
*---------------------------------------------------------------------------*
* Cette procÇdure dÇfinit les valeurs du filtre de l'application. Un filtre *
* est composÇ de trois parties, qui sont trois tableaux de boolÇens :		 * 
* 															 *
*		un tableau de 256 bits pour les ports Midi acceptÇs			 *
*		un tableau de 256 bits pour les types d'ÇvÇnements acceptÇs		 *
*		un tableau de  16 bits pour les canaux Midi acceptÇs			 *
* 															 *
* Dans le code ci dessous, le filtre est paramÇtrÇ pour accepter n'importe	 *
* quel type d'ÇvÇnement. 										 *
* 															 *
* Les paramätres de l'appel :										 *
* ---------------------------										 *
* 															 *
*		aucun												 *
* 															 *
*****************************************************************************/

PRIVATE VOID InstallFilter (SHORT refNum)
{
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	TFilter		*filter = &status->filter;
	register		int i;

	for (i = 0; i<256; i++)
	{ 										
		/* Alle Inputs abschalten */
		AcceptBit(filter->evType,i);
		AcceptBit(filter->port,i);
	} /* for */
											
	for (i = 0; i<16; i++)
		AcceptBit(filter->channel,i);
		
	MidiSetFilter( refNum, filter );
} /* InstallFilter */

PUBLIC VOID cdecl play_task_big (LONG date, SHORT refNum, LONG a1, LONG a2, LONG a3)
{
	/* Wird soundso oft aufgerufen, um neue Daten in
		das Fenster einzublenden */

	MidiEvPtr	myTask;
	RTMCLASSP	module = modulep[refNum];
	STAT_P		status = module->status;
	WINDOWP		window = module->window;
	REG LONG		left 	= status->leftloc;
	REG LONG		right	= status->rightloc;
	REG LONG		posit;
		
	window->milli = 1; /* Updaten */

	/* Zeit weiterzÑhlen */
	status->posit += QUANT;
	posit = status->posit;

#if 0

	/* Cycle Restart */
	if ((status->cycle) && (posit >= right))
		if ((right > left)								/* Zw. Li und Re cyclen */
		 || ((left > right)	&& (posit < left)))	/* oder Åberspringen */
				rtm_pos(refNum, left);

	/* Punch In ? */
	if (status->punch_in && (posit >= left) && (posit <= right) && ~status->record) 
			rtm_record(refNum, TRUE);

	/* Punch Out ? */
	if (status->punch_out && (posit >= left) && (posit >= right) && status->record) 
			rtm_record(refNum, FALSE);

#endif

	/* Wenn weiterhin aufgenommen/gespielt werden soll, 
		muû der Task wieder eingeklinkt werden */
	if (status->play && status->sync)
		myTask = MidiTask(play_task_big, MidiGetTime() + QUANT, refNum, 0, 0, 0);

} /* play_task_big */

/*****************************************************************************/
/* Manage  Locator                                                           */
/*****************************************************************************/

PRIVATE VOID handle_loc (window)
WINDOWP window;

{
RTMCLASSP	module = (RTMCLASSP)window->module;
STAT_P		status = module->status;
WSTAT_P		winstatus	= module->status->winstatus;

INT 			pxyarray[4];
INT 			i,j,k;
ULONG 		posit = status->posit;
ULONG			leftposit, rightposit;
LONG 			ppos = posit / QUANT;

  vswr_mode (vdi_handle, MD_XOR);
  vsl_color (vdi_handle, BLACK);
  vsl_ends (vdi_handle, SQUARED, SQUARED);
  vsl_type (vdi_handle, DOT);
  vsl_width (vdi_handle, 1);

	leftposit = (ULONG)window->doc.x * 1000;
	rightposit = leftposit + (ULONG)(window->scroll.w - BORDERW) * QUANT;

	pxyarray[1] = window->scroll.y;
	pxyarray[3] = pxyarray[1] +  window->scroll.h - 1;

	if((winstatus->locposit >= leftposit) && (winstatus->locposit <= rightposit)) {
		pxyarray[0] = pxyarray[2] = (winstatus->locposit / QUANT) - (leftposit / QUANT) + window->scroll.x + BORDERW;
		draw_loc(1, pxyarray);
	}

	if((posit > leftposit) && (posit < rightposit)) {
		pxyarray[0] = pxyarray[2] = ppos - (leftposit / QUANT) + window->scroll.x + BORDERW;
		draw_loc(1, pxyarray);
		winstatus->locposit = posit;
	}
	
}


/*****************************************************************************/
/* Zeichne Locator                                                           */
/*****************************************************************************/

PRIVATE VOID draw_loc (flag, pxyarray)
INT flag;
INT *pxyarray;

{
INT 			pxy[4];

  vswr_mode (vdi_handle, MD_XOR);
  vsl_color (vdi_handle, BLACK);
  vsl_ends (vdi_handle, SQUARED, SQUARED);
  vsl_type (vdi_handle, DOT);
  vsl_width (vdi_handle, 1);


	v_pline(vdi_handle, 2, pxyarray);
	if(flag) {
		vsf_interior(vdi_handle, FIS_SOLID);
		pxy[0] = pxyarray[0] - 3;
		pxy[1] = pxyarray[1];
		pxy[2] = pxyarray[0] + 3;
		pxy[3] = pxy[1] + RULERH - 1;
		v_bar(vdi_handle, pxy);
	}
}


/*****************************************************************************/
/* Show  Locator                                                           */
/*****************************************************************************/

PRIVATE VOID hide_loc (window)
WINDOWP window;

{
RTMCLASSP	module = (RTMCLASSP)window->module;
STAT_P		status = module->status;
WSTAT_P		winstatus	= module->status->winstatus;

INT 			pxyarray[4];
INT 			i,j,k;
ULONG 		posit = status->posit;
ULONG			leftposit, rightposit;

  vswr_mode (vdi_handle, MD_XOR);
  vsl_color (vdi_handle, BLACK);
  vsl_ends (vdi_handle, SQUARED, SQUARED);
  vsl_type (vdi_handle, SOLID);
  vsl_width (vdi_handle, 1);

	leftposit = (ULONG)window->doc.x * 1000;
	rightposit = leftposit + (ULONG)(window->scroll.w) * QUANT;

	pxyarray[1] = window->scroll.y - RULERH;
	pxyarray[3] = pxyarray[1] +  window->scroll.h + RULERH- 1;

	if((winstatus->locon) && (winstatus->locposit >= leftposit) && (winstatus->locposit <= rightposit)) {
		pxyarray[0] = pxyarray[2] = winstatus->locposit / QUANT - leftposit / QUANT + window->scroll.x;
		draw_loc(1, pxyarray);
		winstatus->locon = FALSE;
	}
}

/*****************************************************************************/
/* Hide  Locator                                                           */
/*****************************************************************************/

PRIVATE VOID show_loc (window)
WINDOWP window;

{
RTMCLASSP	module = (RTMCLASSP)window->module;
STAT_P		status = module->status;
WSTAT_P		winstatus	= module->status->winstatus;

INT 			pxyarray[4];
INT 			i,j,k;
ULONG 		posit = status->posit;
ULONG			leftposit, rightposit;
LONG 			ppos = posit / QUANT;

  vswr_mode (vdi_handle, MD_XOR);
  vsl_color (vdi_handle, BLACK);
  vsl_ends (vdi_handle, SQUARED, SQUARED);
  vsl_type (vdi_handle, SOLID);
  vsl_width (vdi_handle, 1);

	leftposit = (ULONG)window->doc.x * 1000;
	rightposit = leftposit + (ULONG)(window->scroll.w) * QUANT;

	pxyarray[1] = window->scroll.y - RULERH;
	pxyarray[3] = pxyarray[1] +  window->scroll.h + RULERH - 1;

	if((!winstatus->locon) && (posit > leftposit) && (posit < rightposit)) {
		pxyarray[0] = pxyarray[2] = ppos - leftposit / QUANT + window->scroll.x;
		draw_loc(1, pxyarray);
		winstatus->locposit = posit;
		winstatus->locon = TRUE;
	}
	
}

