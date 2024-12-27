/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "EFF_MOD.H"

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
{ "Gruppe",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 7, 1,
  "Port",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "Channel",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 8, 1,
  "Control",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 8, 1,
  "31-32",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Midi 1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 7, 1,
  "16",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "1",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 1,
  "St\204rke",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 7, 1,
  "___",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 4, 6,
  "REALTIME 3 D SYNTHESIZER",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 25, 1,
  "EFF Effektger\204t-Ansteuerung",
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
  "01234567890",
  "Setups: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 20,
  "01234567890",
  "akt. Set: ___________",
  "XXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 22,
  " 12.12.1992",
  "compiliert: ___________",
  "XXXXXXXXXXX",
  SMALL, 0, TE_LEFT , 0x1180, 0, -1, 12, 24
};

static RSIB0MASK[] =
{ 0x0000, 0x0000, 0x01FC, 0x00FE, 
  0x03FC, 0x00FE, 0x07FF, 0xFFFE, 
  0x07FF, 0xFFFE, 0x07FF, 0xFFFE, 
  0x3FFF, 0xFFFE, 0x7FFF, 0xFFFE, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFF8, 
  0x1FFF, 0xFFF8, 0x1FFF, 0xFFFE, 
  0x1FFF, 0xFFFE, 0x1FFF, 0xFFFE, 
  0x1FFF, 0xFFFE, 0x1FFF, 0xFFFE, 
  0x1FFF, 0xFFFE, 0x1FFF, 0xFFFE, 
  0x1FFF, 0xFFFC, 0xFFFF, 0xFFFC, 
  0xFFFF, 0xFFF8, 0xFFFF, 0xFFE0, 
  0xFFFF, 0xFFE0, 0xFFFF, 0xFFE0, 
  0xFF00, 0x1FC0, 0xFF00, 0x1F80
};

static RSIB0DATA[] =
{ 0x0000, 0x0000, 0x0000, 0x0000, 
  0x00F8, 0x007C, 0x0100, 0x0004, 
  0x023F, 0xFFF4, 0x0060, 0x0034, 
  0x00A0, 0x0054, 0x1320, 0x0090, 
  0x2420, 0x0090, 0x4FFF, 0xFF10, 
  0x4820, 0x0110, 0x4820, 0x0110, 
  0x4820, 0x0110, 0x0820, 0x0110, 
  0x0820, 0x0110, 0x0820, 0x0110, 
  0x08A0, 0x0114, 0x08A0, 0x0114, 
  0x08A0, 0x0114, 0x08A0, 0x0114, 
  0x093F, 0xFFF4, 0x0A40, 0x0128, 
  0x0880, 0x0148, 0x0900, 0x0150, 
  0x4A00, 0x0180, 0x4C00, 0x0100, 
  0x4FFF, 0xFF40, 0x4000, 0x0080, 
  0x7E00, 0x0F00, 0x0000, 0x0000
};

