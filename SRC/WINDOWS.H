/*****************************************************************************/
/*                                                                           */
/* Modul: WINDOWS.H                                                          */
/* Datum: 31/10/93                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- WINDOWP: display_objects eingebaut
- WI_JUNK eingebaut
- WINDOWP: module eingebaut                                                 
- WINDOWP: finished eingebaut                                                 
*****************************************************************************/
#ifndef __WINDOWS__
#define __WINDOWS__

#include "lists.h"

/****** DEFINES **************************************************************/

#define MAX_GEMWIND 8                     /* Maximale Anzahl Fenster in GEM */

#define WI_NONE     0x0000                /* Keine Flags */
#define WI_FULLED   0x0001                /* Flag fÅr "Fenster auf voller Grîûe" */
#define WI_LOCKED   0x0002                /* Flag fÅr "Fenster gelockt" */
#define WI_FIRSTDRW 0x0004                /* Flag fÅr "Fenster erstesmal gezeichnet" */
#define WI_DLCLOSE  0x0008                /* Flag fÅr "Dialog-Fenster schlieûen" */
#define WI_ONTOP    0x0010                /* Flag fÅr "Fenster ist oben" */
#define WI_NOTOP    0x0020                /* Flag fÅr "Fenster darf nicht nach oben */
#define WI_RESIDENT 0x0040                /* Flag fÅr "Fenster resident" */
#define WI_MOUSE    0x0080                /* Flag fÅr "Eigene Mausform" */
#define WI_NOSCROLL 0x0100                /* Flag fÅr "Update statt Scrolling */
#define WI_MODAL    0x0200                /* Flag fÅr "Modales Dialog-Fenster" */
#define WI_MODELESS 0x0400                /* Flag fÅr "Nicht-Modales Dialog-Fenster" */
#define WI_CURSKEYS 0x0800                /* Flag fÅr "Cursor Tasten fÅr Scrolling" */
#define WI_MNSCROLL 0x1000                /* Flag fÅr "Scrollbare MenÅzeile */
#define WI_TOPMENU  0x2000                /* Flag fÅr "MenÅzeile soll nur oben funktionieren */
#define WI_JUNK	  0x4000						/* Flag fÅr "MÅll im Fenster, Scrollbereich neu aufbauen */

#define DRAG_OK     ( 0)                  /* Drag-Code fÅr "Funktion OK" */
#define DRAG_SWIND  (-1)                  /* Drag-Code fÅr "Gleiches Fenster" */
#define DRAG_SCLASS (-2)                  /* Drag-Code fÅr "Gleiche Art" */
#define DRAG_NOWIND (-3)                  /* Drag-Code fÅr "UngÅltiges Fenster" */
#define DRAG_NORCVR (-4)                  /* Drag-Code fÅr "Kein EmpfÑnger" */
#define DRAG_NOACTN (-5)                  /* Drag-Code fÅr "Keine EmpfÑnger-Aktion */

#define DO_UNDO     0                     /* Code fÅr "Undo" */
#define DO_CUT      1                     /* Code fÅr "Cut" */
#define DO_COPY     2                     /* Code fÅr "Copy" */
#define DO_PASTE    3                     /* Code fÅr "Paste" */
#define DO_CLEAR    4                     /* Code fÅr "Clear" */
#define DO_SELALL   5                     /* Code fÅr "Select all" */
#define DO_CLOSE    6                     /* Code fÅr "Fenster schlieûen" */
#define DO_DELETE   7                     /* Code fÅr "Fenster lîschen" */
#define DO_EXTERNAL 0x0100                /* Externe Operationen */

#define OBJ_OPEN    0                     /* Code fÅr "Objekt îffnen" */
#define OBJ_INFO    1                     /* Code fÅr "Info von Objekt" */
#define OBJ_HELP    2                     /* Code fÅr "Hilfe von Objekt" */

