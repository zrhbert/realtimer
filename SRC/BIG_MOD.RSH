/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "BIG_MOD.H"

#if !defined(WHITEBAK)
#define WHITEBAK    0x0040
#endif
#if !defined(DRAW3D)
#define DRAW3D      0x0080
#endif

#define FLAGS9  0x0200
#define FLAGS10 0x0400
#define FLAGS11 0x0800
#define FLAGS12 0x1000
#define FLAGS13 0x2000
#define FLAGS14 0x4000
#define FLAGS15 0x8000
#define STATE8  0x0100
#define STATE9  0x0200
#define STATE10 0x0400
#define STATE11 0x0800
#define STATE12 0x1000
#define STATE13 0x2000
#define STATE14 0x4000
#define STATE15 0x8000

static TEDINFO rs_tedinfo[] =
{ "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "BIG Big-Sequencer",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 18, 1,
  "1992 by Cybernetic Arts",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 24, 1,
  "Programm: Bertram Dunskus",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 26, 1,
  "& Detlef Woltmann",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 18, 1,
  "01234567890",
  "Version: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 21,
  "01234567890",
  "Datum: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 19,
  " 12.12.1992",
  "compiliert: ___________",
  "XXXXXXXXXXX",
  SMALL, 0, TE_LEFT , 0x1180, 0, -1, 12, 24,
  "01234567890",
  "Setups: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 20,
  "01234567890",
  "akt. Set: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 22
};

