/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "MAE_MOD.H"

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
  "O-U-Speed",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 1, 10, 1,
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
  "Extern-Maus-Setup",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 18, 1,
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
  "MAE Extern-Maus-Ansteuerung",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 28, 1,
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
  0x0000, 0x0000, 0xFF83, 0xFF80, 
  0xFF8F, 0xFF80, 0xFF8F, 0xFF80, 
  0xFE0F, 0x8000, 0xFE0E, 0x0000, 
  0xEF8F, 0x8000, 0xEF8F, 0xC000, 
  0xEF8F, 0xE000, 0x0003, 0xE000, 
  0x0001, 0xE000, 0x001F, 0xF800, 
  0x001F, 0xFC00, 0x003F, 0xFE00, 
  0x003F, 0xFE00, 0x007F, 0xFE00, 
  0x007F, 0xFE00, 0x00FF, 0xFE00, 
  0x00FF, 0xFC00, 0x00FF, 0xF800, 
  0x00FF, 0xF000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x7701, 0xFF00, 0x5407, 0x0000, 
  0x7404, 0x0000, 0x4404, 0x0000, 
  0x4404, 0x0000, 0x4707, 0x0000, 
  0x0001, 0x8000, 0x0000, 0xC000, 
  0x0000, 0x4000, 0x0000, 0x4000, 
  0x000F, 0xF000, 0x000A, 0x9800, 
  0x001F, 0xE400, 0x0010, 0x2400, 
  0x0020, 0x4400, 0x0020, 0x4800, 
  0x007F, 0x9000, 0x007F, 0xE000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "MAE Ext Maus", 0x1000|'\0',
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
  /******** Tree 0 MAE_SETUP ****************************************************/
        -1, MAEOK   ,       54, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021111L,
  0x0000, 0x0000, 0x042D, 0x0010,
  MAECANCEL,       -1,       -1, G_BUTTON  ,   /* Object 1 MAEOK */
  SELECTABLE|DEFAULT, NORMAL, (LONG)"~OK",
  0x0423, 0x080D, 0x0009, 0x0002,
  MAEHELP ,       -1,       -1, G_BUTTON  ,   /* Object 2 MAECANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0418, 0x080D, 0x0009, 0x0002,
         4,       -1,       -1, G_BUTTON  ,   /* Object 3 MAEHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0418, 0x080A, 0x0009, 0x0002,
        23,        5,       22, G_BOX     ,   /* Object 4  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x0001, 0x0800, 0x000F, 0x000F,
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
        22,       -1,       -1, G_BOXTEXT ,   /* Object 21  */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0000, 0x000C, 0x000F, 0x0801,
         4,       -1,       -1, G_BOXTEXT ,   /* Object 22  */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0000, 0x080D, 0x000F, 0x0801,
        44, MAEPROPROTX, MAESPERREINNEN, G_BOX     ,   /* Object 23  */
  NONE, NORMAL, (LONG)0x00FE1100L,
  0x0010, 0x0800, 0x0007, 0x000F,
  MAEROTX ,       -1,       -1, G_BOXTEXT ,   /* Object 24 MAEPROPROTX */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0000, 0x0000, 0x0002, 0x0801,
  MAEPROPROTY,       -1,       -1, G_FBOXTEXT,   /* Object 25 MAEROTX */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0002, 0x0000, 0x0005, 0x0801,
  MAEROTY ,       -1,       -1, G_BOXTEXT ,   /* Object 26 MAEPROPROTY */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0000, 0x0801, 0x0002, 0x0801,
  MAEPROPROTZ,       -1,       -1, G_FBOXTEXT,   /* Object 27 MAEROTY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0002, 0x0801, 0x0005, 0x0801,
  MAEROTZ ,       -1,       -1, G_BOXTEXT ,   /* Object 28 MAEPROPROTZ */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0000, 0x0003, 0x0002, 0x0801,
  MAEPROPWINKXZ,       -1,       -1, G_FBOXTEXT,   /* Object 29 MAEROTZ */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0002, 0x0003, 0x0005, 0x0801,
  MAEWINKXZ,       -1,       -1, G_BOXTEXT ,   /* Object 30 MAEPROPWINKXZ */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0000, 0x0804, 0x0002, 0x0801,
  MAEPROPWINKXY,       -1,       -1, G_FBOXTEXT,   /* Object 31 MAEWINKXZ */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0002, 0x0804, 0x0005, 0x0801,
  MAEWINKXY,       -1,       -1, G_BOXTEXT ,   /* Object 32 MAEPROPWINKXY */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0000, 0x0006, 0x0002, 0x0801,
  MAEPROPWINKYZ,       -1,       -1, G_FBOXTEXT,   /* Object 33 MAEWINKXY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0002, 0x0006, 0x0005, 0x0801,
  MAEWINKYZ,       -1,       -1, G_BOXTEXT ,   /* Object 34 MAEPROPWINKYZ */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0000, 0x0807, 0x0002, 0x0801,
        36,       -1,       -1, G_FBOXTEXT,   /* Object 35 MAEWINKYZ */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0002, 0x0807, 0x0005, 0x0801,
  MAEZOOM ,       -1,       -1, G_BOXTEXT ,   /* Object 36  */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0000, 0x0009, 0x0002, 0x0801,
        38,       -1,       -1, G_FBOXTEXT,   /* Object 37 MAEZOOM */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0002, 0x0009, 0x0005, 0x0801,
  MAESPEEDOU,       -1,       -1, G_BOXTEXT ,   /* Object 38  */
  NONE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0000, 0x080A, 0x0002, 0x0801,
  MAESPERREAUSAN,       -1,       -1, G_FBOXTEXT,   /* Object 39 MAESPEEDOU */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[25],
  0x0002, 0x080A, 0x0005, 0x0801,
  MAESPERREAUSSEN,       -1,       -1, G_BOXTEXT ,   /* Object 40 MAESPERREAUSAN */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0000, 0x000C, 0x0002, 0x0801,
  MAESPERREINAN,       -1,       -1, G_FBOXTEXT,   /* Object 41 MAESPERREAUSSEN */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0002, 0x000C, 0x0005, 0x0801,
  MAESPERREINNEN,       -1,       -1, G_BOXTEXT ,   /* Object 42 MAESPERREINAN */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0000, 0x080D, 0x0002, 0x0801,
        23,       -1,       -1, G_FBOXTEXT,   /* Object 43 MAESPERREINNEN */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0002, 0x080D, 0x0005, 0x0801,
        45,       -1,       -1, G_BOXTEXT ,   /* Object 44  */
  NONE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0418, 0x0001, 0x0014, 0x0002,
  MAESTANDARD, MAESETDEC,       50, G_BOX     ,   /* Object 45  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0418, 0x0006, 0x0014, 0x0803,
  MAESETINC,       -1,       -1, G_BUTTON  ,   /* Object 46 MAESETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  MAESETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 47 MAESETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  MAESETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 48 MAESETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
        50,       -1,       -1, G_BUTTON  ,   /* Object 49 MAESETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
        45,       51, MAESETNR, G_BOX     ,   /* Object 50  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  MAESETNR,       -1,       -1, G_STRING  ,   /* Object 51  */
  NONE, NORMAL, (LONG)"MAE-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0801,
        50,       -1,       -1, G_FTEXT   ,   /* Object 52 MAESETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0008, 0x0000, 0x0006, 0x0801,
        54,       -1,       -1, G_BUTTON  ,   /* Object 53 MAESTANDARD */
  SELECTABLE, NORMAL, (LONG)"~Standard",
  0x0423, 0x080A, 0x0009, 0x0002,
         0, MAEPORTDEC,       57, G_BOX     ,   /* Object 54  */
  NONE, NORMAL, (LONG)0x00FE1101L,
  0x0418, 0x0004, 0x0014, 0x0001,
  MAEPORTINC,       -1,       -1, G_BUTTON  ,   /* Object 55 MAEPORTDEC */
  SELECTABLE, NORMAL, (LONG)"-",
  0x0000, 0x0000, 0x0003, 0x0001,
        57,       -1,       -1, G_BUTTON  ,   /* Object 56 MAEPORTINC */
  SELECTABLE, NORMAL, (LONG)"+",
  0x0011, 0x0000, 0x0003, 0x0001,
        54,       58, MAEPORTNR, G_BOX     ,   /* Object 57  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0001,
  MAEPORTNR,       -1,       -1, G_STRING  ,   /* Object 58  */
  SELECTABLE, NORMAL, (LONG)"Port-Nr:",
  0x0400, 0x0000, 0x0008, 0x0001,
        57,       -1,       -1, G_FTEXT   ,   /* Object 59 MAEPORTNR */
  EDITABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[32],
  0x000A, 0x0000, 0x0002, 0x0001,
  
  /******** Tree 1 MAE_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der MAE-Einstellungs-Box",
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
  
  /******** Tree 2 MAE_DESK ****************************************************/
        -1, MAEICON , MAEICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 MAEICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0001, 0x0800, 0x0009, 0x0802,
  
  /******** Tree 3 MAE_TEXT ****************************************************/
        -1, FMAEN   , FMAEI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FMAEI   ,       -1,       -1, G_STRING  ,   /* Object 1 FMAEN */
  NONE, NORMAL, (LONG)" MAE-Setups ",
  0x0002, 0x0800, 0x000C, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FMAEI */
  LASTOB, NORMAL, (LONG)" %d Presets",
  0x0002, 0x0801, 0x000B, 0x0001,
  
  /******** Tree 4 MAE_INFO ****************************************************/
        -1,        1, MAEIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0004, 0x0802, 0x001B, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0006, 0x0004, 0x0017, 0x0001,
  MAEIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[36],
  0x0008, 0x0005, 0x0612, 0x0600,
  MAEIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 MAEIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0007, 0x0006, 0x0014, 0x0001,
  MAECOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 MAEIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0009, 0x0007, 0x0012, 0x0001,
  MAEISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 MAECOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[39],
  0x0009, 0x0008, 0x0012, 0x0800,
  MAEIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 MAEISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0008, 0x0809, 0x0013, 0x0001,
  MAEIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 MAEIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[41],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 MAEIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 MAE_SETUP */
  &rs_object[60],   /* Tree  1 MAE_HELP */
  &rs_object[74],   /* Tree  2 MAE_DESK */
  &rs_object[76],   /* Tree  3 MAE_TEXT */
  &rs_object[79]    /* Tree  4 MAE_INFO */
};
