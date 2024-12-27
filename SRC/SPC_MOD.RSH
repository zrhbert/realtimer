/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "SPC_MOD.H"

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
{ "012345678901234",
  "_______________",
  "999999999999999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 16, 16,
  "Momentan ausgef\201hrte Aktion .....................................",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 66, 1,
  "100%",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x0140, 0, 0, 5, 1,
  "0%",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 3, 1,
  "100%",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "hh:mm:ss",
  "Laufzeit: ________",
  "XXXXXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 9, 19,
  "hh:mm:ss",
  "Restzeit: ________",
  "XXXXXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 9, 19,
  "HH:MM:SS:LLL",
  "____________",
  "XXXXXXXXXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 13, 13,
  "0123456789012345678901234567890123456789012345678901234567890",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 62, 1,
  "________",
  "________",
  "XXXXXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 9, 9,
  "________",
  "________",
  "XXXXXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 9, 9,
  "Parametereingabexxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 48, 1,
  "1000",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 1,
  "_____",
  "_____",
  "XXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "-1000",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1181, 0, -1, 6, 1,
  "+1000",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Texteingabe",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 12, 1,
  "__________________________________",
  "__________________________________",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 35, 35
};

static OBJECT rs_object[] =
{ 
  /******** Tree 0 ZAHL ****************************************************/
        -1, ZAHZAHL , ZAHZAHL , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0000, 0x0000, 0x0011, 0x0801,
         0,       -1,       -1, G_FTEXT   ,   /* Object 1 ZAHZAHL */
  SELECTABLE|DEFAULT|EXIT|EDITABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[0],
  0x0000, 0x0000, 0x0011, 0x0801,
  
  /******** Tree 1 AKTSTAT ****************************************************/
        -1, AKTTEXT , AKTERWARTZEIT, G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0449, 0x0805,
  AKTWORKBOX,       -1,       -1, G_BOXTEXT ,   /* Object 1 AKTTEXT */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0001, 0x0800, 0x0447, 0x0801,
  AKTCANCEL, AKTWORKSLIDER, AKTWORKSLIDER, G_BOX     ,   /* Object 2 AKTWORKBOX */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0004, 0x0802, 0x0040, 0x0001,
  AKTWORKBOX, AKTPROZENT, AKTPROZENT, G_BOX     ,   /* Object 3 AKTWORKSLIDER */
  NONE, NORMAL, (LONG)0x00FF1111L,
  0x0000, 0x0000, 0x001C, 0x0001,
  AKTWORKSLIDER,       -1,       -1, G_TEXT    ,   /* Object 4 AKTPROZENT */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0006, 0x0000, 0x0004, 0x0001,
         6,       -1,       -1, G_BUTTON  ,   /* Object 5 AKTCANCEL */
  SELECTABLE|EXIT, NORMAL, (LONG)"Abbruch",
  0x001F, 0x0004, 0x000C, 0x0001,
         7,       -1,       -1, G_BOXTEXT ,   /* Object 6  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0001, 0x0802, 0x0003, 0x0001,
  AKTLAUFZEIT,       -1,       -1, G_BOXTEXT ,   /* Object 7  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0044, 0x0802, 0x0404, 0x0001,
  AKTERWARTZEIT,       -1,       -1, G_FTEXT   ,   /* Object 8 AKTLAUFZEIT */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0001, 0x0004, 0x0012, 0x0001,
         0,       -1,       -1, G_FTEXT   ,   /* Object 9 AKTERWARTZEIT */
  EDITABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[6],
  0x0436, 0x0004, 0x0012, 0x0001,
  
  /******** Tree 2 ZEIT ****************************************************/
        -1, ZEIZEIT , ZEIZEIT , G_BOX     ,   /* Object 0  */
  NONE, NORMAL, (LONG)0x00FF1101L,
  0x0000, 0x0000, 0x000D, 0x0001,
         0,       -1,       -1, G_FBOXTEXT,   /* Object 1 ZEIZEIT */
  SELECTABLE|DEFAULT|EXIT|EDITABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[7],
  0x0000, 0x0000, 0x000D, 0x0001,
  
  /******** Tree 3 SAVE ****************************************************/
        -1,        1, SAVSAVE , G_BOX     ,   /* Object 0  */
  NONE, OUTLINED|WHITEBAK, (LONG)0x00001100L,
  0x0000, 0x0000, 0x043F, 0x0808,
  SAVFILE ,       -1,       -1, G_STRING  ,   /* Object 1  */
  NONE, NORMAL, (LONG)"Die Datei",
  0x0401, 0x0800, 0x0009, 0x0001,
         3,       -1,       -1, G_BOXTEXT ,   /* Object 2 SAVFILE */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0401, 0x0802, 0x003D, 0x0001,
  SAVHELP ,       -1,       -1, G_STRING  ,   /* Object 3  */
  NONE, NORMAL, (LONG)"wurde ge\204ndert aber noch nicht gespeichert.",
  0x0401, 0x0804, 0x002B, 0x0001,
  SAVCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 4 SAVHELP */
  SELECTABLE, WHITEBAK, (LONG)"~Hilfe",
  0x0001, 0x0006, 0x0008, 0x0002,
  SAVSAVEAS,       -1,       -1, G_BUTTON  ,   /* Object 5 SAVCANCEL */
  SELECTABLE|EXIT, WHITEBAK, (LONG)"~Abbruch",
  0x040D, 0x0006, 0x0008, 0x0002,
  SAVSAVE ,       -1,       -1, G_BUTTON  ,   /* Object 6 SAVSAVEAS */
  SELECTABLE|EXIT, WHITEBAK, (LONG)"Speichern ~unter ...",
  0x041A, 0x0006, 0x0015, 0x0002,
         0,       -1,       -1, G_BUTTON  ,   /* Object 7 SAVSAVE */
  SELECTABLE|DEFAULT|EXIT|LASTOB, WHITEBAK, (LONG)"~Speichern",
  0x0434, 0x0006, 0x000A, 0x0002,
  
  /******** Tree 4 LOGIN ****************************************************/
        -1, LOGHELP , LOGCLEAR, G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021121L,
  0x0000, 0x0000, 0x0028, 0x0809,
  LOGOK   ,       -1,       -1, G_BUTTON  ,   /* Object 1 LOGHELP */
  SELECTABLE, WHITEBAK, (LONG)"~Hilfe",
  0x0001, 0x0806, 0x0008, 0x0002,
  LOGCANCEL,       -1,       -1, G_BUTTON  ,   /* Object 2 LOGOK */
  SELECTABLE|DEFAULT|EXIT, WHITEBAK, (LONG)"~OK",
  0x041E, 0x0806, 0x0008, 0x0002,
         4,       -1,       -1, G_BUTTON  ,   /* Object 3 LOGCANCEL */
  SELECTABLE|EXIT, WHITEBAK, (LONG)"~Abbruch",
  0x0414, 0x0806, 0x0008, 0x0002,
  LOGCLEAR,        5, LOGPASSWORD, G_BOX     ,   /* Object 4  */
  NONE, NORMAL, (LONG)0x00FF1140L,
  0x0404, 0x0001, 0x041E, 0x0804,
  LOGUSERNAME,       -1,       -1, G_STRING  ,   /* Object 5  */
  NONE, NORMAL, (LONG)"~Username:",
  0x0404, 0x0001, 0x000A, 0x0001,
         7,       -1,       -1, G_FBOXTEXT,   /* Object 6 LOGUSERNAME */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[9],
  0x040F, 0x0001, 0x0009, 0x0001,
  LOGPASSWORD,       -1,       -1, G_STRING  ,   /* Object 7  */
  NONE, NORMAL, (LONG)"~Passwort:",
  0x0404, 0x0003, 0x000A, 0x0001,
         4,       -1,       -1, G_FBOXTEXT,   /* Object 8 LOGPASSWORD */
  EDITABLE, NORMAL, (LONG)&rs_tedinfo[10],
  0x040F, 0x0003, 0x0009, 0x0001,
         0,       -1,       -1, G_BUTTON  ,   /* Object 9 LOGCLEAR */
  SELECTABLE|LASTOB, WHITEBAK, (LONG)"~L\224schen",
  0x040A, 0x0806, 0x0008, 0x0002,
  
  /******** Tree 5 DPARAM ****************************************************/
        -1,        1,       10, G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0035, 0x000B,
         2,       -1,       -1, G_BOXTEXT ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0002, 0x0001, 0x0031, 0x0002,
         3,       -1,       -1, G_BUTTON  ,   /* Object 2  */
  TOUCHEXIT, NORMAL, (LONG)"+",
  0x001E, 0x0004, 0x0004, 0x0001,
         4,       -1,       -1, G_BUTTON  ,   /* Object 3  */
  TOUCHEXIT, NORMAL, (LONG)"-",
  0x0013, 0x0004, 0x0004, 0x0001,
         5,       -1,       -1, G_BUTTON  ,   /* Object 4  */
  SELECTABLE|EXIT, NORMAL, (LONG)"Abbruch",
  0x0007, 0x0008, 0x000A, 0x0002,
         6,       -1,       -1, G_BUTTON  ,   /* Object 5  */
  SELECTABLE|DEFAULT|EXIT, NORMAL, (LONG)"OK",
  0x0026, 0x0008, 0x000A, 0x0002,
         8,        7,        7, G_BOX     ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00FF1121L,
  0x0006, 0x0006, 0x0028, 0x0001,
         6,       -1,       -1, G_BOXTEXT ,   /* Object 7  */
  TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[12],
  0x0012, 0x0000, 0x0005, 0x0001,
         9,       -1,       -1, G_FBOXTEXT,   /* Object 8  */
  EDITABLE|TOUCHEXIT, NORMAL, (LONG)&rs_tedinfo[13],
  0x0017, 0x0004, 0x0007, 0x0001,
        10,       -1,       -1, G_BOXTEXT ,   /* Object 9  */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0002, 0x0006, 0x0004, 0x0001,
         0,       -1,       -1, G_BOXTEXT ,   /* Object 10  */
  LASTOB, NORMAL, (LONG)&rs_tedinfo[15],
  0x002E, 0x0006, 0x0004, 0x0001,
  
  /******** Tree 6 DTEXT ****************************************************/
        -1,        1,        4, G_BOX     ,   /* Object 0  */
  NONE, OUTLINED, (LONG)0x00021100L,
  0x0000, 0x0000, 0x0027, 0x000C,
         2,       -1,       -1, G_BOXTEXT ,   /* Object 1  */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0006, 0x0001, 0x001B, 0x0002,
         3,       -1,       -1, G_BUTTON  ,   /* Object 2  */
  SELECTABLE|EXIT, NORMAL, (LONG)"Abbruch",
  0x0005, 0x000A, 0x0008, 0x0001,
         4,       -1,       -1, G_BUTTON  ,   /* Object 3  */
  SELECTABLE|DEFAULT|EXIT, NORMAL, (LONG)"OK",
  0x001A, 0x000A, 0x0008, 0x0001,
         0,       -1,       -1, G_FTEXT   ,   /* Object 4  */
  EDITABLE|LASTOB, NORMAL, (LONG)&rs_tedinfo[17],
  0x0003, 0x0006, 0x0022, 0x0001
};

static OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 ZAHL     */
  &rs_object[2],   /* Tree  1 AKTSTAT  */
  &rs_object[12],   /* Tree  2 ZEIT     */
  &rs_object[14],   /* Tree  3 SAVE     */
  &rs_object[22],   /* Tree  4 LOGIN    */
  &rs_object[32],   /* Tree  5 DPARAM   */
  &rs_object[43]    /* Tree  6 DTEXT    */
};
