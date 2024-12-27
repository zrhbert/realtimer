/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "MAN_MOD.H"

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
{ "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "Input",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "Calc",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 5, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "Control",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 8, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "Output",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 7, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "MOD 1",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 6, 1,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "MAN Modul-Manager",
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
  "01234567890",
  "Version: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 21,
  "01234567890",
  "Datum: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 19,
  "01234567890",
  "Setups: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 20,
  "01234567890",
  "akt. Set: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 22,
  " 12.12.1992",
  "compiliert: ___________",
  "XXXXXXXXXXX",
  SMALL, 0, TE_LEFT , 0x1180, 0, -1, 12, 24
};

static RSIB0MASK[] =
{ 0x0000, 0x0000, 0x0000, 0xFFFF, 
  0xFFFF, 0x0000, 0xFFFF, 0xFFFF, 
  0x0000, 0xFFFF, 0xFFFF, 0x0000, 
  0xFFFF, 0xFFFF, 0x0000, 0xFFFF, 
  0xFFFF, 0x0000, 0xFFFF, 0xFFFF, 
  0x0000, 0xFFFF, 0xFFFF, 0x0000, 
  0xFFFF, 0xFFFF, 0x0000, 0xFFFF, 
  0xFFFF, 0x0000, 0x07DC, 0x3BE0, 
  0x0000, 0x03DC, 0x3BC0, 0x0000, 
  0xFFFF, 0xFFFF, 0x0000, 0xFFFF, 
  0xFFFF, 0x0000, 0xFFFF, 0xFFFF, 
  0x0000, 0xFFFF, 0xFFFF, 0x0000, 
  0xFFFF, 0xFFFF, 0x0000, 0xFEFF, 
  0xFFFF, 0x0000, 0xFFFF, 0xFFFF, 
  0x0000, 0x03FC, 0x3FC0, 0x0000, 
  0x03DC, 0x3BE0, 0x0000, 0xFFFF, 
  0xFFFF, 0x0000, 0xFFFF, 0xFFFF, 
  0x0000, 0xFFFF, 0xFFFF, 0x0000, 
  0xFFFF, 0xFFFF, 0x0000, 0xFFFF, 
  0xFFFF, 0x0000, 0xFFFF, 0xFFFF, 
  0x0000, 0xFFFF, 0xFFFF, 0x0000, 
  0xFFFF, 0xFFFF, 0x0000, 0xFFFF, 
  0xFFFF, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x7E7E, 0x7E7E, 
  0x0000, 0x4242, 0x4242, 0x0000, 
  0x4242, 0x4242, 0x0000, 0x4242, 
  0x4242, 0x0000, 0x4242, 0x4242, 
  0x0000, 0x4242, 0x4242, 0x0000, 
  0x7E7E, 0x7E7E, 0x0000, 0x0208, 
  0x1040, 0x0000, 0x0108, 0x1080, 
  0x0000, 0x0088, 0x1100, 0x0000, 
  0x0088, 0x1100, 0x0000, 0x7C7F, 
  0xFE7E, 0x0000, 0x4440, 0x0242, 
  0x0000, 0x47C0, 0x03C2, 0x0000, 
  0x4440, 0x0242, 0x0000, 0x7C7F, 
  0xFE7E, 0x0000, 0x0048, 0x1200, 
  0x0000, 0x0088, 0x1100, 0x0000, 
  0x0108, 0x1080, 0x0000, 0x0108, 
  0x1040, 0x0000, 0x7E7E, 0x7E7E, 
  0x0000, 0x4242, 0x4242, 0x0000, 
  0x4242, 0x4242, 0x0000, 0x4242, 
  0x4242, 0x0000, 0x4242, 0x4242, 
  0x0000, 0x4242, 0x4242, 0x0000, 
  0x7E7E, 0x7E7E, 0x0000, 0x0000, 
  0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "MAN Mod Man.", 0x1000|'\0',
    0,   0,  19,   0,  48,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 MAN_MENU ****************************************************/
        -1,        1,        5, G_IBOX    ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0000, 0x0050, 0x0019,
         5,        2,        2, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0050, 0x0201,
         1, MMANINFO, MMANKONFIG, G_IBOX    ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0002, 0x0000, 0x0015, 0x0301,
  MMANKONFIG,       -1,       -1, G_TITLE   ,   /* Object 3 MMANINFO */
  NONE, NORMAL, (LONG)" Info",
  0x0000, 0x0000, 0x0006, 0x0301,
         2,       -1,       -1, G_TITLE   ,   /* Object 4 MMANKONFIG */
  NONE, NORMAL, (LONG)" Konfigurieren",
  0x0006, 0x0000, 0x000F, 0x0301,
         0,        6,        8, G_IBOX    ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0301, 0x0050, 0x0017,
         8, MMANINFOANZEIG, MMANINFOANZEIG, G_BOX     ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0000, 0x0016, 0x0001,
         6,       -1,       -1, G_STRING  ,   /* Object 7 MMANINFOANZEIG */
  NONE, NORMAL, (LONG)"  Info anzeigen... ^I ",
  0x0000, 0x0000, 0x0016, 0x0001,
         5, MMANINPUT, MMANSEQUENCER, G_BOX     ,   /* Object 8  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0008, 0x0000, 0x0013, 0x0004,
  MMANCALC,       -1,       -1, G_STRING  ,   /* Object 9 MMANINPUT */
  NONE, NORMAL, (LONG)"  Input-Module     ",
  0x0000, 0x0000, 0x0013, 0x0001,
  MMANOUTPUT,       -1,       -1, G_STRING  ,   /* Object 10 MMANCALC */
  NONE, NORMAL, (LONG)"  Calc-Module      ",
  0x0000, 0x0001, 0x0013, 0x0001,
  MMANSEQUENCER,       -1,       -1, G_STRING  ,   /* Object 11 MMANOUTPUT */
  NONE, NORMAL, (LONG)"  Output-Module    ",
  0x0000, 0x0002, 0x0013, 0x0001,
         8,       -1,       -1, G_STRING  ,   /* Object 12 MMANSEQUENCER */
  LASTOB, NORMAL, (LONG)"  Sequencer-Module ",
  0x0000, 0x0003, 0x0013, 0x0001,
  
  /******** Tree 1 MAN_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich im Menu-Manager",
  0x0001, 0x0001, 0x0021, 0x0001,
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
  
  /******** Tree 2 MAN_DESK ****************************************************/
        -1, MANICON , MANICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 MANICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0400, 0x0800, 0x0009, 0x0802,
  
  /******** Tree 3 MAN_SETUP ****************************************************/
        -1, MANOK   ,       79, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021101L,
  0x0000, 0x0000, 0x003C, 0x000D,
         2,       -1,       -1, G_BUTTON  ,   /* Object 1 MANOK */
  SELECTABLE|DEFAULT, NORMAL, (LONG)"~OK",
  0x0032, 0x000A, 0x0009, 0x0002,
  MANHELP , MANSETDEC,        7, G_BOX     ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0027, 0x0801, 0x0014, 0x0803,
  MANSETINC,       -1,       -1, G_BUTTON  ,   /* Object 3 MANSETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  MANSETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 4 MANSETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  MANSETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 5 MANSETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
         7,       -1,       -1, G_BUTTON  ,   /* Object 6 MANSETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
         2,        8, MANSETNR, G_BOX     ,   /* Object 7  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  MANSETNR,       -1,       -1, G_STRING  ,   /* Object 8  */
  NONE, NORMAL, (LONG)"MAN-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0801,
         7,       -1,       -1, G_FTEXT   ,   /* Object 9 MANSETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0008, 0x0000, 0x0006, 0x0801,
  MANSTANDARD,       -1,       -1, G_BUTTON  ,   /* Object 10 MANHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0027, 0x000A, 0x0009, 0x0002,
  MANCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 11 MANSTANDARD */
  SELECTABLE, NORMAL, (LONG)"S~tandard",
  0x0027, 0x0007, 0x0009, 0x0002,
        13,       -1,       -1, G_BUTTON  ,   /* Object 12 MANCANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0032, 0x0007, 0x0009, 0x0002,
        35,       14,       34, G_BOX     ,   /* Object 13  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0001, 0x0001, 0x0008, 0x080B,
  MANINPUT0,       -1,       -1, G_BOXTEXT ,   /* Object 14  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[1],
  0x0400, 0x0000, 0x0405, 0x0001,
  MANINPUTTEXT0,       -1,       -1, G_BOX     ,   /* Object 15 MANINPUT0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0001, 0x0002, 0x0001,
  MANINPUT1,       -1,       -1, G_TEXT    ,   /* Object 16 MANINPUTTEXT0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0402, 0x0001, 0x0005, 0x0001,
        18,       -1,       -1, G_BOX     ,   /* Object 17 MANINPUT1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0002, 0x0002, 0x0001,
        19,       -1,       -1, G_TEXT    ,   /* Object 18  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0402, 0x0002, 0x0005, 0x0001,
        20,       -1,       -1, G_BOX     ,   /* Object 19  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0003, 0x0002, 0x0001,
        21,       -1,       -1, G_TEXT    ,   /* Object 20  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0402, 0x0003, 0x0005, 0x0001,
        22,       -1,       -1, G_BOX     ,   /* Object 21  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0004, 0x0002, 0x0001,
        23,       -1,       -1, G_TEXT    ,   /* Object 22  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0402, 0x0004, 0x0005, 0x0001,
        24,       -1,       -1, G_BOX     ,   /* Object 23  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0005, 0x0002, 0x0001,
        25,       -1,       -1, G_TEXT    ,   /* Object 24  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0402, 0x0005, 0x0005, 0x0001,
        26,       -1,       -1, G_BOX     ,   /* Object 25  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0006, 0x0002, 0x0001,
        27,       -1,       -1, G_TEXT    ,   /* Object 26  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0402, 0x0006, 0x0005, 0x0001,
        28,       -1,       -1, G_BOX     ,   /* Object 27  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0007, 0x0002, 0x0001,
        29,       -1,       -1, G_TEXT    ,   /* Object 28  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0402, 0x0007, 0x0005, 0x0001,
        30,       -1,       -1, G_BOX     ,   /* Object 29  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0008, 0x0002, 0x0001,
        31,       -1,       -1, G_TEXT    ,   /* Object 30  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0402, 0x0008, 0x0005, 0x0001,
        32,       -1,       -1, G_BOX     ,   /* Object 31  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x0009, 0x0002, 0x0001,
        33,       -1,       -1, G_TEXT    ,   /* Object 32  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0402, 0x0009, 0x0005, 0x0001,
        34,       -1,       -1, G_BOX     ,   /* Object 33  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0400, 0x000A, 0x0002, 0x0001,
        13,       -1,       -1, G_TEXT    ,   /* Object 34  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0402, 0x000A, 0x0005, 0x0001,
        57,       36,       56, G_BOX     ,   /* Object 35  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x000A, 0x0001, 0x0408, 0x080B,
  MANCALC0,       -1,       -1, G_BOXTEXT ,   /* Object 36  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[12],
  0x0400, 0x0000, 0x0405, 0x0001,
  MANCALCTEXT0,       -1,       -1, G_BOX     ,   /* Object 37 MANCALC0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0001, 0x0002, 0x0001,
  MANCALC1,       -1,       -1, G_TEXT    ,   /* Object 38 MANCALCTEXT0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0003, 0x0001, 0x0005, 0x0001,
        40,       -1,       -1, G_BOX     ,   /* Object 39 MANCALC1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0002, 0x0002, 0x0001,
        41,       -1,       -1, G_TEXT    ,   /* Object 40  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0003, 0x0002, 0x0005, 0x0001,
        42,       -1,       -1, G_BOX     ,   /* Object 41  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0003, 0x0002, 0x0001,
        43,       -1,       -1, G_TEXT    ,   /* Object 42  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0003, 0x0003, 0x0005, 0x0001,
        44,       -1,       -1, G_BOX     ,   /* Object 43  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0004, 0x0002, 0x0001,
        45,       -1,       -1, G_TEXT    ,   /* Object 44  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0003, 0x0004, 0x0005, 0x0001,
        46,       -1,       -1, G_BOX     ,   /* Object 45  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0005, 0x0002, 0x0001,
        47,       -1,       -1, G_TEXT    ,   /* Object 46  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0003, 0x0005, 0x0005, 0x0001,
        48,       -1,       -1, G_BOX     ,   /* Object 47  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0006, 0x0002, 0x0001,
        49,       -1,       -1, G_TEXT    ,   /* Object 48  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0003, 0x0006, 0x0005, 0x0001,
        50,       -1,       -1, G_BOX     ,   /* Object 49  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0007, 0x0002, 0x0001,
        51,       -1,       -1, G_TEXT    ,   /* Object 50  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0003, 0x0007, 0x0005, 0x0001,
        52,       -1,       -1, G_BOX     ,   /* Object 51  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0008, 0x0002, 0x0001,
        53,       -1,       -1, G_TEXT    ,   /* Object 52  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0003, 0x0008, 0x0005, 0x0001,
        54,       -1,       -1, G_BOX     ,   /* Object 53  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0009, 0x0002, 0x0001,
        55,       -1,       -1, G_TEXT    ,   /* Object 54  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0003, 0x0009, 0x0005, 0x0001,
        56,       -1,       -1, G_BOX     ,   /* Object 55  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x000A, 0x0002, 0x0001,
        35,       -1,       -1, G_TEXT    ,   /* Object 56  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0003, 0x000A, 0x0005, 0x0001,
        79,       58,       78, G_BOX     ,   /* Object 57  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x0413, 0x0001, 0x0408, 0x080B,
  MANCONTROL0,       -1,       -1, G_BOXTEXT ,   /* Object 58  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[23],
  0x0400, 0x0000, 0x0007, 0x0001,
  MANCONTROLTEXT0,       -1,       -1, G_BOX     ,   /* Object 59 MANCONTROL0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0001, 0x0002, 0x0001,
  MANCONTROL1,       -1,       -1, G_TEXT    ,   /* Object 60 MANCONTROLTEXT0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0003, 0x0001, 0x0005, 0x0001,
        62,       -1,       -1, G_BOX     ,   /* Object 61 MANCONTROL1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0002, 0x0002, 0x0001,
        63,       -1,       -1, G_TEXT    ,   /* Object 62  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[25],
  0x0003, 0x0002, 0x0005, 0x0001,
        64,       -1,       -1, G_BOX     ,   /* Object 63  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0003, 0x0002, 0x0001,
        65,       -1,       -1, G_TEXT    ,   /* Object 64  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0003, 0x0003, 0x0005, 0x0001,
        66,       -1,       -1, G_BOX     ,   /* Object 65  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0004, 0x0002, 0x0001,
        67,       -1,       -1, G_TEXT    ,   /* Object 66  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0003, 0x0004, 0x0005, 0x0001,
        68,       -1,       -1, G_BOX     ,   /* Object 67  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0005, 0x0002, 0x0001,
        69,       -1,       -1, G_TEXT    ,   /* Object 68  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0003, 0x0005, 0x0005, 0x0001,
        70,       -1,       -1, G_BOX     ,   /* Object 69  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0006, 0x0002, 0x0001,
        71,       -1,       -1, G_TEXT    ,   /* Object 70  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0003, 0x0006, 0x0005, 0x0001,
        72,       -1,       -1, G_BOX     ,   /* Object 71  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0007, 0x0002, 0x0001,
        73,       -1,       -1, G_TEXT    ,   /* Object 72  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0003, 0x0007, 0x0005, 0x0001,
        74,       -1,       -1, G_BOX     ,   /* Object 73  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0008, 0x0002, 0x0001,
        75,       -1,       -1, G_TEXT    ,   /* Object 74  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0003, 0x0008, 0x0005, 0x0001,
        76,       -1,       -1, G_BOX     ,   /* Object 75  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0009, 0x0002, 0x0001,
        77,       -1,       -1, G_TEXT    ,   /* Object 76  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0003, 0x0009, 0x0005, 0x0001,
        78,       -1,       -1, G_BOX     ,   /* Object 77  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x000A, 0x0002, 0x0001,
        57,       -1,       -1, G_TEXT    ,   /* Object 78  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0003, 0x000A, 0x0005, 0x0001,
         0,       80,      100, G_BOX     ,   /* Object 79  */
  NONE, NORMAL, (LONG)0x00011140L,
  0x001D, 0x0001, 0x0408, 0x080B,
  MANOUTPUT0,       -1,       -1, G_BOXTEXT ,   /* Object 80  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[34],
  0x0400, 0x0000, 0x0406, 0x0001,
  MANOUTPUTTEXT0,       -1,       -1, G_BOX     ,   /* Object 81 MANOUTPUT0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0001, 0x0002, 0x0001,
  MANOUTPUT1,       -1,       -1, G_TEXT    ,   /* Object 82 MANOUTPUTTEXT0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0003, 0x0001, 0x0005, 0x0001,
        84,       -1,       -1, G_BOX     ,   /* Object 83 MANOUTPUT1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0002, 0x0002, 0x0001,
        85,       -1,       -1, G_TEXT    ,   /* Object 84  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[36],
  0x0003, 0x0002, 0x0005, 0x0001,
        86,       -1,       -1, G_BOX     ,   /* Object 85  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0003, 0x0002, 0x0001,
        87,       -1,       -1, G_TEXT    ,   /* Object 86  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0003, 0x0003, 0x0005, 0x0001,
        88,       -1,       -1, G_BOX     ,   /* Object 87  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0004, 0x0002, 0x0001,
        89,       -1,       -1, G_TEXT    ,   /* Object 88  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0003, 0x0004, 0x0005, 0x0001,
        90,       -1,       -1, G_BOX     ,   /* Object 89  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0005, 0x0002, 0x0001,
        91,       -1,       -1, G_TEXT    ,   /* Object 90  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[39],
  0x0003, 0x0005, 0x0005, 0x0001,
        92,       -1,       -1, G_BOX     ,   /* Object 91  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0006, 0x0002, 0x0001,
        93,       -1,       -1, G_TEXT    ,   /* Object 92  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0003, 0x0006, 0x0005, 0x0001,
        94,       -1,       -1, G_BOX     ,   /* Object 93  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0007, 0x0002, 0x0001,
        95,       -1,       -1, G_TEXT    ,   /* Object 94  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[41],
  0x0003, 0x0007, 0x0005, 0x0001,
        96,       -1,       -1, G_BOX     ,   /* Object 95  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0008, 0x0002, 0x0001,
        97,       -1,       -1, G_TEXT    ,   /* Object 96  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[42],
  0x0003, 0x0008, 0x0005, 0x0001,
        98,       -1,       -1, G_BOX     ,   /* Object 97  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0009, 0x0002, 0x0001,
        99,       -1,       -1, G_TEXT    ,   /* Object 98  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[43],
  0x0003, 0x0009, 0x0005, 0x0001,
       100,       -1,       -1, G_BOX     ,   /* Object 99  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x000A, 0x0002, 0x0001,
        79,       -1,       -1, G_TEXT    ,   /* Object 100  */
  SELECTABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[44],
  0x0003, 0x000A, 0x0005, 0x0001,
  
  /******** Tree 4 MAN_TEXT ****************************************************/
        -1, FMANN   , FMANI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x002E, 0x0009,
  FMANI   ,       -1,       -1, G_STRING  ,   /* Object 1 FMANN */
  NONE, NORMAL, (LONG)" MAN-Setups ",
  0x0001, 0x0001, 0x000C, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FMANI */
  LASTOB, NORMAL, (LONG)" %d Presets",
  0x0001, 0x0002, 0x000B, 0x0001,
  
  /******** Tree 5 MAN_INFO ****************************************************/
        -1,        1, MANCOMPILE, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[45],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[46],
  0x0408, 0x0802, 0x0011, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[47],
  0x0006, 0x0004, 0x0017, 0x0001,
  MANIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[48],
  0x0008, 0x0005, 0x0612, 0x0600,
  MANIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 MANIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[49],
  0x0007, 0x0006, 0x0014, 0x0001,
  MANISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 6 MANIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[50],
  0x0009, 0x0007, 0x0012, 0x0001,
  MANIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 7 MANISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[51],
  0x0008, 0x0809, 0x0013, 0x0001,
  MANIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 8 MANIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[52],
  0x0006, 0x080A, 0x0015, 0x0001,
  MANCOMPILE,       -1,       -1, G_BUTTON  |0x4000,   /* Object 9 MANIOK */
  SELECTABLE|DEFAULT|EXIT, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002,
         0,       -1,       -1, G_FTEXT   ,   /* Object 10 MANCOMPILE */
  LASTOB, NORMAL, (LONG)&rs_tedinfo[53],
  0x0009, 0x0008, 0x0012, 0x0800
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 MAN_MENU */
  &rs_object[13],   /* Tree  1 MAN_HELP */
  &rs_object[27],   /* Tree  2 MAN_DESK */
  &rs_object[29],   /* Tree  3 MAN_SETUP */
  &rs_object[130],   /* Tree  4 MAN_TEXT */
  &rs_object[133]    /* Tree  5 MAN_INFO */
};
