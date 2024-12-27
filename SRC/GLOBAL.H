/*****************************************************************************/
/*                                                                           */
/* Modul: GLOBAL.H                                                           */
/* Datum: 21/10/92   (orig)                                                  */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- new eingebaut, 18.12.94
- geÑndert: ASCII-Code fÅr SYN entfernt, BD 24.02.93                       
*****************************************************************************/

#ifndef __GLOBAL__
#define __GLOBAL__

/****** DEFINES **************************************************************/

#define MAX_FUNC     10                   /* Anzahl Funktionstasten */

#define NO_HANDLE   (-1)                  /* Wert fÅr ungÅltiges Handle */

#define DESK          0                   /* Window-Handle/Klasse des Desktop als Objekt */
#define DESKWINDOW    1                   /* Klasse des Desktops im einem Fenster */
#define MIN_WDESK   640                   /* Minimale Breite des Desktop */

#define SCREEN        1                   /* GEM-GerÑte */
#define PLOTTER      11
#define PRINTER      21
#define METAFILE     31
#define CAMERA       41
#define TABLET       51

#define NDC           0                   /* Normalisierte Koordinaten */
#define RC            2                   /* Raster Koordinaten */

#define FONT_SYSTEM   1                   /* GEM System Font */
#define FONT_SWISS    2                   /* Swiss Font */
#define FONT_DUTCH   14                   /* Dutch Font */

#define STRSIZE      82                   /* Stringgrîûe */
#define STRLEN       80                   /* StringlÑnge */
#define LONGSTRSIZE 256                   /* Lange Stringgrîûe */
#define LONGSTRLEN  255                   /* Lange StringlÑnge */
#define HALFSTRSIZE 128                   /* Halblange Stringgrîûe*/
#define HALFSTRLEN  127                   /* Halblange StringlÑnge */

#define SETSIZE     32                    /* Grîûe eines Sets in Longs */
#define SETMAX      (SETSIZE * 32 - 1)    /* Maximale Elemente eines Sets */

#define DRIVESEP    ':'                   /* Laufwerks-Trennzeichen */
#define PATHSEP     '\\'                  /* Pfad-Trennzeichen */
#define SUFFSEP     '.'                   /* Suffix-Trennzeichen */
#define PROGSEP     '/'                   /* Programm-Trennzeichen */
#define SCRAPDIR    "C:\\CLIPBRD\\"       /* Standard Clipboard-Pfad */

#define THEBAR      1                     /* Objekt-Nummer der MenÅzeile */
#define THEACTIVE   2                     /* Objekt-Nummer der aktiven MenÅs */
#define THEFIRST    3                     /* Objekt-Nummer des ersten MenÅs */

#define NUL         0x00                  /* ASCII Codes */
#define SOH         0x01
#define STX         0x02
#define ETX         0x03
#define EOT         0x04
#define ENQ         0x05
#define ACK         0x06
#define BEL         0x07
#define BS          0x08
#define HT          0x09
#define LF          0x0A
#define VT          0x0B
#define FF          0x0C
#define CR          0x0D
#define SO          0x0E
#define SI          0x0F
#define DLE         0x10
#define DC1         0x11
#define DC2         0x12
#define DC3         0x13
#define DC4         0x14
#define NAK         0x15
/* #define SYN         0x16 */
#define ETB         0x17
#define CAN         0x18
#define EM          0x19
#define SUB         0x1A
#define ESC         0x1B
#define FS          0x1C
#define GS          0x1D
#define RS          0x1E
#define US          0x1F
#define SP          0x20
#define DEL         0x7F

#define ESCAPE      0x01                  /* Scan Codes spezieller Tasten */
#define BACKSPACE   0x0E
#define TAB         0x0F
#define RETURN      0x1C
#define CLR_HOME    0x47
#define POS1        0x47
#define UP          0x48
#define PGUP        0x49
#define LEFT        0x4B
#define RIGHT       0x4D
#define ENDKEY      0x4F
#define DOWN        0x50
#define PGDOWN      0x51
#define INSERT      0x52
#define DELETE      0x53
#define UNDO        0x61
#define HELP        0x62
#define ENTER       0x72
#define CNTRL_LEFT  0x73
#define CNTRL_RIGHT 0x74
#define CNTRL_CLR   0x77
#define ALT_1       0x78
#define ALT_2       0x79
#define ALT_3       0x7A
#define ALT_4       0x7B
#define ALT_5       0x7C
#define ALT_6       0x7D
#define ALT_7       0x7E
#define ALT_8       0x7F
#define ALT_9       0x80
#define ALT_0       0x81

