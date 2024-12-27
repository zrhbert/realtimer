/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "SYN_MOD.H"

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
{ "MTC-In",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 7, 1,
  "Input",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x0170, 0, 0, 6, 1,
  "123",
  "___",
  "XXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 4,
  "Start",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1170, 0, 0, 6, 1,
  "Stop",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1170, 0, 0, 5, 1,
  "Offset",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1170, 0, 0, 7, 1,
  "Frames",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 7, 1,
  "24",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 3, 1,
  "25",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 3, 1,
  "30",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 3, 1,
  "30df",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 5, 1,
  "Alle Inp.",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, 2, 10, 1,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "000",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 4, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "000",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 4, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "00",
  "__:",
  "99",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 3, 4,
  "000",
  "___",
  "999",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 4, 4,
  "MTC Out",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 8, 1,
  "Frames",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 7, 1,
  "24",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 3, 1,
  "25",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 3, 1,
  "30",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 3, 1,
  "30df",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 5, 1,
  "Ports",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 6, 1,
  "1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "3",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "4",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 2, 1,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "SYN System-Synchronisation",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 27, 1,
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
{ 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
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
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

static RSIB0DATA[] =
{ 0xFFFF, 0xFFFF, 0x8000, 0x0001, 
  0x803F, 0xE001, 0x80E0, 0x3801, 
  0x8182, 0x0C01, 0x8302, 0x0601, 
  0x8602, 0x0301, 0x8C02, 0x0181, 
  0x8802, 0x0081, 0x8802, 0x0081, 
  0x8802, 0x00E1, 0x8B03, 0x06E1, 
  0x8801, 0x80E1, 0x8800, 0xC081, 
  0x8800, 0x6081, 0x8C00, 0x0181, 
  0x8600, 0x0301, 0x8302, 0x0601, 
  0x8182, 0x0C01, 0x80E0, 0x3801, 
  0x803F, 0xE001, 0x8000, 0x0001, 
  0x8000, 0x0001, 0x8FFF, 0xFF81, 
  0x8924, 0x9281, 0x8924, 0x9281, 
  0x8924, 0x9281, 0x8FFF, 0xFF81, 
  0x8000, 0x0001, 0xFFFF, 0xFFFF
};

static RSIB1MASK[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x1FF0, 0x0FF8, 0x3FF8, 0x1FFC, 
  0x7FFC, 0x3FFE, 0xFFFE, 0x7FFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0x0000, 0x0000
};

static RSIB1DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0FE0, 0x07F0, 
  0x1830, 0x0C18, 0x3458, 0x1A2C, 
  0x6C6C, 0x3636, 0x5EF7, 0xEF7A, 
  0x46C7, 0xE362, 0x4104, 0x2082, 
  0x46C4, 0x2362, 0x5EF4, 0x2F7A, 
  0x6C6C, 0x3636, 0x3458, 0x1A2C, 
  0x5830, 0x0C1A, 0x6FE0, 0x07F6, 
  0x6301, 0x80C6, 0x60C7, 0xE306, 
  0x6034, 0x2C06, 0x600F, 0xF006, 
  0x6000, 0x0006, 0x6007, 0xE006, 
  0x7FFF, 0xFFFE, 0x7FFF, 0xFFFE, 
  0x0000, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "Synchron", 0x1000|'\0',
    0,   0,  19,   0,  32,  30,   0,  32,  72,   8,
  RSIB1MASK, RSIB1DATA, "SYN Synchron", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 SYN_SETUP ****************************************************/
        -1,        1,        7, G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021101L,
  0x0000, 0x0000, 0x0422, 0x0014,
  SYNHELP ,        2, SYNSMPTESTOP, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0401, 0x0001, 0x041F, 0x0806,
         3,       -1,       -1, G_BOXTEXT ,   /* Object 2  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[0],
  0x0001, 0x0000, 0x0406, 0x0001,
  SYNSMPTEINDEV,       -1,       -1, G_BOXTEXT ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0000, 0x0001, 0x0406, 0x0001,
         5,       -1,       -1, G_FBOXTEXT,   /* Object 4 SYNSMPTEINDEV */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0406, 0x0001, 0x0005, 0x0001,
         6,       -1,       -1, G_BOXTEXT ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0000, 0x0802, 0x0406, 0x0001,
         8,       -1,       -1, G_BOXTEXT ,   /* Object 6  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0000, 0x0004, 0x0406, 0x0001,
         0,       -1,       -1, G_BOXTEXT ,   /* Object 7  */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0407, 0x0010, 0x0406, 0x0001,
  SYNSYNALLPORTS,        9,       17, G_BOX     ,   /* Object 8  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0015, 0x0800, 0x0409, 0x0805,
  SYNFRAMESI24,       -1,       -1, G_BOXTEXT ,   /* Object 9  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[6],
  0x0001, 0x0000, 0x0006, 0x0001,
  SYNFRAMEST24,       -1,       -1, G_BOX     ,   /* Object 10 SYNFRAMESI24 */
  RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0001, 0x0002, 0x0001,
  SYNFRAMESI25,       -1,       -1, G_BOXTEXT ,   /* Object 11 SYNFRAMEST24 */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0004, 0x0001, 0x0005, 0x0001,
        13,       -1,       -1, G_BOX     ,   /* Object 12 SYNFRAMESI25 */
  RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0002, 0x0002, 0x0001,
  SYNFRAMESI30,       -1,       -1, G_BOXTEXT ,   /* Object 13  */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0004, 0x0002, 0x0005, 0x0001,
        15,       -1,       -1, G_BOX     ,   /* Object 14 SYNFRAMESI30 */
  RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0003, 0x0002, 0x0001,
  SYNFRAMESI30D,       -1,       -1, G_BOXTEXT ,   /* Object 15  */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0004, 0x0003, 0x0005, 0x0001,
        17,       -1,       -1, G_BOX     ,   /* Object 16 SYNFRAMESI30D */
  RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0004, 0x0002, 0x0001,
         8,       -1,       -1, G_BOXTEXT ,   /* Object 17  */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0004, 0x0004, 0x0005, 0x0001,
  SYNSMPTESTART,       -1,       -1, G_BOXTEXT ,   /* Object 18 SYNSYNALLPORTS */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[11],
  0x000C, 0x0001, 0x0407, 0x0001,
  SYNSMPTESTOP,       20,       23, G_BOX     ,   /* Object 19 SYNSMPTESTART */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0007, 0x0802, 0x000C, 0x0001,
        21,       -1,       -1, G_FBOXTEXT,   /* Object 20  */
  NONE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0000, 0x0000, 0x0003, 0x0001,
        22,       -1,       -1, G_FBOXTEXT,   /* Object 21  */
  NONE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0003, 0x0000, 0x0003, 0x0001,
        23,       -1,       -1, G_FBOXTEXT,   /* Object 22  */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0006, 0x0000, 0x0003, 0x0001,
  SYNSMPTESTART,       -1,       -1, G_FBOXTEXT,   /* Object 23  */
  NONE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0009, 0x0000, 0x0003, 0x0001,
         1,       25,       28, G_BOX     ,   /* Object 24 SYNSMPTESTOP */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0007, 0x0004, 0x000C, 0x0001,
        26,       -1,       -1, G_FBOXTEXT,   /* Object 25  */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0000, 0x0000, 0x0003, 0x0001,
        27,       -1,       -1, G_FBOXTEXT,   /* Object 26  */
  NONE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0003, 0x0000, 0x0003, 0x0001,
        28,       -1,       -1, G_FBOXTEXT,   /* Object 27  */
  NONE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0006, 0x0000, 0x0003, 0x0001,
  SYNSMPTESTOP,       -1,       -1, G_FBOXTEXT,   /* Object 28  */
  NONE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0009, 0x0000, 0x0003, 0x0001,
         7,       30,       33, G_BOX     ,   /* Object 29 SYNSMPTEOFFSET */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x040E, 0x0010, 0x000C, 0x0001,
        31,       -1,       -1, G_FBOXTEXT,   /* Object 30  */
  NONE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0000, 0x0000, 0x0003, 0x0001,
        32,       -1,       -1, G_FBOXTEXT,   /* Object 31  */
  NONE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0003, 0x0000, 0x0003, 0x0001,
        33,       -1,       -1, G_FBOXTEXT,   /* Object 32  */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0006, 0x0000, 0x0003, 0x0001,
  SYNSMPTEOFFSET,       -1,       -1, G_FBOXTEXT,   /* Object 33  */
  NONE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0009, 0x0000, 0x0003, 0x0001,
  SYNCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 34 SYNHELP */
  SELECTABLE, WHITEBAK, (LONG)"~Hilfe",
  0x0402, 0x0012, 0x0406, 0x0801,
  SYNOK   ,       -1,       -1, G_BUTTON  ,   /* Object 35 SYNCANCEL */
  SELECTABLE, WHITEBAK, (LONG)"~Abbr.",
  0x0012, 0x0012, 0x0007, 0x0801,
  SYNSTANDARD,       -1,       -1, G_BUTTON  ,   /* Object 36 SYNOK */
  SELECTABLE|DEFAULT, WHITEBAK, (LONG)"~OK",
  0x041A, 0x0012, 0x0007, 0x0801,
        38,       -1,       -1, G_BUTTON  ,   /* Object 37 SYNSTANDARD */
  SELECTABLE, WHITEBAK, (LONG)"~Stand.",
  0x000A, 0x0012, 0x0007, 0x0801,
  SYNSMPTEOFFSET,       39,       50, G_BOX     ,   /* Object 38  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0401, 0x0808, 0x041F, 0x0007,
        40,       -1,       -1, G_BOXTEXT ,   /* Object 39  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[24],
  0x0001, 0x0000, 0x0407, 0x0001,
        50,       41,       49, G_BOX     ,   /* Object 40  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0001, 0x0409, 0x0805,
  SYNFRAMESO24,       -1,       -1, G_BOXTEXT ,   /* Object 41  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[25],
  0x0001, 0x0000, 0x0006, 0x0001,
        43,       -1,       -1, G_BOX     ,   /* Object 42 SYNFRAMESO24 */
  SELECTABLE|RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0001, 0x0002, 0x0001,
  SYNFRAMESO25,       -1,       -1, G_BOXTEXT ,   /* Object 43  */
  NONE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0004, 0x0001, 0x0005, 0x0001,
        45,       -1,       -1, G_BOX     ,   /* Object 44 SYNFRAMESO25 */
  SELECTABLE|RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0002, 0x0002, 0x0001,
  SYNFRAMESO30,       -1,       -1, G_BOXTEXT ,   /* Object 45  */
  NONE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0004, 0x0002, 0x0005, 0x0001,
        47,       -1,       -1, G_BOX     ,   /* Object 46 SYNFRAMESO30 */
  SELECTABLE|RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0003, 0x0002, 0x0001,
  SYNFRAMESO30D,       -1,       -1, G_BOXTEXT ,   /* Object 47  */
  NONE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0004, 0x0003, 0x0005, 0x0001,
        49,       -1,       -1, G_BOX     ,   /* Object 48 SYNFRAMESO30D */
  SELECTABLE|RBUTTON, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0004, 0x0002, 0x0001,
        40,       -1,       -1, G_BOXTEXT ,   /* Object 49  */
  NONE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0004, 0x0004, 0x0005, 0x0001,
        38,       51,       59, G_BOX     ,   /* Object 50  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0015, 0x0001, 0x0409, 0x0805,
  SYNPORTSO1,       -1,       -1, G_BOXTEXT ,   /* Object 51  */
  NONE, DRAW3D, (LONG)&rs_tedinfo[30],
  0x0001, 0x0000, 0x0006, 0x0001,
        53,       -1,       -1, G_BOX     ,   /* Object 52 SYNPORTSO1 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0001, 0x0002, 0x0001,
  SYNPORTSO2,       -1,       -1, G_BOXTEXT ,   /* Object 53  */
  NONE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0004, 0x0001, 0x0005, 0x0001,
        55,       -1,       -1, G_BOX     ,   /* Object 54 SYNPORTSO2 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0002, 0x0002, 0x0001,
  SYNPORTSO3,       -1,       -1, G_BOXTEXT ,   /* Object 55  */
  NONE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0004, 0x0002, 0x0005, 0x0001,
        57,       -1,       -1, G_BOX     ,   /* Object 56 SYNPORTSO3 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0003, 0x0002, 0x0001,
  SYNPORTSO4,       -1,       -1, G_BOXTEXT ,   /* Object 57  */
  NONE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0004, 0x0003, 0x0005, 0x0001,
        59,       -1,       -1, G_BOX     ,   /* Object 58 SYNPORTSO4 */
  SELECTABLE, SELECTED|CROSSED, (LONG)0x00011101L,
  0x0001, 0x0004, 0x0002, 0x0001,
        50,       -1,       -1, G_BOXTEXT ,   /* Object 59  */
  LASTOB, NORMAL, (LONG)&rs_tedinfo[34],
  0x0004, 0x0004, 0x0005, 0x0001,
  
  /******** Tree 1 SYN_INFO ****************************************************/
        -1,        1, SYNIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[36],
  0x0004, 0x0802, 0x001A, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0006, 0x0004, 0x0017, 0x0001,
  SYNIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0008, 0x0005, 0x0612, 0x0600,
  SYNIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 SYNIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[39],
  0x0007, 0x0006, 0x0014, 0x0001,
  SYNICOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 SYNIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0009, 0x0007, 0x0012, 0x0001,
  SYNISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 SYNICOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[41],
  0x0009, 0x0008, 0x0012, 0x0800,
  SYNIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 SYNISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[42],
  0x0008, 0x0809, 0x0013, 0x0001,
  SYNIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 SYNIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[43],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 SYNIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002,
  
  /******** Tree 2 SYN_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der Synchronisations-Einstellung",
  0x0001, 0x0001, 0x0035, 0x0001,
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
  
  /******** Tree 3 SYN_DESK ****************************************************/
        -1, SYNICON_ALT, SYNICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
  SYNICON ,       -1,       -1, G_ICON    ,   /* Object 1 SYNICON_ALT */
  NONE, NORMAL, (LONG)&rs_iconblk[0],
  0x0003, 0x0005, 0x0009, 0x0802,
         0,       -1,       -1, G_ICON    ,   /* Object 2 SYNICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[1],
  0x0003, 0x0001, 0x0009, 0x0802,
  
  /******** Tree 4 SYN_TEXT ****************************************************/
        -1, FSYNN   , FSYNH   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FSYNI   ,       -1,       -1, G_STRING  ,   /* Object 1 FSYNN */
  NONE, NORMAL, (LONG)" SYN Synchronisation ",
  0x0002, 0x0001, 0x0015, 0x0001,
  FSYNH   ,       -1,       -1, G_STRING  ,   /* Object 2 FSYNI */
  NONE, NORMAL, (LONG)" %d Presets",
  0x0002, 0x0002, 0x000B, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 3 FSYNH */
  LASTOB, NORMAL, (LONG)" SYN Synchron. Hilfe ",
  0x0002, 0x0803, 0x0015, 0x0001
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 SYN_SETUP */
  &rs_object[60],   /* Tree  1 SYN_INFO */
  &rs_object[71],   /* Tree  2 SYN_HELP */
  &rs_object[85],   /* Tree  3 SYN_DESK */
  &rs_object[88]    /* Tree  4 SYN_TEXT */
};