static ICONBLK rs_iconblk[] =
{ RSIB0MASK, RSIB0DATA, "EFF Effekt", 0x1000|'\0',
    0,   0,  20,   0,  32,  30,   0,  32,  72,   8
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 EFF_SETUP ****************************************************/
        -1,        1,       15, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021121L,
  0x0000, 0x0000, 0x002E, 0x0807,
         2,       -1,       -1, G_BOXTEXT ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0001, 0x0800, 0x0008, 0x0001,
         3,       -1,       -1, G_BOXTEXT ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x000A, 0x0800, 0x0008, 0x0001,
         4,       -1,       -1, G_BOXTEXT ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0013, 0x0800, 0x0008, 0x0001,
         5,       -1,       -1, G_BOXTEXT ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x001C, 0x0800, 0x0008, 0x0001,
         6,       -1,       -1, G_BOXTEXT ,   /* Object 5  */
  NONE, SHADOWED, (LONG)&rs_tedinfo[4],
  0x0001, 0x0801, 0x0008, 0x0001,
         7,       -1,       -1, G_BOXTEXT ,   /* Object 6  */
  NONE, SHADOWED, (LONG)&rs_tedinfo[5],
  0x000A, 0x0801, 0x0008, 0x0001,
         8,       -1,       -1, G_BOXTEXT ,   /* Object 7  */
  NONE, SHADOWED, (LONG)&rs_tedinfo[6],
  0x0013, 0x0801, 0x0008, 0x0001,
  EFFHELP ,       -1,       -1, G_BOXTEXT ,   /* Object 8  */
  NONE, SHADOWED, (LONG)&rs_tedinfo[7],
  0x001C, 0x0801, 0x0008, 0x0001,
  EFFSTANDARD,       -1,       -1, G_BUTTON  ,   /* Object 9 EFFHELP */
  NONE, NORMAL, (LONG)"~Help",
  0x0023, 0x0803, 0x0009, 0x0801,
  EFFCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 10 EFFSTANDARD */
  SELECTABLE, NORMAL, (LONG)"S~tandard",
  0x0018, 0x0803, 0x0009, 0x0801,
  EFFOK   ,       -1,       -1, G_BUTTON  ,   /* Object 11 EFFCANCEL */
  SELECTABLE, NORMAL, (LONG)"~Abbruch",
  0x0018, 0x0805, 0x0009, 0x0801,
  EFFSTAERKE,       -1,       -1, G_BUTTON  ,   /* Object 12 EFFOK */
  SELECTABLE|DEFAULT, NORMAL, (LONG)"~OK",
  0x0023, 0x0805, 0x0009, 0x0801,
        14,       -1,       -1, G_BOXTEXT ,   /* Object 13 EFFSTAERKE */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0025, 0x0801, 0x0008, 0x0001,
        15,       -1,       -1, G_BOXTEXT ,   /* Object 14  */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0025, 0x0800, 0x0008, 0x0001,
         0, EFFSETDEC,       20, G_BOX     ,   /* Object 15  */
  NONE, NORMAL, (LONG)0x00FE1121L,
  0x0401, 0x0803, 0x0014, 0x0803,
  EFFSETINC,       -1,       -1, G_BUTTON  ,   /* Object 16 EFFSETDEC */
  SELECTABLE, NORMAL, (LONG)"~-",
  0x0000, 0x0000, 0x0003, 0x0801,
  EFFSETSTORE,       -1,       -1, G_BUTTON  ,   /* Object 17 EFFSETINC */
  SELECTABLE, NORMAL, (LONG)"~+",
  0x0011, 0x0000, 0x0003, 0x0801,
  EFFSETRECALL,       -1,       -1, G_BUTTON  ,   /* Object 18 EFFSETSTORE */
  SELECTABLE, NORMAL, (LONG)"~Store",
  0x0001, 0x0002, 0x0008, 0x0001,
        20,       -1,       -1, G_BUTTON  ,   /* Object 19 EFFSETRECALL */
  SELECTABLE, NORMAL, (LONG)"~Recall",
  0x000B, 0x0002, 0x0008, 0x0001,
        15,       21, EFFSETNR, G_BOX     ,   /* Object 20  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0003, 0x0000, 0x000E, 0x0801,
  EFFSETNR,       -1,       -1, G_STRING  ,   /* Object 21  */
  NONE, NORMAL, (LONG)"EFF-~Nr:",
  0x0001, 0x0000, 0x0008, 0x0801,
        20,       -1,       -1, G_FTEXT   ,   /* Object 22 EFFSETNR */
  EDITABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[10],
  0x0008, 0x0000, 0x0006, 0x0801,
  
  /******** Tree 1 EFF_HELP ****************************************************/
        -1,        1,       13, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003B, 0x0016,
         2,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Sie befinden sich in der EFF-Einstellungs-Box",
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
  
  /******** Tree 2 EFF_DESK ****************************************************/
        -1, EFFICON , EFFICON , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021143L,
  0x0000, 0x0000, 0x0034, 0x0009,
         0,       -1,       -1, G_ICON    ,   /* Object 1 EFFICON */
  LASTOB, NORMAL, (LONG)&rs_iconblk[0],
  0x0001, 0x0001, 0x0009, 0x0802,
  
  /******** Tree 3 EFF_TEXT ****************************************************/
        -1, FEFFN   , FEFFI   , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0034, 0x0009,
  FEFFI   ,       -1,       -1, G_STRING  ,   /* Object 1 FEFFN */
  NONE, NORMAL, (LONG)"EFF-Setups",
  0x0002, 0x0001, 0x000A, 0x0001,
         0,       -1,       -1, G_STRING  ,   /* Object 2 FEFFI */
  LASTOB, NORMAL, (LONG)" %d Presets",
  0x0002, 0x0002, 0x000B, 0x0001,
  
  /******** Tree 4 EFF_INFO ****************************************************/
        -1,        1, EFFCOMPILE, G_BOX     |0x4000,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0022, 0x080F,
         2,       -1,       -1, G_TEXT    ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0005, 0x0001, 0x0018, 0x0001,
         3,       -1,       -1, G_TEXT    ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0004, 0x0802, 0x001B, 0x0001,
         4,       -1,       -1, G_TEXT    ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0006, 0x0004, 0x0017, 0x0001,
  EFFIVERNR,       -1,       -1, G_TEXT    ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0008, 0x0005, 0x0612, 0x0600,
  EFFIVERDA,       -1,       -1, G_FTEXT   ,   /* Object 5 EFFIVERNR */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0007, 0x0006, 0x0014, 0x0001,
  EFFISETUPS,       -1,       -1, G_FTEXT   ,   /* Object 6 EFFIVERDA */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0009, 0x0007, 0x0012, 0x0001,
  EFFIAKT ,       -1,       -1, G_FTEXT   ,   /* Object 7 EFFISETUPS */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0008, 0x0809, 0x0013, 0x0001,
  EFFIOK  ,       -1,       -1, G_FTEXT   ,   /* Object 8 EFFIAKT */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[18],
  0x0006, 0x080A, 0x0015, 0x0001,
  EFFCOMPILE,       -1,       -1, G_BUTTON  |0x4000,   /* Object 9 EFFIOK */
  SELECTABLE|DEFAULT|EXIT, NORMAL, (LONG)"OK",
  0x000D, 0x000D, 0x0008, 0x0002,
         0,       -1,       -1, G_FTEXT   ,   /* Object 10 EFFCOMPILE */
  LASTOB, NORMAL, (LONG)&rs_tedinfo[19],
  0x0009, 0x0008, 0x0012, 0x0800
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 EFF_SETUP */
  &rs_object[23],   /* Tree  1 EFF_HELP */
  &rs_object[37],   /* Tree  2 EFF_DESK */
  &rs_object[39],   /* Tree  3 EFF_TEXT */
  &rs_object[42]    /* Tree  4 EFF_INFO */
};