#define F1          0x3B                  /* Scan-Codes der Funktionstasten */
#define F2          0x3C
#define F3          0x3D
#define F4          0x3E
#define F5          0x3F
#define F6          0x40
#define F7          0x41
#define F8          0x42
#define F9          0x43
#define F10         0x44
#define F11         0x54
#define F12         0x55
#define F13         0x56
#define F14         0x57
#define F15         0x58
#define F16         0x59
#define F17         0x5A
#define F18         0x5B
#define F19         0x5C
#define F20         0x5D

#define UNDO_FLAG   0x0200                /* Undo-Flag im Object */
#define HELP_FLAG   0x0400                /* Help-Flag im Object */
#define NOECHO_FLAG 0x0800                /* Nicht-Echo-Flag im Objekt */

#define new(type) (type*) mem_alloc (sizeof(type))

/****** TYPES ****************************************************************/

typedef BYTE STRING [STRSIZE];            /* Zeichenkette */
typedef BYTE LONGSTR [LONGSTRSIZE];       /* Lange Zeichenkette */
typedef BYTE STR128 [HALFSTRSIZE];        /* Halblange Zeichenkette */
typedef ULONG SET [SETSIZE];              /* Menge */

typedef BYTE HUGE *HPTR;                  /* Huge Pointer */
typedef BYTE FAR  *FPTR;                  /* Far Pointer */

typedef struct
{
  WORD x;                                 /* X-Koordinate */
  WORD y;                                 /* Y-Koordinate */
  WORD w;                                 /* Breite */
  WORD h;                                 /* Hîhe */
} RECT;

typedef struct
{
  LONG x;                                 /* X-Koordinate */
  LONG y;                                 /* Y-Koordinate */
  LONG w;                                 /* Breite */
  LONG h;                                 /* Hîhe */
} LRECT;

typedef struct
{
  WORD    mox;                            /* Maus-X-Position */
  WORD    moy;                            /* Maus-Y-Position */
  WORD    momask;                         /* Mausmaske */
  WORD    mobutton;                       /* Mausknopf oben/unten */
  WORD    kstate;                         /* Shift-Tastatur-Status */
  UWORD   kreturn;                        /* Keycode von Event-Keyboard */
  WORD    breturn;                        /* Anzahl der Klicks */
  WORD    ascii_code;                     /* ASCII-Code der Taste */
  WORD    scan_code;                      /* Scan-Code der Taste */
  BOOLEAN shift;                          /* Shift-Taste betÑtigt ? */
  BOOLEAN ctrl;                           /* Control-Taste betÑtigt ? */
  BOOLEAN alt;                            /* Alternate-Taste betÑtigt ? */
} MKINFO;                                 /* Mouse-Key-Info */

typedef struct
{
  LONG dev_w;                             /* Breite des AusgabegrÑtes in Pixel */
  LONG dev_h;                             /* Hîhe des AusgabegerÑtes in Pixel */
  WORD pix_w;                             /* Breite eines Pixel in 1/1000 mm */
  WORD pix_h;                             /* Hîhe eines Pixel in 1/1000 mm */
} DEVINFO;

/****** VARIABLES ************************************************************/

#if DR_C | LASER_C | MW_C
EXTERN WORD    gl_apid;                   /* Identifikation fÅr Applikation */
#else
GLOBAL WORD    gl_apid;                   /* Identifikation fÅr Applikation */
#endif

#if GEM & (GEM1 | GEM2 | GEM3)
GLOBAL WORD    contrl [12];               /* GEM Arrays fÅr Parameter... */
GLOBAL WORD    intin [256];               /* ...von VDI Aufrufen */
GLOBAL WORD    ptsin [256];
GLOBAL WORD    intout [256];
GLOBAL WORD    ptsout [256];
#endif

