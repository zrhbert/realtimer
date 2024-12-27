/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "PAR_MOD.H"

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
{ "<->",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 4, 1,
  "->|",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 4, 1,
  "LFO 1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Umk",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 4, 1,
  "Paus",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 5, 1,
  "LFO 2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Umk",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 4, 1,
  "Paus",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 5, 1,
  "MTR",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Umk",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 4, 1,
  "Paus",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 5, 1,
  "REC",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "PUF",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Zeitl",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 6, 1,
  "Pause",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 6, 1,
  "PLAY",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "SpIn",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 5, 1,
  "SpOu",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 5, 1,
  "MAE",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "PAR Parameter-Feld",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 19, 1,
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
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 22
};

static RSIB0MASK[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x7FFF, 0xFFFE, 
  0x7000, 0x000E, 0x7FFF, 0xFFFE, 
  0x4000, 0x0002, 0x5DB6, 0xDBBA, 
  0x41B6, 0xD802, 0x4000, 0x0002, 
  0x5DB6, 0xDBBA, 0x41B6, 0xD802, 
  0x4000, 0x0002, 0x5DB6, 0xDBBA, 
  0x41B6, 0xD802, 0x4000, 0x0002, 
  0x5DB6, 0xC3BA, 0x41B6, 0xC002, 
  0x4000, 0x0002, 0x5DB6, 0xC3BA, 
  0x41B6, 0xC002, 0x4000, 0x0002, 
  0x7FFF, 0xFFFE, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "PAR Param.", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 PAR_MENU ****************************************************/
        -1,        1,        5, G_IBOX    ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0000, 0x0050, 0x0019,
         5,        2,        2, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0050, 0x0201,
         1, MPARINFO, MPARKONFIG, G_IBOX    ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0002, 0x0000, 0x0015, 0x0301,
  MPARKONFIG,       -1,       -1, G_TITLE   ,   /* Object 3 MPARINFO */
  NONE, NORMAL, (LONG)" Info",
  0x0000, 0x0000, 0x0006, 0x0301,
         2,       -1,       -1, G_TITLE   ,   /* Object 4 MPARKONFIG */
  NONE, NORMAL, (LONG)" Konfigurieren",
  0x0006, 0x0000, 0x000F, 0x0301,
         0,        6,        8, G_IBOX    ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0301, 0x0050, 0x0017,
         8, MPARINFOANZEIG, MPARINFOANZEIG, G_BOX     ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0000, 0x0016, 0x0001,
         6,       -1,       -1, G_STRING  ,   /* Object 7 MPARINFOANZEIG */
  NONE, NORMAL, (LONG)"  Info anzeigen... ^I ",
  0x0000, 0x0000, 0x0016, 0x0001,
         5, MPARREIHENF, MPARREIHENF, G_BOX     ,   /* Object 8  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0008, 0x0000, 0x000E, 0x0001,
         8,       -1,       -1, G_STRING  ,   /* Object 9 MPARREIHENF */
  LASTOB, NORMAL, (LONG)"  Reihenfolge ",
  0x0000, 0x0000, 0x000E, 0x0001,
  
  /******** Tree 1 PAR_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich im Parameter-Fenster",
  0x0001, 0x0001, 0x0026, 0x0001,
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
  
  /******** Tree 2 PAR_DESK ****************************************************/
        -1, PARICON , PARICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 PARICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0001, 0x0800, 0x0009, 0x0802,
  
  /******** Tree 3 PAR_SETUP ****************************************************/
        -1,        1,       76, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021111L,
  0x0000, 0x0000, 0x042D, 0x0008,
         5,        2,        4, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0800, 0x042B, 0x0001,
         3,       -1,       -1, G_BUTTON  ,   /* Object 2  */
  NONE, NORMAL, (LONG)"      0   1 2 3 4 5 6 7 8 9 0 1 2 ",
  0x0000, 0x0000, 0x0422, 0x0001,
         4,       -1,       -1, G_BOXTEXT ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0422, 0x0000, 0x0404, 0x0001,
         1,       -1,       -1, G_BOXTEXT ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0027, 0x0000, 0x0404, 0x0001,
        21, PARLFA  , PARLFAPAUSE, G_BOX     ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0801, 0x042B, 0x0001,
  PARLFA1 ,       -1,       -1, G_BOXTEXT ,   /* Object 6 PARLFA */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0000, 0x0000, 0x0405, 0x0001,
         8,       -1,       -1, G_BOX     ,   /* Object 7 PARLFA1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0409, 0x0000, 0x0002, 0x0001,
         9,       -1,       -1, G_BOX     ,   /* Object 8  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040B, 0x0000, 0x0002, 0x0001,
        10,       -1,       -1, G_BOX     ,   /* Object 9  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040D, 0x0000, 0x0002, 0x0001,
        11,       -1,       -1, G_BOX     ,   /* Object 10  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040F, 0x0000, 0x0002, 0x0001,
        12,       -1,       -1, G_BOX     ,   /* Object 11  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0411, 0x0000, 0x0002, 0x0001,
        13,       -1,       -1, G_BOX     ,   /* Object 12  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0413, 0x0000, 0x0002, 0x0001,
        14,       -1,       -1, G_BOX     ,   /* Object 13  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0415, 0x0000, 0x0002, 0x0001,
        15,       -1,       -1, G_BOX     ,   /* Object 14  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0417, 0x0000, 0x0002, 0x0001,
        16,       -1,       -1, G_BOX     ,   /* Object 15  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0419, 0x0000, 0x0002, 0x0001,
        17,       -1,       -1, G_BOX     ,   /* Object 16  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x041B, 0x0000, 0x0002, 0x0001,
        18,       -1,       -1, G_BOX     ,   /* Object 17  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x041D, 0x0000, 0x0002, 0x0001,
  PARLFAUMKEHR,       -1,       -1, G_BOX     ,   /* Object 18  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x041F, 0x0000, 0x0002, 0x0001,
  PARLFAPAUSE,       -1,       -1, G_BOXTEXT ,   /* Object 19 PARLFAUMKEHR */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0422, 0x0000, 0x0404, 0x0001,
         5,       -1,       -1, G_BOXTEXT ,   /* Object 20 PARLFAPAUSE */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0027, 0x0000, 0x0404, 0x0001,
        37, PARLFB  , PARLFBPAUSE, G_BOX     ,   /* Object 21  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0802, 0x042B, 0x0001,
  PARLFB1 ,       -1,       -1, G_BOXTEXT ,   /* Object 22 PARLFB */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0000, 0x0000, 0x0405, 0x0001,
        24,       -1,       -1, G_BOX     ,   /* Object 23 PARLFB1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0409, 0x0000, 0x0002, 0x0001,
        25,       -1,       -1, G_BOX     ,   /* Object 24  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040B, 0x0000, 0x0002, 0x0001,
        26,       -1,       -1, G_BOX     ,   /* Object 25  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040D, 0x0000, 0x0002, 0x0001,
        27,       -1,       -1, G_BOX     ,   /* Object 26  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040F, 0x0000, 0x0002, 0x0001,
        28,       -1,       -1, G_BOX     ,   /* Object 27  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0411, 0x0000, 0x0002, 0x0001,
        29,       -1,       -1, G_BOX     ,   /* Object 28  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0413, 0x0000, 0x0002, 0x0001,
        30,       -1,       -1, G_BOX     ,   /* Object 29  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0415, 0x0000, 0x0002, 0x0001,
        31,       -1,       -1, G_BOX     ,   /* Object 30  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0417, 0x0000, 0x0002, 0x0001,
        32,       -1,       -1, G_BOX     ,   /* Object 31  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0419, 0x0000, 0x0002, 0x0001,
        33,       -1,       -1, G_BOX     ,   /* Object 32  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x041B, 0x0000, 0x0002, 0x0001,
        34,       -1,       -1, G_BOX     ,   /* Object 33  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x041D, 0x0000, 0x0002, 0x0001,
  PARLFBUMKEHR,       -1,       -1, G_BOX     ,   /* Object 34  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x041F, 0x0000, 0x0002, 0x0001,
  PARLFBPAUSE,       -1,       -1, G_BOXTEXT ,   /* Object 35 PARLFBUMKEHR */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0422, 0x0000, 0x0404, 0x0001,
        21,       -1,       -1, G_BOXTEXT ,   /* Object 36 PARLFBPAUSE */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0027, 0x0000, 0x0404, 0x0001,
        50, PARMTR  , PARMTRPAUSE, G_BOX     ,   /* Object 37  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0004, 0x042B, 0x0001,
  PARMTR0 ,       -1,       -1, G_BOXTEXT ,   /* Object 38 PARMTR */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0000, 0x0000, 0x0405, 0x0001,
        40,       -1,       -1, G_BOX     ,   /* Object 39 PARMTR0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0405, 0x0000, 0x0002, 0x0001,
        41,       -1,       -1, G_BOX     ,   /* Object 40  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0409, 0x0000, 0x0002, 0x0001,
        42,       -1,       -1, G_BOX     ,   /* Object 41  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040B, 0x0000, 0x0002, 0x0001,
        43,       -1,       -1, G_BOX     ,   /* Object 42  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040D, 0x0000, 0x0002, 0x0001,
        44,       -1,       -1, G_BOX     ,   /* Object 43  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040F, 0x0000, 0x0002, 0x0001,
        45,       -1,       -1, G_BOX     ,   /* Object 44  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0411, 0x0000, 0x0002, 0x0001,
        46,       -1,       -1, G_BOX     ,   /* Object 45  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0413, 0x0000, 0x0002, 0x0001,
        47,       -1,       -1, G_BOX     ,   /* Object 46  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0415, 0x0000, 0x0002, 0x0001,
  PARMTRUMKEHR,       -1,       -1, G_BOX     ,   /* Object 47  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0417, 0x0000, 0x0002, 0x0001,
  PARMTRPAUSE,       -1,       -1, G_BOXTEXT ,   /* Object 48 PARMTRUMKEHR */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0422, 0x0000, 0x0404, 0x0001,
        37,       -1,       -1, G_BOXTEXT ,   /* Object 49 PARMTRPAUSE */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0027, 0x0000, 0x0404, 0x0001,
        61, PARREC  ,       60, G_BOX     ,   /* Object 50  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0805, 0x0419, 0x0001,
  PARREC0 ,       -1,       -1, G_BOXTEXT ,   /* Object 51 PARREC */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0000, 0x0000, 0x0405, 0x0001,
        53,       -1,       -1, G_BOX     ,   /* Object 52 PARREC0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0405, 0x0000, 0x0002, 0x0001,
        54,       -1,       -1, G_BOX     ,   /* Object 53  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0409, 0x0000, 0x0002, 0x0001,
        55,       -1,       -1, G_BOX     ,   /* Object 54  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040B, 0x0000, 0x0002, 0x0001,
        56,       -1,       -1, G_BOX     ,   /* Object 55  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040D, 0x0000, 0x0002, 0x0001,
        57,       -1,       -1, G_BOX     ,   /* Object 56  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040F, 0x0000, 0x0002, 0x0001,
        58,       -1,       -1, G_BOX     ,   /* Object 57  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0411, 0x0000, 0x0002, 0x0001,
        59,       -1,       -1, G_BOX     ,   /* Object 58  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0413, 0x0000, 0x0002, 0x0001,
        60,       -1,       -1, G_BOX     ,   /* Object 59  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0415, 0x0000, 0x0002, 0x0001,
        50,       -1,       -1, G_BOX     ,   /* Object 60  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0417, 0x0000, 0x0002, 0x0001,
        65,       62, PARPUFPAUSE, G_BOX     ,   /* Object 61  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x001D, 0x0805, 0x040F, 0x0001,
  PARPUFZEITLUPE,       -1,       -1, G_BOXTEXT ,   /* Object 62  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0000, 0x0000, 0x0404, 0x0001,
  PARPUFPAUSE,       -1,       -1, G_BOXTEXT ,   /* Object 63 PARPUFZEITLUPE */
  SELECTABLE, DISABLED, (LONG)&rs_tedinfo[13],
  0x0404, 0x0000, 0x0405, 0x0001,
        61,       -1,       -1, G_BOXTEXT ,   /* Object 64 PARPUFPAUSE */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[14],
  0x000A, 0x0000, 0x0405, 0x0001,
        76, PARPLAY ,       75, G_BOX     ,   /* Object 65  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0806, 0x0419, 0x0001,
  PARPLAY0,       -1,       -1, G_BOXTEXT ,   /* Object 66 PARPLAY */
  NONE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0000, 0x0000, 0x0405, 0x0001,
        68,       -1,       -1, G_BOX     ,   /* Object 67 PARPLAY0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0405, 0x0000, 0x0002, 0x0001,
        69,       -1,       -1, G_BOX     ,   /* Object 68  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0409, 0x0000, 0x0002, 0x0001,
        70,       -1,       -1, G_BOX     ,   /* Object 69  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040B, 0x0000, 0x0002, 0x0001,
        71,       -1,       -1, G_BOX     ,   /* Object 70  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040D, 0x0000, 0x0002, 0x0001,
        72,       -1,       -1, G_BOX     ,   /* Object 71  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x040F, 0x0000, 0x0002, 0x0001,
        73,       -1,       -1, G_BOX     ,   /* Object 72  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0411, 0x0000, 0x0002, 0x0001,
        74,       -1,       -1, G_BOX     ,   /* Object 73  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0413, 0x0000, 0x0002, 0x0001,
        75,       -1,       -1, G_BOX     ,   /* Object 74  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0415, 0x0000, 0x0002, 0x0001,
        65,       -1,       -1, G_BOX     ,   /* Object 75  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0417, 0x0000, 0x0002, 0x0001,
         0, PARMAESPIN,       79, G_BOX     ,   /* Object 76  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x001D, 0x0806, 0x040F, 0x0001,
  PARMAESPOUT,       -1,       -1, G_BOXTEXT ,   /* Object 77 PARMAESPIN */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0404, 0x0000, 0x0405, 0x0001,
        79,       -1,       -1, G_BOXTEXT ,   /* Object 78 PARMAESPOUT */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x000A, 0x0000, 0x0405, 0x0001,
        76,       -1,       -1, G_BOXTEXT ,   /* Object 79  */
  SELECTABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[18],
  0x0000, 0x0000, 0x0404, 0x0001,
  
  /******** Tree 4 PAR_TEXT ****************************************************/
        -1, FPARN   , FKOOI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x002E, 0x0009,
  FKOOI   ,       -1,       -1, G_STRING  ,   /* Object 1 FPARN */
  NONE, NORMAL, (LONG)" PAR Parameter-Anzeige ",
  0x0001, 0x0001, 0x0017, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FKOOI */
  LASTOB, NORMAL, (LONG)" %d Test",
  0x0001, 0x0002, 0x0008, 0x0001,
  
  /******** Tree 5 PAR_INFO ****************************************************/
        -1,        1, PARIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0006, 0x0802, 0x0012, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0006, 0x0004, 0x0017, 0x0001,
  PARIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0008, 0x0005, 0x0612, 0x0600,
  PARIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 PARIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0007, 0x0006, 0x0014, 0x0001,
  PARCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 PARIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0009, 0x0007, 0x0012, 0x0001,
  PARISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 PARCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[25],
  0x0009, 0x0008, 0x0012, 0x0800,
  PARIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 PARISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0008, 0x0809, 0x0013, 0x0001,
  PARIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 PARIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 PARIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 PAR_MENU */
  &rs_object[10],   /* Tree  1 PAR_HELP */
  &rs_object[24],   /* Tree  2 PAR_DESK */
  &rs_object[26],   /* Tree  3 PAR_SETUP */
  &rs_object[106],   /* Tree  4 PAR_TEXT */
  &rs_object[109]    /* Tree  5 PAR_INFO */
};
