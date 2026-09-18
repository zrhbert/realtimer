/*****************************************************************************/
/*                                                                           */
/* Modul: MENU.H                                                             */
/* Datum: 06/07/93                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef __MENU__
#define __MENU__

/****** DEFINES **************************************************************/

/****** TYPES ****************************************************************/

typedef struct
{
  WORD title;                             /* Titel des MenÅs */
  WORD item;                              /* Nummer des MenÅs */
} FUNCINFO;

/****** VARIABLES ************************************************************/

GLOBAL BOOLEAN  menu_ok;                  /* MenÅ vorhanden ? */
GLOBAL BOOLEAN  menu_fits;                /* MenÅ paût in MenÅzeile ? */
GLOBAL FUNCINFO funcmenus [MAX_FUNC];     /* MenÅs auf den Funktionstasten */
GLOBAL SET      menus;                    /* WÑhlbare MenÅs vor Zustandswechsel */

/****** FUNCTIONS ************************************************************/

GLOBAL VOID    updt_menu _((WINDOWP window));
GLOBAL VOID    hndl_menu _((WINDOWP window, WORD title, WORD item));

GLOBAL VOID    mabout    _((WORD title));

GLOBAL BOOLEAN init_menu _((VOID));
GLOBAL BOOLEAN term_menu _((VOID));

#endif /* __MENU__ */

