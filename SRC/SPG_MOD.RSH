/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "SPG_MOD.H"

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
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "3",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "4",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "5",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "6",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "7",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "8",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "P",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 2, 1,
  "____",
  "____",
  "XXXX",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 5,
  "Achsen-",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 8, 1,
  "Drehung",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 8, 1,
  "Winkel in \370",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 12, 1,
  "Ebenen-",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 8, 1,
  "Drehung",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 8, 1,
  "Winkel in \370",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 0, 12, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "SPG Grad-Drehungen",
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
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0380, 0x0000, 0x0380, 0x0000, 
  0x0380, 0x0000, 0x0380, 0x0000, 
  0x0380, 0x7000, 0x03FC, 0xF000, 
  0x03FF, 0xF000, 0x03FF, 0xE000, 
  0x038F, 0xF000, 0x038F, 0xF800, 
  0x039F, 0xF800, 0x03BE, 0x7C00, 
  0x03FC, 0x3C00, 0x03F8, 0x3C00, 
  0x03F0, 0x1C00, 0x03FF, 0xFFC0, 
  0x03FF, 0xFFC0, 0x03FF, 0xFFC0, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0100, 0x0000, 
  0x0100, 0x0000, 0x0100, 0x0000, 
  0x0100, 0x0000, 0x0100, 0x2000, 
  0x01F8, 0x4000, 0x0107, 0x8000, 
  0x0101, 0x8000, 0x0102, 0x6000, 
  0x0104, 0x3000, 0x0108, 0x1000, 
  0x0110, 0x1800, 0x0120, 0x0800, 
  0x0140, 0x0800, 0x0180, 0x0800, 
  0x01FF, 0xFF80, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "SPG Grad", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  75,   8
};

static WORD RSBB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0080, 0x0000, 0x01C0, 0x0000, 
  0x0080, 0x0000, 0x0080, 0x0000, 
  0x0080, 0x0000, 0x0080, 0x0000, 
  0x0080, 0xC000, 0x0080, 0xC000, 
  0x0081, 0x0000, 0x0082, 0x0000, 
  0x0084, 0x0000, 0x0088, 0x0000, 
  0x0090, 0x0F80, 0x00A0, 0x18C0, 
  0x00C0, 0x1040, 0x00FF, 0xFFC0, 
  0x0000, 0x1040, 0x0000, 0x18C0, 
  0x0000, 0x0F80, 0x0000, 0x0000, 
  0x0000, 0x0000
};

static WORD RSBB1DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0040, 0x0000, 
  0x0040, 0x0000, 0x03F8, 0x0000, 
  0x064C, 0x0000, 0x0444, 0x0000, 
  0x064C, 0x0000, 0x03F8, 0x6000, 
  0x0040, 0x6000, 0x0040, 0x8000, 
  0x0041, 0x0000, 0x0042, 0x0000, 
  0x0044, 0x0000, 0x0048, 0x0000, 
  0x0050, 0x0000, 0x0060, 0x0040, 
  0x007F, 0xFFE0, 0x0000, 0x0040, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000
};

static WORD RSBB2DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0080, 0x0000, 
  0x01C0, 0x0000, 0x0080, 0x0000, 
  0x0080, 0x0000, 0x0083, 0xE000, 
  0x0086, 0x3000, 0x0084, 0x1000, 
  0x0084, 0x9000, 0x0085, 0x1000, 
  0x0086, 0x3000, 0x0087, 0xE000, 
  0x0088, 0x0000, 0x0090, 0x0000, 
  0x00A0, 0x0000, 0x00C0, 0x0080, 
  0x00FF, 0xFFC0, 0x0000, 0x0080, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000
};

static WORD RSBB3DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x001F, 0x0000, 
  0x0031, 0x8000, 0x0020, 0x8000, 
  0x0060, 0xC000, 0x0040, 0x4000, 
  0x0040, 0x4300, 0x0040, 0x5C00, 
  0x0040, 0xE000, 0x0047, 0x4000, 
  0x0040, 0x4000, 0x0040, 0x4000, 
  0x0040, 0x4000, 0x0040, 0x4000, 
  0x0060, 0xC000, 0x0020, 0x8000, 
  0x0031, 0x8000, 0x001F, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000
};

