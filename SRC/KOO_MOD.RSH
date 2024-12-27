/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "KOO_MOD.H"

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
{ "Sig",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "L-R",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "V-H",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "O-U",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Vol",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "PBr",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "PPos",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "Spd",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "V+Z",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Dehn",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "CM",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
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
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
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
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
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
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "3",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
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
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "4",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
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
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "5",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
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
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "6",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
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
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "7",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
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
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "8",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
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
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 5,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "KOO Koordinatenanzeige",
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
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 22
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
  0x2000, 0x0090, 0x2E6D, 0xB690, 
  0x2000, 0x0090, 0x2E6D, 0xB690, 
  0x2000, 0x0090, 0x2E6D, 0xB690, 
  0x2000, 0x0090, 0x2E6D, 0xB690, 
  0x2000, 0x0090, 0x2E6D, 0xB690, 
  0x2000, 0x0090, 0x2E6D, 0xB690, 
  0x2000, 0x0090, 0x2E6D, 0xB690, 
  0x2000, 0x0090, 0x2E6D, 0xB690, 
  0x2000, 0x0090, 0x3FFF, 0xFFF0, 
  0x2400, 0x0090, 0x2400, 0x00B0, 
  0x3FFF, 0xFFF0, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "KOO Koordin.", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 KOO_MENU ****************************************************/
        -1,        1,        5, G_IBOX    ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0000, 0x0050, 0x0019,
         5,        2,        2, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0050, 0x0201,
         1, MKOOINFO, MKOOKONFIG, G_IBOX    ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0002, 0x0000, 0x0015, 0x0301,
  MKOOKONFIG,       -1,       -1, G_TITLE   ,   /* Object 3 MKOOINFO */
  NONE, NORMAL, (LONG)" Info",
  0x0000, 0x0000, 0x0006, 0x0301,
         2,       -1,       -1, G_TITLE   ,   /* Object 4 MKOOKONFIG */
  NONE, NORMAL, (LONG)" Konfigurieren",
  0x0006, 0x0000, 0x000F, 0x0301,
         0,        6,        8, G_IBOX    ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0301, 0x0050, 0x0017,
         8, MKOOINFOANZEIG, MKOOINFOANZEIG, G_BOX     ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0000, 0x0016, 0x0001,
         6,       -1,       -1, G_STRING  ,   /* Object 7 MKOOINFOANZEIG */
  NONE, NORMAL, (LONG)"  Info anzeigen... ^I ",
  0x0000, 0x0000, 0x0016, 0x0001,
         5, MKOOREIHENF, MKOOREIHENF, G_BOX     ,   /* Object 8  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0008, 0x0000, 0x000E, 0x0001,
         8,       -1,       -1, G_STRING  ,   /* Object 9 MKOOREIHENF */
  LASTOB, NORMAL, (LONG)"  Reihenfolge ",
  0x0000, 0x0000, 0x000E, 0x0001,
  
  /******** Tree 1 KOO_HELP ****************************************************/
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
  
  /******** Tree 2 KOO_DESK ****************************************************/
        -1, KOOICON , KOOICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 KOOICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0400, 0x0001, 0x0009, 0x0802,
  
  /******** Tree 3 KOO_SETUP ****************************************************/
        -1,        1,      109, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021161L,
  0x0000, 0x0000, 0x0038, 0x080B,
        13,        2,       12, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0800, 0x0435, 0x0001,
         3,       -1,       -1, G_BOXTEXT ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0000, 0x0000, 0x0403, 0x0001,
         4,       -1,       -1, G_BOXTEXT ,   /* Object 3  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0403, 0x0000, 0x0005, 0x0001,
         5,       -1,       -1, G_BOXTEXT ,   /* Object 4  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0408, 0x0000, 0x0005, 0x0001,
         6,       -1,       -1, G_BOXTEXT ,   /* Object 5  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[3],
  0x040D, 0x0000, 0x0005, 0x0001,
         7,       -1,       -1, G_BOXTEXT ,   /* Object 6  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0412, 0x0000, 0x0005, 0x0001,
         8,       -1,       -1, G_BOXTEXT ,   /* Object 7  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0417, 0x0000, 0x0005, 0x0001,
         9,       -1,       -1, G_BOXTEXT ,   /* Object 8  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[6],
  0x041C, 0x0000, 0x0005, 0x0001,
        10,       -1,       -1, G_BOXTEXT ,   /* Object 9  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0421, 0x0000, 0x0005, 0x0001,
        11,       -1,       -1, G_BOXTEXT ,   /* Object 10  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0426, 0x0000, 0x0005, 0x0001,
        12,       -1,       -1, G_BOXTEXT ,   /* Object 11  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x042B, 0x0000, 0x0005, 0x0001,
         1,       -1,       -1, G_BOXTEXT ,   /* Object 12  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0430, 0x0000, 0x0005, 0x0001,
        25, KOO0    , KOOCMI0 , G_BOX     ,   /* Object 13  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0002, 0x0435, 0x0001,
  KOOX0   ,       -1,       -1, G_BOXTEXT ,   /* Object 14 KOO0 */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0000, 0x0000, 0x0403, 0x0001,
  KOOY0   ,       -1,       -1, G_FBOXTEXT,   /* Object 15 KOOX0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0403, 0x0000, 0x0005, 0x0001,
  KOOZ0   ,       -1,       -1, G_FBOXTEXT,   /* Object 16 KOOY0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0408, 0x0000, 0x0005, 0x0001,
  KOOVOL0 ,       -1,       -1, G_FBOXTEXT,   /* Object 17 KOOZ0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[14],
  0x040D, 0x0000, 0x0005, 0x0001,
  KOOPBR0 ,       -1,       -1, G_FBOXTEXT,   /* Object 18 KOOVOL0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0412, 0x0000, 0x0005, 0x0001,
  KOOPPOS0,       -1,       -1, G_FBOXTEXT,   /* Object 19 KOOPBR0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0417, 0x0000, 0x0005, 0x0001,
  KOOSPD0 ,       -1,       -1, G_FBOXTEXT,   /* Object 20 KOOPPOS0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x041C, 0x0000, 0x0005, 0x0001,
  KOOVUZ0 ,       -1,       -1, G_FBOXTEXT,   /* Object 21 KOOSPD0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0421, 0x0000, 0x0005, 0x0001,
  KOODEHN0,       -1,       -1, G_FBOXTEXT,   /* Object 22 KOOVUZ0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0426, 0x0000, 0x0005, 0x0001,
  KOOCMI0 ,       -1,       -1, G_FBOXTEXT,   /* Object 23 KOODEHN0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[20],
  0x042B, 0x0000, 0x0005, 0x0001,
        13,       -1,       -1, G_FBOXTEXT,   /* Object 24 KOOCMI0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0430, 0x0000, 0x0005, 0x0001,
        37, KOO1    ,       36, G_BOX     ,   /* Object 25  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0003, 0x0435, 0x0001,
        27,       -1,       -1, G_BOXTEXT ,   /* Object 26 KOO1 */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0000, 0x0000, 0x0403, 0x0001,
        28,       -1,       -1, G_FBOXTEXT,   /* Object 27  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0403, 0x0000, 0x0005, 0x0001,
        29,       -1,       -1, G_FBOXTEXT,   /* Object 28  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0408, 0x0000, 0x0005, 0x0001,
        30,       -1,       -1, G_FBOXTEXT,   /* Object 29  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[25],
  0x040D, 0x0000, 0x0005, 0x0001,
        31,       -1,       -1, G_FBOXTEXT,   /* Object 30  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0412, 0x0000, 0x0005, 0x0001,
        32,       -1,       -1, G_FBOXTEXT,   /* Object 31  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0417, 0x0000, 0x0005, 0x0001,
        33,       -1,       -1, G_FBOXTEXT,   /* Object 32  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[28],
  0x041C, 0x0000, 0x0005, 0x0001,
        34,       -1,       -1, G_FBOXTEXT,   /* Object 33  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0421, 0x0000, 0x0005, 0x0001,
        35,       -1,       -1, G_FBOXTEXT,   /* Object 34  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0426, 0x0000, 0x0005, 0x0001,
        36,       -1,       -1, G_FBOXTEXT,   /* Object 35  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[31],
  0x042B, 0x0000, 0x0005, 0x0001,
        25,       -1,       -1, G_FBOXTEXT,   /* Object 36  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0430, 0x0000, 0x0005, 0x0001,
        49,       38,       48, G_BOX     ,   /* Object 37  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0004, 0x0435, 0x0001,
        39,       -1,       -1, G_BOXTEXT ,   /* Object 38  */
  NONE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0000, 0x0000, 0x0403, 0x0001,
        40,       -1,       -1, G_FBOXTEXT,   /* Object 39  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0403, 0x0000, 0x0005, 0x0001,
        41,       -1,       -1, G_FBOXTEXT,   /* Object 40  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0408, 0x0000, 0x0005, 0x0001,
        42,       -1,       -1, G_FBOXTEXT,   /* Object 41  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[36],
  0x040D, 0x0000, 0x0005, 0x0001,
        43,       -1,       -1, G_FBOXTEXT,   /* Object 42  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0412, 0x0000, 0x0005, 0x0001,
        44,       -1,       -1, G_FBOXTEXT,   /* Object 43  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0417, 0x0000, 0x0005, 0x0001,
        45,       -1,       -1, G_FBOXTEXT,   /* Object 44  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[39],
  0x041C, 0x0000, 0x0005, 0x0001,
        46,       -1,       -1, G_FBOXTEXT,   /* Object 45  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0421, 0x0000, 0x0005, 0x0001,
        47,       -1,       -1, G_FBOXTEXT,   /* Object 46  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[41],
  0x0426, 0x0000, 0x0005, 0x0001,
        48,       -1,       -1, G_FBOXTEXT,   /* Object 47  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[42],
  0x042B, 0x0000, 0x0005, 0x0001,
        37,       -1,       -1, G_FBOXTEXT,   /* Object 48  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[43],
  0x0430, 0x0000, 0x0005, 0x0001,
        61,       50,       60, G_BOX     ,   /* Object 49  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0005, 0x0435, 0x0001,
        51,       -1,       -1, G_BOXTEXT ,   /* Object 50  */
  NONE, NORMAL, (LONG)&rs_tedinfo[44],
  0x0000, 0x0000, 0x0403, 0x0001,
        52,       -1,       -1, G_FBOXTEXT,   /* Object 51  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[45],
  0x0403, 0x0000, 0x0005, 0x0001,
        53,       -1,       -1, G_FBOXTEXT,   /* Object 52  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[46],
  0x0408, 0x0000, 0x0005, 0x0001,
        54,       -1,       -1, G_FBOXTEXT,   /* Object 53  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[47],
  0x040D, 0x0000, 0x0005, 0x0001,
        55,       -1,       -1, G_FBOXTEXT,   /* Object 54  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[48],
  0x0412, 0x0000, 0x0005, 0x0001,
        56,       -1,       -1, G_FBOXTEXT,   /* Object 55  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[49],
  0x0417, 0x0000, 0x0005, 0x0001,
        57,       -1,       -1, G_FBOXTEXT,   /* Object 56  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[50],
  0x041C, 0x0000, 0x0005, 0x0001,
        58,       -1,       -1, G_FBOXTEXT,   /* Object 57  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[51],
  0x0421, 0x0000, 0x0005, 0x0001,
        59,       -1,       -1, G_FBOXTEXT,   /* Object 58  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[52],
  0x0426, 0x0000, 0x0005, 0x0001,
        60,       -1,       -1, G_FBOXTEXT,   /* Object 59  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[53],
  0x042B, 0x0000, 0x0005, 0x0001,
        49,       -1,       -1, G_FBOXTEXT,   /* Object 60  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[54],
  0x0430, 0x0000, 0x0005, 0x0001,
        73,       62,       72, G_BOX     ,   /* Object 61  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0006, 0x0435, 0x0001,
        63,       -1,       -1, G_BOXTEXT ,   /* Object 62  */
  NONE, NORMAL, (LONG)&rs_tedinfo[55],
  0x0000, 0x0000, 0x0403, 0x0001,
        64,       -1,       -1, G_FBOXTEXT,   /* Object 63  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[56],
  0x0403, 0x0000, 0x0005, 0x0001,
        65,       -1,       -1, G_FBOXTEXT,   /* Object 64  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[57],
  0x0408, 0x0000, 0x0005, 0x0001,
        66,       -1,       -1, G_FBOXTEXT,   /* Object 65  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[58],
  0x040D, 0x0000, 0x0005, 0x0001,
        67,       -1,       -1, G_FBOXTEXT,   /* Object 66  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[59],
  0x0412, 0x0000, 0x0005, 0x0001,
        68,       -1,       -1, G_FBOXTEXT,   /* Object 67  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[60],
  0x0417, 0x0000, 0x0005, 0x0001,
        69,       -1,       -1, G_FBOXTEXT,   /* Object 68  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[61],
  0x041C, 0x0000, 0x0005, 0x0001,
        70,       -1,       -1, G_FBOXTEXT,   /* Object 69  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[62],
  0x0421, 0x0000, 0x0005, 0x0001,
        71,       -1,       -1, G_FBOXTEXT,   /* Object 70  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[63],
  0x0426, 0x0000, 0x0005, 0x0001,
        72,       -1,       -1, G_FBOXTEXT,   /* Object 71  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[64],
  0x042B, 0x0000, 0x0005, 0x0001,
        61,       -1,       -1, G_FBOXTEXT,   /* Object 72  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[65],
  0x0430, 0x0000, 0x0005, 0x0001,
        85,       74,       84, G_BOX     ,   /* Object 73  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0007, 0x0435, 0x0001,
        75,       -1,       -1, G_BOXTEXT ,   /* Object 74  */
  NONE, NORMAL, (LONG)&rs_tedinfo[66],
  0x0000, 0x0000, 0x0403, 0x0001,
        76,       -1,       -1, G_FBOXTEXT,   /* Object 75  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[67],
  0x0403, 0x0000, 0x0005, 0x0001,
        77,       -1,       -1, G_FBOXTEXT,   /* Object 76  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[68],
  0x0408, 0x0000, 0x0005, 0x0001,
        78,       -1,       -1, G_FBOXTEXT,   /* Object 77  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[69],
  0x040D, 0x0000, 0x0005, 0x0001,
        79,       -1,       -1, G_FBOXTEXT,   /* Object 78  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[70],
  0x0412, 0x0000, 0x0005, 0x0001,
        80,       -1,       -1, G_FBOXTEXT,   /* Object 79  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[71],
  0x0417, 0x0000, 0x0005, 0x0001,
        81,       -1,       -1, G_FBOXTEXT,   /* Object 80  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[72],
  0x041C, 0x0000, 0x0005, 0x0001,
        82,       -1,       -1, G_FBOXTEXT,   /* Object 81  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[73],
  0x0421, 0x0000, 0x0005, 0x0001,
        83,       -1,       -1, G_FBOXTEXT,   /* Object 82  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[74],
  0x0426, 0x0000, 0x0005, 0x0001,
        84,       -1,       -1, G_FBOXTEXT,   /* Object 83  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[75],
  0x042B, 0x0000, 0x0005, 0x0001,
        73,       -1,       -1, G_FBOXTEXT,   /* Object 84  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[76],
  0x0430, 0x0000, 0x0005, 0x0001,
        97,       86,       96, G_BOX     ,   /* Object 85  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0008, 0x0435, 0x0001,
        87,       -1,       -1, G_BOXTEXT ,   /* Object 86  */
  NONE, NORMAL, (LONG)&rs_tedinfo[77],
  0x0000, 0x0000, 0x0403, 0x0001,
        88,       -1,       -1, G_FBOXTEXT,   /* Object 87  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[78],
  0x0403, 0x0000, 0x0005, 0x0001,
        89,       -1,       -1, G_FBOXTEXT,   /* Object 88  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[79],
  0x0408, 0x0000, 0x0005, 0x0001,
        90,       -1,       -1, G_FBOXTEXT,   /* Object 89  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[80],
  0x040D, 0x0000, 0x0005, 0x0001,
        91,       -1,       -1, G_FBOXTEXT,   /* Object 90  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[81],
  0x0412, 0x0000, 0x0005, 0x0001,
        92,       -1,       -1, G_FBOXTEXT,   /* Object 91  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[82],
  0x0417, 0x0000, 0x0005, 0x0001,
        93,       -1,       -1, G_FBOXTEXT,   /* Object 92  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[83],
  0x041C, 0x0000, 0x0005, 0x0001,
        94,       -1,       -1, G_FBOXTEXT,   /* Object 93  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[84],
  0x0421, 0x0000, 0x0005, 0x0001,
        95,       -1,       -1, G_FBOXTEXT,   /* Object 94  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[85],
  0x0426, 0x0000, 0x0005, 0x0001,
        96,       -1,       -1, G_FBOXTEXT,   /* Object 95  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[86],
  0x042B, 0x0000, 0x0005, 0x0001,
        85,       -1,       -1, G_FBOXTEXT,   /* Object 96  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[87],
  0x0430, 0x0000, 0x0005, 0x0001,
       109,       98,      108, G_BOX     ,   /* Object 97  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0009, 0x0435, 0x0001,
        99,       -1,       -1, G_BOXTEXT ,   /* Object 98  */
  NONE, NORMAL, (LONG)&rs_tedinfo[88],
  0x0000, 0x0000, 0x0403, 0x0001,
       100,       -1,       -1, G_FBOXTEXT,   /* Object 99  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[89],
  0x0403, 0x0000, 0x0005, 0x0001,
       101,       -1,       -1, G_FBOXTEXT,   /* Object 100  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[90],
  0x0408, 0x0000, 0x0005, 0x0001,
       102,       -1,       -1, G_FBOXTEXT,   /* Object 101  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[91],
  0x040D, 0x0000, 0x0005, 0x0001,
       103,       -1,       -1, G_FBOXTEXT,   /* Object 102  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[92],
  0x0412, 0x0000, 0x0005, 0x0001,
       104,       -1,       -1, G_FBOXTEXT,   /* Object 103  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[93],
  0x0417, 0x0000, 0x0005, 0x0001,
       105,       -1,       -1, G_FBOXTEXT,   /* Object 104  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[94],
  0x041C, 0x0000, 0x0005, 0x0001,
       106,       -1,       -1, G_FBOXTEXT,   /* Object 105  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[95],
  0x0421, 0x0000, 0x0005, 0x0001,
       107,       -1,       -1, G_FBOXTEXT,   /* Object 106  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[96],
  0x0426, 0x0000, 0x0005, 0x0001,
       108,       -1,       -1, G_FBOXTEXT,   /* Object 107  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[97],
  0x042B, 0x0000, 0x0005, 0x0001,
        97,       -1,       -1, G_FBOXTEXT,   /* Object 108  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[98],
  0x0430, 0x0000, 0x0005, 0x0001,
         0,      110,      120, G_BOX     ,   /* Object 109  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x000A, 0x0435, 0x0001,
       111,       -1,       -1, G_BOXTEXT ,   /* Object 110  */
  NONE, NORMAL, (LONG)&rs_tedinfo[99],
  0x0000, 0x0000, 0x0403, 0x0001,
       112,       -1,       -1, G_FBOXTEXT,   /* Object 111  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[100],
  0x0403, 0x0000, 0x0005, 0x0001,
       113,       -1,       -1, G_FBOXTEXT,   /* Object 112  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[101],
  0x0408, 0x0000, 0x0005, 0x0001,
       114,       -1,       -1, G_FBOXTEXT,   /* Object 113  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[102],
  0x040D, 0x0000, 0x0005, 0x0001,
       115,       -1,       -1, G_FBOXTEXT,   /* Object 114  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[103],
  0x0412, 0x0000, 0x0005, 0x0001,
       116,       -1,       -1, G_FBOXTEXT,   /* Object 115  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[104],
  0x0417, 0x0000, 0x0005, 0x0001,
       117,       -1,       -1, G_FBOXTEXT,   /* Object 116  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[105],
  0x041C, 0x0000, 0x0005, 0x0001,
       118,       -1,       -1, G_FBOXTEXT,   /* Object 117  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[106],
  0x0421, 0x0000, 0x0005, 0x0001,
       119,       -1,       -1, G_FBOXTEXT,   /* Object 118  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[107],
  0x0426, 0x0000, 0x0005, 0x0001,
       120,       -1,       -1, G_FBOXTEXT,   /* Object 119  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[108],
  0x042B, 0x0000, 0x0005, 0x0001,
       109,       -1,       -1, G_FBOXTEXT,   /* Object 120  */
  SELECTABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[109],
  0x0430, 0x0000, 0x0005, 0x0001,
  
  /******** Tree 4 KOO_TEXT ****************************************************/
        -1, FKOON   , FKOOI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x002E, 0x0009,
  FKOOI   ,       -1,       -1, G_STRING  ,   /* Object 1 FKOON */
  NONE, NORMAL, (LONG)" KOO Koordinaten-Anzeige ",
  0x0001, 0x0001, 0x0019, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FKOOI */
  LASTOB, NORMAL, (LONG)" %d Test",
  0x0001, 0x0002, 0x0008, 0x0001,
  
  /******** Tree 5 KOO_INFO ****************************************************/
        -1,        1, KOOIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[110],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[111],
  0x0006, 0x0802, 0x0016, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[112],
  0x0006, 0x0004, 0x0017, 0x0001,
  KOOIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[113],
  0x0008, 0x0005, 0x0612, 0x0600,
  KOOIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 KOOIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[114],
  0x0007, 0x0006, 0x0014, 0x0001,
  KOOCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 KOOIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[115],
  0x0009, 0x0007, 0x0012, 0x0001,
  KOOISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 KOOCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[116],
  0x0009, 0x0008, 0x0012, 0x0800,
  KOOIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 KOOISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[117],
  0x0008, 0x0809, 0x0013, 0x0001,
  KOOIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 KOOIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[118],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 KOOIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 KOO_MENU */
  &rs_object[10],   /* Tree  1 KOO_HELP */
  &rs_object[24],   /* Tree  2 KOO_DESK */
  &rs_object[26],   /* Tree  3 KOO_SETUP */
  &rs_object[147],   /* Tree  4 KOO_TEXT */
  &rs_object[150]    /* Tree  5 KOO_INFO */
};
