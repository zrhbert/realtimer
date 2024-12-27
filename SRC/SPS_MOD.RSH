/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "SPS_MOD.H"

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
{ "7",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1101, 0, -2, 2, 1,
  "6",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1101, 0, -2, 2, 1,
  "5",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1101, 0, -2, 2, 1,
  "4",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1101, 0, -2, 2, 1,
  "3",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1101, 0, -2, 2, 1,
  "2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1101, 0, -2, 2, 1,
  "1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1101, 0, -2, 2, 1,
  "0",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1101, 0, -2, 2, 1,
  "8",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1101, 0, -2, 2, 1,
  "Spieg",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 6, 1,
  "Tausch",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 7, 1,
  "Setup",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 6, 1,
  "-",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "+",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "-",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "+",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "-",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "+",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "-",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "+",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "-",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "+",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "-",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "+",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "-",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "+",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "-",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "+",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "-",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "+",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "SPS Spiegel/Tausch-Funktionen",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 30, 1,
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
  0x0003, 0xE000, 0x0003, 0xE000, 
  0x0003, 0xE000, 0x03E3, 0xE3E0, 
  0x03F3, 0xE7E0, 0x03FB, 0xEFE0, 
  0x03FF, 0xFFE0, 0x03FF, 0xFFE0, 
  0x01FF, 0xFFC0, 0x00FF, 0xFF80, 
  0x007F, 0xFF00, 0x003F, 0xFE00, 
  0x001F, 0xFC00, 0x000F, 0xF800, 
  0x001F, 0xFC00, 0x003F, 0xFE00, 
  0x007F, 0xFF00, 0x00FF, 0xFF80, 
  0x01FF, 0xFFC0, 0x03FF, 0xFFE0, 
  0x03FF, 0xFFE0, 0x03FB, 0xEFE0, 
  0x03F3, 0xE7E0, 0x03E3, 0xE3E0, 
  0x0003, 0xE000, 0x0003, 0xE000, 
  0x0003, 0xE000, 0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x8000, 0x0000, 0x8000, 
  0x0000, 0x8000, 0x0080, 0x8080, 
  0x0040, 0x8100, 0x0020, 0x8200, 
  0x0010, 0x8400, 0x0008, 0x8800, 
  0x0004, 0x9000, 0x0002, 0xA000, 
  0x0001, 0xC000, 0x0000, 0x8000, 
  0x0001, 0xC000, 0x0002, 0xA000, 
  0x0004, 0x9000, 0x0008, 0x8800, 
  0x0010, 0x8400, 0x0020, 0x8200, 
  0x0040, 0x8100, 0x0080, 0x8080, 
  0x0000, 0x8000, 0x0000, 0x8000, 
  0x0000, 0x8000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "SPS Spiegel", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
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

static WORD RSBB3DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x8000, 0x0000, 0x0001, 0xC000, 
  0x0000, 0x0003, 0xE000, 0x0000, 
  0x0007, 0xF000, 0x0000, 0x0000, 
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

static WORD RSBB4DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x8000, 
  0x0000, 0x0001, 0xC000, 0x0000, 
  0x0003, 0xE000, 0x0000, 0x0007, 
  0xF000, 0x0000, 0x0000, 0x8000, 
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

static WORD RSBB5DATA[] =
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
  0x0000, 0x0000, 0x9000, 0x8000, 
  0x0000, 0xA000, 0xC000, 0x0000, 
  0xC000, 0xE000, 0x0000, 0xFFFF, 
  0xF000, 0x0000, 0x0000, 0xE000, 
  0x0000, 0x0000, 0xC000, 0x0000, 
  0x0000, 0x8000, 0x0000, 0x0000, 
  0x0000
};

