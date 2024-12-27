/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "POW_MOD.H"

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
{ "Achsen-",
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
  "Zoom (in %)",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 1, 12, 1,
  "Sperre aussen",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 1, 14, 1,
  "Sperre innen",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 1, 13, 1,
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
  "\0",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 1, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "an",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 3, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "an",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 1, 3, 1,
  "____",
  "____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 1, 5, 5,
  "Powerglove-Setup",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 17, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "POW Powerglove-Ansteuerung",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 27, 1,
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
{ 0x0000, 0x0000, 0x0038, 0x0000, 
  0x00FE, 0x0000, 0x00FE, 0x0000, 
  0x00FF, 0x8000, 0x00FF, 0x8000, 
  0x00FF, 0x8000, 0x00FF, 0x8000, 
  0x0FFF, 0x8000, 0x0FFF, 0x8000, 
  0x0FFF, 0x8000, 0x0FFF, 0x80F8, 
  0x07FF, 0x81F8, 0x03FF, 0x81F8, 
  0x01FF, 0x81E0, 0x01FF, 0x81E0, 
  0x01FF, 0x81E0, 0x01FF, 0x81F0, 
  0x00FF, 0x00F0, 0x00FF, 0x00F0, 
  0x00FF, 0x00F0, 0x00FF, 0x01F0, 
  0x00FF, 0x03E0, 0x00FF, 0x07E0, 
  0x00FF, 0x0FC0, 0x00FF, 0x3F80, 
  0x000F, 0xFF00, 0x000F, 0xFE00, 
  0x000F, 0xF800, 0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0010, 0x0000, 0x0054, 0x0000, 
  0x0054, 0x0000, 0x0055, 0x0000, 
  0x0055, 0x0000, 0x0055, 0x0000, 
  0x0055, 0x0000, 0x04FF, 0x0000, 
  0x0681, 0x0000, 0x03FF, 0x0000, 
  0x01FF, 0x0070, 0x0081, 0x00C0, 
  0x00FF, 0x0080, 0x0081, 0x0080, 
  0x00FF, 0x00C0, 0x0042, 0x0040, 
  0x005A, 0x0020, 0x005A, 0x0020, 
  0x005A, 0x0060, 0x005A, 0x0040, 
  0x005A, 0x00C0, 0x0042, 0x0180, 
  0x007E, 0x0300, 0x0004, 0x0600, 
  0x0004, 0x1C00, 0x0007, 0xF000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "POW Powergl.", 0x1000|'\0',
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
  0x0000, 0x0000, 0x900F, 0x8000, 
  0x0000, 0xA018, 0xC000, 0x0000, 
  0xC010, 0x4000, 0x0000, 0xFFFF, 
  0xC000, 0x0000, 0x0010, 0x4000, 
  0x0000, 0x0018, 0xC000, 0x0000, 
  0x000F, 0x8000, 0x0000, 0x0000, 
  0x0000
};

static WORD RSBB1DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x8000, 0x0000, 
  0x0000, 0x8000, 0x0000, 0x0007, 
  0xF000, 0x0000, 0x000C, 0x9800, 
  0x0000, 0x0008, 0x8800, 0x0000, 
  0x000C, 0x9800, 0x0000, 0x0007, 
  0xF0C0, 0x0000, 0x0000, 0x80C0, 
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
  0x0000, 0x0000, 0x83E0, 0x0000, 
  0x0000, 0x8630, 0x0000, 0x0000, 
  0x8410, 0x0000, 0x0000, 0x8490, 
  0x0000, 0x0000, 0x8510, 0x0000, 
  0x0000, 0x8630, 0x0000, 0x0000, 
  0x87E0, 0x0000, 0x0000, 0x8800, 
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
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x07C0, 0x0000, 
  0x0000, 0x0C60, 0x0000, 0x0000, 
  0x0820, 0x0000, 0x0000, 0x1830, 
  0x0000, 0x0000, 0x1010, 0x0000, 
  0x0000, 0x1010, 0xC000, 0x0000, 
  0x1017, 0x0000, 0x0000, 0x1038, 
  0x0000, 0x0000, 0x11D0, 0x0000, 
  0x0000, 0x1010, 0x0000, 0x0000, 
  0x1010, 0x0000, 0x0000, 0x1010, 
  0x0000, 0x0000, 0x1010, 0x0000, 
  0x0000, 0x1830, 0x0000, 0x0000, 
  0x0820, 0x0000, 0x0000, 0x0C60, 
  0x0000, 0x0000, 0x07C0, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000
};