GLOBAL WORD    gl_wchar;                  /* Breite eines Zeichens */
GLOBAL WORD    gl_hchar;                  /* Hîhe eines Zeichens */
GLOBAL WORD    gl_wbox;                   /* Breite einer Zeichenbox */
GLOBAL WORD    gl_hbox;                   /* Hîhe einer Zeichenbox */
GLOBAL WORD    gl_wattr;                  /* Breite eines Fensterattributes */
GLOBAL WORD    gl_hattr;                  /* Hîhe eines Fensterattributes */
GLOBAL WORD    gl_point;                  /* Punktgrîûe des Systemzeichensatzes */
GLOBAL WORD    colors;                    /* Anzahl der Farben */

GLOBAL WORD    phys_handle;               /* Physikalisches Workstation Handle */
GLOBAL WORD    vdi_handle;                /* Virtuelles Workstation Handle */
GLOBAL BOOLEAN deskacc;                   /* Als Accessory gestartet ? */
GLOBAL BOOLEAN acc_close;                 /* Letze Meldung war AC_CLOSE ? */
GLOBAL WORD    menu_id;                   /* Identifikation fÅr MenÅ als Accessory */
GLOBAL WORD    class_desk;                /* Desktop-Klasse (0 oder 1) */

GLOBAL RECT    desk;                      /* Desktop Grîûe */
GLOBAL RECT    clip;                      /* Letztes Clipping Rechteck */

GLOBAL WORD    hidden;                    /* Sichtbar-Status der Maus */
GLOBAL WORD    busy;                      /* Arbeits-Status der Maus */
GLOBAL WORD    mousenumber;               /* Aktuelle Mausform-Nummer */
GLOBAL MFORM   *mouseform;                /* Aktuelle Mausform */
GLOBAL BOOLEAN done;                      /* Ende gewÑhlt ? */
GLOBAL BOOLEAN ring_bell;                 /* Glocke eingeschaltet ? */
GLOBAL BOOLEAN grow_shrink;               /* Grow/Shrink eingeschaltet ? */
GLOBAL WORD    blinkrate;                 /* Blinkrate */
GLOBAL BOOLEAN updtmenu;                  /* MenÅ auf neuesten Stand bringen ? */
GLOBAL STR128  cmd;                       /* Programmname */
GLOBAL STR128  tail;                      /* Kommandozeile */
GLOBAL STR128  called_by;                 /* Name des aufrufenden Programms */
GLOBAL STR128  app_name;                  /* Name der Applikation */
GLOBAL STR128  app_path;                  /* Pfad der Applikation */
GLOBAL WORD    act_drv;                   /* Aktuelles Laufwerk beim Start */
GLOBAL STR128  act_path;                  /* Aktueller Pfad beim Start */
GLOBAL STR128  scrapdir;                  /* Clipboard-Pfad */

GLOBAL LONGSTR fs_path;                   /* Pfad der Dateiauswahl-Box */
GLOBAL STRING  fs_sel;                    /* Dateiname der Dateiauswahl-Box */
GLOBAL WORD    fs_button;                 /* Knopf der Dateiauswahl-Box */

GLOBAL OBJECT  *menu;                     /* MenÅ-Objekt-Baum */
GLOBAL OBJECT  *about;                    /* About-Objekt-Baum */
GLOBAL OBJECT  *desktop;                  /* Eigener Desktop */
GLOBAL OBJECT  *freetext;                 /* Freie Texte */

/* FrÅher: BYTE , BD */
GLOBAL OBJECT  **alertmsg;                /* Resource-Fehlermeldungen */
GLOBAL BYTE    **alerts;                  /* Zeiger auf Fehlermeldungen */

/****** FUNCTIONS ************************************************************/

GLOBAL VOID    open_vwork   _((VOID));
GLOBAL VOID    close_vwork  _((VOID));
GLOBAL WORD    open_work    _((WORD device, DEVINFO *dev_info));
GLOBAL VOID    close_work   _((WORD device, WORD out_handle));
GLOBAL BOOLEAN gdos_ok      _((VOID));

