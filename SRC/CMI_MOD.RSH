/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "CMI_MOD.H"

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
{ "Signal",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 7, 1,
  "1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "3",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "4",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "5",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "6",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "7",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "8",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -2, 2, 1,
  "1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 2, 1,
  "2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 2, 1,
  "3",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 2, 1,
  "4",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 2, 1,
  "5",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 2, 1,
  "6",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 2, 1,
  "7",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 2, 1,
  "8",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 2, 1,
  "9",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 2, 1,
  "10",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "11",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "12",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "13",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "14",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "15",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "16",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "17",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "18",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "19",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "20",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "21",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "22",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "23",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "24",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "25",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "26",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "27",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "28",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "29",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "30",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "31",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "32",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "33",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "34",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "35",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "36",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "37",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "38",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "39",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "40",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "41",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "42",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "43",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "44",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "45",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "46",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "47",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "48",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "49",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "50",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "51",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "52",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "53",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "54",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "55",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "56",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "57",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "58",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "59",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "60",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "61",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "62",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "63",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "64",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 3, 1,
  "CM 1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 5, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "CM 2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 5, 1,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "__",
  "__",
  "99",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 3,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "CMI Cybermove-Input-Zuweisung",
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
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x03FF, 0xFFFC, 
  0x0000, 0x03FF, 0xFFFC, 0x7C00, 
  0x03FF, 0xFFFC, 0x7FC0, 0x03FF, 
  0xFFFC, 0x7FFE, 0x03FF, 0xFFFC, 
  0x7FFF, 0xE3FF, 0xFFFC, 0x7CFF, 
  0xFFFF, 0xFFFC, 0x7C07, 0xFFFF, 
  0xFFFC, 0x7FFF, 0xFFFF, 0xFFFC, 
  0x7FFF, 0xFFFF, 0xFFFC, 0x7FFF, 
  0xFFFF, 0xFFFC, 0x7C7F, 0xFFFF, 
  0xFFFC, 0x7FFF, 0xBFFF, 0xFFFC, 
  0x7FFD, 0xFFFF, 0xFFFC, 0x7FE7, 
  0xFFFF, 0xFFFC, 0x7F1F, 0xF3FF, 
  0xFFFC, 0x7CFF, 0x83FF, 0xFFFC, 
  0x7FFE, 0x03FF, 0xFFFC, 0x7FF8, 
  0x03FF, 0xFFFC, 0x7FC0, 0x03FF, 
  0xFFFC, 0x7F00, 0x03FF, 0xFFFC, 
  0x7C00, 0x03FF, 0xFFFC, 0x0000, 
  0x03FF, 0xFFFC, 0x0000, 0x03FF, 
  0xFFFC, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x01FF, 0xFFF8, 0x0000, 
  0x0100, 0x0008, 0x3800, 0x0173, 
  0x9CE8, 0x3F80, 0x0173, 0xBCE8, 
  0x387C, 0x0173, 0xDFE8, 0x0003, 
  0xC10E, 0x1C08, 0x0000, 0x3F70, 
  0x6008, 0x3800, 0x03F3, 0x9CE8, 
  0x3FFF, 0xFFFF, 0x9CE8, 0x3800, 
  0xE173, 0x9CE8, 0x0007, 0x01C0, 
  0x0008, 0x0038, 0x0700, 0x0008, 
  0x39C0, 0x1973, 0x9CE8, 0x3E00, 
  0xE173, 0x9CE8, 0x3803, 0x0173, 
  0x9CE8, 0x000C, 0x0100, 0x0008, 
  0x0070, 0x0100, 0x0008, 0x3980, 
  0x0173, 0x9CE8, 0x3E00, 0x0173, 
  0x9CE8, 0x3800, 0x0173, 0x9CE8, 
  0x0000, 0x0100, 0x0008, 0x0000, 
  0x01FF, 0xFFF8, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "CMI-Setups", 0x1000|'\0',
    0,   0,  19,   0,  48,  30,   0,  32,  75,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 CMI_SETUP ****************************************************/
        -1,        1, CMIOK   , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00FE1141L,
  0x0000, 0x0000, 0x002F, 0x080E,
        11,        2, CMISIGNAL8, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0800, 0x002D, 0x0001,
  CMISIGNAL1,       -1,       -1, G_BOXTEXT ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0000, 0x0000, 0x000D, 0x0001,
  CMISIGNAL2,       -1,       -1, G_BOXTEXT ,   /* Object 3 CMISIGNAL1 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[1],
  0x000D, 0x0000, 0x0004, 0x0001,
  CMISIGNAL3,       -1,       -1, G_BOXTEXT ,   /* Object 4 CMISIGNAL2 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[2],
  0x0011, 0x0000, 0x0004, 0x0001,
  CMISIGNAL4,       -1,       -1, G_BOXTEXT ,   /* Object 5 CMISIGNAL3 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[3],
  0x0015, 0x0000, 0x0004, 0x0001,
  CMISIGNAL5,       -1,       -1, G_BOXTEXT ,   /* Object 6 CMISIGNAL4 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[4],
  0x0019, 0x0000, 0x0004, 0x0001,
  CMISIGNAL6,       -1,       -1, G_BOXTEXT ,   /* Object 7 CMISIGNAL5 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[5],
  0x001D, 0x0000, 0x0004, 0x0001,
  CMISIGNAL7,       -1,       -1, G_BOXTEXT ,   /* Object 8 CMISIGNAL6 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[6],
  0x0021, 0x0000, 0x0004, 0x0001,
  CMISIGNAL8,       -1,       -1, G_BOXTEXT ,   /* Object 9 CMISIGNAL7 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[7],
  0x0025, 0x0000, 0x0004, 0x0001,
         1,       -1,       -1, G_BOXTEXT ,   /* Object 10 CMISIGNAL8 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[8],
  0x0029, 0x0000, 0x0004, 0x0001,
       101,       12,       89, G_BOX     ,   /* Object 11  */
  NONE, NORMAL, (LONG)0x00FE1101L,
  0x0001, 0x0002, 0x002D, 0x0008,
        77, CMICM1INP1,       76, G_BOX     ,   /* Object 12  */
  NONE, NORMAL, (LONG)0x00FE1101L,
  0x000D, 0x0000, 0x0020, 0x0008,
        14,       -1,       -1, G_BOXTEXT ,   /* Object 13 CMICM1INP1 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[9],
  0x0000, 0x0000, 0x0004, 0x0001,
        15,       -1,       -1, G_BOXTEXT ,   /* Object 14  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[10],
  0x0004, 0x0000, 0x0004, 0x0001,
        16,       -1,       -1, G_BOXTEXT ,   /* Object 15  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[11],
  0x0008, 0x0000, 0x0004, 0x0001,
        17,       -1,       -1, G_BOXTEXT ,   /* Object 16  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[12],
  0x000C, 0x0000, 0x0004, 0x0001,
        18,       -1,       -1, G_BOXTEXT ,   /* Object 17  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[13],
  0x0010, 0x0000, 0x0004, 0x0001,
        19,       -1,       -1, G_BOXTEXT ,   /* Object 18  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[14],
  0x0014, 0x0000, 0x0004, 0x0001,
        20,       -1,       -1, G_BOXTEXT ,   /* Object 19  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[15],
  0x0018, 0x0000, 0x0004, 0x0001,
        21,       -1,       -1, G_BOXTEXT ,   /* Object 20  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[16],
  0x001C, 0x0000, 0x0004, 0x0001,
        22,       -1,       -1, G_BOXTEXT ,   /* Object 21  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[17],
  0x0000, 0x0001, 0x0004, 0x0001,
        23,       -1,       -1, G_BOXTEXT ,   /* Object 22  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[18],
  0x0004, 0x0001, 0x0004, 0x0001,
        24,       -1,       -1, G_BOXTEXT ,   /* Object 23  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[19],
  0x0008, 0x0001, 0x0004, 0x0001,
        25,       -1,       -1, G_BOXTEXT ,   /* Object 24  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[20],
  0x000C, 0x0001, 0x0004, 0x0001,
        26,       -1,       -1, G_BOXTEXT ,   /* Object 25  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[21],
  0x0010, 0x0001, 0x0004, 0x0001,
        27,       -1,       -1, G_BOXTEXT ,   /* Object 26  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[22],
  0x0014, 0x0001, 0x0004, 0x0001,
        28,       -1,       -1, G_BOXTEXT ,   /* Object 27  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[23],
  0x0018, 0x0001, 0x0004, 0x0001,
        29,       -1,       -1, G_BOXTEXT ,   /* Object 28  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[24],
  0x001C, 0x0001, 0x0004, 0x0001,
        30,       -1,       -1, G_BOXTEXT ,   /* Object 29  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[25],
  0x0000, 0x0002, 0x0004, 0x0001,
        31,       -1,       -1, G_BOXTEXT ,   /* Object 30  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[26],
  0x0004, 0x0002, 0x0004, 0x0001,
        32,       -1,       -1, G_BOXTEXT ,   /* Object 31  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[27],
  0x0008, 0x0002, 0x0004, 0x0001,
        33,       -1,       -1, G_BOXTEXT ,   /* Object 32  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[28],
  0x000C, 0x0002, 0x0004, 0x0001,
        34,       -1,       -1, G_BOXTEXT ,   /* Object 33  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[29],
  0x0010, 0x0002, 0x0004, 0x0001,
        35,       -1,       -1, G_BOXTEXT ,   /* Object 34  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[30],
  0x0014, 0x0002, 0x0004, 0x0001,
        36,       -1,       -1, G_BOXTEXT ,   /* Object 35  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[31],
  0x0018, 0x0002, 0x0004, 0x0001,
        37,       -1,       -1, G_BOXTEXT ,   /* Object 36  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[32],
  0x001C, 0x0002, 0x0004, 0x0001,
        38,       -1,       -1, G_BOXTEXT ,   /* Object 37  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[33],
  0x0000, 0x0003, 0x0004, 0x0001,
        39,       -1,       -1, G_BOXTEXT ,   /* Object 38  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[34],
  0x0004, 0x0003, 0x0004, 0x0001,
        40,       -1,       -1, G_BOXTEXT ,   /* Object 39  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[35],
  0x0008, 0x0003, 0x0004, 0x0001,
        41,       -1,       -1, G_BOXTEXT ,   /* Object 40  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[36],
  0x000C, 0x0003, 0x0004, 0x0001,
        42,       -1,       -1, G_BOXTEXT ,   /* Object 41  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[37],
  0x0010, 0x0003, 0x0004, 0x0001,
        43,       -1,       -1, G_BOXTEXT ,   /* Object 42  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[38],
  0x0014, 0x0003, 0x0004, 0x0001,
        44,       -1,       -1, G_BOXTEXT ,   /* Object 43  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[39],
  0x0018, 0x0003, 0x0004, 0x0001,
  CMICM2INP1,       -1,       -1, G_BOXTEXT ,   /* Object 44  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[40],
  0x001C, 0x0003, 0x0004, 0x0001,
        46,       -1,       -1, G_BOXTEXT ,   /* Object 45 CMICM2INP1 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[41],
  0x0000, 0x0004, 0x0004, 0x0001,
        47,       -1,       -1, G_BOXTEXT ,   /* Object 46  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[42],
  0x0004, 0x0004, 0x0004, 0x0001,
        48,       -1,       -1, G_BOXTEXT ,   /* Object 47  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[43],
  0x0008, 0x0004, 0x0004, 0x0001,
        49,       -1,       -1, G_BOXTEXT ,   /* Object 48  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[44],
  0x000C, 0x0004, 0x0004, 0x0001,
        50,       -1,       -1, G_BOXTEXT ,   /* Object 49  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[45],
  0x0010, 0x0004, 0x0004, 0x0001,
        51,       -1,       -1, G_BOXTEXT ,   /* Object 50  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[46],
  0x0014, 0x0004, 0x0004, 0x0001,
        52,       -1,       -1, G_BOXTEXT ,   /* Object 51  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[47],
  0x0018, 0x0004, 0x0004, 0x0001,
        53,       -1,       -1, G_BOXTEXT ,   /* Object 52  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[48],
  0x001C, 0x0004, 0x0004, 0x0001,
        54,       -1,       -1, G_BOXTEXT ,   /* Object 53  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[49],
  0x0000, 0x0005, 0x0004, 0x0001,
        55,       -1,       -1, G_BOXTEXT ,   /* Object 54  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[50],
  0x0004, 0x0005, 0x0004, 0x0001,
        56,       -1,       -1, G_BOXTEXT ,   /* Object 55  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[51],
  0x0008, 0x0005, 0x0004, 0x0001,
        57,       -1,       -1, G_BOXTEXT ,   /* Object 56  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[52],
  0x000C, 0x0005, 0x0004, 0x0001,
        58,       -1,       -1, G_BOXTEXT ,   /* Object 57  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[53],
  0x0010, 0x0005, 0x0004, 0x0001,
        59,       -1,       -1, G_BOXTEXT ,   /* Object 58  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[54],
  0x0014, 0x0005, 0x0004, 0x0001,
        60,       -1,       -1, G_BOXTEXT ,   /* Object 59  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[55],
  0x0018, 0x0005, 0x0004, 0x0001,
        61,       -1,       -1, G_BOXTEXT ,   /* Object 60  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[56],
  0x001C, 0x0005, 0x0004, 0x0001,
        62,       -1,       -1, G_BOXTEXT ,   /* Object 61  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[57],
  0x0000, 0x0006, 0x0004, 0x0001,
        63,       -1,       -1, G_BOXTEXT ,   /* Object 62  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[58],
  0x0004, 0x0006, 0x0004, 0x0001,
        64,       -1,       -1, G_BOXTEXT ,   /* Object 63  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[59],
  0x0008, 0x0006, 0x0004, 0x0001,
        65,       -1,       -1, G_BOXTEXT ,   /* Object 64  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[60],
  0x000C, 0x0006, 0x0004, 0x0001,
        66,       -1,       -1, G_BOXTEXT ,   /* Object 65  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[61],
  0x0010, 0x0006, 0x0004, 0x0001,
        67,       -1,       -1, G_BOXTEXT ,   /* Object 66  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[62],
  0x0014, 0x0006, 0x0004, 0x0001,
        68,       -1,       -1, G_BOXTEXT ,   /* Object 67  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[63],
  0x0018, 0x0006, 0x0004, 0x0001,
        69,       -1,       -1, G_BOXTEXT ,   /* Object 68  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[64],
  0x001C, 0x0006, 0x0004, 0x0001,
        70,       -1,       -1, G_BOXTEXT ,   /* Object 69  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[65],
  0x0000, 0x0007, 0x0004, 0x0001,
        71,       -1,       -1, G_BOXTEXT ,   /* Object 70  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[66],
  0x0004, 0x0007, 0x0004, 0x0001,
        72,       -1,       -1, G_BOXTEXT ,   /* Object 71  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[67],
  0x0008, 0x0007, 0x0004, 0x0001,
        73,       -1,       -1, G_BOXTEXT ,   /* Object 72  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[68],
  0x000C, 0x0007, 0x0004, 0x0001,
        74,       -1,       -1, G_BOXTEXT ,   /* Object 73  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[69],
  0x0010, 0x0007, 0x0004, 0x0001,
        75,       -1,       -1, G_BOXTEXT ,   /* Object 74  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[70],
  0x0014, 0x0007, 0x0004, 0x0001,
        76,       -1,       -1, G_BOXTEXT ,   /* Object 75  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[71],
  0x0018, 0x0007, 0x0004, 0x0001,
        12,       -1,       -1, G_BOXTEXT ,   /* Object 76  */
  SELECTABLE|RBUTTON, NORMAL, (LONG)&rs_tedinfo[72],
  0x001C, 0x0007, 0x0004, 0x0001,
        89,       78, CMICM1PORTINC, G_BOX     ,   /* Object 77  */
  NONE, NORMAL, (LONG)0x00FE1101L,
  0x0000, 0x0000, 0x000D, 0x0004,
        79,       -1,       -1, G_BOXTEXT ,   /* Object 78  */
  NONE, NORMAL, (LONG)&rs_tedinfo[73],
  0x0001, 0x0000, 0x000B, 0x0801,
  CMICM1CHANNELINC, CMICM1CHANNELNR, CMICM1CHANNELDEC, G_BOX     ,   /* Object 79  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0801, 0x0009, 0x0001,
        81,       -1,       -1, G_FTEXT   ,   /* Object 80 CMICM1CHANNELNR */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[74],
  0x0006, 0x0000, 0x0003, 0x0001,
  CMICM1CHANNELDEC,       -1,       -1, G_STRING  ,   /* Object 81  */
  SELECTABLE, NORMAL, (LONG)"Ch:",
  0x0402, 0x0000, 0x0003, 0x0001,
        79,       -1,       -1, G_BUTTON  ,   /* Object 82 CMICM1CHANNELDEC */
  SELECTABLE, NORMAL, (LONG)"-",
  0x0000, 0x0000, 0x0002, 0x0001,
        84,       -1,       -1, G_BUTTON  ,   /* Object 83 CMICM1CHANNELINC */
  SELECTABLE, NORMAL, (LONG)"+",
  0x000A, 0x0801, 0x0002, 0x0001,
  CMICM1PORTINC, CMICM1PORTNR, CMICM1PORTDEC, G_BOX     ,   /* Object 84  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0802, 0x0009, 0x0001,
        86,       -1,       -1, G_FTEXT   ,   /* Object 85 CMICM1PORTNR */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[75],
  0x0406, 0x0000, 0x0002, 0x0001,
  CMICM1PORTDEC,       -1,       -1, G_STRING  ,   /* Object 86  */
  SELECTABLE, NORMAL, (LONG)"Po:",
  0x0402, 0x0000, 0x0003, 0x0001,
        84,       -1,       -1, G_BUTTON  ,   /* Object 87 CMICM1PORTDEC */
  SELECTABLE, NORMAL, (LONG)"-",
  0x0000, 0x0000, 0x0002, 0x0001,
        77,       -1,       -1, G_BUTTON  ,   /* Object 88 CMICM1PORTINC */
  SELECTABLE, NORMAL, (LONG)"+",
  0x000A, 0x0802, 0x0002, 0x0001,
        11,       90, CMICM2PORTINC, G_BOX     ,   /* Object 89  */
  NONE, NORMAL, (LONG)0x00FE1101L,
  0x0000, 0x0004, 0x000D, 0x0004,
        91,       -1,       -1, G_BOXTEXT ,   /* Object 90  */
  NONE, NORMAL, (LONG)&rs_tedinfo[76],
  0x0001, 0x0000, 0x000B, 0x0801,
  CMICM2CHANNELINC, CMICM2CHANNELNR, CMICM2CHANNELDEC, G_BOX     ,   /* Object 91  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0801, 0x0009, 0x0001,
        93,       -1,       -1, G_FTEXT   ,   /* Object 92 CMICM2CHANNELNR */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[77],
  0x0006, 0x0000, 0x0003, 0x0001,
  CMICM2CHANNELDEC,       -1,       -1, G_STRING  ,   /* Object 93  */
  SELECTABLE, NORMAL, (LONG)"Ch:",
  0x0402, 0x0000, 0x0003, 0x0001,
        91,       -1,       -1, G_BUTTON  ,   /* Object 94 CMICM2CHANNELDEC */
  SELECTABLE, NORMAL, (LONG)"-",
  0x0000, 0x0000, 0x0002, 0x0001,
        96,       -1,       -1, G_BUTTON  ,   /* Object 95 CMICM2CHANNELINC */
  SELECTABLE, NORMAL, (LONG)"+",
  0x000A, 0x0801, 0x0002, 0x0001,
  CMICM2PORTINC, CMICM2PORTNR, CMICM2PORTDEC, G_BOX     ,   /* Object 96  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0802, 0x0009, 0x0001,
        98,       -1,       -1, G_FTEXT   ,   /* Object 97 CMICM2PORTNR */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[78],
  0x0406, 0x0000, 0x0002, 0x0001,
  CMICM2PORTDEC,       -1,       -1, G_STRING  ,   /* Object 98  */
  SELECTABLE, NORMAL, (LONG)"Po:",
  0x0402, 0x0000, 0x0003, 0x0001,
        96,       -1,       -1, G_BUTTON  ,   /* Object 99 CMICM2PORTDEC */
  SELECTABLE, NORMAL, (LONG)"-",
  0x0000, 0x0000, 0x0002, 0x0001,
        89,       -1,       -1, G_BUTTON  ,   /* Object 100 CMICM2PORTINC */
  SELECTABLE, NORMAL, (LONG)"+",
  0x000A, 0x0802, 0x0002, 0x0001,
  CMIHELP , CMISETDEC,      106, G_BOX     ,   /* Object 101  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0001, 0x080A, 0x0014, 0x0803,
  CMISETINC,       -1,       -1, G_BUTTON  ,   /* Object 102 CMISETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  CMISETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 103 CMISETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  CMISETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 104 CMISETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
       106,       -1,       -1, G_BUTTON  ,   /* Object 105 CMISETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
       101,      107, CMISETNR, G_BOX     ,   /* Object 106  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  CMISETNR,       -1,       -1, G_STRING  ,   /* Object 107  */
  NONE, NORMAL, (LONG)"CMI-~Nr:",
  0x0000, 0x0000, 0x0008, 0x0801,
       106,       -1,       -1, G_FTEXT   ,   /* Object 108 CMISETNR */
  SELECTABLE|EDITABLE, NORMAL, (LONG)&rs_tedinfo[79],
  0x0008, 0x0000, 0x0006, 0x0801,
  CMISTANDARD,       -1,       -1, G_BUTTON  ,   /* Object 109 CMIHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0024, 0x080A, 0x0009, 0x0801,
  CMICANCEL,       -1,       -1, G_BUTTON  ,   /* Object 110 CMISTANDARD */
  SELECTABLE, NORMAL, (LONG)"Standard",
  0x0018, 0x080A, 0x0009, 0x0801,
  CMIOK   ,       -1,       -1, G_BUTTON  ,   /* Object 111 CMICANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0018, 0x080C, 0x0009, 0x0801,
         0,       -1,       -1, G_BUTTON  ,   /* Object 112 CMIOK */
  SELECTABLE|DEFAULT|LASTOB, NORMAL, (LONG)"~OK",
  0x0024, 0x080C, 0x0009, 0x0801,
  
  /******** Tree 1 CMI_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der CMI-Einstellungs-Box",
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
  
  /******** Tree 2 CMI_DESK ****************************************************/
        -1, CMIICON , CMIICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 CMIICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0001, 0x0800, 0x0309, 0x0802,
  
  /******** Tree 3 CMI_TEXT ****************************************************/
        -1, FCMIN   , FCMII   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FCMII   ,       -1,       -1, G_STRING  ,   /* Object 1 FCMIN */
  NONE, NORMAL, (LONG)" CMI-Setups ",
  0x0001, 0x0001, 0x000C, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FCMII */
  LASTOB, NORMAL, (LONG)" %d Presets ",
  0x0001, 0x0002, 0x000C, 0x0001,
  
  /******** Tree 4 CMI_INFO ****************************************************/
        -1,        1, CMIIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0023, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[80],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[81],
  0x0003, 0x0802, 0x001D, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[82],
  0x0006, 0x0004, 0x0017, 0x0001,
  CMIIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[83],
  0x0008, 0x0005, 0x0612, 0x0600,
  CMIIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 CMIIVERNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[84],
  0x0007, 0x0806, 0x0014, 0x0001,
  CMICOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 CMIIVERDA */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[85],
  0x0009, 0x0807, 0x0012, 0x0001,
  CMIISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 CMICOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[86],
  0x0009, 0x0808, 0x0012, 0x0800,
  CMIIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 CMIISETUPS */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[87],
  0x0008, 0x0809, 0x0013, 0x0001,
  CMIIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 CMIIAKT */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[88],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 CMIIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 CMI_SETUP */
  &rs_object[113],   /* Tree  1 CMI_HELP */
  &rs_object[127],   /* Tree  2 CMI_DESK */
  &rs_object[129],   /* Tree  3 CMI_TEXT */
  &rs_object[132]    /* Tree  4 CMI_INFO */
};
