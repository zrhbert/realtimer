/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "GMI_MOD.H"

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
{ "GEN",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "LF1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "LF2",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "MAA",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "MAE",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "MTR",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "SPG",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "SPO",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "SPS",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
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
  "VAR",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "_____",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "GMI GEN-Mini",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 13, 1,
  "1994 by Cybernetic Arts",
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
{ 0xFFF0, 0x0000, 0xFFF0, 0x0000, 
  0xFFF1, 0xFFFC, 0xFFF1, 0xFFFC, 
  0xFFFF, 0xFFFC, 0xFFFF, 0xFFFC, 
  0xFFFF, 0xFFFC, 0xFFF1, 0xFFFC, 
  0xFFF1, 0xFFFC, 0xFFF0, 0x0000, 
  0xFFF0, 0x0000, 0xFFF1, 0xFFFC, 
  0xFFF1, 0xFFFC, 0xFFFF, 0xFFFC, 
  0xFFFF, 0xFFFC, 0xFFFF, 0xFFFC, 
  0xFFF1, 0xFFFC, 0xFFF1, 0xFFFC, 
  0xFFF0, 0x0000, 0xFFF0, 0x0000, 
  0xFFF1, 0xFFFC, 0xFFF1, 0xFFFC, 
  0xFFFF, 0xFFFC, 0xFFFF, 0xFFFC, 
  0xFFFF, 0xFFFC, 0xFFF1, 0xFFFC, 
  0xFFF1, 0xFFFC, 0xFFF0, 0x0000, 
  0xFFF0, 0x0000, 0xFFF0, 0x0000
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x7FE0, 0x0000, 
  0x6060, 0x0000, 0x4020, 0x9FF8, 
  0x5FA0, 0xD008, 0x5FFF, 0xF008, 
  0x4020, 0xD008, 0x5FA0, 0x9FF8, 
  0x5FA0, 0x0000, 0x4020, 0x0000, 
  0x5FA0, 0x0000, 0x5FA0, 0x0000, 
  0x4020, 0x9FF8, 0x5FA0, 0xD008, 
  0x5FFF, 0xF008, 0x4020, 0xD008, 
  0x5FA0, 0x9FF8, 0x5FA0, 0x0000, 
  0x4020, 0x0000, 0x5FA0, 0x0000, 
  0x5FA0, 0x0000, 0x4020, 0x9FF8, 
  0x5FA0, 0xD008, 0x5FFF, 0xF008, 
  0x4020, 0xD008, 0x5FA0, 0x9FF8, 
  0x5FA0, 0x0000, 0x4020, 0x0000, 
  0x7FE0, 0x0000, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "GMI-Setups", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 GMI_MENU ****************************************************/
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
  
  /******** Tree 1 GMI_SETUP ****************************************************/
        -1,        1,       34, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021121L,
  0x0000, 0x0000, 0x000C, 0x080D,
         4, GMIGENT , GMIGEN  , G_BOX     ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0800, 0x000A, 0x0001,
  GMIGEN  ,       -1,       -1, G_BOXTEXT ,   /* Object 2 GMIGENT */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0000, 0x0000, 0x0004, 0x0001,
         1,       -1,       -1, G_FBOXTEXT,   /* Object 3 GMIGEN */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0004, 0x0000, 0x0006, 0x0001,
         7, GMILFAT , GMILFA  , G_BOX     ,   /* Object 4  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0002, 0x000A, 0x0001,
  GMILFA  ,       -1,       -1, G_BOXTEXT ,   /* Object 5 GMILFAT */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0000, 0x0000, 0x0004, 0x0001,
         4,       -1,       -1, G_FBOXTEXT,   /* Object 6 GMILFA */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0004, 0x0000, 0x0006, 0x0001,
        10, GMILFBT , GMILFB  , G_BOX     ,   /* Object 7  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0003, 0x000A, 0x0001,
  GMILFB  ,       -1,       -1, G_BOXTEXT ,   /* Object 8 GMILFBT */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0000, 0x0000, 0x0004, 0x0001,
         7,       -1,       -1, G_FBOXTEXT,   /* Object 9 GMILFB */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0004, 0x0000, 0x0006, 0x0001,
        13, GMIMAAT , GMIMAA  , G_BOX     ,   /* Object 10  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0004, 0x000A, 0x0001,
  GMIMAA  ,       -1,       -1, G_BOXTEXT ,   /* Object 11 GMIMAAT */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0000, 0x0000, 0x0004, 0x0001,
        10,       -1,       -1, G_FBOXTEXT,   /* Object 12 GMIMAA */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0004, 0x0000, 0x0006, 0x0001,
        16, GMIMAET , GMIMAE  , G_BOX     ,   /* Object 13  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0005, 0x000A, 0x0001,
  GMIMAE  ,       -1,       -1, G_BOXTEXT ,   /* Object 14 GMIMAET */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0000, 0x0000, 0x0004, 0x0001,
        13,       -1,       -1, G_FBOXTEXT,   /* Object 15 GMIMAE */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0004, 0x0000, 0x0006, 0x0001,
        19, GMIMTRT , GMIMTR  , G_BOX     ,   /* Object 16  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0006, 0x000A, 0x0001,
  GMIMTR  ,       -1,       -1, G_BOXTEXT ,   /* Object 17 GMIMTRT */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0000, 0x0000, 0x0004, 0x0001,
        16,       -1,       -1, G_FBOXTEXT,   /* Object 18 GMIMTR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0004, 0x0000, 0x0006, 0x0001,
        22, GMISPGT , GMISPG  , G_BOX     ,   /* Object 19  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0007, 0x000A, 0x0001,
  GMISPG  ,       -1,       -1, G_BOXTEXT ,   /* Object 20 GMISPGT */
  NONE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0000, 0x0000, 0x0004, 0x0001,
        19,       -1,       -1, G_FBOXTEXT,   /* Object 21 GMISPG */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0004, 0x0000, 0x0006, 0x0001,
        25, GMISPOT , GMISPO  , G_BOX     ,   /* Object 22  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0008, 0x000A, 0x0001,
  GMISPO  ,       -1,       -1, G_BOXTEXT ,   /* Object 23 GMISPOT */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0000, 0x0000, 0x0004, 0x0001,
        22,       -1,       -1, G_FBOXTEXT,   /* Object 24 GMISPO */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0004, 0x0000, 0x0006, 0x0001,
        28, GMISPST , GMISPS  , G_BOX     ,   /* Object 25  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x0009, 0x000A, 0x0001,
  GMISPS  ,       -1,       -1, G_BOXTEXT ,   /* Object 26 GMISPST */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0000, 0x0000, 0x0004, 0x0001,
        25,       -1,       -1, G_FBOXTEXT,   /* Object 27 GMISPS */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0004, 0x0000, 0x0006, 0x0001,
        31, GMIMANT , GMIMAN  , G_BOX     ,   /* Object 28  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x000A, 0x000A, 0x0001,
  GMIMAN  ,       -1,       -1, G_BOXTEXT ,   /* Object 29 GMIMANT */
  NONE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0000, 0x0000, 0x0004, 0x0001,
        28,       -1,       -1, G_FBOXTEXT,   /* Object 30 GMIMAN */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0004, 0x0000, 0x0006, 0x0001,
        34, GMIROTT , GMIROT  , G_BOX     ,   /* Object 31  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x000B, 0x000A, 0x0001,
  GMIROT  ,       -1,       -1, G_BOXTEXT ,   /* Object 32 GMIROTT */
  NONE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0000, 0x0000, 0x0004, 0x0001,
        31,       -1,       -1, G_FBOXTEXT,   /* Object 33 GMIROT */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0004, 0x0000, 0x0006, 0x0001,
         0, GMIVART , GMIVAR  , G_BOX     ,   /* Object 34  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0001, 0x000C, 0x000A, 0x0001,
  GMIVAR  ,       -1,       -1, G_BOXTEXT ,   /* Object 35 GMIVART */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0000, 0x0000, 0x0004, 0x0001,
        34,       -1,       -1, G_FBOXTEXT,   /* Object 36 GMIVAR */
  EDITABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[23],
  0x0004, 0x0000, 0x0006, 0x0001,
  
  /******** Tree 2 GMI_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der GMI-Einstellungs-Box",
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
  
  /******** Tree 3 GMI_DESK ****************************************************/
        -1, GMIICON , GMIICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 GMIICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0002, 0x0001, 0x0009, 0x0802,
  
  /******** Tree 4 GMI_TEXT ****************************************************/
        -1, FGMIN   , FGMII   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FGMII   ,       -1,       -1, G_STRING  ,   /* Object 1 FGMIN */
  NONE, NORMAL, (LONG)" GEN-Mini ",
  0x0002, 0x0001, 0x000A, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FGMII */
  LASTOB, NORMAL, (LONG)" %d Presets",
  0x0002, 0x0002, 0x000B, 0x0001,
  
  /******** Tree 5 GMI_INFO ****************************************************/
        -1,        1, GMIIOK  , G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[25],
  0x000B, 0x0802, 0x000C, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0006, 0x0004, 0x0017, 0x0001,
  GMIIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0008, 0x0005, 0x0612, 0x0600,
  GMIIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 GMIIVERNR */
  NONE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0007, 0x0006, 0x0014, 0x0001,
  GMICOMPILE,       -1,       -1, G_FTEXT   ,   /* Object 6 GMIIVERDA */
  NONE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0009, 0x0007, 0x0012, 0x0001,
  GMIISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 7 GMICOMPILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[30],
  0x0009, 0x0008, 0x0012, 0x0800,
  GMIIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 8 GMIISETUPS */
  NONE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0008, 0x0809, 0x0013, 0x0001,
  GMIIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 9 GMIIAKT */
  NONE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0006, 0x080A, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON  |0x4000,   /* Object 10 GMIIOK */
  SELECTABLE|DEFAULT|EXIT|LASTOB, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 GMI_MENU */
  &rs_object[10],   /* Tree  1 GMI_SETUP */
  &rs_object[47],   /* Tree  2 GMI_HELP */
  &rs_object[61],   /* Tree  3 GMI_DESK */
  &rs_object[63],   /* Tree  4 GMI_TEXT */
  &rs_object[66]    /* Tree  5 GMI_INFO */
};
