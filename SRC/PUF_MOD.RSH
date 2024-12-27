/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "PUF_MOD.H"

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
{ "An",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 3, 1,
  "Koord.",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 7, 1,
  "Anzeige",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 8, 1,
  "Reihenfolge",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 12, 1,
  "Gr\224\236e",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 6, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 4,
  "___",
  "___",
  "XXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 4, 4,
  "An",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 3, 1,
  "L-R",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 1,
  "O-U",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 1,
  "V-H",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 1,
  "Vol",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 1,
  "Dub",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 1,
  "Aufnahme",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 9, 1,
  "Ausg",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 5, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "PUF Puffersequenzer",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 20, 1,
  "1993 by Cybernetic Arts",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 24, 1,
  "Programm: Bertram Dunskus",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 26, 1,
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
  "Max. Events: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 25,
  "01234567890",
  "freie Ev.: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 23
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
  0x3FFF, 0xFFF0, 0x2400, 0x00B0, 
  0x2400, 0x0090, 0x3FFF, 0xFFF0, 
  0x2000, 0x0090, 0x2FFF, 0xFE90, 
  0x2000, 0x0090, 0x2000, 0x0090, 
  0x2000, 0x0090, 0x2FFF, 0xFE90, 
  0x2000, 0x0090, 0x2000, 0x0090, 
  0x2000, 0x0090, 0x2FFF, 0xFE90, 
  0x2000, 0x0090, 0x2000, 0x0090, 
  0x2000, 0x0090, 0x2FFF, 0xFE90, 
  0x2000, 0x0090, 0x2000, 0x0090, 
  0x2000, 0x0090, 0x3FFF, 0xFFF0, 
  0x2400, 0x0090, 0x2400, 0x00B0, 
  0x3FFF, 0xFFF0, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "Puffer Seq", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  75,   8
};

static WORD RSBB0DATA[] =
{ 0x7FFF, 0x3FFE, 0x1FFC, 0x0FF8, 
  0x07F0, 0x03E0, 0x01C0, 0x0080
};

static WORD RSBB1DATA[] =
{ 0x7F80, 0x3F80, 0x1F80, 0x0F80, 
  0x0780, 0x0380, 0x0180, 0x0080
};

static WORD RSBB2DATA[] =
{ 0x00FF, 0x00FE, 0x00FC, 0x00F8, 
  0x00F0, 0x00E0, 0x00C0, 0x0080
};

