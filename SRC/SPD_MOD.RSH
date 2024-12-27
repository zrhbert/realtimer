/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "SPD_MOD.H"

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
  "SPD Speed-Feld",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 15, 1,
  "1994 by Cybernetic Arts",
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
{ 0x0000, 0x0000, 0x001F, 0xF000, 
  0x00FF, 0xFE00, 0x01FF, 0xFF00, 
  0x07FF, 0xFFC0, 0x0FFF, 0xFFE0, 
  0x1FFF, 0xFFF0, 0x1FFF, 0xFFF0, 
  0x3FFF, 0xFFF8, 0x3FFF, 0xFFF8, 
  0x7FFF, 0xFFFC, 0x7FFF, 0xFFFC, 
  0x7FFF, 0xFFFC, 0x7FFF, 0xFFFC, 
  0x7FFF, 0xFFFC, 0x7FFF, 0xFFFC, 
  0x7FFF, 0xFFFC, 0x7FFF, 0xFFFC, 
  0x7FFF, 0xFFFC, 0x3FFF, 0xFFF8, 
  0x3FFF, 0xFFF8, 0x1FFF, 0xFFF0, 
  0x1FFF, 0xFFF0, 0x0FFF, 0xFFE0, 
  0x07FF, 0xFFC0, 0x3DFF, 0xFF00, 
  0x3CFF, 0xFE00, 0x3C1F, 0xF000, 
  0x3C00, 0x0000, 0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x000F, 0xE000, 0x0071, 0x1C00, 
  0x0081, 0x0200, 0x0301, 0x0180, 
  0x0480, 0x0240, 0x0840, 0x0420, 
  0x0800, 0x0020, 0x101F, 0xF010, 
  0x101F, 0xF010, 0x2000, 0x0008, 
  0x2000, 0x0008, 0x2007, 0x0008, 
  0x3C0F, 0x0078, 0x203C, 0x0008, 
  0x2070, 0x0008, 0x21C0, 0x0008, 
  0x1700, 0x0010, 0x1000, 0x0010, 
  0x0800, 0x0020, 0x0840, 0x0420, 
  0x0487, 0xC240, 0x0307, 0xC180, 
  0x0080, 0x0200, 0x0070, 0x1C00, 
  0x180F, 0xE000, 0x1800, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "SPD Speed", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 SPD_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich im Speed-Fenster",
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
  
  /******** Tree 1 SPD_DESK ****************************************************/
        -1, SPDICON , SPDICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 SPDICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0401, 0x0800, 0x0009, 0x0802,
  
  /******** Tree 2 SPD_SETUP ****************************************************/
        -1,       -1,       -1, G_BOX     ,   /* Object 0  */
  LASTOB, NORMAL, (LONG)0x00021111L,
  0x0000, 0x0000, 0x002B, 0x0809,
  
  /******** Tree 3 SPD_TEXT ****************************************************/
        -1, FSPDN   , FKOOI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x002E, 0x0009,
  FKOOI   ,       -1,       -1, G_STRING  ,   /* Object 1 FSPDN */
  NONE, NORMAL, (LONG)" SPD Speed",
  0x0001, 0x0001, 0x000A, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FKOOI */
  LASTOB, NORMAL, (LONG)" %d Test",
  0x0001, 0x0002, 0x0008, 0x0001,
  
  /******** Tree 4 SPD_INFO ****************************************************/
        -1,        1, SPDIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x040A, 0x0802, 0x000E, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0006, 0x0004, 0x0017, 0x0001,
         5,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0008, 0x0005, 0x0612, 0x0600,
  SPDIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0007, 0x0006, 0x0014, 0x0001,
  SPDCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 SPDIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0009, 0x0007, 0x0012, 0x0001,
  SPDISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 SPDCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0009, 0x0008, 0x0012, 0x0800,
  SPDIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 SPDISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0008, 0x0809, 0x0013, 0x0001,
  SPDIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 SPDIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 SPDIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 SPD_HELP */
  &rs_object[14],   /* Tree  1 SPD_DESK */
  &rs_object[16],   /* Tree  2 SPD_SETUP */
  &rs_object[17],   /* Tree  3 SPD_TEXT */
  &rs_object[20]    /* Tree  4 SPD_INFO */
};
