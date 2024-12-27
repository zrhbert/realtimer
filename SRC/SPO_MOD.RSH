/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "SPO_MOD.H"

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
{ "0",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "3",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "4",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "5",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "6",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "7",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "8",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "Off",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "SPO Offset-Funktionen",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 22, 1,
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
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x00E0, 0x0000, 0x01F0, 0x0000, 
  0x03F8, 0x0000, 0x03F8, 0x0000, 
  0x03F9, 0xF800, 0x00E1, 0xF800, 
  0x00E1, 0xF800, 0x00E1, 0xF800, 
  0x00E3, 0xF800, 0x00E7, 0xF800, 
  0x00EF, 0x8000, 0x00FF, 0x0000, 
  0x00FE, 0x0380, 0x00FC, 0x03C0, 
  0x00FF, 0xFFE0, 0x00FF, 0xFFE0, 
  0x00FF, 0xFFE0, 0x0000, 0x03C0, 
  0x0000, 0x0380, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0040, 0x0000, 
  0x00E0, 0x0000, 0x01F0, 0x0000, 
  0x0040, 0x0000, 0x0040, 0xF000, 
  0x0040, 0x7000, 0x0040, 0x7000, 
  0x0040, 0x9000, 0x0041, 0x0000, 
  0x0042, 0x0000, 0x0044, 0x0000, 
  0x0048, 0x0000, 0x0050, 0x0100, 
  0x0060, 0x0180, 0x007F, 0xFFC0, 
  0x0000, 0x0180, 0x0000, 0x0100, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "SPO Offset", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  75,   8
};

static WORD RSBB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x8000, 0x0000, 
  0x0001, 0xC000, 0x0000, 0x0000, 
  0x8000, 0x0000, 0x0000, 0x8000, 
  0x0000, 0x0000, 0x8000, 0x0000, 
  0x0000, 0x8000, 0x0000, 0x0000, 
  0x80C0, 0x0000, 0x0000, 0x80C0, 
  0x0000, 0x0000, 0x8100, 0x0000, 
  0x0000, 0x8200, 0x0000, 0x0000, 
  0x8400, 0x0000, 0x0000, 0x8800, 
  0x0000, 0x0000, 0x9001, 0x0000, 
  0x0000, 0xA001, 0x8000, 0x0000, 
  0xC001, 0xC000, 0x0000, 0xFFFF, 
  0xE000, 0x0000, 0x0001, 0xC000, 
  0x0000, 0x0001, 0x8000, 0x0000, 
  0x0001, 0x0000, 0x0000, 0x0000, 
  0x0000
};

static WORD RSBB1DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x8000, 
  0x0000, 0x0001, 0xC000, 0x0000, 
  0x0003, 0xE000, 0x0000, 0x0007, 
  0xF000, 0x0000, 0x0000, 0x8000, 
  0x0000, 0x0000, 0x8000, 0x0000, 
  0x0000, 0x8000, 0x0000, 0x0000, 
  0x80C0, 0x0000, 0x0000, 0x80C0, 
  0x0000, 0x0000, 0x8100, 0x0000, 
  0x0000, 0x8200, 0x0000, 0x0000, 
  0x8400, 0x0000, 0x0000, 0x8800, 
  0x0000, 0x0000, 0x9000, 0x0000, 
  0x0000, 0xA000, 0x0000, 0x0000, 
  0xC000, 0x8000, 0x0000, 0xFFFF, 
  0xC000, 0x0000, 0x0000, 0x8000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000
};

static WORD RSBB2DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x8000, 0x0000, 
  0x0001, 0xC000, 0x0000, 0x0000, 
  0x8000, 0x0000, 0x0000, 0x8000, 
  0x0000, 0x0000, 0x8000, 0x0000, 
  0x0000, 0x87E0, 0x0000, 0x0000, 
  0x83E0, 0x0000, 0x0000, 0x81E0, 
  0x0000, 0x0000, 0x81E0, 0x0000, 
  0x0000, 0x8260, 0x0000, 0x0000, 
  0x8420, 0x0000, 0x0000, 0x8800, 
  0x0000, 0x0000, 0x9000, 0x0000, 
  0x0000, 0xA000, 0x0000, 0x0000, 
  0xC000, 0x8000, 0x0000, 0xFFFF, 
  0xC000, 0x0000, 0x0000, 0x8000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000
};

