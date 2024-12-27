/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "VAR_MOD.H"

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
{ "Name",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "Qul",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Nr",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "01",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "02",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "01",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "01",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "01",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "01",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "01",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "01",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "01",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "01",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "Name",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "Qul",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Nr",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "Name",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "Qul",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Nr",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "Name",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "Qul",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Nr",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "0123456789",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 11, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "VAR Variablen-System",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 21, 1,
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
  "Setups: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 20,
  "01234567890",
  "akt. Set: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 22
};

static RSIB0MASK[] =
{ 0x01FF, 0xFFF8, 0xFDFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFDFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFBF8, 
  0xFFFF, 0xFDF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xF9F8, 0x01FF, 0xF9F8, 
  0x01FF, 0xF9F8, 0x0000, 0x01F8
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x007F, 0xE1E0, 0x3040, 0x2E60, 
  0x3F40, 0x3000, 0x00FF, 0xE0E0, 
  0x005F, 0xB360, 0x3040, 0x2C00, 
  0x3C40, 0x3660, 0x03C0, 0x6160, 
  0x007F, 0xA080, 0x305F, 0xA060, 
  0x3C40, 0x2060, 0x03C0, 0x2000, 
  0x007F, 0xE1E0, 0x305F, 0xBE60, 
  0x3040, 0x3F00, 0x0040, 0x60E0, 
  0x007F, 0xA060, 0x33DF, 0xA000, 
  0x3C40, 0x2060, 0x0040, 0x2060, 
  0x007F, 0xF000, 0x33DF, 0xAFE0, 
  0x3C40, 0x2060, 0x0040, 0x2000, 
  0x007F, 0xE060, 0x0000, 0x0060, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "VAR Setups", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 VAR_SETUP ****************************************************/
        -1,        1, VAROK   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021121L,
  0x0000, 0x0000, 0x004D, 0x080E,
         5,        2,        4, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0800, 0x0012, 0x0001,
         3,       -1,       -1, G_BOXTEXT ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0007, 0x0000, 0x000B, 0x0001,
         4,       -1,       -1, G_BOXTEXT ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0003, 0x0000, 0x0004, 0x0001,
         1,       -1,       -1, G_BOXTEXT ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0000, 0x0000, 0x0003, 0x0001,
         9, VARQUELLNAME1, VARNUMMER1, G_BOX     ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0801, 0x0012, 0x0001,
  VARQUELLE1,       -1,       -1, G_BOXTEXT ,   /* Object 6 VARQUELLNAME1 */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0007, 0x0000, 0x000B, 0x0001,
  VARNUMMER1,       -1,       -1, G_BOXTEXT ,   /* Object 7 VARQUELLE1 */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0003, 0x0000, 0x0004, 0x0001,
         5,       -1,       -1, G_BOXTEXT ,   /* Object 8 VARNUMMER1 */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0000, 0x0000, 0x0003, 0x0001,
        13,       10, VARNUMMER2, G_BOX     ,   /* Object 9  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0802, 0x0012, 0x0001,
        11,       -1,       -1, G_BOXTEXT ,   /* Object 10  */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0007, 0x0000, 0x000B, 0x0001,
  VARNUMMER2,       -1,       -1, G_BOXTEXT ,   /* Object 11  */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0003, 0x0000, 0x0004, 0x0001,
         9,       -1,       -1, G_BOXTEXT ,   /* Object 12 VARNUMMER2 */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0000, 0x0000, 0x0003, 0x0001,
        17,       14,       16, G_BOX     ,   /* Object 13  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0803, 0x0012, 0x0001,
        15,       -1,       -1, G_BOXTEXT ,   /* Object 14  */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0007, 0x0000, 0x000B, 0x0001,
        16,       -1,       -1, G_BOXTEXT ,   /* Object 15  */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0003, 0x0000, 0x0004, 0x0001,
        13,       -1,       -1, G_BOXTEXT ,   /* Object 16  */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0000, 0x0000, 0x0003, 0x0001,
        21,       18,       20, G_BOX     ,   /* Object 17  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0804, 0x0012, 0x0001,
        19,       -1,       -1, G_BOXTEXT ,   /* Object 18  */
  NONE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0007, 0x0000, 0x000B, 0x0001,
        20,       -1,       -1, G_BOXTEXT ,   /* Object 19  */
  NONE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0003, 0x0000, 0x0004, 0x0001,
        17,       -1,       -1, G_BOXTEXT ,   /* Object 20  */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0000, 0x0000, 0x0003, 0x0001,
        25,       22,       24, G_BOX     ,   /* Object 21  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0805, 0x0012, 0x0001,
        23,       -1,       -1, G_BOXTEXT ,   /* Object 22  */
  NONE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0007, 0x0000, 0x000B, 0x0001,
        24,       -1,       -1, G_BOXTEXT ,   /* Object 23  */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0003, 0x0000, 0x0004, 0x0001,
        21,       -1,       -1, G_BOXTEXT ,   /* Object 24  */
  NONE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0000, 0x0000, 0x0003, 0x0001,
        29,       26,       28, G_BOX     ,   /* Object 25  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0806, 0x0012, 0x0001,
        27,       -1,       -1, G_BOXTEXT ,   /* Object 26  */
  NONE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0007, 0x0000, 0x000B, 0x0001,
        28,       -1,       -1, G_BOXTEXT ,   /* Object 27  */
  NONE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0003, 0x0000, 0x0004, 0x0001,
        25,       -1,       -1, G_BOXTEXT ,   /* Object 28  */
  NONE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0000, 0x0000, 0x0003, 0x0001,
        33,       30,       32, G_BOX     ,   /* Object 29  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0807, 0x0012, 0x0001,
        31,       -1,       -1, G_BOXTEXT ,   /* Object 30  */
  NONE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0007, 0x0000, 0x000B, 0x0001,
        32,       -1,       -1, G_BOXTEXT ,   /* Object 31  */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0003, 0x0000, 0x0004, 0x0001,
        29,       -1,       -1, G_BOXTEXT ,   /* Object 32  */
  NONE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0000, 0x0000, 0x0003, 0x0001,
        37,       34,       36, G_BOX     ,   /* Object 33  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0808, 0x0012, 0x0001,
        35,       -1,       -1, G_BOXTEXT ,   /* Object 34  */
  NONE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0007, 0x0000, 0x000B, 0x0001,
        36,       -1,       -1, G_BOXTEXT ,   /* Object 35  */
  NONE, NORMAL, (LONG)&rs_tedinfo[25],
  0x0003, 0x0000, 0x0004, 0x0001,
        33,       -1,       -1, G_BOXTEXT ,   /* Object 36  */
  NONE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0000, 0x0000, 0x0003, 0x0001,
        41,       38,       40, G_BOX     ,   /* Object 37  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0809, 0x0012, 0x0001,
        39,       -1,       -1, G_BOXTEXT ,   /* Object 38  */
  NONE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0007, 0x0000, 0x000B, 0x0001,
        40,       -1,       -1, G_BOXTEXT ,   /* Object 39  */
  NONE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0003, 0x0000, 0x0004, 0x0001,
        37,       -1,       -1, G_BOXTEXT ,   /* Object 40  */
  NONE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0000, 0x0000, 0x0003, 0x0001,
        45,       42,       44, G_BOX     ,   /* Object 41  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x080A, 0x0012, 0x0001,
        43,       -1,       -1, G_BOXTEXT ,   /* Object 42  */
  NONE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0007, 0x0000, 0x000B, 0x0001,
        44,       -1,       -1, G_BOXTEXT ,   /* Object 43  */
  NONE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0003, 0x0000, 0x0004, 0x0001,
        41,       -1,       -1, G_BOXTEXT ,   /* Object 44  */
  NONE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0000, 0x0000, 0x0003, 0x0001,
        53, VARSETDEC,       50, G_BOX     ,   /* Object 45  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0001, 0x000C, 0x001C, 0x0002,
  VARSETINC,       -1,       -1, G_BUTTON  ,   /* Object 46 VARSETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0002,
  VARSETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 47 VARSETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0002,
  VARSETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 48 VARSETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0014, 0x0000, 0x0008, 0x0001,
        50,       -1,       -1, G_BUTTON  ,   /* Object 49 VARSETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x0014, 0x0001, 0x0008, 0x0001,
        45,       51, VARSETNR, G_BOX     ,   /* Object 50  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0002,
  VARSETNR,       -1,       -1, G_STRING  ,   /* Object 51  */
  NONE, NORMAL, (LONG)"VAR-~Nr:",
  0x0400, 0x0000, 0x0008, 0x0002,
        50,       -1,       -1, G_FTEXT   ,   /* Object 52 VARSETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0008, 0x0000, 0x0006, 0x0002,
        57,       54,       56, G_BOX     ,   /* Object 53  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0800, 0x0012, 0x0001,
        55,       -1,       -1, G_BOXTEXT ,   /* Object 54  */
  NONE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0007, 0x0000, 0x000B, 0x0001,
        56,       -1,       -1, G_BOXTEXT ,   /* Object 55  */
  NONE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0003, 0x0000, 0x0004, 0x0001,
        53,       -1,       -1, G_BOXTEXT ,   /* Object 56  */
  NONE, NORMAL, (LONG)&rs_tedinfo[36],
  0x0000, 0x0000, 0x0003, 0x0001,
        61,       58, VARNUMMER11, G_BOX     ,   /* Object 57  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0801, 0x0012, 0x0001,
        59,       -1,       -1, G_BOXTEXT ,   /* Object 58  */
  NONE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0007, 0x0000, 0x000B, 0x0001,
  VARNUMMER11,       -1,       -1, G_BOXTEXT ,   /* Object 59  */
  NONE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0003, 0x0000, 0x0004, 0x0001,
        57,       -1,       -1, G_BOXTEXT ,   /* Object 60 VARNUMMER11 */
  NONE, NORMAL, (LONG)&rs_tedinfo[39],
  0x0000, 0x0000, 0x0003, 0x0001,
        65,       62,       64, G_BOX     ,   /* Object 61  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0802, 0x0012, 0x0001,
        63,       -1,       -1, G_BOXTEXT ,   /* Object 62  */
  NONE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0007, 0x0000, 0x000B, 0x0001,
        64,       -1,       -1, G_BOXTEXT ,   /* Object 63  */
  NONE, NORMAL, (LONG)&rs_tedinfo[41],
  0x0003, 0x0000, 0x0004, 0x0001,
        61,       -1,       -1, G_BOXTEXT ,   /* Object 64  */
  NONE, NORMAL, (LONG)&rs_tedinfo[42],
  0x0000, 0x0000, 0x0003, 0x0001,
        69,       66,       68, G_BOX     ,   /* Object 65  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0803, 0x0012, 0x0001,
        67,       -1,       -1, G_BOXTEXT ,   /* Object 66  */
  NONE, NORMAL, (LONG)&rs_tedinfo[43],
  0x0007, 0x0000, 0x000B, 0x0001,
        68,       -1,       -1, G_BOXTEXT ,   /* Object 67  */
  NONE, NORMAL, (LONG)&rs_tedinfo[44],
  0x0003, 0x0000, 0x0004, 0x0001,
        65,       -1,       -1, G_BOXTEXT ,   /* Object 68  */
  NONE, NORMAL, (LONG)&rs_tedinfo[45],
  0x0000, 0x0000, 0x0003, 0x0001,
        73,       70,       72, G_BOX     ,   /* Object 69  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0804, 0x0012, 0x0001,
        71,       -1,       -1, G_BOXTEXT ,   /* Object 70  */
  NONE, NORMAL, (LONG)&rs_tedinfo[46],
  0x0007, 0x0000, 0x000B, 0x0001,
        72,       -1,       -1, G_BOXTEXT ,   /* Object 71  */
  NONE, NORMAL, (LONG)&rs_tedinfo[47],
  0x0003, 0x0000, 0x0004, 0x0001,
        69,       -1,       -1, G_BOXTEXT ,   /* Object 72  */
  NONE, NORMAL, (LONG)&rs_tedinfo[48],
  0x0000, 0x0000, 0x0003, 0x0001,
        77,       74,       76, G_BOX     ,   /* Object 73  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0805, 0x0012, 0x0001,
        75,       -1,       -1, G_BOXTEXT ,   /* Object 74  */
  NONE, NORMAL, (LONG)&rs_tedinfo[49],
  0x0007, 0x0000, 0x000B, 0x0001,
        76,       -1,       -1, G_BOXTEXT ,   /* Object 75  */
  NONE, NORMAL, (LONG)&rs_tedinfo[50],
  0x0003, 0x0000, 0x0004, 0x0001,
        73,       -1,       -1, G_BOXTEXT ,   /* Object 76  */
  NONE, NORMAL, (LONG)&rs_tedinfo[51],
  0x0000, 0x0000, 0x0003, 0x0001,
        81,       78,       80, G_BOX     ,   /* Object 77  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0806, 0x0012, 0x0001,
        79,       -1,       -1, G_BOXTEXT ,   /* Object 78  */
  NONE, NORMAL, (LONG)&rs_tedinfo[52],
  0x0007, 0x0000, 0x000B, 0x0001,
        80,       -1,       -1, G_BOXTEXT ,   /* Object 79  */
  NONE, NORMAL, (LONG)&rs_tedinfo[53],
  0x0003, 0x0000, 0x0004, 0x0001,
        77,       -1,       -1, G_BOXTEXT ,   /* Object 80  */
  NONE, NORMAL, (LONG)&rs_tedinfo[54],
  0x0000, 0x0000, 0x0003, 0x0001,
        85,       82,       84, G_BOX     ,   /* Object 81  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0807, 0x0012, 0x0001,
        83,       -1,       -1, G_BOXTEXT ,   /* Object 82  */
  NONE, NORMAL, (LONG)&rs_tedinfo[55],
  0x0007, 0x0000, 0x000B, 0x0001,
        84,       -1,       -1, G_BOXTEXT ,   /* Object 83  */
  NONE, NORMAL, (LONG)&rs_tedinfo[56],
  0x0003, 0x0000, 0x0004, 0x0001,
        81,       -1,       -1, G_BOXTEXT ,   /* Object 84  */
  NONE, NORMAL, (LONG)&rs_tedinfo[57],
  0x0000, 0x0000, 0x0003, 0x0001,
        89,       86,       88, G_BOX     ,   /* Object 85  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0808, 0x0012, 0x0001,
        87,       -1,       -1, G_BOXTEXT ,   /* Object 86  */
  NONE, NORMAL, (LONG)&rs_tedinfo[58],
  0x0007, 0x0000, 0x000B, 0x0001,
        88,       -1,       -1, G_BOXTEXT ,   /* Object 87  */
  NONE, NORMAL, (LONG)&rs_tedinfo[59],
  0x0003, 0x0000, 0x0004, 0x0001,
        85,       -1,       -1, G_BOXTEXT ,   /* Object 88  */
  NONE, NORMAL, (LONG)&rs_tedinfo[60],
  0x0000, 0x0000, 0x0003, 0x0001,
        93,       90,       92, G_BOX     ,   /* Object 89  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x0809, 0x0012, 0x0001,
        91,       -1,       -1, G_BOXTEXT ,   /* Object 90  */
  NONE, NORMAL, (LONG)&rs_tedinfo[61],
  0x0007, 0x0000, 0x000B, 0x0001,
        92,       -1,       -1, G_BOXTEXT ,   /* Object 91  */
  NONE, NORMAL, (LONG)&rs_tedinfo[62],
  0x0003, 0x0000, 0x0004, 0x0001,
        89,       -1,       -1, G_BOXTEXT ,   /* Object 92  */
  NONE, NORMAL, (LONG)&rs_tedinfo[63],
  0x0000, 0x0000, 0x0003, 0x0001,
  VARHELP ,       94,       96, G_BOX     ,   /* Object 93  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0014, 0x080A, 0x0012, 0x0001,
        95,       -1,       -1, G_BOXTEXT ,   /* Object 94  */
  NONE, NORMAL, (LONG)&rs_tedinfo[64],
  0x0007, 0x0000, 0x000B, 0x0001,
        96,       -1,       -1, G_BOXTEXT ,   /* Object 95  */
  NONE, NORMAL, (LONG)&rs_tedinfo[65],
  0x0003, 0x0000, 0x0004, 0x0001,
        93,       -1,       -1, G_BOXTEXT ,   /* Object 96  */
  NONE, NORMAL, (LONG)&rs_tedinfo[66],
  0x0000, 0x0000, 0x0003, 0x0001,
  VARSTANDARD,       -1,       -1, G_BUTTON  ,   /* Object 97 VARHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x001F, 0x000C, 0x000A, 0x0002,
        99,       -1,       -1, G_BUTTON  ,   /* Object 98 VARSTANDARD */
  SELECTABLE, NORMAL, (LONG)"S~tandard",
  0x002B, 0x000C, 0x000A, 0x0002,
       103,      100,      102, G_BOX     ,   /* Object 99  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0800, 0x0012, 0x0001,
       101,       -1,       -1, G_BOXTEXT ,   /* Object 100  */
  NONE, NORMAL, (LONG)&rs_tedinfo[67],
  0x0007, 0x0000, 0x000B, 0x0001,
       102,       -1,       -1, G_BOXTEXT ,   /* Object 101  */
  NONE, NORMAL, (LONG)&rs_tedinfo[68],
  0x0003, 0x0000, 0x0004, 0x0001,
        99,       -1,       -1, G_BOXTEXT ,   /* Object 102  */
  NONE, NORMAL, (LONG)&rs_tedinfo[69],
  0x0000, 0x0000, 0x0003, 0x0001,
       107, VARQUELLNAME21, VARNUMMER21, G_BOX     ,   /* Object 103  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0801, 0x0012, 0x0001,
  VARQUELLE21,       -1,       -1, G_BOXTEXT ,   /* Object 104 VARQUELLNAME21 */
  NONE, NORMAL, (LONG)&rs_tedinfo[70],
  0x0007, 0x0000, 0x000B, 0x0001,
  VARNUMMER21,       -1,       -1, G_BOXTEXT ,   /* Object 105 VARQUELLE21 */
  NONE, NORMAL, (LONG)&rs_tedinfo[71],
  0x0003, 0x0000, 0x0004, 0x0001,
       103,       -1,       -1, G_BOXTEXT ,   /* Object 106 VARNUMMER21 */
  NONE, NORMAL, (LONG)&rs_tedinfo[72],
  0x0000, 0x0000, 0x0003, 0x0001,
       111,      108,      110, G_BOX     ,   /* Object 107  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0802, 0x0012, 0x0001,
       109,       -1,       -1, G_BOXTEXT ,   /* Object 108  */
  NONE, NORMAL, (LONG)&rs_tedinfo[73],
  0x0007, 0x0000, 0x000B, 0x0001,
       110,       -1,       -1, G_BOXTEXT ,   /* Object 109  */
  NONE, NORMAL, (LONG)&rs_tedinfo[74],
  0x0003, 0x0000, 0x0004, 0x0001,
       107,       -1,       -1, G_BOXTEXT ,   /* Object 110  */
  NONE, NORMAL, (LONG)&rs_tedinfo[75],
  0x0000, 0x0000, 0x0003, 0x0001,
       115,      112,      114, G_BOX     ,   /* Object 111  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0803, 0x0012, 0x0001,
       113,       -1,       -1, G_BOXTEXT ,   /* Object 112  */
  NONE, NORMAL, (LONG)&rs_tedinfo[76],
  0x0007, 0x0000, 0x000B, 0x0001,
       114,       -1,       -1, G_BOXTEXT ,   /* Object 113  */
  NONE, NORMAL, (LONG)&rs_tedinfo[77],
  0x0003, 0x0000, 0x0004, 0x0001,
       111,       -1,       -1, G_BOXTEXT ,   /* Object 114  */
  NONE, NORMAL, (LONG)&rs_tedinfo[78],
  0x0000, 0x0000, 0x0003, 0x0001,
       119,      116,      118, G_BOX     ,   /* Object 115  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0804, 0x0012, 0x0001,
       117,       -1,       -1, G_BOXTEXT ,   /* Object 116  */
  NONE, NORMAL, (LONG)&rs_tedinfo[79],
  0x0007, 0x0000, 0x000B, 0x0001,
       118,       -1,       -1, G_BOXTEXT ,   /* Object 117  */
  NONE, NORMAL, (LONG)&rs_tedinfo[80],
  0x0003, 0x0000, 0x0004, 0x0001,
       115,       -1,       -1, G_BOXTEXT ,   /* Object 118  */
  NONE, NORMAL, (LONG)&rs_tedinfo[81],
  0x0000, 0x0000, 0x0003, 0x0001,
       123,      120,      122, G_BOX     ,   /* Object 119  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0805, 0x0012, 0x0001,
       121,       -1,       -1, G_BOXTEXT ,   /* Object 120  */
  NONE, NORMAL, (LONG)&rs_tedinfo[82],
  0x0007, 0x0000, 0x000B, 0x0001,
       122,       -1,       -1, G_BOXTEXT ,   /* Object 121  */
  NONE, NORMAL, (LONG)&rs_tedinfo[83],
  0x0003, 0x0000, 0x0004, 0x0001,
       119,       -1,       -1, G_BOXTEXT ,   /* Object 122  */
  NONE, NORMAL, (LONG)&rs_tedinfo[84],
  0x0000, 0x0000, 0x0003, 0x0001,
       127,      124,      126, G_BOX     ,   /* Object 123  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0806, 0x0012, 0x0001,
       125,       -1,       -1, G_BOXTEXT ,   /* Object 124  */
  NONE, NORMAL, (LONG)&rs_tedinfo[85],
  0x0007, 0x0000, 0x000B, 0x0001,
       126,       -1,       -1, G_BOXTEXT ,   /* Object 125  */
  NONE, NORMAL, (LONG)&rs_tedinfo[86],
  0x0003, 0x0000, 0x0004, 0x0001,
       123,       -1,       -1, G_BOXTEXT ,   /* Object 126  */
  NONE, NORMAL, (LONG)&rs_tedinfo[87],
  0x0000, 0x0000, 0x0003, 0x0001,
       131,      128,      130, G_BOX     ,   /* Object 127  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0807, 0x0012, 0x0001,
       129,       -1,       -1, G_BOXTEXT ,   /* Object 128  */
  NONE, NORMAL, (LONG)&rs_tedinfo[88],
  0x0007, 0x0000, 0x000B, 0x0001,
       130,       -1,       -1, G_BOXTEXT ,   /* Object 129  */
  NONE, NORMAL, (LONG)&rs_tedinfo[89],
  0x0003, 0x0000, 0x0004, 0x0001,
       127,       -1,       -1, G_BOXTEXT ,   /* Object 130  */
  NONE, NORMAL, (LONG)&rs_tedinfo[90],
  0x0000, 0x0000, 0x0003, 0x0001,
       135,      132,      134, G_BOX     ,   /* Object 131  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0808, 0x0012, 0x0001,
       133,       -1,       -1, G_BOXTEXT ,   /* Object 132  */
  NONE, NORMAL, (LONG)&rs_tedinfo[91],
  0x0007, 0x0000, 0x000B, 0x0001,
       134,       -1,       -1, G_BOXTEXT ,   /* Object 133  */
  NONE, NORMAL, (LONG)&rs_tedinfo[92],
  0x0003, 0x0000, 0x0004, 0x0001,
       131,       -1,       -1, G_BOXTEXT ,   /* Object 134  */
  NONE, NORMAL, (LONG)&rs_tedinfo[93],
  0x0000, 0x0000, 0x0003, 0x0001,
       139,      136,      138, G_BOX     ,   /* Object 135  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x0809, 0x0012, 0x0001,
       137,       -1,       -1, G_BOXTEXT ,   /* Object 136  */
  NONE, NORMAL, (LONG)&rs_tedinfo[94],
  0x0007, 0x0000, 0x000B, 0x0001,
       138,       -1,       -1, G_BOXTEXT ,   /* Object 137  */
  NONE, NORMAL, (LONG)&rs_tedinfo[95],
  0x0003, 0x0000, 0x0004, 0x0001,
       135,       -1,       -1, G_BOXTEXT ,   /* Object 138  */
  NONE, NORMAL, (LONG)&rs_tedinfo[96],
  0x0000, 0x0000, 0x0003, 0x0001,
  VARCANCEL,      140,      142, G_BOX     ,   /* Object 139  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0027, 0x080A, 0x0012, 0x0001,
       141,       -1,       -1, G_BOXTEXT ,   /* Object 140  */
  NONE, NORMAL, (LONG)&rs_tedinfo[97],
  0x0007, 0x0000, 0x000B, 0x0001,
       142,       -1,       -1, G_BOXTEXT ,   /* Object 141  */
  NONE, NORMAL, (LONG)&rs_tedinfo[98],
  0x0003, 0x0000, 0x0004, 0x0001,
       139,       -1,       -1, G_BOXTEXT ,   /* Object 142  */
  NONE, NORMAL, (LONG)&rs_tedinfo[99],
  0x0000, 0x0000, 0x0003, 0x0001,
       144,       -1,       -1, G_BUTTON  ,   /* Object 143 VARCANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0436, 0x000C, 0x000A, 0x0002,
       148,      145,      147, G_BOX     ,   /* Object 144  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0800, 0x0012, 0x0001,
       146,       -1,       -1, G_BOXTEXT ,   /* Object 145  */
  NONE, NORMAL, (LONG)&rs_tedinfo[100],
  0x0007, 0x0000, 0x000B, 0x0001,
       147,       -1,       -1, G_BOXTEXT ,   /* Object 146  */
  NONE, NORMAL, (LONG)&rs_tedinfo[101],
  0x0003, 0x0000, 0x0004, 0x0001,
       144,       -1,       -1, G_BOXTEXT ,   /* Object 147  */
  NONE, NORMAL, (LONG)&rs_tedinfo[102],
  0x0000, 0x0000, 0x0003, 0x0001,
       152,      149, VARNUMMER31, G_BOX     ,   /* Object 148  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0801, 0x0012, 0x0001,
       150,       -1,       -1, G_BOXTEXT ,   /* Object 149  */
  NONE, NORMAL, (LONG)&rs_tedinfo[103],
  0x0007, 0x0000, 0x000B, 0x0001,
  VARNUMMER31,       -1,       -1, G_BOXTEXT ,   /* Object 150  */
  NONE, NORMAL, (LONG)&rs_tedinfo[104],
  0x0003, 0x0000, 0x0004, 0x0001,
       148,       -1,       -1, G_BOXTEXT ,   /* Object 151 VARNUMMER31 */
  NONE, NORMAL, (LONG)&rs_tedinfo[105],
  0x0000, 0x0000, 0x0003, 0x0001,
       156,      153,      155, G_BOX     ,   /* Object 152  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0802, 0x0012, 0x0001,
       154,       -1,       -1, G_BOXTEXT ,   /* Object 153  */
  NONE, NORMAL, (LONG)&rs_tedinfo[106],
  0x0007, 0x0000, 0x000B, 0x0001,
       155,       -1,       -1, G_BOXTEXT ,   /* Object 154  */
  NONE, NORMAL, (LONG)&rs_tedinfo[107],
  0x0003, 0x0000, 0x0004, 0x0001,
       152,       -1,       -1, G_BOXTEXT ,   /* Object 155  */
  NONE, NORMAL, (LONG)&rs_tedinfo[108],
  0x0000, 0x0000, 0x0003, 0x0001,
       160,      157,      159, G_BOX     ,   /* Object 156  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0803, 0x0012, 0x0001,
       158,       -1,       -1, G_BOXTEXT ,   /* Object 157  */
  NONE, NORMAL, (LONG)&rs_tedinfo[109],
  0x0007, 0x0000, 0x000B, 0x0001,
       159,       -1,       -1, G_BOXTEXT ,   /* Object 158  */
  NONE, NORMAL, (LONG)&rs_tedinfo[110],
  0x0003, 0x0000, 0x0004, 0x0001,
       156,       -1,       -1, G_BOXTEXT ,   /* Object 159  */
  NONE, NORMAL, (LONG)&rs_tedinfo[111],
  0x0000, 0x0000, 0x0003, 0x0001,
       164,      161,      163, G_BOX     ,   /* Object 160  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0804, 0x0012, 0x0001,
       162,       -1,       -1, G_BOXTEXT ,   /* Object 161  */
  NONE, NORMAL, (LONG)&rs_tedinfo[112],
  0x0007, 0x0000, 0x000B, 0x0001,
       163,       -1,       -1, G_BOXTEXT ,   /* Object 162  */
  NONE, NORMAL, (LONG)&rs_tedinfo[113],
  0x0003, 0x0000, 0x0004, 0x0001,
       160,       -1,       -1, G_BOXTEXT ,   /* Object 163  */
  NONE, NORMAL, (LONG)&rs_tedinfo[114],
  0x0000, 0x0000, 0x0003, 0x0001,
       168,      165,      167, G_BOX     ,   /* Object 164  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0805, 0x0012, 0x0001,
       166,       -1,       -1, G_BOXTEXT ,   /* Object 165  */
  NONE, NORMAL, (LONG)&rs_tedinfo[115],
  0x0007, 0x0000, 0x000B, 0x0001,
       167,       -1,       -1, G_BOXTEXT ,   /* Object 166  */
  NONE, NORMAL, (LONG)&rs_tedinfo[116],
  0x0003, 0x0000, 0x0004, 0x0001,
       164,       -1,       -1, G_BOXTEXT ,   /* Object 167  */
  NONE, NORMAL, (LONG)&rs_tedinfo[117],
  0x0000, 0x0000, 0x0003, 0x0001,
       172,      169,      171, G_BOX     ,   /* Object 168  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0806, 0x0012, 0x0001,
       170,       -1,       -1, G_BOXTEXT ,   /* Object 169  */
  NONE, NORMAL, (LONG)&rs_tedinfo[118],
  0x0007, 0x0000, 0x000B, 0x0001,
       171,       -1,       -1, G_BOXTEXT ,   /* Object 170  */
  NONE, NORMAL, (LONG)&rs_tedinfo[119],
  0x0003, 0x0000, 0x0004, 0x0001,
       168,       -1,       -1, G_BOXTEXT ,   /* Object 171  */
  NONE, NORMAL, (LONG)&rs_tedinfo[120],
  0x0000, 0x0000, 0x0003, 0x0001,
       176,      173,      175, G_BOX     ,   /* Object 172  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0807, 0x0012, 0x0001,
       174,       -1,       -1, G_BOXTEXT ,   /* Object 173  */
  NONE, NORMAL, (LONG)&rs_tedinfo[121],
  0x0007, 0x0000, 0x000B, 0x0001,
       175,       -1,       -1, G_BOXTEXT ,   /* Object 174  */
  NONE, NORMAL, (LONG)&rs_tedinfo[122],
  0x0003, 0x0000, 0x0004, 0x0001,
       172,       -1,       -1, G_BOXTEXT ,   /* Object 175  */
  NONE, NORMAL, (LONG)&rs_tedinfo[123],
  0x0000, 0x0000, 0x0003, 0x0001,
       180,      177,      179, G_BOX     ,   /* Object 176  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0808, 0x0012, 0x0001,
       178,       -1,       -1, G_BOXTEXT ,   /* Object 177  */
  NONE, NORMAL, (LONG)&rs_tedinfo[124],
  0x0007, 0x0000, 0x000B, 0x0001,
       179,       -1,       -1, G_BOXTEXT ,   /* Object 178  */
  NONE, NORMAL, (LONG)&rs_tedinfo[125],
  0x0003, 0x0000, 0x0004, 0x0001,
       176,       -1,       -1, G_BOXTEXT ,   /* Object 179  */
  NONE, NORMAL, (LONG)&rs_tedinfo[126],
  0x0000, 0x0000, 0x0003, 0x0001,
       184,      181,      183, G_BOX     ,   /* Object 180  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x0809, 0x0012, 0x0001,
       182,       -1,       -1, G_BOXTEXT ,   /* Object 181  */
  NONE, NORMAL, (LONG)&rs_tedinfo[127],
  0x0007, 0x0000, 0x000B, 0x0001,
       183,       -1,       -1, G_BOXTEXT ,   /* Object 182  */
  NONE, NORMAL, (LONG)&rs_tedinfo[128],
  0x0003, 0x0000, 0x0004, 0x0001,
       180,       -1,       -1, G_BOXTEXT ,   /* Object 183  */
  NONE, NORMAL, (LONG)&rs_tedinfo[129],
  0x0000, 0x0000, 0x0003, 0x0001,
  VAROK   ,      185,      187, G_BOX     ,   /* Object 184  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x003A, 0x080A, 0x0012, 0x0001,
       186,       -1,       -1, G_BOXTEXT ,   /* Object 185  */
  NONE, NORMAL, (LONG)&rs_tedinfo[130],
  0x0007, 0x0000, 0x000B, 0x0001,
       187,       -1,       -1, G_BOXTEXT ,   /* Object 186  */
  NONE, NORMAL, (LONG)&rs_tedinfo[131],
  0x0003, 0x0000, 0x0004, 0x0001,
       184,       -1,       -1, G_BOXTEXT ,   /* Object 187  */
  NONE, NORMAL, (LONG)&rs_tedinfo[132],
  0x0000, 0x0000, 0x0003, 0x0001,
         0,       -1,       -1, G_BUTTON  ,   /* Object 188 VAROK */
  SELECTABLE|DEFAULT|LASTOB, NORMAL, (LONG)"~OK",
  0x0042, 0x000C, 0x000A, 0x0002,
  
  /******** Tree 1 VAR_INFO ****************************************************/
        -1,        1, VARIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[133],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[134],
  0x0007, 0x0802, 0x0014, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[135],
  0x0006, 0x0004, 0x0017, 0x0001,
  VARIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[136],
  0x0008, 0x0005, 0x0612, 0x0600,
  VARIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 VARIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[137],
  0x0007, 0x0006, 0x0014, 0x0001,
  VARICOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 VARIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[138],
  0x0009, 0x0007, 0x0012, 0x0001,
  VARISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 VARICOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[139],
  0x0009, 0x0008, 0x0012, 0x0800,
  VARIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 VARISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[140],
  0x0008, 0x0809, 0x0013, 0x0001,
  VARIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 VARIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[141],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 VARIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002,
  
  /******** Tree 2 VAR_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der VAR-Einstellungs-Box",
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
  
  /******** Tree 3 VAR_DESK ****************************************************/
        -1, VARICON , VARICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 VARICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0001, 0x0001, 0x0009, 0x0802,
  
  /******** Tree 4 VAR_TEXT ****************************************************/
        -1, FVARN   , FVARH   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FVARI   ,       -1,       -1, G_STRING  ,   /* Object 1 FVARN */
  NONE, NORMAL, (LONG)" VAR-Setups ",
  0x0002, 0x0001, 0x000C, 0x0001,
  FVARH   ,       -1,       -1, G_STRING  ,   /* Object 2 FVARI */
  NONE, NORMAL, (LONG)" %d Presets",
  0x0002, 0x0002, 0x000B, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 3 FVARH */
  LASTOB, NORMAL, (LONG)"VAR Setup Hilfe",
  0x0002, 0x0004, 0x000F, 0x0001
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 VAR_SETUP */
  &rs_object[189],   /* Tree  1 VAR_INFO */
  &rs_object[200],   /* Tree  2 VAR_HELP */
  &rs_object[214],   /* Tree  3 VAR_DESK */
  &rs_object[216]    /* Tree  4 VAR_TEXT */
};