#define SRCH_CLOSED 0x01                  /* Search-Code fÅr geschlossene Fenster */
#define SRCH_OPENED 0x02                  /* Search-Code fÅr geîffnete Fenster */
#define SRCH_ANY    (SRCH_CLOSED | SRCH_OPENED) /* Search-Code fÅr jedes Fenster */
#define SRCH_SUB    0x04                  /* Search-Code fÅr Unterklasse */

#define MOVED       0x01                  /* FÅr Verschieben und Vergrîûern */
#define SIZED       0x02

#define HORIZONTAL  0x01                  /* FÅr Scrolling... */
#define VERTICAL    0x02                  /* ...und Schieber setzen */

#define SLPOS       0x01                  /* FÅr Schieber setzen */
#define SLSIZE      0x02

#define LIST_INIT   0x01                  /* Initialisiere Listbox */
#define LIST_DRAW   0x02                  /* Zeichne Listbox */
#define LIST_CLICK  0x04                  /* Klick in Listbox */

/****** TYPES ****************************************************************/

typedef struct window *WINDOWP;           /* Zeiger fÅr Parameter */

typedef struct window
{
  WORD    handle;                         /* Handle fÅr Fenster */
  WORD    opened;                         /* Wie oft wurde Fenster geîffnet ? */
  UWORD   flags;                          /* Flags des Fensters */
  UWORD   kind;                           /* Art des Fensters */
  WORD    class;                          /* Klasse des Fensters */
  WORD    subclass;                       /* Unterklasse des Fensters */
  WORD    icon;                           /* Objektnummer des Icons des Fensters */
  LRECT   doc;                            /* Position und Breite der Schieber/Dokumentes */
  WORD    xfac;                           /* X-Factor des Dokumentes */
  WORD    yfac;                           /* Y-Factor des Dokumentes */
  WORD    xunits;                         /* X-Scroll-Einheiten */
  WORD    yunits;                         /* Y-Scroll-Einheiten */
  RECT    scroll;                         /* Scrollbereich */
  RECT    work;                           /* Arbeitsbereich */
  WORD    bg_color;                       /* Hintergrund-Farbe */
  WORD    mousenum;                       /* Nummer der Mausform */
  MFORM   *mouseform;                     /* Mausform, falls mousenum = 255 */
  LONG    milli;                          /* Anzahl der Millisekunden */
  LONG    count;                          /* ZÑhler fÅr Millisekunden */
  LONG    special;                        /* FÅr speziellen Gebrauch */
  WORD    edit_obj;                       /* Aktuelles editiertes Objekt */
  WORD    edit_inx;                       /* Aktueller Index im editierten Objekt */
  WORD    exit_obj;                       /* Objekt, mit dem Box verlassen wurde */
  STRING  name;                           /* Name des Fensters */
  STRING  info;                           /* Infozeile des Fensters */
  OBJECT  *object;                        /* Objektbaum fÅr Fenster */
  OBJECT  *menu;                          /* MenÅzeile fÅr Fenster */
  WORD    first_menu;                     /* Erstes angezeigtes MenÅ */
  VOID 	 *module;								/* Modulzeiger fÅr RTM-Module */
  LIST_P	 dispobjs;								/* Eine Liste von Display-Objekten */
  VOID    (*updt_menu) _((WINDOWP window));             /* MenÅ auf neuen Stand bringen */
  VOID    (*hndl_menu) _((WINDOWP window, WORD title, WORD item)); /* MenÅ-Handler */
  BOOLEAN (*test)      _((WINDOWP window, WORD action));/* Test vor einer Aktion */
  VOID    (*open)      _((WINDOWP window));             /* Aktion vor dem ôffnen */
  VOID    (*close)     _((WINDOWP window));             /* Aktion nach dem Schlieûen */
  VOID    (*delete)    _((WINDOWP window));             /* Aktion nach dem Lîschen */
  VOID    (*draw)      _((WINDOWP window));             /* Zeichnen-Aktion */
  VOID    (*start)  	  _((WINDOWP window));             /* Wird vor redraw aufgerufen */
  VOID    (*finished)  _((WINDOWP window));             /* Wird nach redraw aufgerufen */
  VOID    (*arrow)     _((WINDOWP window, WORD dir, LONG oldpos, LONG newpos)); /* Pfeil-Aktion */
  VOID    (*snap)      _((WINDOWP window, RECT *new, WORD mode));  /* Schnapp-Aktion */
  VOID    (*objop)     _((WINDOWP window, SET objs, WORD action)); /* Objekt-Operationen im Fenster */
  WORD    (*drag)      _((WINDOWP src_window, WORD src_obj, WINDOWP dest_window, WORD dest_obj)); /* Ziehen von Objekten */
  VOID    (*click)     _((WINDOWP window, MKINFO *mk)); /* Klick-Aktion (selektieren) */
  VOID    (*unclick)   _((WINDOWP window));             /* Klick-Aktion (deselektieren) */
  BOOLEAN (*key)       _((WINDOWP window, MKINFO *mk)); /* Tastatur-Aktion */
  VOID    (*timer)     _((WINDOWP window));             /* Aktion nach Ablaufen einer Zeitspanne */
  VOID    (*top)       _((WINDOWP window));             /* Aktion nach Top */
  VOID    (*untop)     _((WINDOWP window));             /* Aktion vor Untop */
  VOID    (*edit)      _((WINDOWP window, WORD action));/* Cut/Copy/Paste */
  BOOLEAN (*showinfo)  _((WINDOWP window, WORD icon));  /* Information des Fensters eines Moduls */
  BOOLEAN (*showhelp)  _((WINDOWP window, WORD icon));  /* Hilfe des Fensters eines Moduls */
} WINDOW;