static WORD RSBB4DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0080, 
  0x0000, 0x0100, 0x0000, 0x0200, 
  0x0000, 0x0400, 0x001F, 0xF400, 
  0x0060, 0x1300, 0x0180, 0x20C0, 
  0x0100, 0x4040, 0x0100, 0x8040, 
  0x0100, 0x0040, 0x0180, 0x00C0, 
  0x0060, 0x0300, 0x001F, 0xFC00, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000
};

static WORD RSBB5DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x000F, 0xF800, 0x0018, 0x0C00, 
  0x0060, 0x0300, 0x0040, 0x0100, 
  0x0080, 0x0080, 0x0190, 0x00C0, 
  0x0108, 0x0040, 0x0104, 0x0040, 
  0x0102, 0x0040, 0x0101, 0x0040, 
  0x0100, 0xC040, 0x0100, 0x0040, 
  0x0100, 0x0040, 0x0180, 0x00C0, 
  0x0080, 0x0080, 0x0040, 0x0100, 
  0x0060, 0x0300, 0x0018, 0x0C00, 
  0x000F, 0xF800, 0x0000, 0x0000, 
  0x0000, 0x0000
};

static BITBLK rs_bitblk[] =
{ RSBB0DATA,   4,  23,   0,   0, 0x0001,
  RSBB1DATA,   4,  23,   0,   0, 0x0001,
  RSBB2DATA,   4,  23,   0,   0, 0x0001,
  RSBB3DATA,   4,  23,   0,   0, 0x0001,
  RSBB4DATA,   4,  23,   0,   0, 0x0001,
  RSBB5DATA,   4,  23,   0,   0, 0x0001
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 SPG_SETUP ****************************************************/
        -1,        1, SPGOK   , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021121L,
  0x0000, 0x0000, 0x0047, 0x080F,
        15, SPG0    , SPGWINKYZ0, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x040D, 0x0800, 0x0006, 0x080A,
  SPGPROPDX0,       -1,       -1, G_BOXTEXT ,   /* Object 2 SPG0 */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0000, 0x0000, 0x0006, 0x0001,
  SPGDREHX0,       -1,       -1, G_BOXTEXT ,   /* Object 3 SPGPROPDX0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0000, 0x0001, 0x0401, 0x0001,
  SPGPROPDY0,       -1,       -1, G_FBOXTEXT,   /* Object 4 SPGDREHX0 */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0401, 0x0001, 0x0404, 0x0801,
  SPGDREHY0,       -1,       -1, G_BOXTEXT ,   /* Object 5 SPGPROPDY0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0000, 0x0802, 0x0401, 0x0001,
  SPGPROPDZ0,       -1,       -1, G_FBOXTEXT,   /* Object 6 SPGDREHY0 */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0401, 0x0802, 0x0404, 0x0801,
  SPGDREHZ0,       -1,       -1, G_BOXTEXT ,   /* Object 7 SPGPROPDZ0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0000, 0x0004, 0x0401, 0x0001,
  SPGPROPWXZ0,       -1,       -1, G_FBOXTEXT,   /* Object 8 SPGDREHZ0 */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0401, 0x0004, 0x0404, 0x0801,
  SPGWINKXZ0,       -1,       -1, G_BOXTEXT ,   /* Object 9 SPGPROPWXZ0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0000, 0x0006, 0x0401, 0x0001,
  SPGPROPWXY0,       -1,       -1, G_FBOXTEXT,   /* Object 10 SPGWINKXZ0 */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0401, 0x0006, 0x0404, 0x0801,
  SPGWINKXY0,       -1,       -1, G_BOXTEXT ,   /* Object 11 SPGPROPWXY0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0000, 0x0807, 0x0401, 0x0001,
  SPGPROPWYZ0,       -1,       -1, G_FBOXTEXT,   /* Object 12 SPGWINKXY0 */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0401, 0x0807, 0x0404, 0x0801,
  SPGWINKYZ0,       -1,       -1, G_BOXTEXT ,   /* Object 13 SPGPROPWYZ0 */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0000, 0x0009, 0x0401, 0x0001,
         1,       -1,       -1, G_FBOXTEXT,   /* Object 14 SPGWINKYZ0 */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0401, 0x0009, 0x0404, 0x0801,
        29, SPG1    ,       28, G_BOX     ,   /* Object 15  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0016, 0x0800, 0x0006, 0x080A,
        17,       -1,       -1, G_BOXTEXT ,   /* Object 16 SPG1 */
  NONE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0000, 0x0000, 0x0006, 0x0001,
        18,       -1,       -1, G_BOXTEXT ,   /* Object 17  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0000, 0x0001, 0x0401, 0x0001,
        19,       -1,       -1, G_FBOXTEXT,   /* Object 18  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0401, 0x0001, 0x0404, 0x0801,
        20,       -1,       -1, G_BOXTEXT ,   /* Object 19  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0000, 0x0802, 0x0401, 0x0001,
        21,       -1,       -1, G_FBOXTEXT,   /* Object 20  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0401, 0x0802, 0x0404, 0x0801,
        22,       -1,       -1, G_BOXTEXT ,   /* Object 21  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0000, 0x0004, 0x0401, 0x0001,
        23,       -1,       -1, G_FBOXTEXT,   /* Object 22  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0401, 0x0004, 0x0404, 0x0801,
        24,       -1,       -1, G_BOXTEXT ,   /* Object 23  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0000, 0x0006, 0x0401, 0x0001,
        25,       -1,       -1, G_FBOXTEXT,   /* Object 24  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0401, 0x0006, 0x0404, 0x0801,
        26,       -1,       -1, G_BOXTEXT ,   /* Object 25  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0000, 0x0807, 0x0401, 0x0001,
        27,       -1,       -1, G_FBOXTEXT,   /* Object 26  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0401, 0x0807, 0x0404, 0x0801,
        28,       -1,       -1, G_BOXTEXT ,   /* Object 27  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0000, 0x0009, 0x0401, 0x0001,
        15,       -1,       -1, G_FBOXTEXT,   /* Object 28  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[25],
  0x0401, 0x0009, 0x0404, 0x0801,
        43,       30,       42, G_BOX     ,   /* Object 29  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x001C, 0x0800, 0x0006, 0x080A,
        31,       -1,       -1, G_BOXTEXT ,   /* Object 30  */
  NONE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0000, 0x0000, 0x0006, 0x0001,
        32,       -1,       -1, G_BOXTEXT ,   /* Object 31  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0000, 0x0001, 0x0401, 0x0001,
        33,       -1,       -1, G_FBOXTEXT,   /* Object 32  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0401, 0x0001, 0x0404, 0x0801,
        34,       -1,       -1, G_BOXTEXT ,   /* Object 33  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0000, 0x0802, 0x0401, 0x0001,
        35,       -1,       -1, G_FBOXTEXT,   /* Object 34  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0401, 0x0802, 0x0404, 0x0801,
        36,       -1,       -1, G_BOXTEXT ,   /* Object 35  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0000, 0x0004, 0x0401, 0x0001,
        37,       -1,       -1, G_FBOXTEXT,   /* Object 36  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0401, 0x0004, 0x0404, 0x0801,
        38,       -1,       -1, G_BOXTEXT ,   /* Object 37  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0000, 0x0006, 0x0401, 0x0001,
        39,       -1,       -1, G_FBOXTEXT,   /* Object 38  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0401, 0x0006, 0x0404, 0x0801,
        40,       -1,       -1, G_BOXTEXT ,   /* Object 39  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0000, 0x0807, 0x0401, 0x0001,
        41,       -1,       -1, G_FBOXTEXT,   /* Object 40  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[36],
  0x0401, 0x0807, 0x0404, 0x0801,
        42,       -1,       -1, G_BOXTEXT ,   /* Object 41  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0000, 0x0009, 0x0401, 0x0001,
        29,       -1,       -1, G_FBOXTEXT,   /* Object 42  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0401, 0x0009, 0x0404, 0x0801,
        57, SPG3    ,       56, G_BOX     ,   /* Object 43  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0022, 0x0800, 0x0006, 0x080A,
        45,       -1,       -1, G_BOXTEXT ,   /* Object 44 SPG3 */
  NONE, NORMAL, (LONG)&rs_tedinfo[39],
  0x0000, 0x0000, 0x0006, 0x0001,
        46,       -1,       -1, G_BOXTEXT ,   /* Object 45  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0000, 0x0001, 0x0401, 0x0001,
        47,       -1,       -1, G_FBOXTEXT,   /* Object 46  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[41],
  0x0401, 0x0001, 0x0404, 0x0801,
        48,       -1,       -1, G_BOXTEXT ,   /* Object 47  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[42],
  0x0000, 0x0802, 0x0401, 0x0001,
        49,       -1,       -1, G_FBOXTEXT,   /* Object 48  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[43],
  0x0401, 0x0802, 0x0404, 0x0801,
        50,       -1,       -1, G_BOXTEXT ,   /* Object 49  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[44],
  0x0000, 0x0004, 0x0401, 0x0001,
        51,       -1,       -1, G_FBOXTEXT,   /* Object 50  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[45],
  0x0401, 0x0004, 0x0404, 0x0801,
        52,       -1,       -1, G_BOXTEXT ,   /* Object 51  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[46],
  0x0000, 0x0006, 0x0401, 0x0001,
        53,       -1,       -1, G_FBOXTEXT,   /* Object 52  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[47],
  0x0401, 0x0006, 0x0404, 0x0801,
        54,       -1,       -1, G_BOXTEXT ,   /* Object 53  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[48],
  0x0000, 0x0807, 0x0401, 0x0001,
        55,       -1,       -1, G_FBOXTEXT,   /* Object 54  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[49],
  0x0401, 0x0807, 0x0404, 0x0801,
        56,       -1,       -1, G_BOXTEXT ,   /* Object 55  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[50],
  0x0000, 0x0009, 0x0401, 0x0001,
        43,       -1,       -1, G_FBOXTEXT,   /* Object 56  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[51],
  0x0401, 0x0009, 0x0404, 0x0801,
        71,       58,       70, G_BOX     ,   /* Object 57  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0028, 0x0800, 0x0006, 0x080A,
        59,       -1,       -1, G_BOXTEXT ,   /* Object 58  */
  NONE, NORMAL, (LONG)&rs_tedinfo[52],
  0x0000, 0x0000, 0x0006, 0x0001,
        60,       -1,       -1, G_BOXTEXT ,   /* Object 59  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[53],
  0x0000, 0x0001, 0x0401, 0x0001,
        61,       -1,       -1, G_FBOXTEXT,   /* Object 60  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[54],
  0x0401, 0x0001, 0x0404, 0x0801,
        62,       -1,       -1, G_BOXTEXT ,   /* Object 61  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[55],
  0x0000, 0x0802, 0x0401, 0x0001,
        63,       -1,       -1, G_FBOXTEXT,   /* Object 62  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[56],
  0x0401, 0x0802, 0x0404, 0x0801,
        64,       -1,       -1, G_BOXTEXT ,   /* Object 63  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[57],
  0x0000, 0x0004, 0x0401, 0x0001,
        65,       -1,       -1, G_FBOXTEXT,   /* Object 64  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[58],
  0x0401, 0x0004, 0x0404, 0x0801,
        66,       -1,       -1, G_BOXTEXT ,   /* Object 65  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[59],
  0x0000, 0x0006, 0x0401, 0x0001,
        67,       -1,       -1, G_FBOXTEXT,   /* Object 66  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[60],
  0x0401, 0x0006, 0x0404, 0x0801,
        68,       -1,       -1, G_BOXTEXT ,   /* Object 67  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[61],
  0x0000, 0x0807, 0x0401, 0x0001,
        69,       -1,       -1, G_FBOXTEXT,   /* Object 68  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[62],
  0x0401, 0x0807, 0x0404, 0x0801,
        70,       -1,       -1, G_BOXTEXT ,   /* Object 69  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[63],
  0x0000, 0x0009, 0x0401, 0x0001,
        57,       -1,       -1, G_FBOXTEXT,   /* Object 70  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[64],
  0x0401, 0x0009, 0x0404, 0x0801,
        85,       72,       84, G_BOX     ,   /* Object 71  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x002E, 0x0800, 0x0006, 0x080A,
        73,       -1,       -1, G_BOXTEXT ,   /* Object 72  */
  NONE, NORMAL, (LONG)&rs_tedinfo[65],
  0x0000, 0x0000, 0x0006, 0x0001,
        74,       -1,       -1, G_BOXTEXT ,   /* Object 73  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[66],
  0x0000, 0x0001, 0x0401, 0x0001,
        75,       -1,       -1, G_FBOXTEXT,   /* Object 74  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[67],
  0x0401, 0x0001, 0x0404, 0x0801,
        76,       -1,       -1, G_BOXTEXT ,   /* Object 75  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[68],
  0x0000, 0x0802, 0x0401, 0x0001,
        77,       -1,       -1, G_FBOXTEXT,   /* Object 76  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[69],
  0x0401, 0x0802, 0x0404, 0x0801,
        78,       -1,       -1, G_BOXTEXT ,   /* Object 77  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[70],
  0x0000, 0x0004, 0x0401, 0x0001,
        79,       -1,       -1, G_FBOXTEXT,   /* Object 78  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[71],
  0x0401, 0x0004, 0x0404, 0x0801,
        80,       -1,       -1, G_BOXTEXT ,   /* Object 79  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[72],
  0x0000, 0x0006, 0x0401, 0x0001,
        81,       -1,       -1, G_FBOXTEXT,   /* Object 80  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[73],
  0x0401, 0x0006, 0x0404, 0x0801,
        82,       -1,       -1, G_BOXTEXT ,   /* Object 81  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[74],
  0x0000, 0x0807, 0x0401, 0x0001,
        83,       -1,       -1, G_FBOXTEXT,   /* Object 82  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[75],
  0x0401, 0x0807, 0x0404, 0x0801,
        84,       -1,       -1, G_BOXTEXT ,   /* Object 83  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[76],
  0x0000, 0x0009, 0x0401, 0x0001,
        71,       -1,       -1, G_FBOXTEXT,   /* Object 84  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[77],
  0x0401, 0x0009, 0x0404, 0x0801,
        99,       86,       98, G_BOX     ,   /* Object 85  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0034, 0x0800, 0x0006, 0x080A,
        87,       -1,       -1, G_BOXTEXT ,   /* Object 86  */
  NONE, NORMAL, (LONG)&rs_tedinfo[78],
  0x0000, 0x0000, 0x0006, 0x0001,
        88,       -1,       -1, G_BOXTEXT ,   /* Object 87  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[79],
  0x0000, 0x0001, 0x0401, 0x0001,
        89,       -1,       -1, G_FBOXTEXT,   /* Object 88  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[80],
  0x0401, 0x0001, 0x0404, 0x0801,
        90,       -1,       -1, G_BOXTEXT ,   /* Object 89  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[81],
  0x0000, 0x0802, 0x0401, 0x0001,
        91,       -1,       -1, G_FBOXTEXT,   /* Object 90  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[82],
  0x0401, 0x0802, 0x0404, 0x0801,
        92,       -1,       -1, G_BOXTEXT ,   /* Object 91  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[83],
  0x0000, 0x0004, 0x0401, 0x0001,
        93,       -1,       -1, G_FBOXTEXT,   /* Object 92  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[84],
  0x0401, 0x0004, 0x0404, 0x0801,
        94,       -1,       -1, G_BOXTEXT ,   /* Object 93  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[85],
  0x0000, 0x0006, 0x0401, 0x0001,
        95,       -1,       -1, G_FBOXTEXT,   /* Object 94  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[86],
  0x0401, 0x0006, 0x0404, 0x0801,
        96,       -1,       -1, G_BOXTEXT ,   /* Object 95  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[87],
  0x0000, 0x0807, 0x0401, 0x0001,
        97,       -1,       -1, G_FBOXTEXT,   /* Object 96  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[88],
  0x0401, 0x0807, 0x0404, 0x0801,
        98,       -1,       -1, G_BOXTEXT ,   /* Object 97  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[89],
  0x0000, 0x0009, 0x0401, 0x0001,
        85,       -1,       -1, G_FBOXTEXT,   /* Object 98  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[90],
  0x0401, 0x0009, 0x0404, 0x0801,
       113,      100,      112, G_BOX     ,   /* Object 99  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x003A, 0x0800, 0x0006, 0x080A,
       101,       -1,       -1, G_BOXTEXT ,   /* Object 100  */
  NONE, NORMAL, (LONG)&rs_tedinfo[91],
  0x0000, 0x0000, 0x0006, 0x0001,
       102,       -1,       -1, G_BOXTEXT ,   /* Object 101  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[92],
  0x0000, 0x0001, 0x0401, 0x0001,
       103,       -1,       -1, G_FBOXTEXT,   /* Object 102  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[93],
  0x0401, 0x0001, 0x0404, 0x0801,
       104,       -1,       -1, G_BOXTEXT ,   /* Object 103  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[94],
  0x0000, 0x0802, 0x0401, 0x0001,
       105,       -1,       -1, G_FBOXTEXT,   /* Object 104  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[95],
  0x0401, 0x0802, 0x0404, 0x0801,
       106,       -1,       -1, G_BOXTEXT ,   /* Object 105  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[96],
  0x0000, 0x0004, 0x0401, 0x0001,
       107,       -1,       -1, G_FBOXTEXT,   /* Object 106  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[97],
  0x0401, 0x0004, 0x0404, 0x0801,
       108,       -1,       -1, G_BOXTEXT ,   /* Object 107  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[98],
  0x0000, 0x0006, 0x0401, 0x0001,
       109,       -1,       -1, G_FBOXTEXT,   /* Object 108  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[99],
  0x0401, 0x0006, 0x0404, 0x0801,
       110,       -1,       -1, G_BOXTEXT ,   /* Object 109  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[100],
  0x0000, 0x0807, 0x0401, 0x0001,
       111,       -1,       -1, G_FBOXTEXT,   /* Object 110  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[101],
  0x0401, 0x0807, 0x0404, 0x0801,
       112,       -1,       -1, G_BOXTEXT ,   /* Object 111  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[102],
  0x0000, 0x0009, 0x0401, 0x0001,
        99,       -1,       -1, G_FBOXTEXT,   /* Object 112  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[103],
  0x0401, 0x0009, 0x0404, 0x0801,
       127,      114,      126, G_BOX     ,   /* Object 113  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0040, 0x0800, 0x0006, 0x080A,
       115,       -1,       -1, G_BOXTEXT ,   /* Object 114  */
  NONE, NORMAL, (LONG)&rs_tedinfo[104],
  0x0000, 0x0000, 0x0006, 0x0001,
       116,       -1,       -1, G_BOXTEXT ,   /* Object 115  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[105],
  0x0000, 0x0001, 0x0401, 0x0001,
       117,       -1,       -1, G_FBOXTEXT,   /* Object 116  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[106],
  0x0401, 0x0001, 0x0404, 0x0801,
       118,       -1,       -1, G_BOXTEXT ,   /* Object 117  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[107],
  0x0000, 0x0802, 0x0401, 0x0001,
       119,       -1,       -1, G_FBOXTEXT,   /* Object 118  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[108],
  0x0401, 0x0802, 0x0404, 0x0801,
       120,       -1,       -1, G_BOXTEXT ,   /* Object 119  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[109],
  0x0000, 0x0004, 0x0401, 0x0001,
       121,       -1,       -1, G_FBOXTEXT,   /* Object 120  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[110],
  0x0401, 0x0004, 0x0404, 0x0801,
       122,       -1,       -1, G_BOXTEXT ,   /* Object 121  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[111],
  0x0000, 0x0006, 0x0401, 0x0001,
       123,       -1,       -1, G_FBOXTEXT,   /* Object 122  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[112],
  0x0401, 0x0006, 0x0404, 0x0801,
       124,       -1,       -1, G_BOXTEXT ,   /* Object 123  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[113],
  0x0000, 0x0807, 0x0401, 0x0001,
       125,       -1,       -1, G_FBOXTEXT,   /* Object 124  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[114],
  0x0401, 0x0807, 0x0404, 0x0801,
       126,       -1,       -1, G_BOXTEXT ,   /* Object 125  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[115],
  0x0000, 0x0009, 0x0401, 0x0001,
       113,       -1,       -1, G_FBOXTEXT,   /* Object 126  */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[116],
  0x0401, 0x0009, 0x0404, 0x0801,
       142,      128,      135, G_BOX     ,   /* Object 127  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0400, 0x0801, 0x040C, 0x0809,
       135,      129,      134, G_BOX     ,   /* Object 128  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0000, 0x0000, 0x040C, 0x0804,
       130,       -1,       -1, G_IMAGE   ,   /* Object 129  */
  NONE, NORMAL, (LONG)&rs_bitblk[0],
  0x0408, 0x0000, 0x0004, 0x0701,
       131,       -1,       -1, G_IMAGE   ,   /* Object 130  */
  NONE, NORMAL, (LONG)&rs_bitblk[1],
  0x0408, 0x0801, 0x0004, 0x0701,
       132,       -1,       -1, G_IMAGE   ,   /* Object 131  */
  NONE, NORMAL, (LONG)&rs_bitblk[2],
  0x0408, 0x0003, 0x0004, 0x0701,
       133,       -1,       -1, G_BOXTEXT ,   /* Object 132  */
  NONE, NORMAL, (LONG)&rs_tedinfo[117],
  0x0001, 0x0001, 0x0008, 0x0001,
       134,       -1,       -1, G_BOXTEXT ,   /* Object 133  */
  NONE, NORMAL, (LONG)&rs_tedinfo[118],
  0x0001, 0x0002, 0x0407, 0x0001,
       128,       -1,       -1, G_BOXTEXT ,   /* Object 134  */
  NONE, NORMAL, (LONG)&rs_tedinfo[119],
  0x0001, 0x0003, 0x0208, 0x0001,
       127,      136,      141, G_BOX     ,   /* Object 135  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0000, 0x0005, 0x040C, 0x0804,
       137,       -1,       -1, G_IMAGE   ,   /* Object 136  */
  NONE, NORMAL, (LONG)&rs_bitblk[3],
  0x0408, 0x0003, 0x0004, 0x0701,
       138,       -1,       -1, G_IMAGE   ,   /* Object 137  */
  NONE, NORMAL, (LONG)&rs_bitblk[4],
  0x0408, 0x0000, 0x0004, 0x0701,
       139,       -1,       -1, G_IMAGE   ,   /* Object 138  */
  NONE, NORMAL, (LONG)&rs_bitblk[5],
  0x0408, 0x0801, 0x0004, 0x0701,
       140,       -1,       -1, G_BOXTEXT ,   /* Object 139  */
  NONE, NORMAL, (LONG)&rs_tedinfo[120],
  0x0001, 0x0001, 0x0008, 0x0001,
       141,       -1,       -1, G_BOXTEXT ,   /* Object 140  */
  NONE, NORMAL, (LONG)&rs_tedinfo[121],
  0x0001, 0x0002, 0x0407, 0x0001,
       135,       -1,       -1, G_BOXTEXT ,   /* Object 141  */
  NONE, NORMAL, (LONG)&rs_tedinfo[122],
  0x0001, 0x0003, 0x0208, 0x0001,
  SPGHELP , SPGSETDEC,      147, G_BOX     ,   /* Object 142  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0002, 0x080B, 0x0014, 0x0803,
  SPGSETINC,       -1,       -1, G_BUTTON  ,   /* Object 143 SPGSETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  SPGSETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 144 SPGSETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  SPGSETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 145 SPGSETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
       147,       -1,       -1, G_BUTTON  ,   /* Object 146 SPGSETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
       142,      148, SPGSETNR, G_BOX     ,   /* Object 147  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  SPGSETNR,       -1,       -1, G_STRING  ,   /* Object 148  */
  NONE, NORMAL, (LONG)"SPG-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0801,
       147,       -1,       -1, G_FTEXT   ,   /* Object 149 SPGSETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[123],
  0x0008, 0x0000, 0x0006, 0x0801,
  SPGSTANDARD,       -1,       -1, G_BUTTON  ,   /* Object 150 SPGHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0019, 0x080C, 0x0009, 0x0002,
  SPGCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 151 SPGSTANDARD */
  SELECTABLE, NORMAL, (LONG)"S~tandard",
  0x0024, 0x080C, 0x0009, 0x0002,
  SPGOK   ,       -1,       -1, G_BUTTON  ,   /* Object 152 SPGCANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x002F, 0x080C, 0x0009, 0x0002,
         0,       -1,       -1, G_BUTTON  ,   /* Object 153 SPGOK */
  SELECTABLE|DEFAULT|LASTOB, NORMAL, (LONG)"~OK",
  0x003A, 0x080C, 0x0009, 0x0002,
  
  /******** Tree 1 SPG_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der SPG-Einstellungs-Box",
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
  
  /******** Tree 2 SPG_DESK ****************************************************/
        -1, SPGICON , SPGICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 SPGICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0001, 0x0003, 0x0309, 0x0802,
  
  /******** Tree 3 SPG_TEXT ****************************************************/
        -1, FSPGN   , FSPGI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FSPGI   ,       -1,       -1, G_STRING  ,   /* Object 1 FSPGN */
  NONE, NORMAL, (LONG)" SPG-Setups ",
  0x0002, 0x0001, 0x000C, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FSPGI */
  LASTOB, NORMAL, (LONG)" %d Presets",
  0x0002, 0x0002, 0x000B, 0x0001,
  
  /******** Tree 4 SPG_INFO ****************************************************/
        -1,        1, SPGIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x0010,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[124],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[125],
  0x0009, 0x0802, 0x0012, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[126],
  0x0006, 0x0004, 0x0017, 0x0001,
  SPGIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[127],
  0x0008, 0x0005, 0x0612, 0x0600,
  SPGIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 SPGIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[128],
  0x0007, 0x0006, 0x0014, 0x0001,
  SPGCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 SPGIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[129],
  0x0009, 0x0007, 0x0012, 0x0001,
  SPGISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 SPGCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[130],
  0x0009, 0x0008, 0x0012, 0x0800,
  SPGIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 SPGISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[131],
  0x0008, 0x0809, 0x0013, 0x0001,
  SPGIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 SPGIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[132],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 SPGIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 SPG_SETUP */
  &rs_object[154],   /* Tree  1 SPG_HELP */
  &rs_object[168],   /* Tree  2 SPG_DESK */
  &rs_object[170],   /* Tree  3 SPG_TEXT */
  &rs_object[173]    /* Tree  4 SPG_INFO */
};
