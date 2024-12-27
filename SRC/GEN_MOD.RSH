/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "GEN_MOD.H"

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
{ "Prop.",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Typ",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Setup",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "LFA",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "LFB",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "MAA",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "MAE",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "MTR",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "SPG",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "GEP",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "___",
  "X:_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 8,
  "___",
  "Y:_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 8,
  "___",
  "Z:_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 4, 8,
  "SPO",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "SPS",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "MAN",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "ROT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "MTR-LFO",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 8, 1,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "VAR",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "Zoom",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "GEN General-Setups",
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
{ 0x7FFC, 0x0000, 0x7FFC, 0x0000, 
  0x7FFC, 0xFFFE, 0x7FFC, 0xFFFE, 
  0x7FFF, 0xFFFE, 0x7FFF, 0xFFFE, 
  0x7FFF, 0xFFFE, 0x7FFC, 0xFFFE, 
  0x7FFC, 0xFFFE, 0x7FFC, 0x0000, 
  0x7FFC, 0x0000, 0x7FFC, 0xFFFE, 
  0x7FFC, 0xFFFE, 0x7FFF, 0xFFFE, 
  0x7FFF, 0xFFFE, 0x7FFF, 0xFFFE, 
  0x7FFC, 0xFFFE, 0x7FFC, 0xFFFE, 
  0x7FFC, 0x0000, 0x7FFC, 0x0000, 
  0x7FFC, 0xFFFE, 0x7FFC, 0xFFFE, 
  0x7FFF, 0xFFFE, 0x7FFF, 0xFFFE, 
  0x7FFF, 0xFFFE, 0x7FFC, 0xFFFE, 
  0x7FFC, 0xFFFE, 0x7FFC, 0x0000, 
  0x7FFC, 0x0000, 0x7FFC, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x3FF8, 0x0000, 
  0x3018, 0x0000, 0x2008, 0x4FFC, 
  0x2EE8, 0x6804, 0x2EEF, 0xF804, 
  0x2008, 0x6804, 0x2EE8, 0x4FFC, 
  0x2EE8, 0x0000, 0x2008, 0x0000, 
  0x2EE8, 0x0000, 0x2EE8, 0x0000, 
  0x2008, 0x4FFC, 0x2EE8, 0x6804, 
  0x2EEF, 0xF804, 0x2008, 0x6804, 
  0x2EE8, 0x4FFC, 0x2EE8, 0x0000, 
  0x2008, 0x0000, 0x2EE8, 0x0000, 
  0x2EE8, 0x0000, 0x2008, 0x4FFC, 
  0x2EE8, 0x6804, 0x2EEF, 0xF804, 
  0x2008, 0x6804, 0x2EE8, 0x4FFC, 
  0x2EE8, 0x0000, 0x2008, 0x0000, 
  0x3FF8, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "GEN-Setups", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 GEN_MENU ****************************************************/
        -1,        1,        5, G_IBOX    ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0000, 0x0050, 0x0019,
         5,        2,        2, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0050, 0x0201,
         1, MGENINFO, MGENANZEIGEN, G_IBOX    ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0002, 0x0000, 0x0010, 0x0301,
  MGENANZEIGEN,       -1,       -1, G_TITLE   ,   /* Object 3 MGENINFO */
  NONE, NORMAL, (LONG)" Info",
  0x0000, 0x0000, 0x0006, 0x0301,
         2,       -1,       -1, G_TITLE   ,   /* Object 4 MGENANZEIGEN */
  NONE, NORMAL, (LONG)" Anzeigen",
  0x0006, 0x0000, 0x000A, 0x0301,
         0,        6,        8, G_IBOX    ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00000000L,
  0x0000, 0x0301, 0x0050, 0x0017,
         8, MGENINFOANZEIG, MGENINFOANZEIG, G_BOX     ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0000, 0x0016, 0x0001,
         6,       -1,       -1, G_STRING  ,   /* Object 7 MGENINFOANZEIG */
  NONE, NORMAL, (LONG)"  Info anzeigen... ^I ",
  0x0000, 0x0000, 0x0016, 0x0001,
         5, MGENSETUPS, MGENSETUPS, G_BOX     ,   /* Object 8  */
  NONE, NORMAL, (LONG)0x00FF1000L,
  0x0008, 0x0000, 0x000D, 0x0001,
         8,       -1,       -1, G_STRING  ,   /* Object 9 MGENSETUPS */
  LASTOB, NORMAL, (LONG)"  Setups  \007S ",
  0x0000, 0x0000, 0x000D, 0x0001,
  
  /******** Tree 1 GEN_SETUP ****************************************************/
        -1,        1,       68, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021121L,
  0x0000, 0x0000, 0x0029, 0x080F,
         5,        2,        4, G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0800, 0x0011, 0x0001,
         3,       -1,       -1, G_BOXTEXT ,   /* Object 2  */
  TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[0],
  0x000B, 0x0000, 0x0006, 0x0001,
         4,       -1,       -1, G_BOXTEXT ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0000, 0x0000, 0x0005, 0x0001,
         1,       -1,       -1, G_BOXTEXT ,   /* Object 4  */
  TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[2],
  0x0005, 0x0000, 0x0006, 0x0001,
        13, GENSETDEC,       10, G_BOX     ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0014, 0x0800, 0x0014, 0x0803,
  GENSETINC,       -1,       -1, G_BUTTON  ,   /* Object 6 GENSETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  GENSETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 7 GENSETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  GENSETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 8 GENSETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
        10,       -1,       -1, G_BUTTON  ,   /* Object 9 GENSETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
         5,       11, GENSETNR, G_BOX     ,   /* Object 10  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  GENSETNR,       -1,       -1, G_STRING  ,   /* Object 11  */
  NONE, NORMAL, (LONG)"GEN-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0801,
        10,       -1,       -1, G_FTEXT   ,   /* Object 12 GENSETNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0008, 0x0000, 0x0006, 0x0801,
        17, GENLFATITLE, GENLFAPROP, G_BOX     ,   /* Object 13  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0002, 0x0011, 0x0001,
  GENLFASET,       -1,       -1, G_BOXTEXT ,   /* Object 14 GENLFATITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0000, 0x0000, 0x0005, 0x0001,
  GENLFAPROP,       -1,       -1, G_FBOXTEXT,   /* Object 15 GENLFASET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0005, 0x0000, 0x0006, 0x0001,
        13,       -1,       -1, G_FBOXTEXT,   /* Object 16 GENLFAPROP */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[6],
  0x000B, 0x0000, 0x0006, 0x0001,
        21, GENLFBTITLE, GENLFBPROP, G_BOX     ,   /* Object 17  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0003, 0x0011, 0x0001,
  GENLFBSET,       -1,       -1, G_BOXTEXT ,   /* Object 18 GENLFBTITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0000, 0x0000, 0x0005, 0x0001,
  GENLFBPROP,       -1,       -1, G_FBOXTEXT,   /* Object 19 GENLFBSET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0005, 0x0000, 0x0006, 0x0001,
        17,       -1,       -1, G_FBOXTEXT,   /* Object 20 GENLFBPROP */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x000B, 0x0000, 0x0006, 0x0001,
  GENHELP , GENMAATITLE, GENMAAPROP, G_BOX     ,   /* Object 21  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0804, 0x0011, 0x0001,
  GENMAASET,       -1,       -1, G_BOXTEXT ,   /* Object 22 GENMAATITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0000, 0x0000, 0x0005, 0x0001,
  GENMAAPROP,       -1,       -1, G_FBOXTEXT,   /* Object 23 GENMAASET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0005, 0x0000, 0x0006, 0x0001,
        21,       -1,       -1, G_FBOXTEXT,   /* Object 24 GENMAAPROP */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[12],
  0x000B, 0x0000, 0x0006, 0x0001,
  GENSTANDARD,       -1,       -1, G_BUTTON  ,   /* Object 25 GENHELP */
  SELECTABLE, NORMAL, (LONG)"~Hilfe",
  0x0014, 0x0804, 0x0009, 0x0801,
        27,       -1,       -1, G_BUTTON  ,   /* Object 26 GENSTANDARD */
  SELECTABLE, NORMAL, (LONG)"S~tandard",
  0x001F, 0x0804, 0x0009, 0x0801,
  GENCANCEL, GENMAETITLE, GENMAEPROP, G_BOX     ,   /* Object 27  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0805, 0x0011, 0x0001,
  GENMAESET,       -1,       -1, G_BOXTEXT ,   /* Object 28 GENMAETITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0000, 0x0000, 0x0005, 0x0001,
  GENMAEPROP,       -1,       -1, G_FBOXTEXT,   /* Object 29 GENMAESET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0005, 0x0000, 0x0006, 0x0001,
        27,       -1,       -1, G_FBOXTEXT,   /* Object 30 GENMAEPROP */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x000B, 0x0000, 0x0006, 0x0001,
  GENOK   ,       -1,       -1, G_BUTTON  ,   /* Object 31 GENCANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0014, 0x0806, 0x0009, 0x0801,
        33,       -1,       -1, G_BUTTON  ,   /* Object 32 GENOK */
  SELECTABLE|DEFAULT, NORMAL, (LONG)"~OK",
  0x001F, 0x0806, 0x0009, 0x0801,
        37, GENMTRTITLE, GENMTRPROP, G_BOX     ,   /* Object 33  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0007, 0x0011, 0x0001,
  GENMTRSET,       -1,       -1, G_BOXTEXT ,   /* Object 34 GENMTRTITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0000, 0x0000, 0x0005, 0x0001,
  GENMTRPROP,       -1,       -1, G_FBOXTEXT,   /* Object 35 GENMTRSET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0005, 0x0000, 0x0006, 0x0001,
        33,       -1,       -1, G_FBOXTEXT,   /* Object 36 GENMTRPROP */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[18],
  0x000B, 0x0000, 0x0006, 0x0001,
        41, GENSPGTITLE, GENSPGPROP, G_BOX     ,   /* Object 37  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0808, 0x0011, 0x0001,
  GENSPGSET,       -1,       -1, G_BOXTEXT ,   /* Object 38 GENSPGTITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0000, 0x0000, 0x0005, 0x0001,
  GENSPGPROP,       -1,       -1, G_FBOXTEXT,   /* Object 39 GENSPGSET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0005, 0x0000, 0x0006, 0x0001,
        37,       -1,       -1, G_FBOXTEXT,   /* Object 40 GENSPGPROP */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[21],
  0x000B, 0x0000, 0x0006, 0x0001,
        48, GENGEPTITLE, GENGEPZ , G_BOX     ,   /* Object 41  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x000E, 0x080C, 0x0019, 0x0802,
  GENGEPSET,       -1,       -1, G_BOXTEXT ,   /* Object 42 GENGEPTITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0000, 0x0000, 0x0005, 0x0001,
  GENLEARN,       -1,       -1, G_FBOXTEXT,   /* Object 43 GENGEPSET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0005, 0x0000, 0x0006, 0x0001,
  GENGEPX ,       -1,       -1, G_BUTTON  ,   /* Object 44 GENLEARN */
  SELECTABLE, NORMAL, (LONG)"~Learn",
  0x0010, 0x0000, 0x0009, 0x0001,
  GENGEPY ,       -1,       -1, G_FBOXTEXT,   /* Object 45 GENGEPX */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0000, 0x0801, 0x0007, 0x0001,
  GENGEPZ ,       -1,       -1, G_FBOXTEXT,   /* Object 46 GENGEPY */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[25],
  0x000A, 0x0801, 0x0007, 0x0001,
        41,       -1,       -1, G_FBOXTEXT,   /* Object 47 GENGEPZ */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0012, 0x0801, 0x0007, 0x0001,
        52, GENSPOTITLE, GENSPOPROP, G_BOX     ,   /* Object 48  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0809, 0x0011, 0x0001,
  GENSPOSET,       -1,       -1, G_BOXTEXT ,   /* Object 49 GENSPOTITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0000, 0x0000, 0x0005, 0x0001,
  GENSPOPROP,       -1,       -1, G_FBOXTEXT,   /* Object 50 GENSPOSET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0005, 0x0000, 0x0006, 0x0001,
        48,       -1,       -1, G_FBOXTEXT,   /* Object 51 GENSPOPROP */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[29],
  0x000B, 0x0000, 0x0006, 0x0001,
        56, GENSPSTITLE, GENSPSPROP, G_BOX     ,   /* Object 52  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x080A, 0x0011, 0x0001,
  GENSPSSET,       -1,       -1, G_BOXTEXT ,   /* Object 53 GENSPSTITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0000, 0x0000, 0x0005, 0x0001,
  GENSPSPROP,       -1,       -1, G_FBOXTEXT,   /* Object 54 GENSPSSET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0005, 0x0000, 0x0006, 0x0001,
        52,       -1,       -1, G_FBOXTEXT,   /* Object 55 GENSPSPROP */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[32],
  0x000B, 0x0000, 0x0006, 0x0001,
        59,       57, GENMANSET, G_BOX     ,   /* Object 56  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x000C, 0x000B, 0x0001,
  GENMANSET,       -1,       -1, G_BOXTEXT ,   /* Object 57  */
  NONE, NORMAL, (LONG)&rs_tedinfo[33],
  0x0000, 0x0000, 0x0005, 0x0001,
        56,       -1,       -1, G_FBOXTEXT,   /* Object 58 GENMANSET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0005, 0x0000, 0x0006, 0x0001,
        62,       60, GENROTSET, G_BOX     ,   /* Object 59  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x000D, 0x000B, 0x0001,
  GENROTSET,       -1,       -1, G_BOXTEXT ,   /* Object 60  */
  NONE, NORMAL, (LONG)&rs_tedinfo[35],
  0x0000, 0x0000, 0x0005, 0x0001,
        59,       -1,       -1, G_FBOXTEXT,   /* Object 61 GENROTSET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[36],
  0x0005, 0x0000, 0x0006, 0x0001,
        65,       63, GENRELMTRLFO, G_BOX     ,   /* Object 62  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0017, 0x0009, 0x000E, 0x0001,
  GENRELMTRLFO,       -1,       -1, G_BOXTEXT ,   /* Object 63  */
  NONE, NORMAL, (LONG)&rs_tedinfo[37],
  0x0000, 0x0000, 0x0008, 0x0001,
        62,       -1,       -1, G_FBOXTEXT,   /* Object 64 GENRELMTRLFO */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[38],
  0x0008, 0x0000, 0x0006, 0x0001,
        68,       66, GENVARSET, G_BOX     ,   /* Object 65  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x000E, 0x000B, 0x0001,
  GENVARSET,       -1,       -1, G_BOXTEXT ,   /* Object 66  */
  NONE, NORMAL, (LONG)&rs_tedinfo[39],
  0x0000, 0x0000, 0x0005, 0x0001,
        65,       -1,       -1, G_FBOXTEXT,   /* Object 67 GENVARSET */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[40],
  0x0005, 0x0000, 0x0006, 0x0001,
         0, GENZOOMTITLE, GENZOOMPROP, G_BOX     ,   /* Object 68  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0017, 0x080A, 0x000E, 0x0001,
  GENZOOMPROP,       -1,       -1, G_BOXTEXT ,   /* Object 69 GENZOOMTITLE */
  NONE, NORMAL, (LONG)&rs_tedinfo[41],
  0x0000, 0x0000, 0x0008, 0x0001,
        68,       -1,       -1, G_FBOXTEXT,   /* Object 70 GENZOOMPROP */
  EDITABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[42],
  0x0008, 0x0000, 0x0006, 0x0001,
  
  /******** Tree 2 GEN_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der GEN-Einstellungs-Box",
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
  
  /******** Tree 3 GEN_DESK ****************************************************/
        -1, GENICON , GENICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 GENICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0002, 0x0001, 0x0009, 0x0802,
  
  /******** Tree 4 GEN_TEXT ****************************************************/
        -1, FGENN   , FGENI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FGENI   ,       -1,       -1, G_STRING  ,   /* Object 1 FGENN */
  NONE, NORMAL, (LONG)" GEN-Setups ",
  0x0002, 0x0001, 0x000C, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FGENI */
  LASTOB, NORMAL, (LONG)" %d Presets",
  0x0002, 0x0002, 0x000B, 0x0001,
  
  /******** Tree 5 GEN_INFO ****************************************************/
        -1,        1, GENIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[43],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[44],
  0x0008, 0x0802, 0x0012, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[45],
  0x0006, 0x0004, 0x0017, 0x0001,
  GENIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[46],
  0x0008, 0x0005, 0x0612, 0x0600,
  GENIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 GENIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[47],
  0x0007, 0x0006, 0x0014, 0x0001,
  GENCOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 GENIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[48],
  0x0009, 0x0007, 0x0012, 0x0001,
  GENISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 GENCOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[49],
  0x0009, 0x0008, 0x0012, 0x0800,
  GENIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 GENISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[50],
  0x0008, 0x0809, 0x0013, 0x0001,
  GENIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 GENIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[51],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 GENIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 GEN_MENU */
  &rs_object[10],   /* Tree  1 GEN_SETUP */
  &rs_object[81],   /* Tree  2 GEN_HELP */
  &rs_object[95],   /* Tree  3 GEN_DESK */
  &rs_object[97],   /* Tree  4 GEN_TEXT */
  &rs_object[100]    /* Tree  5 GEN_INFO */
};