typedef struct
{
  WINDOWP window;               /* Fenster der Listbox */
  OBJECT  *tree;                /* Objektbaum der Listbox */
  VOID    *itemlist;            /* Liste der Structs aller EintrÑge */
  SIZE_T  itemsize;             /* Grîûe eines Elements */
  BOOLEAN indirect;             /* Elemente sind Zeiger auf Zeichenketten */
  WORD    num_items;            /* Anzahl verfÅgbarer EintrÑge */
  WORD    vis_items;            /* Anzahl sichtbarer EintrÑge  */
  WORD    width;                /* Breite eines Eintrags */
  WORD    first_item;           /* Erster Eintrag in Listbox */
  WORD    active;               /* Aktiver Eintrag in Listbox */
  UWORD   sel_state;            /* Status, mit dem selektiert werden soll */
  WORD    root;                 /* Objektnummer der Listbox */
  WORD    items;                /* Objektnummer der Box mit EintrÑgen */
  WORD    up;                   /* Objektnummer des Hoch-Pfeils */
  WORD    down;                 /* Objektnummer des Unten-Pfeils */
  WORD    parent;               /* Objektnummer des Elternteils des Schiebers */
  WORD    slider;               /* Objektnummer des Schiebers */
} LISTBOX;

/****** VARIABLES ************************************************************/

GLOBAL WINDOWP sel_window;      /* Zeiger auf selektiertes Fenster */
GLOBAL SET     sel_objs;        /* Menge selektierter Objekte */

/****** FUNCTIONS ************************************************************/

GLOBAL WINDOWP search_window  _((WORD class, WORD mode, WORD icon));
GLOBAL WINDOWP find_window    _((WORD wh));
GLOBAL WINDOWP find_top       _((VOID));
GLOBAL BOOLEAN is_top         _((WINDOWP window));
GLOBAL BOOLEAN any_open       _((BOOLEAN incl_desk, BOOLEAN incl_closer, BOOLEAN incl_modal));
GLOBAL WORD    num_windows    _((WORD class, WORD mode, WINDOWP winds []));
GLOBAL WORD    num_locked     _((VOID));