GLOBAL VOID    set_mouse    _((WORD number, MFORM *addr));
GLOBAL VOID    last_mouse   _((VOID));
GLOBAL VOID    hide_mouse   _((VOID));
GLOBAL VOID    show_mouse   _((VOID));
GLOBAL VOID    busy_mouse   _((VOID));
GLOBAL VOID    arrow_mouse  _((VOID));

GLOBAL VOID    do_state     _((OBJECT *tree, WORD obj, UWORD state));
GLOBAL VOID    undo_state   _((OBJECT *tree, WORD obj, UWORD state));
GLOBAL VOID    flip_state   _((OBJECT *tree, WORD obj, UWORD state));
GLOBAL WORD    find_state   _((OBJECT *tree, WORD obj, UWORD state));
GLOBAL BOOLEAN is_state     _((OBJECT *tree, WORD obj, UWORD state));

GLOBAL VOID    do_flags     _((OBJECT *tree, WORD obj, UWORD flags));
GLOBAL VOID    undo_flags   _((OBJECT *tree, WORD obj, UWORD flags));
GLOBAL VOID    flip_flags   _((OBJECT *tree, WORD obj, UWORD flags));
GLOBAL WORD    find_flags   _((OBJECT *tree, WORD obj, UWORD flags));
GLOBAL BOOLEAN is_flags     _((OBJECT *tree, WORD obj, UWORD flags));

GLOBAL WORD    find_type    _((OBJECT *tree, WORD obj, UWORD type));
GLOBAL VOID    set_checkbox _((OBJECT *tree, WORD obj, BOOLEAN selected));
GLOBAL BOOLEAN get_checkbox _((OBJECT *tree, WORD obj));
GLOBAL VOID    set_rbutton  _((OBJECT *tree, WORD obj, WORD lower, WORD upper));
GLOBAL WORD    get_rbutton  _((OBJECT *tree, WORD obj));
GLOBAL VOID    set_ptext    _((OBJECT *tree, WORD obj, BYTE *s));
GLOBAL VOID    get_ptext    _((OBJECT *tree, WORD obj, BYTE *s));
GLOBAL VOID    menu_check   _((OBJECT *tree, WORD obj, BOOLEAN checkit));
GLOBAL VOID    menu_enable  _((OBJECT *tree, WORD obj, BOOLEAN enableit));

GLOBAL VOID    objc_rect    _((OBJECT *tree, WORD obj, RECT *rect, BOOLEAN calc_border));
GLOBAL VOID    trans_gimage _((OBJECT *tree, WORD obj));

GLOBAL VOID    line_default _((WORD vdi_handle));
GLOBAL VOID    text_default _((WORD vdi_handle));
GLOBAL VOID    v_text       _((WORD vdi_handle, WORD x, WORD y, BYTE *string, WORD charwidth));

GLOBAL BOOLEAN background   _((OBJECT *tree, WORD obj, BOOLEAN get, MFDB *screen, MFDB *buffer));
GLOBAL BOOLEAN opendial     _((OBJECT *tree, BOOLEAN grow, RECT *size, MFDB *screen, MFDB *buffer));
GLOBAL BOOLEAN closedial    _((OBJECT *tree, BOOLEAN shrink, RECT *size, MFDB *screen, MFDB *buffer));
GLOBAL WORD    hndl_dial    _((OBJECT *tree, WORD def, BOOLEAN grow_shrink, BOOLEAN save_back, RECT *size, BOOLEAN *ok));
GLOBAL WORD    formdo       _((OBJECT *tree, WORD start));
GLOBAL VOID    blink        _((OBJECT *tree, WORD obj, WORD blinkrate));
GLOBAL WORD    popup_menu   _((OBJECT *tree, WORD obj, WORD x, WORD y, WORD center_obj, BOOLEAN relative, WORD bmsk));
GLOBAL BOOLEAN is_menu_key  _((OBJECT *menu, MKINFO *mk, WORD *title, WORD *item));
GLOBAL WORD    check_alt    _((OBJECT *tree, MKINFO *mk));
GLOBAL VOID    draw_oblines _((OBJECT *object));
GLOBAL VOID    draw_obline  _((OBJECT *object, WORD obj));