static BITBLK rs_bitblk[] =
{ RSBB0DATA,   2,   8,   0,   0, 0x0001,
  RSBB1DATA,   2,   8,   0,   0, 0x0001,
  RSBB2DATA,   2,   8,   0,   0, 0x0001
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 PUF_MENU ****************************************************/
        -1,        1,        8, G_IBOX    ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0000, 0x0050, 0x0019,
         8,        2,        2, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0050, 0x0201,
         1, MPUFINFO, MPUFOPTIONS, G_IBOX    ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0002, 0x0000, 0x002F, 0x0301,
  MPUFCOMMANDS,       -1,       -1, G_TITLE   ,   /* Object 3 MPUFINFO */
  NONE, NORMAL, (LONG)" Info",
  0x0000, 0x0000, 0x0006, 0x0301,
  MPUFANZEIGEN,       -1,       -1, G_TITLE   ,   /* Object 4 MPUFCOMMANDS */
  NONE, NORMAL, (LONG)" Befehle",
  0x0006, 0x0000, 0x0009, 0x0301,
  MPUFCOMM,       -1,       -1, G_TITLE   ,   /* Object 5 MPUFANZEIGEN */
  NONE, DISABLED, (LONG)" Anzeige",
  0x000F, 0x0000, 0x0009, 0x0301,
  MPUFOPTIONS,       -1,       -1, G_TITLE   ,   /* Object 6 MPUFCOMM */
  NONE, DISABLED, (LONG)" \232bertragung",
  0x0018, 0x0000, 0x000D, 0x0301,
         2,       -1,       -1, G_TITLE   ,   /* Object 7 MPUFOPTIONS */
  NONE, NORMAL, (LONG)" Optionen",
  0x0025, 0x0000, 0x000A, 0x0301,
         0,        9,       19, G_IBOX    ,   /* Object 8  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0301, 0x0050, 0x0017,
        11, MPUFINFOANZEIG, MPUFINFOANZEIG, G_BOX     ,   /* Object 9  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0000, 0x0016, 0x0001,
         9,       -1,       -1, G_STRING  ,   /* Object 10 MPUFINFOANZEIG */
  NONE, NORMAL, (LONG)"  Info anzeigen... ^I ",
  0x0000, 0x0000, 0x0016, 0x0001,
        14, MPUFDELETEREST, MPUFDELETEALL, G_BOX     ,   /* Object 11  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0008, 0x0000, 0x001C, 0x0002,
  MPUFDELETEALL,       -1,       -1, G_STRING  ,   /* Object 12 MPUFDELETEREST */
  NONE, NORMAL, (LONG)"  L\224schen ab Position  \001DEL ",
  0x0000, 0x0000, 0x001C, 0x0001,
        11,       -1,       -1, G_STRING  ,   /* Object 13 MPUFDELETEALL */
  NONE, NORMAL, (LONG)"  L\224schen Alles        \007DEL ",
  0x0000, 0x0001, 0x001C, 0x0001,
        17,       15,       16, G_BOX     ,   /* Object 14  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0011, 0x0000, 0x0016, 0x0002,
        16,       -1,       -1, G_STRING  ,   /* Object 15  */
  NONE, DISABLED, (LONG)"  .. nach Signalen    ",
  0x0000, 0x0000, 0x0016, 0x0001,
        14,       -1,       -1, G_STRING  ,   /* Object 16  */
  NONE, DISABLED, (LONG)"  .. nach Koordinaten ",
  0x0000, 0x0001, 0x0016, 0x0001,
        19, MPUFCOMMBIG, MPUFCOMMBIG, G_BOX     ,   /* Object 17  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x001A, 0x0000, 0x000D, 0x0001,
        17,       -1,       -1, G_STRING  ,   /* Object 18 MPUFCOMMBIG */
  NONE, DISABLED, (LONG)"  ... in BIG ",
  0x0000, 0x0000, 0x000D, 0x0001,
         8, MPUFSETUPS, MPUFSETUPS, G_BOX     ,   /* Object 19  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0027, 0x0000, 0x000D, 0x0001,
        19,       -1,       -1, G_STRING  ,   /* Object 20 MPUFSETUPS */
  LASTOB, NORMAL, (LONG)"  Setups  \007S ",
  0x0000, 0x0000, 0x000D, 0x0001,
  
  /******** Tree 1 PUF_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich im PUF Puffer-Sequenzer",
  0x0001, 0x0001, 0x0029, 0x0001,
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
  
  /******** Tree 2 PUF_DESK ****************************************************/
        -1, PUFICON , PUFICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 PUFICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0001, 0x0001, 0x0309, 0x0802,
  
  /******** Tree 3 PUF_SETUP ****************************************************/
        -1,        1, PUFOK   , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021160L,
  0x0000, 0x0000, 0x003C, 0x0014,
        47,        2,       41, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0001, 0x0001, 0x040F, 0x0811,
        23,        3,       22, G_BOX     ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0001, 0x0001, 0x0005, 0x000C,
  PUFANZALL,       -1,       -1, G_BOXTEXT ,   /* Object 3  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[0],
  0x0400, 0x0000, 0x0004, 0x0001,
  PUFANZ0 ,       -1,       -1, G_BUTTON  ,   /* Object 4 PUFANZALL */
  SELECTABLE, NORMAL, (LONG)"All",
  0x0400, 0x0001, 0x0004, 0x0001,
  PUFANZT0,       -1,       -1, G_BOX     ,   /* Object 5 PUFANZ0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0802, 0x0002, 0x0001,
  PUFANZ1 ,       -1,       -1, G_STRING  ,   /* Object 6 PUFANZT0 */
  SELECTABLE, NORMAL, (LONG)" 0",
  0x0402, 0x0802, 0x0002, 0x0001,
         8,       -1,       -1, G_BOX     ,   /* Object 7 PUFANZ1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0803, 0x0002, 0x0001,
         9,       -1,       -1, G_STRING  ,   /* Object 8  */
  SELECTABLE, NORMAL, (LONG)" 1",
  0x0402, 0x0803, 0x0002, 0x0001,
        10,       -1,       -1, G_BOX     ,   /* Object 9  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0804, 0x0002, 0x0001,
        11,       -1,       -1, G_STRING  ,   /* Object 10  */
  SELECTABLE, NORMAL, (LONG)" 2",
  0x0402, 0x0804, 0x0002, 0x0001,
        12,       -1,       -1, G_BOX     ,   /* Object 11  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0805, 0x0002, 0x0001,
        13,       -1,       -1, G_STRING  ,   /* Object 12  */
  SELECTABLE, NORMAL, (LONG)" 3",
  0x0402, 0x0805, 0x0002, 0x0001,
        14,       -1,       -1, G_BOX     ,   /* Object 13  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0806, 0x0002, 0x0001,
        15,       -1,       -1, G_STRING  ,   /* Object 14  */
  SELECTABLE, NORMAL, (LONG)" 4",
  0x0402, 0x0806, 0x0002, 0x0001,
        16,       -1,       -1, G_BOX     ,   /* Object 15  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0807, 0x0002, 0x0001,
        17,       -1,       -1, G_STRING  ,   /* Object 16  */
  SELECTABLE, NORMAL, (LONG)" 5",
  0x0402, 0x0807, 0x0002, 0x0001,
        18,       -1,       -1, G_BOX     ,   /* Object 17  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0808, 0x0002, 0x0001,
        19,       -1,       -1, G_STRING  ,   /* Object 18  */
  SELECTABLE, NORMAL, (LONG)" 6",
  0x0402, 0x0808, 0x0002, 0x0001,
        20,       -1,       -1, G_BOX     ,   /* Object 19  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0809, 0x0002, 0x0001,
        21,       -1,       -1, G_STRING  ,   /* Object 20  */
  SELECTABLE, NORMAL, (LONG)" 7",
  0x0402, 0x0809, 0x0002, 0x0001,
        22,       -1,       -1, G_BOX     ,   /* Object 21  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x080A, 0x0002, 0x0001,
         2,       -1,       -1, G_STRING  ,   /* Object 22  */
  SELECTABLE, NORMAL, (LONG)" 8",
  0x0402, 0x080A, 0x0002, 0x0001,
        34,       24,       33, G_BOX     ,   /* Object 23  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0406, 0x0001, 0x0008, 0x0007,
  PUFANZKOORALL,       -1,       -1, G_BOXTEXT ,   /* Object 24  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[1],
  0x0001, 0x0000, 0x0406, 0x0001,
  PUFANZX ,       -1,       -1, G_BUTTON  ,   /* Object 25 PUFANZKOORALL */
  SELECTABLE, NORMAL, (LONG)"Alle",
  0x0001, 0x0001, 0x0405, 0x0001,
  PUFANZXT,       -1,       -1, G_BOX     ,   /* Object 26 PUFANZX */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0802, 0x0002, 0x0001,
  PUFANZY ,       -1,       -1, G_STRING  ,   /* Object 27 PUFANZXT */
  SELECTABLE, NORMAL, (LONG)"~L-R",
  0x0402, 0x0802, 0x0004, 0x0001,
        29,       -1,       -1, G_BOX     ,   /* Object 28 PUFANZY */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0803, 0x0002, 0x0001,
  PUFANZZ ,       -1,       -1, G_STRING  ,   /* Object 29  */
  SELECTABLE, NORMAL, (LONG)"~O-U",
  0x0402, 0x0803, 0x0004, 0x0001,
        31,       -1,       -1, G_BOX     ,   /* Object 30 PUFANZZ */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0804, 0x0002, 0x0001,
  PUFANZVOL,       -1,       -1, G_STRING  ,   /* Object 31  */
  SELECTABLE, NORMAL, (LONG)"~V-H",
  0x0402, 0x0804, 0x0004, 0x0001,
        33,       -1,       -1, G_BOX     ,   /* Object 32 PUFANZVOL */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0805, 0x0002, 0x0001,
        23,       -1,       -1, G_STRING  ,   /* Object 33  */
  SELECTABLE, NORMAL, (LONG)"~Vol",
  0x0402, 0x0805, 0x0004, 0x0001,
        35,       -1,       -1, G_BOXTEXT ,   /* Object 34  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[2],
  0x0001, 0x0000, 0x0008, 0x0001,
        41,       36, PUFREIHENFKOORT, G_BOX     ,   /* Object 35  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0001, 0x080D, 0x040D, 0x0803,
  PUFREIHENFSIG,       -1,       -1, G_BOXTEXT ,   /* Object 36  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[3],
  0x0400, 0x0000, 0x000C, 0x0001,
  PUFREIHENFSIGT,       -1,       -1, G_BOX     ,   /* Object 37 PUFREIHENFSIG */
  SELECTABLE|RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0001, 0x0002, 0x0001,
  PUFREIHENFKOOR,       -1,       -1, G_STRING  ,   /* Object 38 PUFREIHENFSIGT */
  SELECTABLE, NORMAL, (LONG)"S~ignale",
  0x0402, 0x0001, 0x0008, 0x0001,
  PUFREIHENFKOORT,       -1,       -1, G_BOX     ,   /* Object 39 PUFREIHENFKOOR */
  SELECTABLE|RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0002, 0x0002, 0x0001,
        35,       -1,       -1, G_STRING  ,   /* Object 40 PUFREIHENFKOORT */
  SELECTABLE, NORMAL, (LONG)"~Koord.",
  0x0402, 0x0002, 0x0007, 0x0001,
         1,       42, PUFHOEHE, G_BOX     ,   /* Object 41  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0406, 0x0809, 0x0008, 0x0803,
  PUFZOOMXT,       -1,       -1, G_BOXTEXT ,   /* Object 42  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[4],
  0x0400, 0x0000, 0x0005, 0x0001,
  PUFBREITE,       -1,       -1, G_STRING  ,   /* Object 43 PUFZOOMXT */
  SELECTABLE, NORMAL, (LONG)"~B:",
  0x0400, 0x0001, 0x0003, 0x0001,
  PUFZOOMYT,       -1,       -1, G_FBOXTEXT,   /* Object 44 PUFBREITE */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0003, 0x0001, 0x0004, 0x0001,
  PUFHOEHE,       -1,       -1, G_STRING  ,   /* Object 45 PUFZOOMYT */
  SELECTABLE, NORMAL, (LONG)"~H:",
  0x0400, 0x0002, 0x0003, 0x0001,
        41,       -1,       -1, G_FBOXTEXT,   /* Object 46 PUFHOEHE */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0403, 0x0002, 0x0403, 0x0001,
       175,       48,      174, G_BOX     ,   /* Object 47  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0012, 0x0001, 0x0422, 0x080D,
        69,       49,       68, G_BOX     ,   /* Object 48  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0001, 0x0001, 0x0005, 0x000C,
  PUFRECONALL,       -1,       -1, G_BOXTEXT ,   /* Object 49  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[7],
  0x0400, 0x0000, 0x0004, 0x0001,
  PUFRECON0,       -1,       -1, G_BUTTON  ,   /* Object 50 PUFRECONALL */
  SELECTABLE, NORMAL, (LONG)"All",
  0x0400, 0x0001, 0x0004, 0x0001,
        52,       -1,       -1, G_BOX     ,   /* Object 51 PUFRECON0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0802, 0x0002, 0x0001,
  PUFRECON1,       -1,       -1, G_STRING  ,   /* Object 52  */
  SELECTABLE, NORMAL, (LONG)" 0",
  0x0402, 0x0802, 0x0002, 0x0001,
        54,       -1,       -1, G_BOX     ,   /* Object 53 PUFRECON1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0803, 0x0002, 0x0001,
        55,       -1,       -1, G_STRING  ,   /* Object 54  */
  SELECTABLE, NORMAL, (LONG)" 1",
  0x0402, 0x0803, 0x0002, 0x0001,
        56,       -1,       -1, G_BOX     ,   /* Object 55  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0804, 0x0002, 0x0001,
        57,       -1,       -1, G_STRING  ,   /* Object 56  */
  SELECTABLE, NORMAL, (LONG)" 2",
  0x0402, 0x0804, 0x0002, 0x0001,
        58,       -1,       -1, G_BOX     ,   /* Object 57  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0805, 0x0002, 0x0001,
        59,       -1,       -1, G_STRING  ,   /* Object 58  */
  SELECTABLE, NORMAL, (LONG)" 3",
  0x0402, 0x0805, 0x0002, 0x0001,
        60,       -1,       -1, G_BOX     ,   /* Object 59  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0806, 0x0002, 0x0001,
        61,       -1,       -1, G_STRING  ,   /* Object 60  */
  SELECTABLE, NORMAL, (LONG)" 4",
  0x0402, 0x0806, 0x0002, 0x0001,
        62,       -1,       -1, G_BOX     ,   /* Object 61  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0807, 0x0002, 0x0001,
        63,       -1,       -1, G_STRING  ,   /* Object 62  */
  SELECTABLE, NORMAL, (LONG)" 5",
  0x0402, 0x0807, 0x0002, 0x0001,
        64,       -1,       -1, G_BOX     ,   /* Object 63  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0808, 0x0002, 0x0001,
        65,       -1,       -1, G_STRING  ,   /* Object 64  */
  SELECTABLE, NORMAL, (LONG)" 6",
  0x0402, 0x0808, 0x0002, 0x0001,
        66,       -1,       -1, G_BOX     ,   /* Object 65  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0809, 0x0002, 0x0001,
        67,       -1,       -1, G_STRING  ,   /* Object 66  */
  SELECTABLE, NORMAL, (LONG)" 7",
  0x0402, 0x0809, 0x0002, 0x0001,
        68,       -1,       -1, G_BOX     ,   /* Object 67  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x080A, 0x0002, 0x0001,
        48,       -1,       -1, G_STRING  ,   /* Object 68  */
  SELECTABLE, NORMAL, (LONG)" 8",
  0x0402, 0x080A, 0x0002, 0x0001,
        90,       70,       89, G_BOX     ,   /* Object 69  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0406, 0x0001, 0x0005, 0x000C,
  PUFRECXALL,       -1,       -1, G_BOXTEXT ,   /* Object 70  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[8],
  0x0400, 0x0000, 0x0004, 0x0001,
  PUFRECX0,       -1,       -1, G_BUTTON  ,   /* Object 71 PUFRECXALL */
  SELECTABLE, NORMAL, (LONG)"All",
  0x0400, 0x0001, 0x0004, 0x0001,
        73,       -1,       -1, G_BOX     ,   /* Object 72 PUFRECX0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0802, 0x0002, 0x0001,
  PUFRECX1,       -1,       -1, G_STRING  ,   /* Object 73  */
  SELECTABLE, NORMAL, (LONG)" 0",
  0x0402, 0x0802, 0x0002, 0x0001,
        75,       -1,       -1, G_BOX     ,   /* Object 74 PUFRECX1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0803, 0x0002, 0x0001,
        76,       -1,       -1, G_STRING  ,   /* Object 75  */
  SELECTABLE, NORMAL, (LONG)" 1",
  0x0402, 0x0803, 0x0002, 0x0001,
        77,       -1,       -1, G_BOX     ,   /* Object 76  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0804, 0x0002, 0x0001,
        78,       -1,       -1, G_STRING  ,   /* Object 77  */
  SELECTABLE, NORMAL, (LONG)" 2",
  0x0402, 0x0804, 0x0002, 0x0001,
        79,       -1,       -1, G_BOX     ,   /* Object 78  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0805, 0x0002, 0x0001,
        80,       -1,       -1, G_STRING  ,   /* Object 79  */
  SELECTABLE, NORMAL, (LONG)" 3",
  0x0402, 0x0805, 0x0002, 0x0001,
        81,       -1,       -1, G_BOX     ,   /* Object 80  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0806, 0x0002, 0x0001,
        82,       -1,       -1, G_STRING  ,   /* Object 81  */
  SELECTABLE, NORMAL, (LONG)" 4",
  0x0402, 0x0806, 0x0002, 0x0001,
        83,       -1,       -1, G_BOX     ,   /* Object 82  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0807, 0x0002, 0x0001,
        84,       -1,       -1, G_STRING  ,   /* Object 83  */
  SELECTABLE, NORMAL, (LONG)" 5",
  0x0402, 0x0807, 0x0002, 0x0001,
        85,       -1,       -1, G_BOX     ,   /* Object 84  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0808, 0x0002, 0x0001,
        86,       -1,       -1, G_STRING  ,   /* Object 85  */
  SELECTABLE, NORMAL, (LONG)" 6",
  0x0402, 0x0808, 0x0002, 0x0001,
        87,       -1,       -1, G_BOX     ,   /* Object 86  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0809, 0x0002, 0x0001,
        88,       -1,       -1, G_STRING  ,   /* Object 87  */
  SELECTABLE, NORMAL, (LONG)" 7",
  0x0402, 0x0809, 0x0002, 0x0001,
        89,       -1,       -1, G_BOX     ,   /* Object 88  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x080A, 0x0002, 0x0001,
        69,       -1,       -1, G_STRING  ,   /* Object 89  */
  SELECTABLE, NORMAL, (LONG)" 8",
  0x0402, 0x080A, 0x0002, 0x0001,
       111,       91,      110, G_BOX     ,   /* Object 90  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x000C, 0x0001, 0x0005, 0x000C,
  PUFRECYALL,       -1,       -1, G_BOXTEXT ,   /* Object 91  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[9],
  0x0400, 0x0000, 0x0004, 0x0001,
  PUFRECY0,       -1,       -1, G_BUTTON  ,   /* Object 92 PUFRECYALL */
  SELECTABLE, NORMAL, (LONG)"All",
  0x0400, 0x0001, 0x0004, 0x0001,
        94,       -1,       -1, G_BOX     ,   /* Object 93 PUFRECY0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0802, 0x0002, 0x0001,
  PUFRECY1,       -1,       -1, G_STRING  ,   /* Object 94  */
  SELECTABLE, NORMAL, (LONG)" 0",
  0x0402, 0x0802, 0x0002, 0x0001,
        96,       -1,       -1, G_BOX     ,   /* Object 95 PUFRECY1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0803, 0x0002, 0x0001,
        97,       -1,       -1, G_STRING  ,   /* Object 96  */
  SELECTABLE, NORMAL, (LONG)" 1",
  0x0402, 0x0803, 0x0002, 0x0001,
        98,       -1,       -1, G_BOX     ,   /* Object 97  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0804, 0x0002, 0x0001,
        99,       -1,       -1, G_STRING  ,   /* Object 98  */
  SELECTABLE, NORMAL, (LONG)" 2",
  0x0402, 0x0804, 0x0002, 0x0001,
       100,       -1,       -1, G_BOX     ,   /* Object 99  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0805, 0x0002, 0x0001,
       101,       -1,       -1, G_STRING  ,   /* Object 100  */
  SELECTABLE, NORMAL, (LONG)" 3",
  0x0402, 0x0805, 0x0002, 0x0001,
       102,       -1,       -1, G_BOX     ,   /* Object 101  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0806, 0x0002, 0x0001,
       103,       -1,       -1, G_STRING  ,   /* Object 102  */
  SELECTABLE, NORMAL, (LONG)" 4",
  0x0402, 0x0806, 0x0002, 0x0001,
       104,       -1,       -1, G_BOX     ,   /* Object 103  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0807, 0x0002, 0x0001,
       105,       -1,       -1, G_STRING  ,   /* Object 104  */
  SELECTABLE, NORMAL, (LONG)" 5",
  0x0402, 0x0807, 0x0002, 0x0001,
       106,       -1,       -1, G_BOX     ,   /* Object 105  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0808, 0x0002, 0x0001,
       107,       -1,       -1, G_STRING  ,   /* Object 106  */
  SELECTABLE, NORMAL, (LONG)" 6",
  0x0402, 0x0808, 0x0002, 0x0001,
       108,       -1,       -1, G_BOX     ,   /* Object 107  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0809, 0x0002, 0x0001,
       109,       -1,       -1, G_STRING  ,   /* Object 108  */
  SELECTABLE, NORMAL, (LONG)" 7",
  0x0402, 0x0809, 0x0002, 0x0001,
       110,       -1,       -1, G_BOX     ,   /* Object 109  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x080A, 0x0002, 0x0001,
        90,       -1,       -1, G_STRING  ,   /* Object 110  */
  SELECTABLE, NORMAL, (LONG)" 8",
  0x0402, 0x080A, 0x0002, 0x0001,
       132,      112,      131, G_BOX     ,   /* Object 111  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0411, 0x0001, 0x0005, 0x000C,
  PUFRECZALL,       -1,       -1, G_BOXTEXT ,   /* Object 112  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[10],
  0x0400, 0x0000, 0x0004, 0x0001,
  PUFRECZ0,       -1,       -1, G_BUTTON  ,   /* Object 113 PUFRECZALL */
  SELECTABLE, NORMAL, (LONG)"All",
  0x0400, 0x0001, 0x0004, 0x0001,
       115,       -1,       -1, G_BOX     ,   /* Object 114 PUFRECZ0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0802, 0x0002, 0x0001,
  PUFRECZ1,       -1,       -1, G_STRING  ,   /* Object 115  */
  SELECTABLE, NORMAL, (LONG)" 0",
  0x0402, 0x0802, 0x0002, 0x0001,
       117,       -1,       -1, G_BOX     ,   /* Object 116 PUFRECZ1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0803, 0x0002, 0x0001,
       118,       -1,       -1, G_STRING  ,   /* Object 117  */
  SELECTABLE, NORMAL, (LONG)" 1",
  0x0402, 0x0803, 0x0002, 0x0001,
       119,       -1,       -1, G_BOX     ,   /* Object 118  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0804, 0x0002, 0x0001,
       120,       -1,       -1, G_STRING  ,   /* Object 119  */
  SELECTABLE, NORMAL, (LONG)" 2",
  0x0402, 0x0804, 0x0002, 0x0001,
       121,       -1,       -1, G_BOX     ,   /* Object 120  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0805, 0x0002, 0x0001,
       122,       -1,       -1, G_STRING  ,   /* Object 121  */
  SELECTABLE, NORMAL, (LONG)" 3",
  0x0402, 0x0805, 0x0002, 0x0001,
       123,       -1,       -1, G_BOX     ,   /* Object 122  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0806, 0x0002, 0x0001,
       124,       -1,       -1, G_STRING  ,   /* Object 123  */
  SELECTABLE, NORMAL, (LONG)" 4",
  0x0402, 0x0806, 0x0002, 0x0001,
       125,       -1,       -1, G_BOX     ,   /* Object 124  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0807, 0x0002, 0x0001,
       126,       -1,       -1, G_STRING  ,   /* Object 125  */
  SELECTABLE, NORMAL, (LONG)" 5",
  0x0402, 0x0807, 0x0002, 0x0001,
       127,       -1,       -1, G_BOX     ,   /* Object 126  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0808, 0x0002, 0x0001,
       128,       -1,       -1, G_STRING  ,   /* Object 127  */
  SELECTABLE, NORMAL, (LONG)" 6",
  0x0402, 0x0808, 0x0002, 0x0001,
       129,       -1,       -1, G_BOX     ,   /* Object 128  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0809, 0x0002, 0x0001,
       130,       -1,       -1, G_STRING  ,   /* Object 129  */
  SELECTABLE, NORMAL, (LONG)" 7",
  0x0402, 0x0809, 0x0002, 0x0001,
       131,       -1,       -1, G_BOX     ,   /* Object 130  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x080A, 0x0002, 0x0001,
       111,       -1,       -1, G_STRING  ,   /* Object 131  */
  SELECTABLE, NORMAL, (LONG)" 8",
  0x0402, 0x080A, 0x0002, 0x0001,
       153,      133,      152, G_BOX     ,   /* Object 132  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0017, 0x0001, 0x0005, 0x000C,
  PUFRECVOLALL,       -1,       -1, G_BOXTEXT ,   /* Object 133  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[11],
  0x0400, 0x0000, 0x0004, 0x0001,
  PUFRECVOL0,       -1,       -1, G_BUTTON  ,   /* Object 134 PUFRECVOLALL */
  SELECTABLE, NORMAL, (LONG)"All",
  0x0400, 0x0001, 0x0004, 0x0001,
       136,       -1,       -1, G_BOX     ,   /* Object 135 PUFRECVOL0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0802, 0x0002, 0x0001,
  PUFRECVOL1,       -1,       -1, G_STRING  ,   /* Object 136  */
  SELECTABLE, NORMAL, (LONG)" 0",
  0x0402, 0x0802, 0x0002, 0x0001,
       138,       -1,       -1, G_BOX     ,   /* Object 137 PUFRECVOL1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0803, 0x0002, 0x0001,
       139,       -1,       -1, G_STRING  ,   /* Object 138  */
  SELECTABLE, NORMAL, (LONG)" 1",
  0x0402, 0x0803, 0x0002, 0x0001,
       140,       -1,       -1, G_BOX     ,   /* Object 139  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0804, 0x0002, 0x0001,
       141,       -1,       -1, G_STRING  ,   /* Object 140  */
  SELECTABLE, NORMAL, (LONG)" 2",
  0x0402, 0x0804, 0x0002, 0x0001,
       142,       -1,       -1, G_BOX     ,   /* Object 141  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0805, 0x0002, 0x0001,
       143,       -1,       -1, G_STRING  ,   /* Object 142  */
  SELECTABLE, NORMAL, (LONG)" 3",
  0x0402, 0x0805, 0x0002, 0x0001,
       144,       -1,       -1, G_BOX     ,   /* Object 143  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0806, 0x0002, 0x0001,
       145,       -1,       -1, G_STRING  ,   /* Object 144  */
  SELECTABLE, NORMAL, (LONG)" 4",
  0x0402, 0x0806, 0x0002, 0x0001,
       146,       -1,       -1, G_BOX     ,   /* Object 145  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0807, 0x0002, 0x0001,
       147,       -1,       -1, G_STRING  ,   /* Object 146  */
  SELECTABLE, NORMAL, (LONG)" 5",
  0x0402, 0x0807, 0x0002, 0x0001,
       148,       -1,       -1, G_BOX     ,   /* Object 147  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0808, 0x0002, 0x0001,
       149,       -1,       -1, G_STRING  ,   /* Object 148  */
  SELECTABLE, NORMAL, (LONG)" 6",
  0x0402, 0x0808, 0x0002, 0x0001,
       150,       -1,       -1, G_BOX     ,   /* Object 149  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0809, 0x0002, 0x0001,
       151,       -1,       -1, G_STRING  ,   /* Object 150  */
  SELECTABLE, NORMAL, (LONG)" 7",
  0x0402, 0x0809, 0x0002, 0x0001,
       152,       -1,       -1, G_BOX     ,   /* Object 151  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x080A, 0x0002, 0x0001,
       132,       -1,       -1, G_STRING  ,   /* Object 152  */
  SELECTABLE, NORMAL, (LONG)" 8",
  0x0402, 0x080A, 0x0002, 0x0001,
       174,      154,      173, G_BOX     ,   /* Object 153  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x041C, 0x0001, 0x0005, 0x000C,
  PUFRECDUBALL,       -1,       -1, G_BOXTEXT ,   /* Object 154  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[12],
  0x0400, 0x0000, 0x0004, 0x0001,
  PUFRECDUB0,       -1,       -1, G_BUTTON  ,   /* Object 155 PUFRECDUBALL */
  SELECTABLE, NORMAL, (LONG)"All",
  0x0400, 0x0001, 0x0004, 0x0001,
       157,       -1,       -1, G_BOX     ,   /* Object 156 PUFRECDUB0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0802, 0x0002, 0x0001,
  PUFRECDUB1,       -1,       -1, G_STRING  ,   /* Object 157  */
  SELECTABLE, NORMAL, (LONG)" 0",
  0x0402, 0x0802, 0x0002, 0x0001,
       159,       -1,       -1, G_BOX     ,   /* Object 158 PUFRECDUB1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0803, 0x0002, 0x0001,
       160,       -1,       -1, G_STRING  ,   /* Object 159  */
  SELECTABLE, NORMAL, (LONG)" 1",
  0x0402, 0x0803, 0x0002, 0x0001,
       161,       -1,       -1, G_BOX     ,   /* Object 160  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0804, 0x0002, 0x0001,
       162,       -1,       -1, G_STRING  ,   /* Object 161  */
  SELECTABLE, NORMAL, (LONG)" 2",
  0x0402, 0x0804, 0x0002, 0x0001,
       163,       -1,       -1, G_BOX     ,   /* Object 162  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0805, 0x0002, 0x0001,
       164,       -1,       -1, G_STRING  ,   /* Object 163  */
  SELECTABLE, NORMAL, (LONG)" 3",
  0x0402, 0x0805, 0x0002, 0x0001,
       165,       -1,       -1, G_BOX     ,   /* Object 164  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0806, 0x0002, 0x0001,
       166,       -1,       -1, G_STRING  ,   /* Object 165  */
  SELECTABLE, NORMAL, (LONG)" 4",
  0x0402, 0x0806, 0x0002, 0x0001,
       167,       -1,       -1, G_BOX     ,   /* Object 166  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0807, 0x0002, 0x0001,
       168,       -1,       -1, G_STRING  ,   /* Object 167  */
  SELECTABLE, NORMAL, (LONG)" 5",
  0x0402, 0x0807, 0x0002, 0x0001,
       169,       -1,       -1, G_BOX     ,   /* Object 168  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0808, 0x0002, 0x0001,
       170,       -1,       -1, G_STRING  ,   /* Object 169  */
  SELECTABLE, NORMAL, (LONG)" 6",
  0x0402, 0x0808, 0x0002, 0x0001,
       171,       -1,       -1, G_BOX     ,   /* Object 170  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0809, 0x0002, 0x0001,
       172,       -1,       -1, G_STRING  ,   /* Object 171  */
  SELECTABLE, NORMAL, (LONG)" 7",
  0x0402, 0x0809, 0x0002, 0x0001,
       173,       -1,       -1, G_BOX     ,   /* Object 172  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x080A, 0x0002, 0x0001,
       153,       -1,       -1, G_STRING  ,   /* Object 173  */
  SELECTABLE, NORMAL, (LONG)" 8",
  0x0402, 0x080A, 0x0002, 0x0001,
        47,       -1,       -1, G_BOXTEXT ,   /* Object 174  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[13],
  0x0001, 0x0000, 0x0009, 0x0001,
       196,      176,      195, G_BOX     ,   /* Object 175  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0435, 0x0002, 0x0005, 0x000C,
  PUFAUSGALL,       -1,       -1, G_BOXTEXT ,   /* Object 176  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[14],
  0x0400, 0x0000, 0x0004, 0x0001,
  PUFAUSG0,       -1,       -1, G_BUTTON  ,   /* Object 177 PUFAUSGALL */
  SELECTABLE, NORMAL, (LONG)"All",
  0x0400, 0x0001, 0x0004, 0x0001,
       179,       -1,       -1, G_BOX     ,   /* Object 178 PUFAUSG0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0802, 0x0002, 0x0001,
  PUFAUSG1,       -1,       -1, G_STRING  ,   /* Object 179  */
  SELECTABLE, NORMAL, (LONG)" 0",
  0x0402, 0x0802, 0x0002, 0x0001,
       181,       -1,       -1, G_BOX     ,   /* Object 180 PUFAUSG1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0803, 0x0002, 0x0001,
       182,       -1,       -1, G_STRING  ,   /* Object 181  */
  SELECTABLE, NORMAL, (LONG)" 1",
  0x0402, 0x0803, 0x0002, 0x0001,
       183,       -1,       -1, G_BOX     ,   /* Object 182  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0804, 0x0002, 0x0001,
       184,       -1,       -1, G_STRING  ,   /* Object 183  */
  SELECTABLE, NORMAL, (LONG)" 2",
  0x0402, 0x0804, 0x0002, 0x0001,
       185,       -1,       -1, G_BOX     ,   /* Object 184  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0805, 0x0002, 0x0001,
       186,       -1,       -1, G_STRING  ,   /* Object 185  */
  SELECTABLE, NORMAL, (LONG)" 3",
  0x0402, 0x0805, 0x0002, 0x0001,
       187,       -1,       -1, G_BOX     ,   /* Object 186  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0806, 0x0002, 0x0001,
       188,       -1,       -1, G_STRING  ,   /* Object 187  */
  SELECTABLE, NORMAL, (LONG)" 4",
  0x0402, 0x0806, 0x0002, 0x0001,
       189,       -1,       -1, G_BOX     ,   /* Object 188  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0807, 0x0002, 0x0001,
       190,       -1,       -1, G_STRING  ,   /* Object 189  */
  SELECTABLE, NORMAL, (LONG)" 5",
  0x0402, 0x0807, 0x0002, 0x0001,
       191,       -1,       -1, G_BOX     ,   /* Object 190  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0808, 0x0002, 0x0001,
       192,       -1,       -1, G_STRING  ,   /* Object 191  */
  SELECTABLE, NORMAL, (LONG)" 6",
  0x0402, 0x0808, 0x0002, 0x0001,
       193,       -1,       -1, G_BOX     ,   /* Object 192  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0809, 0x0002, 0x0001,
       194,       -1,       -1, G_STRING  ,   /* Object 193  */
  SELECTABLE, NORMAL, (LONG)" 7",
  0x0402, 0x0809, 0x0002, 0x0001,
       195,       -1,       -1, G_BOX     ,   /* Object 194  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x080A, 0x0002, 0x0001,
       175,       -1,       -1, G_STRING  ,   /* Object 195  */
  SELECTABLE, NORMAL, (LONG)" 8",
  0x0402, 0x080A, 0x0002, 0x0001,
  PUFSTANDARD, PUFSETDEC,      201, G_BOX     ,   /* Object 196  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0411, 0x080F, 0x0014, 0x0803,
  PUFSETINC,       -1,       -1, G_BUTTON  ,   /* Object 197 PUFSETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  PUFSETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 198 PUFSETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  PUFSETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 199 PUFSETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
       201,       -1,       -1, G_BUTTON  ,   /* Object 200 PUFSETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
       196,      202, PUFSETNR, G_BOX     ,   /* Object 201  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  PUFSETNR,       -1,       -1, G_STRING  ,   /* Object 202  */
  NONE, NORMAL, (LONG)"PUF-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0801,
       201,       -1,       -1, G_FTEXT   ,   /* Object 203 PUFSETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0008, 0x0000, 0x0006, 0x0801,
  PUFCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 204 PUFSTANDARD */
  SELECTABLE, NORMAL, (LONG)"~Standard",
  0x0027, 0x000F, 0x0009, 0x0002,
  PUFHELP ,       -1,       -1, G_BUTTON  ,   /* Object 205 PUFCANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0027, 0x0811, 0x0009, 0x0002,
  PUFOK   ,       -1,       -1, G_BUTTON  ,   /* Object 206 PUFHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0431, 0x000F, 0x0009, 0x0002,
         0,       -1,       -1, G_BUTTON  ,   /* Object 207 PUFOK */
  SELECTABLE|DEFAULT|LASTOB, NORMAL, (LONG)"~OK",
  0x0431, 0x0811, 0x0009, 0x0002,
  
  /******** Tree 4 PUF_SHELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Dialogbox PUF Setups",
  0x0001, 0x0001, 0x0014, 0x0001,
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
  
  /******** Tree 5 PUF_TEXT ****************************************************/
        -1, FPUFN   , FPUFSN  , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x002E, 0x0009,
  FPUFI   ,       -1,       -1, G_STRING  ,   /* Object 1 FPUFN */
  NONE, NORMAL, (LONG)" PUF Puffer-Sequenzer ",
  0x0001, 0x0001, 0x0016, 0x0001,
  FPUFIN  ,       -1,       -1, G_STRING  ,   /* Object 2 FPUFI */
  NONE, NORMAL, (LONG)" %d Events",
  0x0001, 0x0002, 0x000A, 0x0001,
  FPUFH   ,       -1,       -1, G_STRING  ,   /* Object 3 FPUFIN */
  NONE, NORMAL, (LONG)"PUF-Info",
  0x0002, 0x0803, 0x0008, 0x0001,
  FPUFSH  ,       -1,       -1, G_STRING  ,   /* Object 4 FPUFH */
  NONE, NORMAL, (LONG)" PUF-Sequenzer-Hilfe ",
  0x0001, 0x0005, 0x0015, 0x0001,
  FPUFSN  ,       -1,       -1, G_STRING  ,   /* Object 5 FPUFSH */
  NONE, NORMAL, (LONG)" PUF-Setup-Hilfe ",
  0x0001, 0x0007, 0x0011, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 6 FPUFSN */
  LASTOB, NORMAL, (LONG)" PUF-Setups ",
  0x0001, 0x0006, 0x000C, 0x0001,
  
  /******** Tree 6 PUF_INFO ****************************************************/
        -1,        1, PUFIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0008, 0x0802, 0x0013, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0006, 0x0004, 0x0017, 0x0001,
  PUFIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0008, 0x0005, 0x0612, 0x0600,
  PUFIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 PUFIVERNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0008, 0x0006, 0x0014, 0x0001,
  PUFCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 PUFIVERDA */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[21],
  0x000A, 0x0007, 0x0012, 0x0001,
  PUFIEVENTS,       -1,       -1, G_FTEXT   ,   /* Object 7 PUFCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x000A, 0x0008, 0x0012, 0x0800,
  PUFIFREE,       -1,       -1, G_FTEXT   ,   /* Object 8 PUFIEVENTS */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0004, 0x0809, 0x0018, 0x0001,
  PUFIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 PUFIFREE */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0006, 0x080A, 0x0016, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 PUFIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002,
  
  /******** Tree 7 PUF_ELEMENTS ****************************************************/
        -1, PUFLOCATOR, PUFRIGHTLOC, G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  PUFLEFTLOC,       -1,       -1, G_IMAGE   ,   /* Object 1 PUFLOCATOR */
  NONE, NORMAL, (LONG)&rs_bitblk[0],
  0x000A, 0x0805, 0x0002, 0x0800,
  PUFRIGHTLOC,       -1,       -1, G_IMAGE   ,   /* Object 2 PUFLEFTLOC */
  NONE, NORMAL, (LONG)&rs_bitblk[1],
  0x0007, 0x0805, 0x0002, 0x0800,
         0,       -1,       -1, G_IMAGE   ,   /* Object 3 PUFRIGHTLOC */
  LASTOB, NORMAL, (LONG)&rs_bitblk[2],
  0x000D, 0x0805, 0x0002, 0x0800
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 PUF_MENU */
  &rs_object[21],   /* Tree  1 PUF_HELP */
  &rs_object[35],   /* Tree  2 PUF_DESK */
  &rs_object[37],   /* Tree  3 PUF_SETUP */
  &rs_object[245],   /* Tree  4 PUF_SHELP */
  &rs_object[259],   /* Tree  5 PUF_TEXT */
  &rs_object[266],   /* Tree  6 PUF_INFO */
  &rs_object[277]    /* Tree  7 PUF_ELEMENTS */
};