GLOBAL WINDOWP create_window  _((UWORD kind, WORD class));
GLOBAL VOID    delete_window  _((WINDOWP window));
GLOBAL BOOLEAN open_window    _((WINDOWP window));
GLOBAL VOID    close_window   _((WINDOWP window));
GLOBAL VOID    close_top      _((VOID));
GLOBAL VOID    close_all      _((BOOLEAN delete, BOOLEAN close_desk));
GLOBAL VOID    draw_window    _((WINDOWP window));
GLOBAL VOID    redraw_window  _((WINDOWP window, CONST RECT *area));
GLOBAL VOID    top_window     _((WINDOWP window));
GLOBAL VOID    untop_window   _((WINDOWP window));
GLOBAL VOID    cycle_window   _((VOID));
GLOBAL VOID    scroll_window  _((WINDOWP window, WORD dir, LONG delta));
GLOBAL VOID    arrow_window   _((WINDOWP window, WORD arrow, WORD amount));
GLOBAL VOID    h_slider       _((WINDOWP window, WORD new_value));
GLOBAL VOID    v_slider       _((WINDOWP window, WORD new_value));
GLOBAL VOID    set_sliders    _((WINDOWP window, WORD which, WORD mode));
GLOBAL VOID    snap_window    _((WINDOWP window, RECT *new, WORD mode));
GLOBAL VOID    full_window    _((WINDOWP window));
GLOBAL VOID    size_window    _((WINDOWP window, CONST RECT *new));
GLOBAL VOID    move_window    _((WINDOWP window, CONST RECT *new));
GLOBAL WORD    drag_to_window _((WORD mox, WORD moy, WINDOWP src_window, WORD src_obj, WINDOWP *dest_window, WORD *dest_obj));
GLOBAL VOID    click_window   _((WINDOWP window, MKINFO *mk));
GLOBAL VOID    unclick_window _((WINDOWP window));
GLOBAL BOOLEAN key_window     _((WINDOWP window, MKINFO *mk));
GLOBAL BOOLEAN key_all        _((MKINFO *mk));
GLOBAL VOID    timer_window   _((WINDOWP window));
GLOBAL VOID    timer_all      _((LONG milli));

GLOBAL VOID    get_border     _((WINDOWP window, WORD obj, RECT *border));
GLOBAL VOID    draw_object    _((WINDOWP window, WORD obj));
GLOBAL VOID    set_cursor     _((WINDOWP window, WORD obj, WORD inx));
GLOBAL VOID    drag_boxes     _((WORD num_objs, CONST RECT *boxes, WINDOWP inv_window, SET inv_objs, RECT *diff, CONST RECT *bound, WORD x_raster, WORD y_raster));
GLOBAL VOID    draw_listobj   _((LISTBOX *list, WORD obj, BOOLEAN flip));
GLOBAL BOOLEAN listbox        _((LISTBOX *list, UWORD flags, MKINFO *mk));
GLOBAL VOID    edit_noecho    _((MKINFO *mk, WORD cursor, BYTE *s, WORD maxlen));

GLOBAL VOID    scroll_area    _((CONST RECT *area, WORD dir, WORD delta));
GLOBAL VOID    clr_area       _((CONST RECT *area));
GLOBAL VOID    clr_work       _((WINDOWP window));
GLOBAL VOID    clr_scroll     _((WINDOWP window));
GLOBAL VOID    clr_left       _((WINDOWP window));
GLOBAL VOID    clr_top        _((WINDOWP window));
GLOBAL VOID    clr_right      _((WINDOWP window));
GLOBAL VOID    clr_bottom     _((WINDOWP window));
GLOBAL VOID    set_redraw     _((WINDOWP window, CONST RECT *area));

GLOBAL VOID    draw_mtitle    _((WINDOWP window, WORD title));
GLOBAL VOID    draw_mbar      _((WINDOWP window));
GLOBAL VOID    menu_normal    _((WINDOWP window, WORD title, BOOLEAN normal));
GLOBAL BOOLEAN menu_manager   _((WINDOWP window, WORD mox, WORD moy, WORD mobutton, WORD breturn));
GLOBAL BOOLEAN menu_key       _((WINDOWP window, MKINFO *mk));

GLOBAL BOOLEAN init_windows   _((WORD err_nowindow, WORD maxreswind, WORD class_help));
GLOBAL BOOLEAN term_windows   _((VOID));

#endif /* __WINDOWS__ */