static RSIB0MASK[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x3FFF, 0xFFF0, 0x3FFF, 0xFFF0, 
  0x2000, 0x0030, 0x2BF9, 0xFC30, 
  0x23F9, 0xFC30, 0x2BF9, 0xFC30, 
  0x2000, 0x0030, 0x287F, 0xF030, 
  0x207F, 0xF030, 0x287F, 0xF030, 
  0x2000, 0x0030, 0x29F8, 0x0FB0, 
  0x21F8, 0x0FB0, 0x29F8, 0x0FB0, 
  0x2000, 0x0030, 0x28FD, 0xE030, 
  0x20FD, 0xE030, 0x28FD, 0xE030, 
  0x2000, 0x0030, 0x29FF, 0xFFB0, 
  0x21FF, 0xFFB0, 0x29FF, 0xFFB0, 
  0x2000, 0x0030, 0x3FFF, 0xFFF0, 
  0x3FFF, 0xFFF0, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "Biq Seq", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  75,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 BIG_MENU ****************************************************/
        -1,        1,        6, G_IBOX    ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0000, 0x0050, 0x0019,
         6,        2,        2, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0050, 0x0201,
         1, MBIGINFO, MBIGANZEIG, G_IBOX    ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0002, 0x0000, 0x0016, 0x0301,
  MBIGEDIT,       -1,       -1, G_TITLE   ,   /* Object 3 MBIGINFO */
  NONE, NORMAL, (LONG)" Info",
  0x0000, 0x0000, 0x0006, 0x0301,
  MBIGANZEIG,       -1,       -1, G_TITLE   ,   /* Object 4 MBIGEDIT */
  NONE, NORMAL, (LONG)" Edit",
  0x0006, 0x0000, 0x0006, 0x0301,
         2,       -1,       -1, G_TITLE   ,   /* Object 5 MBIGANZEIG */
  NONE, NORMAL, (LONG)" Anzeigen",
  0x000C, 0x0000, 0x000A, 0x0301,
         0,        7,       14, G_IBOX    ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0301, 0x0050, 0x0017,
         9, MBIGINFOANZEIGE, MBIGINFOANZEIGE, G_BOX     ,   /* Object 7  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0000, 0x0016, 0x0001,
         7,       -1,       -1, G_STRING  ,   /* Object 8 MBIGINFOANZEIGE */
  NONE, NORMAL, (LONG)"  Info anzeigen... ^I ",
  0x0000, 0x0000, 0x0016, 0x0001,
        14, MBIGEDITVOL, MBIGEDITP2B, G_BOX     ,   /* Object 9  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0008, 0x0000, 0x0011, 0x0004,
        11,       -1,       -1, G_STRING  ,   /* Object 10 MBIGEDITVOL */
  NONE, NORMAL, (LONG)"  Volume      \007O ",
  0x0000, 0x0000, 0x0011, 0x0001,
  MBIGEDITB2P,       -1,       -1, G_STRING  ,   /* Object 11  */
  NONE, DISABLED, (LONG)"-----------------",
  0x0000, 0x0001, 0x0011, 0x0001,
  MBIGEDITP2B,       -1,       -1, G_STRING  ,   /* Object 12 MBIGEDITB2P */
  NONE, NORMAL, (LONG)"  BIG -> PUF  \007C ",
  0x0000, 0x0002, 0x0011, 0x0001,
         9,       -1,       -1, G_STRING  ,   /* Object 13 MBIGEDITP2B */
  NONE, NORMAL, (LONG)"  PUF -> BIG  \007V ",
  0x0000, 0x0003, 0x0011, 0x0001,
         6, MBIGANZEIGINFO, MBIGANZEIGINFO, G_BOX     ,   /* Object 14  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x000E, 0x0000, 0x000B, 0x0001,
        14,       -1,       -1, G_STRING  ,   /* Object 15 MBIGANZEIGINFO */
  LASTOB, NORMAL, (LONG)"  Info  \007I ",
  0x0000, 0x0000, 0x000B, 0x0001,
  
  /******** Tree 1 BIG_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich im Big-Sequencer",
  0x0001, 0x0001, 0x0022, 0x0001,
         3,       -1,       -1, G_STRING  ,   /* Object 2  */
  NONE, NORMAL, (LONG)"\042Blinkrate\042 ist eine Zahl von 0 bis 9 und gibt die Anzahl",
  0x0001, 0x0003, 0x0039, 0x0001,
         4,       -1,       -1, G_STRING  ,   /* Object 3  */
  NONE, NORMAL, (LONG)"der Blinks f\201r Pop-Up-Men\201s und Men\201zeilen im Fenster an.",
  0x0001, 0x0004, 0x0039, 0x0001,
         5,       -1,       -1, G_STRING  ,   /* Object 4  */
  NONE, NORMAL, (LONG)"\042Passwort\042 ist ein Test-Feld f\201r verdeckte Passw\224rter.",
  0x0001, 0x0006, 0x0036, 0x0001,
         6,       -1,       -1, G_STRING  ,   /* Object 5  */
  NONE, NORMAL, (LONG)"\042Beep bei Fehlermeldungen\042 schaltet das akustische Signal",
  0x0001, 0x0008, 0x0039, 0x0001,
         7,       -1,       -1, G_STRING  ,   /* Object 6  */
  NONE, NORMAL, (LONG)"bei Fehlermeldungen ein bzw. aus.",
  0x0001, 0x0009, 0x0021, 0x0001,
         8,       -1,       -1, G_STRING  ,   /* Object 7  */
  NONE, NORMAL, (LONG)"\042Grow/Shrinkbox\042 schaltet das Zeichnen eines sich ver-",
  0x0001, 0x000B, 0x0036, 0x0001,
         9,       -1,       -1, G_STRING  ,   /* Object 8  */
  NONE, NORMAL, (LONG)"gr\224\236ernden oder verkleinerden Rechtecks beim \231ffnen von",
  0x0001, 0x000C, 0x0037, 0x0001,
        10,       -1,       -1, G_STRING  ,   /* Object 9  */
  NONE, NORMAL, (LONG)"Fenstern und Dialogboxen ein bzw. aus.",
  0x0001, 0x000D, 0x0026, 0x0001,
        11,       -1,       -1, G_STRING  ,   /* Object 10  */
  NONE, NORMAL, (LONG)"\042OK\042 \201bernimmt die gemachten Einstellungen.",
  0x0001, 0x000F, 0x002B, 0x0001,
        12,       -1,       -1, G_STRING  ,   /* Object 11  */
  NONE, NORMAL, (LONG)"\042ABBRUCH\042 verl\204\236t den Dialog ohne \216nderung.",
  0x0001, 0x0010, 0x002B, 0x0001,
        13,       -1,       -1, G_STRING  ,   /* Object 12  */
  NONE, NORMAL, (LONG)"\042HILFE\042 zeigt diese Dialogbox.",
  0x0001, 0x0011, 0x001E, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 13  */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x0014, 0x0013, 0x0012, 0x0002,
  
  /******** Tree 2 BIG_DESK ****************************************************/
        -1, BIGICON , BIGICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 BIGICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0003, 0x0801, 0x0309, 0x0802,
  
  /******** Tree 3 BIG_TEXT ****************************************************/
        -1, FBIGN   , FBIGI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FBIGI   ,       -1,       -1, G_STRING  ,   /* Object 1 FBIGN */
  NONE, NORMAL, (LONG)"Big Sequencer",
  0x0002, 0x0001, 0x000D, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FBIGI */
  LASTOB, NORMAL, (LONG)" %6d Events",
  0x0002, 0x0002, 0x000B, 0x0001,
  
  /******** Tree 4 BIG_INFO ****************************************************/
        -1,        1, BIGIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0009, 0x0802, 0x0011, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0006, 0x0004, 0x0017, 0x0001,
         5,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0008, 0x0005, 0x0612, 0x0600,
  BIGIVERNR,       -1,       -1, G_TEXT    ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x000E, 0x0805, 0x060C, 0x0600,
  BIGIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 6 BIGIVERNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0007, 0x0806, 0x0014, 0x0001,
  BIGCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 7 BIGIVERDA */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0009, 0x0807, 0x0012, 0x0001,
  BIGISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 8 BIGCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0009, 0x0808, 0x0012, 0x0800,
  BIGIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 9 BIGISETUPS */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0008, 0x0809, 0x0013, 0x0001,
  BIGIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 10 BIGIAKT */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 11 BIGIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 BIG_MENU */
  &rs_object[16],   /* Tree  1 BIG_HELP */
  &rs_object[30],   /* Tree  2 BIG_DESK */
  &rs_object[32],   /* Tree  3 BIG_TEXT */
  &rs_object[35]    /* Tree  4 BIG_INFO */
};
