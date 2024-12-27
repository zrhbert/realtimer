/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "EC4_MOD.H"

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
  "EC4 4D Cue-List-Editor",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 23, 1,
  "1992 by Cybernetic Arts",
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
  "Setups: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 20,
  "01234567890",
  "akt. Set: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 22,
  "Timing",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 7, 1,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "000",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 4, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "000",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 4, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "000",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 4, 4,
  "Parameter",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 10, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "Midi-Key-Event",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 15, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "___",
  "___",
  "XXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "Sig 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "Sig 2",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "Sig 3",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "Sig 4",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "Sig 5",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "Sig 6",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "Sig 7",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "Sig 8",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "___",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "Anzeige",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 8, 1
};

static RSIB0MASK[] =
{ 0x0000, 0x0000, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x7FFF, 0xFFF8, 0x7FFF, 0xFFF8, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x3FFF, 0xFFF0, 0x2400, 0x00B0, 
  0x2400, 0x0090, 0x3FFF, 0xFFF0, 
  0x2000, 0x0090, 0x2E7E, 0xFE90, 
  0x2E7E, 0xFE90, 0x2E7E, 0xFE90, 
  0x2000, 0x0090, 0x2E7E, 0xFE90, 
  0x2E7E, 0xFE90, 0x2E7E, 0xFE90, 
  0x2000, 0x0090, 0x2000, 0x0090, 
  0x2000, 0x0090, 0x2000, 0x0090, 
  0x2E7E, 0xFE90, 0x2E7E, 0xFE90, 
  0x2E7E, 0xFE90, 0x2E7E, 0xFE90, 
  0x2000, 0x0090, 0x3FFF, 0xFFF0, 
  0x2400, 0x0090, 0x2400, 0x00B0, 
  0x3FFF, 0xFFF0, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "EC4 Cue-List", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 EC4_MENU ****************************************************/
        -1,        1,        5, G_IBOX    ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0000, 0x0050, 0x0019,
         5,        2,        2, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0050, 0x0201,
         1, MEC4INFO, MEC4KONFIG, G_IBOX    ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0002, 0x0000, 0x0015, 0x0301,
  MEC4KONFIG,       -1,       -1, G_TITLE   ,   /* Object 3 MEC4INFO */
  NONE, NORMAL, (LONG)" Info",
  0x0000, 0x0000, 0x0006, 0x0301,
         2,       -1,       -1, G_TITLE   ,   /* Object 4 MEC4KONFIG */
  NONE, NORMAL, (LONG)" Konfigurieren",
  0x0006, 0x0000, 0x000F, 0x0301,
         0,        6,        8, G_IBOX    ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0301, 0x0050, 0x0017,
         8, MEC4INFOANZEIG, MEC4INFOANZEIG, G_BOX     ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0000, 0x0016, 0x0001,
         6,       -1,       -1, G_STRING  ,   /* Object 7 MEC4INFOANZEIG */
  NONE, NORMAL, (LONG)"  Info anzeigen... ^I ",
  0x0000, 0x0000, 0x0016, 0x0001,
         5, MEC4REIHENF, MEC4REIHENF, G_BOX     ,   /* Object 8  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0008, 0x0000, 0x000E, 0x0001,
         8,       -1,       -1, G_STRING  ,   /* Object 9 MEC4REIHENF */
  LASTOB, NORMAL, (LONG)"  Reihenfolge ",
  0x0000, 0x0000, 0x000E, 0x0001,
  
  /******** Tree 1 EC4_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich im Koordinaten-Fenster",
  0x0001, 0x0001, 0x0028, 0x0001,
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
  
  /******** Tree 2 EC4_DESK ****************************************************/
        -1, EC4ICON , EC4ICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 EC4ICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0400, 0x0001, 0x0009, 0x0802,
  
  /******** Tree 3 EC4_TEXT ****************************************************/
        -1, FEC4N   , FEC4I   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x002E, 0x0009,
  FEC4I   ,       -1,       -1, G_STRING  ,   /* Object 1 FEC4N */
  NONE, NORMAL, (LONG)"  EC4 Cue-List-Editor",
  0x0001, 0x0001, 0x0015, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FEC4I */
  LASTOB, NORMAL, (LONG)" %d Test",
  0x0001, 0x0002, 0x0008, 0x0001,
  
  /******** Tree 4 EC4_INFO ****************************************************/
        -1,        1, EC4IOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0006, 0x0802, 0x0016, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0006, 0x0004, 0x0017, 0x0001,
  EC4IVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0008, 0x0005, 0x0612, 0x0600,
  EC4IVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 EC4IVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0007, 0x0006, 0x0014, 0x0001,
  EC4COMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 EC4IVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0009, 0x0007, 0x0012, 0x0001,
  EC4ISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 EC4COMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0009, 0x0008, 0x0012, 0x0800,
  EC4IAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 EC4ISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0008, 0x0809, 0x0013, 0x0001,
  EC4IOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 EC4IAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 EC4IOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002,
  
  /******** Tree 5 EC4_CUE ****************************************************/
        -1,        1, EC4CUEOK, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021101L,
  0x0000, 0x0000, 0x0455, 0x0810,
        21,        2, EC4EXITTIME, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0001, 0x0001, 0x0418, 0x0805,
  EC4CUETIMET,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[9],
  0x0001, 0x0000, 0x0006, 0x0001,
  EC4ENTRYTIMET,       -1,       -1, G_STRING  ,   /* Object 3 EC4CUETIMET */
  NONE, NORMAL, (LONG)"Cue:",
  0x0001, 0x0001, 0x0004, 0x0001,
  EC4EXITTIMET,       -1,       -1, G_STRING  ,   /* Object 4 EC4ENTRYTIMET */
  NONE, NORMAL, (LONG)"Eintritt:",
  0x0001, 0x0802, 0x0009, 0x0001,
  EC4CUETIME,       -1,       -1, G_STRING  ,   /* Object 5 EC4EXITTIMET */
  NONE, NORMAL, (LONG)"Austritt:",
  0x0001, 0x0004, 0x0009, 0x0001,
  EC4ENTRYTIME,        7,       10, G_BOX     ,   /* Object 6 EC4CUETIME */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x000B, 0x0001, 0x000C, 0x0001,
         8,       -1,       -1, G_FBOXTEXT,   /* Object 7  */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0000, 0x0000, 0x0003, 0x0001,
         9,       -1,       -1, G_FBOXTEXT,   /* Object 8  */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0003, 0x0000, 0x0003, 0x0001,
        10,       -1,       -1, G_FBOXTEXT,   /* Object 9  */
  NONE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0006, 0x0000, 0x0003, 0x0001,
  EC4CUETIME,       -1,       -1, G_FBOXTEXT,   /* Object 10  */
  NONE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0009, 0x0000, 0x0003, 0x0001,
  EC4EXITTIME,       12,       15, G_BOX     ,   /* Object 11 EC4ENTRYTIME */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x000B, 0x0802, 0x000C, 0x0001,
        13,       -1,       -1, G_FBOXTEXT,   /* Object 12  */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0000, 0x0000, 0x0003, 0x0001,
        14,       -1,       -1, G_FBOXTEXT,   /* Object 13  */
  NONE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0003, 0x0000, 0x0003, 0x0001,
        15,       -1,       -1, G_FBOXTEXT,   /* Object 14  */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0006, 0x0000, 0x0003, 0x0001,
  EC4ENTRYTIME,       -1,       -1, G_FBOXTEXT,   /* Object 15  */
  NONE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0009, 0x0000, 0x0003, 0x0001,
         1,       17,       20, G_BOX     ,   /* Object 16 EC4EXITTIME */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x000B, 0x0004, 0x000C, 0x0001,
        18,       -1,       -1, G_FBOXTEXT,   /* Object 17  */
  NONE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0000, 0x0000, 0x0003, 0x0001,
        19,       -1,       -1, G_FBOXTEXT,   /* Object 18  */
  NONE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0003, 0x0000, 0x0003, 0x0001,
        20,       -1,       -1, G_FBOXTEXT,   /* Object 19  */
  NONE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0006, 0x0000, 0x0003, 0x0001,
  EC4EXITTIME,       -1,       -1, G_FBOXTEXT,   /* Object 20  */
  NONE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0009, 0x0000, 0x0003, 0x0001,
        31,       22, EC4SPEEDZ, G_BOX     ,   /* Object 21  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x001B, 0x0001, 0x0410, 0x0805,
        23,       -1,       -1, G_TEXT    ,   /* Object 22  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[22],
  0x0001, 0x0000, 0x0009, 0x0001,
  EC4VOLUME,       -1,       -1, G_STRING  ,   /* Object 23  */
  NONE, NORMAL, (LONG)"Volume:",
  0x0001, 0x0001, 0x0007, 0x0001,
        25,       -1,       -1, G_FTEXT   ,   /* Object 24 EC4VOLUME */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[23],
  0x000B, 0x0001, 0x0003, 0x0001,
  EC4SPEEDX,       -1,       -1, G_STRING  ,   /* Object 25  */
  NONE, NORMAL, (LONG)"Speed-X:",
  0x0001, 0x0002, 0x0008, 0x0001,
        27,       -1,       -1, G_FTEXT   ,   /* Object 26 EC4SPEEDX */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[24],
  0x000B, 0x0002, 0x0004, 0x0001,
  EC4SPEEDY,       -1,       -1, G_STRING  ,   /* Object 27  */
  NONE, NORMAL, (LONG)"Speed-Y:",
  0x0001, 0x0003, 0x0008, 0x0001,
        29,       -1,       -1, G_FTEXT   ,   /* Object 28 EC4SPEEDY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[25],
  0x000B, 0x0003, 0x0004, 0x0001,
  EC4SPEEDZ,       -1,       -1, G_STRING  ,   /* Object 29  */
  NONE, NORMAL, (LONG)"Speed-Z:",
  0x0001, 0x0004, 0x0008, 0x0001,
        21,       -1,       -1, G_FTEXT   ,   /* Object 30 EC4SPEEDZ */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[26],
  0x000B, 0x0004, 0x0004, 0x0001,
        43,       32, EC4TEST , G_BOX     ,   /* Object 31  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x002D, 0x0001, 0x0012, 0x0805,
        33,       -1,       -1, G_TEXT    ,   /* Object 32  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[27],
  0x0001, 0x0000, 0x000E, 0x0001,
        34,       -1,       -1, G_STRING  ,   /* Object 33  */
  NONE, NORMAL, (LONG)"Chan:",
  0x0001, 0x0001, 0x0005, 0x0001,
        35,       -1,       -1, G_STRING  ,   /* Object 34  */
  NONE, NORMAL, (LONG)"Port:",
  0x0001, 0x0002, 0x0005, 0x0001,
        36,       -1,       -1, G_STRING  ,   /* Object 35  */
  NONE, NORMAL, (LONG)"Key:",
  0x0001, 0x0003, 0x0004, 0x0001,
  EC4CHANNEL,       -1,       -1, G_STRING  ,   /* Object 36  */
  NONE, NORMAL, (LONG)"Vel:",
  0x0001, 0x0004, 0x0004, 0x0001,
  EC4PORT ,       -1,       -1, G_FTEXT   ,   /* Object 37 EC4CHANNEL */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0007, 0x0001, 0x0003, 0x0001,
  EC4KEY  ,       -1,       -1, G_FTEXT   ,   /* Object 38 EC4PORT */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0007, 0x0002, 0x0003, 0x0001,
  EC4VELOCITY,       -1,       -1, G_FTEXT   ,   /* Object 39 EC4KEY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0007, 0x0003, 0x0003, 0x0001,
  EC4LEARN,       -1,       -1, G_FTEXT   ,   /* Object 40 EC4VELOCITY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0007, 0x0004, 0x0003, 0x0001,
  EC4TEST ,       -1,       -1, G_BUTTON  ,   /* Object 41 EC4LEARN */
  SELECTABLE, NORMAL, (LONG)"~Learn",
  0x000B, 0x0001, 0x0006, 0x0801,
        31,       -1,       -1, G_BUTTON  ,   /* Object 42 EC4TEST */
  SELECTABLE, NORMAL, (LONG)"~Test",
  0x000B, 0x0803, 0x0006, 0x0801,
        51, EC4SIG1 , EC4POSY , G_BOX     ,   /* Object 43  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0001, 0x0807, 0x0409, 0x0805,
        45,       -1,       -1, G_TEXT    ,   /* Object 44 EC4SIG1 */
  NONE, DRAW3D, (LONG)&rs_tedinfo[32],
  0x0001, 0x0000, 0x0005, 0x0001,
  EC4SIG1CHANNEL,       -1,       -1, G_STRING  ,   /* Object 45  */
  NONE, NORMAL, (LONG)"Ch:",
  0x0001, 0x0001, 0x0003, 0x0001,
        47,       -1,       -1, G_FTEXT   ,   /* Object 46 EC4SIG1CHANNEL */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0005, 0x0001, 0x0003, 0x0001,
  EC4POSX ,       -1,       -1, G_STRING  ,   /* Object 47  */
  NONE, NORMAL, (LONG)"X:",
  0x0001, 0x0002, 0x0002, 0x0001,
        49,       -1,       -1, G_FTEXT   ,   /* Object 48 EC4POSX */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0404, 0x0002, 0x0004, 0x0001,
  EC4POSY ,       -1,       -1, G_STRING  ,   /* Object 49  */
  NONE, NORMAL, (LONG)"Y:",
  0x0001, 0x0003, 0x0002, 0x0001,
        43,       -1,       -1, G_FTEXT   ,   /* Object 50 EC4POSY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0404, 0x0003, 0x0004, 0x0001,
        61, EC4SIG2 , EC4SIG2OFFZ, G_BOX     ,   /* Object 51  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x040B, 0x0807, 0x0409, 0x0805,
        53,       -1,       -1, G_TEXT    ,   /* Object 52 EC4SIG2 */
  NONE, DRAW3D, (LONG)&rs_tedinfo[36],
  0x0001, 0x0000, 0x0005, 0x0001,
  EC4SIG2CHANNEL,       -1,       -1, G_STRING  ,   /* Object 53  */
  NONE, NORMAL, (LONG)"Ch:",
  0x0001, 0x0001, 0x0003, 0x0001,
        55,       -1,       -1, G_FTEXT   ,   /* Object 54 EC4SIG2CHANNEL */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0404, 0x0001, 0x0003, 0x0001,
  EC4SIG2OFFX,       -1,       -1, G_STRING  ,   /* Object 55  */
  NONE, NORMAL, (LONG)"XO:",
  0x0001, 0x0002, 0x0003, 0x0001,
        57,       -1,       -1, G_FTEXT   ,   /* Object 56 EC4SIG2OFFX */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0404, 0x0002, 0x0004, 0x0001,
  EC4SIG2OFFY,       -1,       -1, G_STRING  ,   /* Object 57  */
  NONE, NORMAL, (LONG)"YO:",
  0x0001, 0x0003, 0x0003, 0x0001,
        59,       -1,       -1, G_FTEXT   ,   /* Object 58 EC4SIG2OFFY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[39],
  0x0404, 0x0003, 0x0004, 0x0001,
  EC4SIG2OFFZ,       -1,       -1, G_STRING  ,   /* Object 59  */
  NONE, NORMAL, (LONG)"ZO:",
  0x0001, 0x0004, 0x0003, 0x0001,
        51,       -1,       -1, G_FTEXT   ,   /* Object 60 EC4SIG2OFFZ */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0404, 0x0004, 0x0004, 0x0001,
        71, EC4SIG3 ,       70, G_BOX     ,   /* Object 61  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0016, 0x0807, 0x0409, 0x0805,
        63,       -1,       -1, G_TEXT    ,   /* Object 62 EC4SIG3 */
  NONE, DRAW3D, (LONG)&rs_tedinfo[41],
  0x0001, 0x0000, 0x0005, 0x0001,
        64,       -1,       -1, G_STRING  ,   /* Object 63  */
  NONE, NORMAL, (LONG)"Ch:",
  0x0001, 0x0001, 0x0003, 0x0001,
        65,       -1,       -1, G_FTEXT   ,   /* Object 64  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[42],
  0x0404, 0x0001, 0x0003, 0x0001,
        66,       -1,       -1, G_STRING  ,   /* Object 65  */
  NONE, NORMAL, (LONG)"XO:",
  0x0001, 0x0002, 0x0003, 0x0001,
        67,       -1,       -1, G_FTEXT   ,   /* Object 66  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[43],
  0x0404, 0x0002, 0x0004, 0x0001,
        68,       -1,       -1, G_STRING  ,   /* Object 67  */
  NONE, NORMAL, (LONG)"YO:",
  0x0001, 0x0003, 0x0003, 0x0001,
        69,       -1,       -1, G_FTEXT   ,   /* Object 68  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[44],
  0x0404, 0x0003, 0x0004, 0x0001,
        70,       -1,       -1, G_STRING  ,   /* Object 69  */
  NONE, NORMAL, (LONG)"ZO:",
  0x0001, 0x0004, 0x0003, 0x0001,
        61,       -1,       -1, G_FTEXT   ,   /* Object 70  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[45],
  0x0404, 0x0004, 0x0004, 0x0001,
        81, EC4SIG4 ,       80, G_BOX     ,   /* Object 71  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0420, 0x0807, 0x0409, 0x0805,
        73,       -1,       -1, G_TEXT    ,   /* Object 72 EC4SIG4 */
  NONE, DRAW3D, (LONG)&rs_tedinfo[46],
  0x0001, 0x0000, 0x0005, 0x0001,
        74,       -1,       -1, G_STRING  ,   /* Object 73  */
  NONE, NORMAL, (LONG)"Ch:",
  0x0001, 0x0001, 0x0003, 0x0001,
        75,       -1,       -1, G_FTEXT   ,   /* Object 74  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[47],
  0x0404, 0x0001, 0x0003, 0x0001,
        76,       -1,       -1, G_STRING  ,   /* Object 75  */
  NONE, NORMAL, (LONG)"XO:",
  0x0001, 0x0002, 0x0003, 0x0001,
        77,       -1,       -1, G_FTEXT   ,   /* Object 76  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[48],
  0x0404, 0x0002, 0x0004, 0x0001,
        78,       -1,       -1, G_STRING  ,   /* Object 77  */
  NONE, NORMAL, (LONG)"YO:",
  0x0001, 0x0003, 0x0003, 0x0001,
        79,       -1,       -1, G_FTEXT   ,   /* Object 78  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[49],
  0x0404, 0x0003, 0x0004, 0x0001,
        80,       -1,       -1, G_STRING  ,   /* Object 79  */
  NONE, NORMAL, (LONG)"ZO:",
  0x0001, 0x0004, 0x0003, 0x0001,
        71,       -1,       -1, G_FTEXT   ,   /* Object 80  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[50],
  0x0404, 0x0004, 0x0004, 0x0001,
        91, EC4SIG5 ,       90, G_BOX     ,   /* Object 81  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x002B, 0x0807, 0x0409, 0x0805,
        83,       -1,       -1, G_TEXT    ,   /* Object 82 EC4SIG5 */
  NONE, DRAW3D, (LONG)&rs_tedinfo[51],
  0x0001, 0x0000, 0x0005, 0x0001,
        84,       -1,       -1, G_STRING  ,   /* Object 83  */
  NONE, NORMAL, (LONG)"Ch:",
  0x0001, 0x0001, 0x0003, 0x0001,
        85,       -1,       -1, G_FTEXT   ,   /* Object 84  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[52],
  0x0404, 0x0001, 0x0003, 0x0001,
        86,       -1,       -1, G_STRING  ,   /* Object 85  */
  NONE, NORMAL, (LONG)"XO:",
  0x0001, 0x0002, 0x0003, 0x0001,
        87,       -1,       -1, G_FTEXT   ,   /* Object 86  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[53],
  0x0404, 0x0002, 0x0004, 0x0001,
        88,       -1,       -1, G_STRING  ,   /* Object 87  */
  NONE, NORMAL, (LONG)"YO:",
  0x0001, 0x0003, 0x0003, 0x0001,
        89,       -1,       -1, G_FTEXT   ,   /* Object 88  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[54],
  0x0404, 0x0003, 0x0004, 0x0001,
        90,       -1,       -1, G_STRING  ,   /* Object 89  */
  NONE, NORMAL, (LONG)"ZO:",
  0x0001, 0x0004, 0x0003, 0x0001,
        81,       -1,       -1, G_FTEXT   ,   /* Object 90  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[55],
  0x0404, 0x0004, 0x0004, 0x0001,
       101, EC4SIG6 ,      100, G_BOX     ,   /* Object 91  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0435, 0x0807, 0x0409, 0x0805,
        93,       -1,       -1, G_TEXT    ,   /* Object 92 EC4SIG6 */
  NONE, DRAW3D, (LONG)&rs_tedinfo[56],
  0x0001, 0x0000, 0x0005, 0x0001,
        94,       -1,       -1, G_STRING  ,   /* Object 93  */
  NONE, NORMAL, (LONG)"Ch:",
  0x0001, 0x0001, 0x0003, 0x0001,
        95,       -1,       -1, G_FTEXT   ,   /* Object 94  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[57],
  0x0404, 0x0001, 0x0003, 0x0001,
        96,       -1,       -1, G_STRING  ,   /* Object 95  */
  NONE, NORMAL, (LONG)"XO:",
  0x0001, 0x0002, 0x0003, 0x0001,
        97,       -1,       -1, G_FTEXT   ,   /* Object 96  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[58],
  0x0404, 0x0002, 0x0004, 0x0001,
        98,       -1,       -1, G_STRING  ,   /* Object 97  */
  NONE, NORMAL, (LONG)"YO:",
  0x0001, 0x0003, 0x0003, 0x0001,
        99,       -1,       -1, G_FTEXT   ,   /* Object 98  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[59],
  0x0404, 0x0003, 0x0004, 0x0001,
       100,       -1,       -1, G_STRING  ,   /* Object 99  */
  NONE, NORMAL, (LONG)"ZO:",
  0x0001, 0x0004, 0x0003, 0x0001,
        91,       -1,       -1, G_FTEXT   ,   /* Object 100  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[60],
  0x0404, 0x0004, 0x0004, 0x0001,
       111, EC4SIG7 ,      110, G_BOX     ,   /* Object 101  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0040, 0x0807, 0x0409, 0x0805,
       103,       -1,       -1, G_TEXT    ,   /* Object 102 EC4SIG7 */
  NONE, DRAW3D, (LONG)&rs_tedinfo[61],
  0x0001, 0x0000, 0x0005, 0x0001,
       104,       -1,       -1, G_STRING  ,   /* Object 103  */
  NONE, NORMAL, (LONG)"Ch:",
  0x0001, 0x0001, 0x0003, 0x0001,
       105,       -1,       -1, G_FTEXT   ,   /* Object 104  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[62],
  0x0404, 0x0001, 0x0003, 0x0001,
       106,       -1,       -1, G_STRING  ,   /* Object 105  */
  NONE, NORMAL, (LONG)"XO:",
  0x0001, 0x0002, 0x0003, 0x0001,
       107,       -1,       -1, G_FTEXT   ,   /* Object 106  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[63],
  0x0404, 0x0002, 0x0004, 0x0001,
       108,       -1,       -1, G_STRING  ,   /* Object 107  */
  NONE, NORMAL, (LONG)"YO:",
  0x0001, 0x0003, 0x0003, 0x0001,
       109,       -1,       -1, G_FTEXT   ,   /* Object 108  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[64],
  0x0404, 0x0003, 0x0004, 0x0001,
       110,       -1,       -1, G_STRING  ,   /* Object 109  */
  NONE, NORMAL, (LONG)"ZO:",
  0x0001, 0x0004, 0x0003, 0x0001,
       101,       -1,       -1, G_FTEXT   ,   /* Object 110  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[65],
  0x0404, 0x0004, 0x0004, 0x0001,
  EC4CUEPREVIOUS, EC4SIG8 ,      120, G_BOX     ,   /* Object 111  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x044A, 0x0807, 0x0409, 0x0805,
       113,       -1,       -1, G_TEXT    ,   /* Object 112 EC4SIG8 */
  NONE, DRAW3D, (LONG)&rs_tedinfo[66],
  0x0001, 0x0000, 0x0005, 0x0001,
       114,       -1,       -1, G_STRING  ,   /* Object 113  */
  NONE, NORMAL, (LONG)"Ch:",
  0x0001, 0x0001, 0x0003, 0x0001,
       115,       -1,       -1, G_FTEXT   ,   /* Object 114  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[67],
  0x0404, 0x0001, 0x0003, 0x0001,
       116,       -1,       -1, G_STRING  ,   /* Object 115  */
  NONE, NORMAL, (LONG)"XO:",
  0x0001, 0x0002, 0x0003, 0x0001,
       117,       -1,       -1, G_FTEXT   ,   /* Object 116  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[68],
  0x0404, 0x0002, 0x0004, 0x0001,
       118,       -1,       -1, G_STRING  ,   /* Object 117  */
  NONE, NORMAL, (LONG)"YO:",
  0x0001, 0x0003, 0x0003, 0x0001,
       119,       -1,       -1, G_FTEXT   ,   /* Object 118  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[69],
  0x0404, 0x0003, 0x0004, 0x0001,
       120,       -1,       -1, G_STRING  ,   /* Object 119  */
  NONE, NORMAL, (LONG)"ZO:",
  0x0001, 0x0004, 0x0003, 0x0001,
       111,       -1,       -1, G_FTEXT   ,   /* Object 120  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[70],
  0x0404, 0x0004, 0x0004, 0x0001,
  EC4CUENEXT,       -1,       -1, G_BUTTON  ,   /* Object 121 EC4CUEPREVIOUS */
  SELECTABLE, NORMAL, (LONG)"~Voriger",
  0x0001, 0x000E, 0x0409, 0x0002,
  EC4CUEDELETE,       -1,       -1, G_BUTTON  ,   /* Object 122 EC4CUENEXT */
  SELECTABLE, NORMAL, (LONG)"~N\204chster",
  0x040B, 0x000E, 0x0009, 0x0002,
  EC4CUEINSERT,       -1,       -1, G_BUTTON  ,   /* Object 123 EC4CUEDELETE */
  SELECTABLE, NORMAL, (LONG)"~Delete",
  0x0016, 0x000E, 0x0009, 0x0002,
  EC4CUESTANDARD,       -1,       -1, G_BUTTON  ,   /* Object 124 EC4CUEINSERT */
  SELECTABLE, NORMAL, (LONG)"~Insert",
  0x0420, 0x000E, 0x0009, 0x0002,
  EC4CUEHELP,       -1,       -1, G_BUTTON  ,   /* Object 125 EC4CUESTANDARD */
  SELECTABLE, NORMAL, (LONG)"~Standard",
  0x002B, 0x000E, 0x0009, 0x0002,
  EC4CUECANCEL,       -1,       -1, G_BUTTON  ,   /* Object 126 EC4CUEHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0435, 0x000E, 0x0009, 0x0002,
  EC4CUEOK,       -1,       -1, G_BUTTON  ,   /* Object 127 EC4CUECANCEL */
  SELECTABLE|EXIT, NORMAL, (LONG)"~Abbruch",
  0x0040, 0x000E, 0x0009, 0x0002,
         0,       -1,       -1, G_BUTTON  ,   /* Object 128 EC4CUEOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"~OK",
  0x004B, 0x000E, 0x0009, 0x0002,
  
  /******** Tree 6 EC4_SETUP ****************************************************/
        -1,        1,       13, G_BOX     ,   /* Object 0 EC4SETUPROOT */
  NONE, NORMAL, (LONG)0x00021101L,
  0x0000, 0x0000, 0x0441, 0x0008,
  EC4STANDARD, EC4SETDEC,        6, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0401, 0x0004, 0x0014, 0x0803,
  EC4SETINC,       -1,       -1, G_BUTTON  ,   /* Object 2 EC4SETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0002,
  EC4SETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 3 EC4SETINC */
  SELECTABLE, CROSSED, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0002,
  EC4SETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 4 EC4SETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0402, 0x0008, 0x0001,
         6,       -1,       -1, G_BUTTON  ,   /* Object 5 EC4SETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0402, 0x0008, 0x0001,
         1,        7, EC4SETNR, G_BOX     ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0002,
  EC4SETNR,       -1,       -1, G_STRING  ,   /* Object 7  */
  NONE, NORMAL, (LONG)"EC4-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0002,
         6,       -1,       -1, G_FTEXT   ,   /* Object 8 EC4SETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[71],
  0x0008, 0x0000, 0x0006, 0x0002,
  EC4CANCEL,       -1,       -1, G_BUTTON  ,   /* Object 9 EC4STANDARD */
  SELECTABLE, NORMAL, (LONG)"~Standard",
  0x0421, 0x0804, 0x0009, 0x0002,
  EC4HELP ,       -1,       -1, G_BUTTON  ,   /* Object 10 EC4CANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x002C, 0x0804, 0x0009, 0x0002,
  EC4OK   ,       -1,       -1, G_BUTTON  ,   /* Object 11 EC4HELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0416, 0x0804, 0x0009, 0x0002,
        13,       -1,       -1, G_BUTTON  ,   /* Object 12 EC4OK */
  SELECTABLE|DEFAULT, NORMAL, (LONG)"~OK",
  0x0436, 0x0804, 0x0009, 0x0002,
  EC4SETUPROOT,       14,       24, G_BOX     ,   /* Object 13  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0001, 0x0001, 0x043F, 0x0802,
        15,       -1,       -1, G_TEXT    ,   /* Object 14  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[72],
  0x0001, 0x0000, 0x0007, 0x0001,
        16,       -1,       -1, G_BUTTON  ,   /* Object 15  */
  SELECTABLE, NORMAL, (LONG)"Cue",
  0x0001, 0x0001, 0x0405, 0x0001,
        17,       -1,       -1, G_BUTTON  ,   /* Object 16  */
  SELECTABLE, NORMAL, (LONG)"Start",
  0x0007, 0x0001, 0x0405, 0x0001,
        18,       -1,       -1, G_BUTTON  ,   /* Object 17  */
  SELECTABLE, NORMAL, (LONG)"Stop",
  0x000D, 0x0001, 0x0405, 0x0001,
        19,       -1,       -1, G_BUTTON  ,   /* Object 18  */
  SELECTABLE, NORMAL, (LONG)"Ch",
  0x0014, 0x0001, 0x0405, 0x0001,
        20,       -1,       -1, G_BUTTON  ,   /* Object 19  */
  SELECTABLE, NORMAL, (LONG)"Vol",
  0x001A, 0x0001, 0x0405, 0x0001,
        21,       -1,       -1, G_BUTTON  ,   /* Object 20  */
  SELECTABLE, NORMAL, (LONG)"Spd",
  0x0020, 0x0001, 0x0405, 0x0001,
        22,       -1,       -1, G_BUTTON  ,   /* Object 21  */
  SELECTABLE, NORMAL, (LONG)"MKey",
  0x0027, 0x0001, 0x0405, 0x0001,
        23,       -1,       -1, G_BUTTON  ,   /* Object 22  */
  SELECTABLE, NORMAL, (LONG)"MVel",
  0x002D, 0x0001, 0x0405, 0x0001,
        24,       -1,       -1, G_BUTTON  ,   /* Object 23  */
  SELECTABLE, NORMAL, (LONG)"MCha",
  0x0033, 0x0001, 0x0405, 0x0001,
        13,       -1,       -1, G_BUTTON  ,   /* Object 24  */
  SELECTABLE|LASTOB, NORMAL, (LONG)"MPo",
  0x0039, 0x0001, 0x0405, 0x0001
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 EC4_MENU */
  &rs_object[10],   /* Tree  1 EC4_HELP */
  &rs_object[24],   /* Tree  2 EC4_DESK */
  &rs_object[26],   /* Tree  3 EC4_TEXT */
  &rs_object[29],   /* Tree  4 EC4_INFO */
  &rs_object[40],   /* Tree  5 EC4_CUE  */
  &rs_object[169]    /* Tree  6 EC4_SETUP */
};
