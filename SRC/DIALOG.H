/*****************************************************************************/
/*                                                                           */
/* Modul: DIALOG.H                                                           */
/* Datum: 23/05/90                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef __DIALOG__
#define __DIALOG__

/****** DEFINES **************************************************************/

#define CLASS_DIALOG  3                 /* Klasse Dialogfenster */

#define NUM_SEP       5                 /* Anzahl Separatoren */
#define SEP_OPEN      '['               /* Zeichen fÅr Separator offen */
#define SEP_CLOSE     ']'               /* Zeichen fÅr Separator geschlossen */
#define SEP_LINE      '|'               /* Zeichen fÅr Zeilentrenner */

/****** TYPES ****************************************************************/

typedef BOOLEAN (*HELPFUNC) _((BYTE *helpmsg));

/****** VARIABLES ************************************************************/
GLOBAL BYTE     **alert_msgs;    /* Zeiger auf Fehlermeldungen */

/****** FUNCTIONS ************************************************************/

GLOBAL BOOLEAN init_dialog  _((BYTE **alerts, OBJECT *tree, WORD index, BYTE *title));
GLOBAL BOOLEAN term_dialog  _((VOID));

GLOBAL VOID    set_helpfunc _((HELPFUNC help));

GLOBAL WINDOWP crt_dialog   _((OBJECT *obj, OBJECT *menu, WORD icon, BYTE *title, UWORD flags));
GLOBAL BOOLEAN open_dialog  _((WORD icon));

GLOBAL WORD    hndl_alert   _((WORD alert_id));
GLOBAL WORD    open_alert   _((BYTE *alertmsg));

GLOBAL VOID    hndl_modal   _((BOOLEAN use_timer));

#endif /* __DIALOG__ */