GLOBAL BOOLEAN rc_equal     _((CONST RECT *p1, CONST RECT *p2));
GLOBAL VOID    rc_copy      _((CONST RECT *ps, RECT *pd));
GLOBAL VOID    rc_union     _((CONST RECT *p1, RECT *p2));
GLOBAL BOOLEAN rc_intersect _((CONST RECT *p1, RECT *p2));
GLOBAL BOOLEAN inside       _((WORD x, WORD y, CONST RECT *r));
GLOBAL VOID    rect2array   _((CONST RECT *rect, WORD *array));
GLOBAL VOID    array2rect   _((CONST WORD *array, RECT *rect));
GLOBAL VOID    xywh2array   _((WORD x, WORD y, WORD w, WORD h, WORD *array));
GLOBAL VOID    array2xywh   _((CONST WORD *array, WORD *x, WORD *y, WORD *w, WORD *h));
GLOBAL VOID    xywh2rect    _((WORD x, WORD y, WORD w, WORD h, RECT *rect));
GLOBAL VOID    rect2xywh    _((CONST RECT *rect, WORD *x, WORD *y, WORD *w, WORD *h));
GLOBAL VOID    set_clip     _((BOOLEAN flag, CONST RECT *size));
GLOBAL VOID    growbox      _((CONST RECT *st, CONST RECT *fin));
GLOBAL VOID    shrinkbox    _((CONST RECT *fin, CONST RECT *st));

GLOBAL VOID    beep         _((VOID));
GLOBAL WORD    note         _((WORD button, WORD index, WORD helpinx, OBJECT *helptree));
GLOBAL WORD    error        _((WORD button, WORD index, WORD helpinx, OBJECT *helptree));

GLOBAL VOID    *mem_alloc   _((LONG mem));
GLOBAL VOID    mem_free     _((VOID *memptr));
GLOBAL LONG    mem_avail    _((VOID));
GLOBAL VOID    *mem_set     _((VOID *dest, WORD val, UWORD len));
GLOBAL VOID    *mem_move    _((VOID *dest, CONST VOID *src, UWORD len));
GLOBAL VOID    *mem_lset    _((VOID *dest, WORD val, ULONG len));
GLOBAL VOID    *mem_lmove   _((VOID *dest, CONST VOID *src, ULONG len));

GLOBAL BYTE    *str_upper   _((BYTE *s));
GLOBAL BYTE    *str_lower   _((BYTE *s));

GLOBAL VOID    setcpy       _((SET set1, SET set2));
GLOBAL VOID    setall       _((SET set));
GLOBAL VOID    setclr       _((SET set));
GLOBAL VOID    setnot       _((SET set));
GLOBAL VOID    setand       _((SET set1, SET set2));
GLOBAL VOID    setor        _((SET set1, SET set2));
GLOBAL VOID    setxor       _((SET set1, SET set2));
GLOBAL VOID    setincl      _((SET set, WORD elt));
GLOBAL VOID    setexcl      _((SET set, WORD elt));
GLOBAL BOOLEAN setin        _((SET set, WORD elt));
GLOBAL BOOLEAN setcmp       _((SET set1, SET set2));
GLOBAL WORD    setcard      _((SET set));

GLOBAL VOID    file_split   _((BYTE *fullname, WORD *drive, BYTE *path, BYTE *filename, BYTE *ext));
GLOBAL BOOLEAN get_path     _((BYTE *path, WORD drive));
GLOBAL BOOLEAN set_path     _((CONST BYTE *path));
GLOBAL BOOLEAN file_exist   _((CONST BYTE *filename));
GLOBAL BOOLEAN path_exist   _((CONST BYTE *pathname));
GLOBAL BOOLEAN select_file  _((BYTE *name, BYTE *path, BYTE *suffix, BYTE *label, BYTE *filename));

GLOBAL BOOLEAN init_global  _((INT argc, BYTE *argv [], BYTE *acc_menu, WORD class));
GLOBAL BOOLEAN term_global  _((VOID));

#endif /* __GLOBAL__ */