static BITBLK rs_bitblk[] =
{ RSBB0DATA,   6,  23,   0,   0, 0x0001,
  RSBB1DATA,   6,  23,   0,   0, 0x0001,
  RSBB2DATA,   6,  23,   0,   0, 0x0001
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 SPO_SETUP ****************************************************/
        -1,        1, SPOOK   , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021121L,
  0x0000, 0x0000, 0x004C, 0x080A,
         9, SPO0    , SPOZ0   , G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x000A, 0x0800, 0x0007, 0x0805,
  SPOPROPX0,       -1,       -1, G_BOXTEXT ,   /* Object 2 SPO0 */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0000, 0x0000, 0x0007, 0x0001,
  SPOX0   ,       -1,       -1, G_BOXTEXT ,   /* Object 3 SPOPROPX0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0000, 0x0001, 0x0002, 0x0801,
  SPOPROPY0,       -1,       -1, G_FBOXTEXT,   /* Object 4 SPOX0 */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0002, 0x0001, 0x0005, 0x0801,
  SPOY0   ,       -1,       -1, G_BOXTEXT ,   /* Object 5 SPOPROPY0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0000, 0x0802, 0x0002, 0x0801,
  SPOPROPZ0,       -1,       -1, G_FBOXTEXT,   /* Object 6 SPOY0 */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0002, 0x0802, 0x0005, 0x0801,
  SPOZ0   ,       -1,       -1, G_BOXTEXT ,   /* Object 7 SPOPROPZ0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0000, 0x0004, 0x0002, 0x0801,
         1,       -1,       -1, G_FBOXTEXT,   /* Object 8 SPOZ0 */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0002, 0x0004, 0x0005, 0x0801,
        17, SPO1    ,       16, G_BOX     ,   /* Object 9  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x0013, 0x0800, 0x0007, 0x0805,
        11,       -1,       -1, G_BOXTEXT ,   /* Object 10 SPO1 */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0000, 0x0000, 0x0007, 0x0001,
        12,       -1,       -1, G_BOXTEXT ,   /* Object 11  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0000, 0x0001, 0x0002, 0x0801,
        13,       -1,       -1, G_FBOXTEXT,   /* Object 12  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0002, 0x0001, 0x0005, 0x0801,
        14,       -1,       -1, G_BOXTEXT ,   /* Object 13  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0000, 0x0802, 0x0002, 0x0801,
        15,       -1,       -1, G_FBOXTEXT,   /* Object 14  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0002, 0x0802, 0x0005, 0x0801,
        16,       -1,       -1, G_BOXTEXT ,   /* Object 15  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0000, 0x0004, 0x0002, 0x0801,
         9,       -1,       -1, G_FBOXTEXT,   /* Object 16  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0002, 0x0004, 0x0005, 0x0801,
        25,       18,       24, G_BOX     ,   /* Object 17  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x001A, 0x0800, 0x0007, 0x0805,
        19,       -1,       -1, G_BOXTEXT ,   /* Object 18  */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0000, 0x0000, 0x0007, 0x0001,
        20,       -1,       -1, G_BOXTEXT ,   /* Object 19  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0000, 0x0001, 0x0002, 0x0801,
        21,       -1,       -1, G_FBOXTEXT,   /* Object 20  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0002, 0x0001, 0x0005, 0x0801,
        22,       -1,       -1, G_BOXTEXT ,   /* Object 21  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0000, 0x0802, 0x0002, 0x0801,
        23,       -1,       -1, G_FBOXTEXT,   /* Object 22  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0002, 0x0802, 0x0005, 0x0801,
        24,       -1,       -1, G_BOXTEXT ,   /* Object 23  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0000, 0x0004, 0x0002, 0x0801,
        17,       -1,       -1, G_FBOXTEXT,   /* Object 24  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0002, 0x0004, 0x0005, 0x0801,
        33,       26,       32, G_BOX     ,   /* Object 25  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x0021, 0x0800, 0x0007, 0x0805,
        27,       -1,       -1, G_BOXTEXT ,   /* Object 26  */
  NONE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0000, 0x0000, 0x0007, 0x0001,
        28,       -1,       -1, G_BOXTEXT ,   /* Object 27  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0000, 0x0001, 0x0002, 0x0801,
        29,       -1,       -1, G_FBOXTEXT,   /* Object 28  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0002, 0x0001, 0x0005, 0x0801,
        30,       -1,       -1, G_BOXTEXT ,   /* Object 29  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0000, 0x0802, 0x0002, 0x0801,
        31,       -1,       -1, G_FBOXTEXT,   /* Object 30  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[25],
  0x0002, 0x0802, 0x0005, 0x0801,
        32,       -1,       -1, G_BOXTEXT ,   /* Object 31  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0000, 0x0004, 0x0002, 0x0801,
        25,       -1,       -1, G_FBOXTEXT,   /* Object 32  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0002, 0x0004, 0x0005, 0x0801,
        41,       34,       40, G_BOX     ,   /* Object 33  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x0028, 0x0800, 0x0007, 0x0805,
        35,       -1,       -1, G_BOXTEXT ,   /* Object 34  */
  NONE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0000, 0x0000, 0x0007, 0x0001,
        36,       -1,       -1, G_BOXTEXT ,   /* Object 35  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0000, 0x0001, 0x0002, 0x0801,
        37,       -1,       -1, G_FBOXTEXT,   /* Object 36  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0002, 0x0001, 0x0005, 0x0801,
        38,       -1,       -1, G_BOXTEXT ,   /* Object 37  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0000, 0x0802, 0x0002, 0x0801,
        39,       -1,       -1, G_FBOXTEXT,   /* Object 38  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0002, 0x0802, 0x0005, 0x0801,
        40,       -1,       -1, G_BOXTEXT ,   /* Object 39  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0000, 0x0004, 0x0002, 0x0801,
        33,       -1,       -1, G_FBOXTEXT,   /* Object 40  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0002, 0x0004, 0x0005, 0x0801,
        49,       42,       48, G_BOX     ,   /* Object 41  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x002F, 0x0800, 0x0007, 0x0805,
        43,       -1,       -1, G_BOXTEXT ,   /* Object 42  */
  NONE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0000, 0x0000, 0x0007, 0x0001,
        44,       -1,       -1, G_BOXTEXT ,   /* Object 43  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[36],
  0x0000, 0x0001, 0x0002, 0x0801,
        45,       -1,       -1, G_FBOXTEXT,   /* Object 44  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0002, 0x0001, 0x0005, 0x0801,
        46,       -1,       -1, G_BOXTEXT ,   /* Object 45  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0000, 0x0802, 0x0002, 0x0801,
        47,       -1,       -1, G_FBOXTEXT,   /* Object 46  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[39],
  0x0002, 0x0802, 0x0005, 0x0801,
        48,       -1,       -1, G_BOXTEXT ,   /* Object 47  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0000, 0x0004, 0x0002, 0x0801,
        41,       -1,       -1, G_FBOXTEXT,   /* Object 48  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[41],
  0x0002, 0x0004, 0x0005, 0x0801,
        57,       50,       56, G_BOX     ,   /* Object 49  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x0036, 0x0800, 0x0007, 0x0805,
        51,       -1,       -1, G_BOXTEXT ,   /* Object 50  */
  NONE, NORMAL, (LONG)&rs_tedinfo[42],
  0x0000, 0x0000, 0x0007, 0x0001,
        52,       -1,       -1, G_BOXTEXT ,   /* Object 51  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[43],
  0x0000, 0x0001, 0x0002, 0x0801,
        53,       -1,       -1, G_FBOXTEXT,   /* Object 52  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[44],
  0x0002, 0x0001, 0x0005, 0x0801,
        54,       -1,       -1, G_BOXTEXT ,   /* Object 53  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[45],
  0x0000, 0x0802, 0x0002, 0x0801,
        55,       -1,       -1, G_FBOXTEXT,   /* Object 54  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[46],
  0x0002, 0x0802, 0x0005, 0x0801,
        56,       -1,       -1, G_BOXTEXT ,   /* Object 55  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[47],
  0x0000, 0x0004, 0x0002, 0x0801,
        49,       -1,       -1, G_FBOXTEXT,   /* Object 56  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[48],
  0x0002, 0x0004, 0x0005, 0x0801,
        65,       58,       64, G_BOX     ,   /* Object 57  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x003D, 0x0800, 0x0007, 0x0805,
        59,       -1,       -1, G_BOXTEXT ,   /* Object 58  */
  NONE, NORMAL, (LONG)&rs_tedinfo[49],
  0x0000, 0x0000, 0x0007, 0x0001,
        60,       -1,       -1, G_BOXTEXT ,   /* Object 59  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[50],
  0x0000, 0x0001, 0x0002, 0x0801,
        61,       -1,       -1, G_FBOXTEXT,   /* Object 60  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[51],
  0x0002, 0x0001, 0x0005, 0x0801,
        62,       -1,       -1, G_BOXTEXT ,   /* Object 61  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[52],
  0x0000, 0x0802, 0x0002, 0x0801,
        63,       -1,       -1, G_FBOXTEXT,   /* Object 62  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[53],
  0x0002, 0x0802, 0x0005, 0x0801,
        64,       -1,       -1, G_BOXTEXT ,   /* Object 63  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[54],
  0x0000, 0x0004, 0x0002, 0x0801,
        57,       -1,       -1, G_FBOXTEXT,   /* Object 64  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[55],
  0x0002, 0x0004, 0x0005, 0x0801,
        73,       66,       72, G_BOX     ,   /* Object 65  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x0044, 0x0800, 0x0007, 0x0805,
        67,       -1,       -1, G_BOXTEXT ,   /* Object 66  */
  NONE, NORMAL, (LONG)&rs_tedinfo[56],
  0x0000, 0x0000, 0x0007, 0x0001,
        68,       -1,       -1, G_BOXTEXT ,   /* Object 67  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[57],
  0x0000, 0x0001, 0x0002, 0x0801,
        69,       -1,       -1, G_FBOXTEXT,   /* Object 68  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[58],
  0x0002, 0x0001, 0x0005, 0x0801,
        70,       -1,       -1, G_BOXTEXT ,   /* Object 69  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[59],
  0x0000, 0x0802, 0x0002, 0x0801,
        71,       -1,       -1, G_FBOXTEXT,   /* Object 70  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[60],
  0x0002, 0x0802, 0x0005, 0x0801,
        72,       -1,       -1, G_BOXTEXT ,   /* Object 71  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[61],
  0x0000, 0x0004, 0x0002, 0x0801,
        65,       -1,       -1, G_FBOXTEXT,   /* Object 72  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[62],
  0x0002, 0x0004, 0x0005, 0x0801,
        79,       74,       74, G_BOX     ,   /* Object 73  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0801, 0x0009, 0x0804,
        73,       75,       78, G_BOX     ,   /* Object 74  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0000, 0x0000, 0x0009, 0x0804,
        76,       -1,       -1, G_IMAGE   ,   /* Object 75  */
  NONE, NORMAL, (LONG)&rs_bitblk[0],
  0x0003, 0x0000, 0x0006, 0x0701,
        77,       -1,       -1, G_IMAGE   ,   /* Object 76  */
  NONE, NORMAL, (LONG)&rs_bitblk[1],
  0x0003, 0x0801, 0x0006, 0x0701,
        78,       -1,       -1, G_IMAGE   ,   /* Object 77  */
  NONE, NORMAL, (LONG)&rs_bitblk[2],
  0x0003, 0x0003, 0x0006, 0x0701,
        74,       -1,       -1, G_TEXT    ,   /* Object 78  */
  NONE, NORMAL, (LONG)&rs_tedinfo[63],
  0x0001, 0x0800, 0x0003, 0x0803,
  SPOLEARN, SPOSETDEC,       84, G_BOX     ,   /* Object 79  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0001, 0x0806, 0x0014, 0x0803,
  SPOSETINC,       -1,       -1, G_BUTTON  ,   /* Object 80 SPOSETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  SPOSETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 81 SPOSETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  SPOSETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 82 SPOSETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
        84,       -1,       -1, G_BUTTON  ,   /* Object 83 SPOSETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
        79,       85, SPOSETNR, G_BOX     ,   /* Object 84  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  SPOSETNR,       -1,       -1, G_STRING  ,   /* Object 85  */
  NONE, NORMAL, (LONG)"SPO-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0801,
        84,       -1,       -1, G_FTEXT   ,   /* Object 86 SPOSETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[64],
  0x0008, 0x0000, 0x0006, 0x0801,
  SPOSTANDARD,       -1,       -1, G_BUTTON  ,   /* Object 87 SPOLEARN */
  SELECTABLE, NORMAL, (LONG)"~Lernen",
  0x0018, 0x0007, 0x0008, 0x0002,
  SPOHELP ,       -1,       -1, G_BUTTON  ,   /* Object 88 SPOSTANDARD */
  SELECTABLE, NORMAL, (LONG)"S~tandard",
  0x0022, 0x0007, 0x0009, 0x0002,
  SPOCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 89 SPOHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x002C, 0x0007, 0x0008, 0x0002,
  SPOOK   ,       -1,       -1, G_BUTTON  ,   /* Object 90 SPOCANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0036, 0x0007, 0x0008, 0x0002,
         0,       -1,       -1, G_BUTTON  ,   /* Object 91 SPOOK */
  SELECTABLE|DEFAULT|LASTOB, NORMAL, (LONG)"~OK",
  0x0040, 0x0007, 0x0008, 0x0002,
  
  /******** Tree 1 SPO_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der SPS-Einstellungs-Box",
  0x0001, 0x0001, 0x0031, 0x0001,
         3,       -1,       -1, G_STRING  ,   /* Object 2  */
  NONE, NORMAL, (LONG)"Links sehen Sie die Bezeichnungen f\201r die einstellbaren",
  0x0001, 0x0003, 0x0039, 0x0001,
         4,       -1,       -1, G_STRING  ,   /* Object 3  */
  NONE, NORMAL, (LONG)"Werte, oben die Nummern der internen Signale.",
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
  
  /******** Tree 2 SPO_DESK ****************************************************/
        -1, SPOICON , SPOICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 SPOICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0004, 0x0006, 0x0309, 0x0802,
  
  /******** Tree 3 SPO_TEXT ****************************************************/
        -1, FSPON   , FSPOI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FSPOI   ,       -1,       -1, G_STRING  ,   /* Object 1 FSPON */
  NONE, NORMAL, (LONG)" SPO-Setups ",
  0x0002, 0x0801, 0x000C, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FSPOI */
  LASTOB, NORMAL, (LONG)" %d Presets",
  0x0002, 0x0802, 0x000B, 0x0001,
  
  /******** Tree 4 SPO_INFO ****************************************************/
        -1,        1, SPOIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[65],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[66],
  0x0007, 0x0802, 0x0015, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[67],
  0x0006, 0x0004, 0x0017, 0x0001,
  SPOIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[68],
  0x0008, 0x0005, 0x0612, 0x0600,
  SPOIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 SPOIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[69],
  0x0007, 0x0006, 0x0014, 0x0001,
  SPOCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 SPOIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[70],
  0x0009, 0x0007, 0x0012, 0x0001,
  SPOISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 SPOCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[71],
  0x0009, 0x0008, 0x0012, 0x0800,
  SPOIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 SPOISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[72],
  0x0008, 0x0809, 0x0013, 0x0001,
  SPOIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 SPOIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[73],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 SPOIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 SPO_SETUP */
  &rs_object[92],   /* Tree  1 SPO_HELP */
  &rs_object[106],   /* Tree  2 SPO_DESK */
  &rs_object[108],   /* Tree  3 SPO_TEXT */
  &rs_object[111]    /* Tree  4 SPO_INFO */
};