static WORD RSBB4DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x8000, 0x0000, 0x0001, 
  0x0000, 0x0000, 0x0002, 0x0000, 
  0x0000, 0x0004, 0x0000, 0x0000, 
  0x1FF4, 0x0000, 0x0000, 0x6013, 
  0x0000, 0x0001, 0x8020, 0xC000, 
  0x0001, 0x0040, 0x4000, 0x0001, 
  0x0080, 0x4000, 0x0001, 0x0000, 
  0x4000, 0x0001, 0x8000, 0xC000, 
  0x0000, 0x6003, 0x0000, 0x0000, 
  0x1FFC, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000
};

static WORD RSBB5DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0FF8, 
  0x0000, 0x0000, 0x180C, 0x0000, 
  0x0000, 0x6003, 0x0000, 0x0000, 
  0x4001, 0x0000, 0x0000, 0x8000, 
  0x8000, 0x0001, 0x9000, 0xC000, 
  0x0001, 0x0800, 0x4000, 0x0001, 
  0x0400, 0x4000, 0x0001, 0x0200, 
  0x4000, 0x0001, 0x0100, 0x4000, 
  0x0001, 0x00C0, 0x4000, 0x0001, 
  0x0000, 0x4000, 0x0001, 0x0000, 
  0x4000, 0x0001, 0x8000, 0xC000, 
  0x0000, 0x8000, 0x8000, 0x0000, 
  0x4001, 0x0000, 0x0000, 0x6003, 
  0x0000, 0x0000, 0x180C, 0x0000, 
  0x0000, 0x0FF8, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
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
  /******** Tree 0 POW_SETUP ****************************************************/
        -1, POWOK   ,       51, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021111L,
  0x0000, 0x0000, 0x042D, 0x080E,
  POWCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 1 POWOK */
  SELECTABLE|DEFAULT, NORMAL, (LONG)"~OK",
  0x0423, 0x000C, 0x0009, 0x0002,
  POWHELP ,       -1,       -1, G_BUTTON  ,   /* Object 2 POWCANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0418, 0x000C, 0x0009, 0x0002,
         4,       -1,       -1, G_BUTTON  ,   /* Object 3 POWHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0418, 0x000A, 0x0009, 0x0801,
        22,        5,       21, G_BOX     ,   /* Object 4  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x0001, 0x0800, 0x000F, 0x080D,
        12,        6,       11, G_BOX     ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0000, 0x0000, 0x000F, 0x0804,
         7,       -1,       -1, G_IMAGE   ,   /* Object 6  */
  NONE, NORMAL, (LONG)&rs_bitblk[0],
  0x0009, 0x0000, 0x0006, 0x0701,
         8,       -1,       -1, G_IMAGE   ,   /* Object 7  */
  NONE, NORMAL, (LONG)&rs_bitblk[1],
  0x0009, 0x0801, 0x0006, 0x0701,
         9,       -1,       -1, G_IMAGE   ,   /* Object 8  */
  NONE, NORMAL, (LONG)&rs_bitblk[2],
  0x0009, 0x0003, 0x0006, 0x0701,
        10,       -1,       -1, G_BOXTEXT ,   /* Object 9  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0002, 0x0001, 0x0008, 0x0001,
        11,       -1,       -1, G_BOXTEXT ,   /* Object 10  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0002, 0x0002, 0x0407, 0x0001,
         5,       -1,       -1, G_BOXTEXT ,   /* Object 11  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0002, 0x0003, 0x0208, 0x0001,
        19,       13,       18, G_BOX     ,   /* Object 12  */
  NONE, NORMAL, (LONG)0x00011101L,
  0x0000, 0x0804, 0x000F, 0x0804,
        14,       -1,       -1, G_IMAGE   ,   /* Object 13  */
  NONE, NORMAL, (LONG)&rs_bitblk[3],
  0x0009, 0x0003, 0x0006, 0x0701,
        15,       -1,       -1, G_IMAGE   ,   /* Object 14  */
  NONE, NORMAL, (LONG)&rs_bitblk[4],
  0x0009, 0x0000, 0x0006, 0x0701,
        16,       -1,       -1, G_IMAGE   ,   /* Object 15  */
  NONE, NORMAL, (LONG)&rs_bitblk[5],
  0x0009, 0x0801, 0x0006, 0x0701,
        17,       -1,       -1, G_BOXTEXT ,   /* Object 16  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0002, 0x0001, 0x0008, 0x0001,
        18,       -1,       -1, G_BOXTEXT ,   /* Object 17  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0002, 0x0002, 0x0407, 0x0001,
        12,       -1,       -1, G_BOXTEXT ,   /* Object 18  */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0002, 0x0003, 0x0208, 0x0001,
        20,       -1,       -1, G_BOXTEXT ,   /* Object 19  */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0000, 0x0009, 0x000F, 0x0801,
        21,       -1,       -1, G_BOXTEXT ,   /* Object 20  */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0000, 0x080A, 0x000F, 0x0801,
         4,       -1,       -1, G_BOXTEXT ,   /* Object 21  */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0000, 0x000C, 0x000F, 0x0801,
        41, POWPROPROTX, POWSPERREINNEN, G_BOX     ,   /* Object 22  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x0010, 0x0800, 0x0007, 0x080D,
  POWROTX ,       -1,       -1, G_BOXTEXT ,   /* Object 23 POWPROPROTX */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0000, 0x0000, 0x0002, 0x0801,
  POWPROPROTY,       -1,       -1, G_FBOXTEXT,   /* Object 24 POWROTX */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0002, 0x0000, 0x0005, 0x0801,
  POWROTY ,       -1,       -1, G_BOXTEXT ,   /* Object 25 POWPROPROTY */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0000, 0x0801, 0x0002, 0x0801,
  POWPROPROTZ,       -1,       -1, G_FBOXTEXT,   /* Object 26 POWROTY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0002, 0x0801, 0x0005, 0x0801,
  POWROTZ ,       -1,       -1, G_BOXTEXT ,   /* Object 27 POWPROPROTZ */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0000, 0x0003, 0x0002, 0x0801,
  POWPROPWINKXZ,       -1,       -1, G_FBOXTEXT,   /* Object 28 POWROTZ */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0002, 0x0003, 0x0005, 0x0801,
  POWWINKXZ,       -1,       -1, G_BOXTEXT ,   /* Object 29 POWPROPWINKXZ */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0000, 0x0804, 0x0002, 0x0801,
  POWPROPWINKXY,       -1,       -1, G_FBOXTEXT,   /* Object 30 POWWINKXZ */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0002, 0x0804, 0x0005, 0x0801,
  POWWINKXY,       -1,       -1, G_BOXTEXT ,   /* Object 31 POWPROPWINKXY */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0000, 0x0006, 0x0002, 0x0801,
  POWPROPWINKYZ,       -1,       -1, G_FBOXTEXT,   /* Object 32 POWWINKXY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0002, 0x0006, 0x0005, 0x0801,
  POWWINKYZ,       -1,       -1, G_BOXTEXT ,   /* Object 33 POWPROPWINKYZ */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0000, 0x0807, 0x0002, 0x0801,
        35,       -1,       -1, G_FBOXTEXT,   /* Object 34 POWWINKYZ */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0002, 0x0807, 0x0005, 0x0801,
  POWZOOM ,       -1,       -1, G_BOXTEXT ,   /* Object 35  */
  NONE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0000, 0x0009, 0x0002, 0x0801,
  POWSPERREAUSAN,       -1,       -1, G_FBOXTEXT,   /* Object 36 POWZOOM */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0002, 0x0009, 0x0005, 0x0801,
  POWSPERREAUSSEN,       -1,       -1, G_BOXTEXT ,   /* Object 37 POWSPERREAUSAN */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0000, 0x080A, 0x0002, 0x0801,
  POWSPERREINAN,       -1,       -1, G_FBOXTEXT,   /* Object 38 POWSPERREAUSSEN */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0002, 0x080A, 0x0005, 0x0801,
  POWSPERREINNEN,       -1,       -1, G_BOXTEXT ,   /* Object 39 POWSPERREINAN */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[25],
  0x0000, 0x000C, 0x0002, 0x0801,
        22,       -1,       -1, G_FBOXTEXT,   /* Object 40 POWSPERREINNEN */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0002, 0x000C, 0x0005, 0x0801,
        42,       -1,       -1, G_BOXTEXT ,   /* Object 41  */
  NONE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0418, 0x0001, 0x0014, 0x0002,
  POWSTANDARD, POWSETDEC,       47, G_BOX     ,   /* Object 42  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0418, 0x0006, 0x0014, 0x0803,
  POWSETINC,       -1,       -1, G_BUTTON  ,   /* Object 43 POWSETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  POWSETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 44 POWSETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  POWSETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 45 POWSETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
        47,       -1,       -1, G_BUTTON  ,   /* Object 46 POWSETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
        42,       48, POWSETNR, G_BOX     ,   /* Object 47  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  POWSETNR,       -1,       -1, G_STRING  ,   /* Object 48  */
  NONE, NORMAL, (LONG)"POW-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0801,
        47,       -1,       -1, G_FTEXT   ,   /* Object 49 POWSETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0008, 0x0000, 0x0006, 0x0801,
        51,       -1,       -1, G_BUTTON  ,   /* Object 50 POWSTANDARD */
  SELECTABLE, NORMAL, (LONG)"~Standard",
  0x0423, 0x000A, 0x0009, 0x0801,
         0, POWPORTDEC,       54, G_BOX     ,   /* Object 51  */
  NONE, NORMAL, (LONG)0x00FE1101L,
  0x0418, 0x0004, 0x0014, 0x0001,
  POWPORTINC,       -1,       -1, G_BUTTON  ,   /* Object 52 POWPORTDEC */
  SELECTABLE, NORMAL, (LONG)"-",
  0x0000, 0x0000, 0x0003, 0x0001,
        54,       -1,       -1, G_BUTTON  ,   /* Object 53 POWPORTINC */
  SELECTABLE, NORMAL, (LONG)"+",
  0x0011, 0x0000, 0x0003, 0x0001,
        51,       55, POWPORTNR, G_BOX     ,   /* Object 54  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0001,
  POWPORTNR,       -1,       -1, G_STRING  ,   /* Object 55  */
  SELECTABLE, NORMAL, (LONG)"Port-Nr:",
  0x0400, 0x0000, 0x0008, 0x0001,
        54,       -1,       -1, G_FTEXT   ,   /* Object 56 POWPORTNR */
  EDITABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[29],
  0x000A, 0x0000, 0x0002, 0x0001,
  
  /******** Tree 1 POW_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der POW-Einstellungs-Box",
  0x0001, 0x0001, 0x002D, 0x0001,
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
  
  /******** Tree 2 POW_DESK ****************************************************/
        -1, POWICON , POWICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 POWICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0002, 0x0000, 0x0009, 0x0802,
  
  /******** Tree 3 POW_TEXT ****************************************************/
        -1, FPOWN   , FPOWI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FPOWI   ,       -1,       -1, G_STRING  ,   /* Object 1 FPOWN */
  NONE, NORMAL, (LONG)" POW-Setups ",
  0x0002, 0x0800, 0x000C, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FPOWI */
  LASTOB, NORMAL, (LONG)" %d Presets",
  0x0002, 0x0801, 0x000B, 0x0001,
  
  /******** Tree 4 POW_INFO ****************************************************/
        -1,        1, POWIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0004, 0x0802, 0x001A, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0006, 0x0004, 0x0017, 0x0001,
  POWIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0008, 0x0005, 0x0612, 0x0600,
  POWIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 POWIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0007, 0x0006, 0x0014, 0x0001,
  POWCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 POWIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0009, 0x0007, 0x0012, 0x0001,
  POWISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 POWCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[36],
  0x0009, 0x0008, 0x0012, 0x0800,
  POWIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 POWISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0008, 0x0809, 0x0013, 0x0001,
  POWIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 POWIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 POWIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 POW_SETUP */
  &rs_object[57],   /* Tree  1 POW_HELP */
  &rs_object[71],   /* Tree  2 POW_DESK */
  &rs_object[73],   /* Tree  3 POW_TEXT */
  &rs_object[76]    /* Tree  4 POW_INFO */
};