static BITBLK rs_bitblk[] =
{ RSBB0DATA,   6,  23,   0,   0, 0x0001,
  RSBB1DATA,   6,  23,   0,   0, 0x0001,
  RSBB2DATA,   6,  23,   0,   0, 0x0001,
  RSBB3DATA,   6,  23,   0,   0, 0x0001,
  RSBB4DATA,   6,  23,   0,   0, 0x0001,
  RSBB5DATA,   6,  23,   0,   0, 0x0001
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 SPS_SETUP ****************************************************/
        -1,        1, SPSOK   , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021121L,
  0x0000, 0x0000, 0x0034, 0x0010,
        11,        2,       10, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FE1101L,
  0x040C, 0x0800, 0x0026, 0x0001,
         3,       -1,       -1, G_BOXTEXT ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x001E, 0x0000, 0x0004, 0x0001,
         4,       -1,       -1, G_BOXTEXT ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x001A, 0x0000, 0x0004, 0x0001,
         5,       -1,       -1, G_BOXTEXT ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0016, 0x0000, 0x0004, 0x0001,
         6,       -1,       -1, G_BOXTEXT ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0012, 0x0000, 0x0004, 0x0001,
         7,       -1,       -1, G_BOXTEXT ,   /* Object 6  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x000E, 0x0000, 0x0004, 0x0001,
         8,       -1,       -1, G_BOXTEXT ,   /* Object 7  */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x000A, 0x0000, 0x0004, 0x0001,
         9,       -1,       -1, G_BOXTEXT ,   /* Object 8  */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0006, 0x0000, 0x0004, 0x0001,
        10,       -1,       -1, G_BOXTEXT ,   /* Object 9  */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0000, 0x0000, 0x0004, 0x0001,
         1,       -1,       -1, G_BOXTEXT ,   /* Object 10  */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0022, 0x0000, 0x0004, 0x0001,
  SPS0    ,       12,       22, G_BOX     ,   /* Object 11  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0801, 0x040B, 0x000A,
        17,       13,       16, G_BOX     ,   /* Object 12  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0000, 0x0000, 0x040B, 0x0804,
        14,       -1,       -1, G_IMAGE   ,   /* Object 13  */
  NONE, NORMAL, (LONG)&rs_bitblk[0],
  0x0405, 0x0000, 0x0006, 0x0701,
        15,       -1,       -1, G_IMAGE   ,   /* Object 14  */
  NONE, NORMAL, (LONG)&rs_bitblk[1],
  0x0405, 0x0801, 0x0006, 0x0701,
        16,       -1,       -1, G_IMAGE   ,   /* Object 15  */
  NONE, NORMAL, (LONG)&rs_bitblk[2],
  0x0405, 0x0003, 0x0006, 0x0701,
        12,       -1,       -1, G_TEXT    ,   /* Object 16  */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0400, 0x0800, 0x0005, 0x0803,
        22,       18,       21, G_BOX     ,   /* Object 17  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0000, 0x0804, 0x040B, 0x0804,
        19,       -1,       -1, G_IMAGE   ,   /* Object 18  */
  NONE, NORMAL, (LONG)&rs_bitblk[3],
  0x0405, 0x0000, 0x0006, 0x0701,
        20,       -1,       -1, G_IMAGE   ,   /* Object 19  */
  NONE, NORMAL, (LONG)&rs_bitblk[4],
  0x0405, 0x0003, 0x0006, 0x0701,
        21,       -1,       -1, G_IMAGE   ,   /* Object 20  */
  NONE, NORMAL, (LONG)&rs_bitblk[5],
  0x0405, 0x0801, 0x0006, 0x0701,
        17,       -1,       -1, G_TEXT    ,   /* Object 21  */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0400, 0x0800, 0x0006, 0x0803,
        11,       -1,       -1, G_BOXTEXT ,   /* Object 22  */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0000, 0x0009, 0x040B, 0x0001,
  SPS1    , SPSSPIEGELX0, SPSSINGLEPLUS, G_BOX     ,   /* Object 23 SPS0 */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x040C, 0x0801, 0x0004, 0x000A,
  SPSSPIEGELY0,       -1,       -1, G_BOX     |0x8000,   /* Object 24 SPSSPIEGELX0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0300, 0x0002, 0x0001,
  SPSSPIEGELZ0,       -1,       -1, G_BOX     |0x8000,   /* Object 25 SPSSPIEGELY0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B01, 0x0002, 0x0001,
  SPSTAUSCHXY0,       -1,       -1, G_BOX     |0x8000,   /* Object 26 SPSSPIEGELZ0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0303, 0x0002, 0x0001,
  SPSTAUSCHXZ0,       -1,       -1, G_BOX     |0x8000,   /* Object 27 SPSTAUSCHXY0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B04, 0x0002, 0x0001,
  SPSTAUSCHYZ0,       -1,       -1, G_BOX     |0x8000,   /* Object 28 SPSTAUSCHXZ0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0306, 0x0002, 0x0001,
  SPSSINGLEMINUS,       -1,       -1, G_BOX     |0x8000,   /* Object 29 SPSTAUSCHYZ0 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B07, 0x0002, 0x0001,
  SPSSINGLENR,       -1,       -1, G_BOXTEXT ,   /* Object 30 SPSSINGLEMINUS */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0000, 0x0009, 0x0001, 0x0001,
  SPSSINGLEPLUS,       -1,       -1, G_FBOXTEXT,   /* Object 31 SPSSINGLENR */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0001, 0x0009, 0x0002, 0x0001,
  SPS0    ,       -1,       -1, G_BOXTEXT ,   /* Object 32 SPSSINGLEPLUS */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0003, 0x0009, 0x0001, 0x0001,
        43,       34,       42, G_BOX     ,   /* Object 33 SPS1 */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0412, 0x0801, 0x0004, 0x000A,
        35,       -1,       -1, G_BOX     |0x8000,   /* Object 34  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0300, 0x0002, 0x0001,
        36,       -1,       -1, G_BOX     |0x8000,   /* Object 35  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B01, 0x0002, 0x0001,
        37,       -1,       -1, G_BOX     |0x8000,   /* Object 36  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0303, 0x0002, 0x0001,
        38,       -1,       -1, G_BOX     |0x8000,   /* Object 37  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B04, 0x0002, 0x0001,
        39,       -1,       -1, G_BOX     |0x8000,   /* Object 38  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0306, 0x0002, 0x0001,
        40,       -1,       -1, G_BOX     |0x8000,   /* Object 39  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B07, 0x0002, 0x0001,
        41,       -1,       -1, G_BOXTEXT ,   /* Object 40  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0000, 0x0009, 0x0001, 0x0001,
        42,       -1,       -1, G_FBOXTEXT,   /* Object 41  */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0001, 0x0009, 0x0002, 0x0001,
  SPS1    ,       -1,       -1, G_BOXTEXT ,   /* Object 42  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0003, 0x0009, 0x0001, 0x0001,
        53,       44,       52, G_BOX     ,   /* Object 43  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0416, 0x0801, 0x0004, 0x000A,
        45,       -1,       -1, G_BOX     |0x8000,   /* Object 44  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0300, 0x0002, 0x0001,
        46,       -1,       -1, G_BOX     |0x8000,   /* Object 45  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B01, 0x0002, 0x0001,
        47,       -1,       -1, G_BOX     |0x8000,   /* Object 46  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0303, 0x0002, 0x0001,
        48,       -1,       -1, G_BOX     |0x8000,   /* Object 47  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B04, 0x0002, 0x0001,
        49,       -1,       -1, G_BOX     |0x8000,   /* Object 48  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0306, 0x0002, 0x0001,
        50,       -1,       -1, G_BOX     |0x8000,   /* Object 49  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B07, 0x0002, 0x0001,
        51,       -1,       -1, G_BOXTEXT ,   /* Object 50  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0000, 0x0009, 0x0001, 0x0001,
        52,       -1,       -1, G_FBOXTEXT,   /* Object 51  */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0001, 0x0009, 0x0002, 0x0001,
        43,       -1,       -1, G_BOXTEXT ,   /* Object 52  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0003, 0x0009, 0x0001, 0x0001,
        63,       54,       62, G_BOX     ,   /* Object 53  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x041A, 0x0801, 0x0004, 0x000A,
        55,       -1,       -1, G_BOX     |0x8000,   /* Object 54  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0300, 0x0002, 0x0001,
        56,       -1,       -1, G_BOX     |0x8000,   /* Object 55  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B01, 0x0002, 0x0001,
        57,       -1,       -1, G_BOX     |0x8000,   /* Object 56  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0303, 0x0002, 0x0001,
        58,       -1,       -1, G_BOX     |0x8000,   /* Object 57  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B04, 0x0002, 0x0001,
        59,       -1,       -1, G_BOX     |0x8000,   /* Object 58  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0306, 0x0002, 0x0001,
        60,       -1,       -1, G_BOX     |0x8000,   /* Object 59  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B07, 0x0002, 0x0001,
        61,       -1,       -1, G_BOXTEXT ,   /* Object 60  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0000, 0x0009, 0x0001, 0x0001,
        62,       -1,       -1, G_FBOXTEXT,   /* Object 61  */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0001, 0x0009, 0x0002, 0x0001,
        53,       -1,       -1, G_BOXTEXT ,   /* Object 62  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0003, 0x0009, 0x0001, 0x0001,
        73,       64,       72, G_BOX     ,   /* Object 63  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x041E, 0x0801, 0x0004, 0x000A,
        65,       -1,       -1, G_BOX     |0x8000,   /* Object 64  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0300, 0x0002, 0x0001,
        66,       -1,       -1, G_BOX     |0x8000,   /* Object 65  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B01, 0x0002, 0x0001,
        67,       -1,       -1, G_BOX     |0x8000,   /* Object 66  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0303, 0x0002, 0x0001,
        68,       -1,       -1, G_BOX     |0x8000,   /* Object 67  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B04, 0x0002, 0x0001,
        69,       -1,       -1, G_BOX     |0x8000,   /* Object 68  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0306, 0x0002, 0x0001,
        70,       -1,       -1, G_BOX     |0x8000,   /* Object 69  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B07, 0x0002, 0x0001,
        71,       -1,       -1, G_BOXTEXT ,   /* Object 70  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0000, 0x0009, 0x0001, 0x0001,
        72,       -1,       -1, G_FBOXTEXT,   /* Object 71  */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[25],
  0x0001, 0x0009, 0x0002, 0x0001,
        63,       -1,       -1, G_BOXTEXT ,   /* Object 72  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0003, 0x0009, 0x0001, 0x0001,
        83,       74,       82, G_BOX     ,   /* Object 73  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0422, 0x0801, 0x0004, 0x000A,
        75,       -1,       -1, G_BOX     |0x8000,   /* Object 74  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0300, 0x0002, 0x0001,
        76,       -1,       -1, G_BOX     |0x8000,   /* Object 75  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B01, 0x0002, 0x0001,
        77,       -1,       -1, G_BOX     |0x8000,   /* Object 76  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0303, 0x0002, 0x0001,
        78,       -1,       -1, G_BOX     |0x8000,   /* Object 77  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B04, 0x0002, 0x0001,
        79,       -1,       -1, G_BOX     |0x8000,   /* Object 78  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0306, 0x0002, 0x0001,
        80,       -1,       -1, G_BOX     |0x8000,   /* Object 79  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B07, 0x0002, 0x0001,
        81,       -1,       -1, G_BOXTEXT ,   /* Object 80  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0000, 0x0009, 0x0001, 0x0001,
        82,       -1,       -1, G_FBOXTEXT,   /* Object 81  */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0001, 0x0009, 0x0002, 0x0001,
        73,       -1,       -1, G_BOXTEXT ,   /* Object 82  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0003, 0x0009, 0x0001, 0x0001,
        93,       84,       92, G_BOX     ,   /* Object 83  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0426, 0x0801, 0x0004, 0x000A,
        85,       -1,       -1, G_BOX     |0x8000,   /* Object 84  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0300, 0x0002, 0x0001,
        86,       -1,       -1, G_BOX     |0x8000,   /* Object 85  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B01, 0x0002, 0x0001,
        87,       -1,       -1, G_BOX     |0x8000,   /* Object 86  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0303, 0x0002, 0x0001,
        88,       -1,       -1, G_BOX     |0x8000,   /* Object 87  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B04, 0x0002, 0x0001,
        89,       -1,       -1, G_BOX     |0x8000,   /* Object 88  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0306, 0x0002, 0x0001,
        90,       -1,       -1, G_BOX     |0x8000,   /* Object 89  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B07, 0x0002, 0x0001,
        91,       -1,       -1, G_BOXTEXT ,   /* Object 90  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0000, 0x0009, 0x0001, 0x0001,
        92,       -1,       -1, G_FBOXTEXT,   /* Object 91  */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0001, 0x0009, 0x0002, 0x0001,
        83,       -1,       -1, G_BOXTEXT ,   /* Object 92  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0003, 0x0009, 0x0001, 0x0001,
       103,       94,      102, G_BOX     ,   /* Object 93  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x042A, 0x0801, 0x0004, 0x000A,
        95,       -1,       -1, G_BOX     |0x8000,   /* Object 94  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0300, 0x0002, 0x0001,
        96,       -1,       -1, G_BOX     |0x8000,   /* Object 95  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B01, 0x0002, 0x0001,
        97,       -1,       -1, G_BOX     |0x8000,   /* Object 96  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0303, 0x0002, 0x0001,
        98,       -1,       -1, G_BOX     |0x8000,   /* Object 97  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B04, 0x0002, 0x0001,
        99,       -1,       -1, G_BOX     |0x8000,   /* Object 98  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0306, 0x0002, 0x0001,
       100,       -1,       -1, G_BOX     |0x8000,   /* Object 99  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B07, 0x0002, 0x0001,
       101,       -1,       -1, G_BOXTEXT ,   /* Object 100  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0000, 0x0009, 0x0001, 0x0001,
       102,       -1,       -1, G_FBOXTEXT,   /* Object 101  */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0001, 0x0009, 0x0002, 0x0001,
        93,       -1,       -1, G_BOXTEXT ,   /* Object 102  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0003, 0x0009, 0x0001, 0x0001,
       113,      104,      112, G_BOX     ,   /* Object 103  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x042E, 0x0801, 0x0004, 0x000A,
       105,       -1,       -1, G_BOX     |0x8000,   /* Object 104  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0300, 0x0002, 0x0001,
       106,       -1,       -1, G_BOX     |0x8000,   /* Object 105  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B01, 0x0002, 0x0001,
       107,       -1,       -1, G_BOX     |0x8000,   /* Object 106  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0303, 0x0002, 0x0001,
       108,       -1,       -1, G_BOX     |0x8000,   /* Object 107  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B04, 0x0002, 0x0001,
       109,       -1,       -1, G_BOX     |0x8000,   /* Object 108  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0306, 0x0002, 0x0001,
       110,       -1,       -1, G_BOX     |0x8000,   /* Object 109  */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0B07, 0x0002, 0x0001,
       111,       -1,       -1, G_BOXTEXT ,   /* Object 110  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[36],
  0x0000, 0x0009, 0x0001, 0x0001,
       112,       -1,       -1, G_FBOXTEXT,   /* Object 111  */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0001, 0x0009, 0x0002, 0x0001,
       103,       -1,       -1, G_BOXTEXT ,   /* Object 112  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0003, 0x0009, 0x0001, 0x0001,
  SPSSTANDARD, SPSSETDEC,      118, G_BOX     ,   /* Object 113  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0001, 0x000C, 0x0014, 0x0803,
  SPSSETINC,       -1,       -1, G_BUTTON  ,   /* Object 114 SPSSETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  SPSSETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 115 SPSSETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  SPSSETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 116 SPSSETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
       118,       -1,       -1, G_BUTTON  ,   /* Object 117 SPSSETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
       113,      119, SPSSETNR, G_BOX     ,   /* Object 118  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  SPSSETNR,       -1,       -1, G_STRING  ,   /* Object 119  */
  NONE, NORMAL, (LONG)"SPS-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0801,
       118,       -1,       -1, G_FTEXT   ,   /* Object 120 SPSSETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[39],
  0x0008, 0x0000, 0x0006, 0x0801,
  SPSCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 121 SPSSTANDARD */
  SELECTABLE, NORMAL, (LONG)"S~tandard",
  0x0019, 0x000C, 0x0009, 0x0801,
  SPSHELP ,       -1,       -1, G_BUTTON  ,   /* Object 122 SPSCANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0425, 0x000C, 0x0009, 0x0801,
  SPSOK   ,       -1,       -1, G_BUTTON  ,   /* Object 123 SPSHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0019, 0x000E, 0x0009, 0x0801,
         0,       -1,       -1, G_BUTTON  ,   /* Object 124 SPSOK */
  SELECTABLE|DEFAULT|LASTOB, NORMAL, (LONG)"~OK",
  0x0425, 0x000E, 0x0009, 0x0801,
  
  /******** Tree 1 SPS_HELP ****************************************************/
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
  
  /******** Tree 2 SPS_DESK ****************************************************/
        -1, SPSICON , SPSICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 SPSICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0004, 0x0804, 0x0009, 0x0802,
  
  /******** Tree 3 SPS_TEXT ****************************************************/
        -1, FSPSN   , FSPSH   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FSPSI   ,       -1,       -1, G_STRING  ,   /* Object 1 FSPSN */
  NONE, NORMAL, (LONG)" SPS-Setups ",
  0x0001, 0x0801, 0x000C, 0x0001,
  FSPSH   ,       -1,       -1, G_STRING  ,   /* Object 2 FSPSI */
  NONE, NORMAL, (LONG)" %d Presets",
  0x0001, 0x0802, 0x000B, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 3 FSPSH */
  LASTOB, NORMAL, (LONG)"SPS-Setup-Hilfe",
  0x0001, 0x0004, 0x000F, 0x0001,
  
  /******** Tree 4 SPS_INFO ****************************************************/
        -1,        1, SPSIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0023, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[41],
  0x0003, 0x0802, 0x001D, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[42],
  0x0006, 0x0004, 0x0017, 0x0001,
  SPSIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[43],
  0x0008, 0x0005, 0x0612, 0x0600,
  SPSIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 SPSIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[44],
  0x0007, 0x0006, 0x0014, 0x0001,
  SPSCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 SPSIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[45],
  0x0009, 0x0007, 0x0012, 0x0001,
  SPSISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 SPSCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[46],
  0x0009, 0x0008, 0x0012, 0x0800,
  SPSIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 SPSISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[47],
  0x0008, 0x0809, 0x0013, 0x0001,
  SPSIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 SPSIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[48],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 SPSIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000E, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 SPS_SETUP */
  &rs_object[125],   /* Tree  1 SPS_HELP */
  &rs_object[139],   /* Tree  2 SPS_DESK */
  &rs_object[141],   /* Tree  3 SPS_TEXT */
  &rs_object[145]    /* Tree  4 SPS_INFO */
};
