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
  WORD title;                             /* Titel des Men�s */
  WORD item;                              /* Nummer des Men�s */
} FUNCINFO;

/****** VARIABLES ************************************************************/

GLOBAL BOOLEAN  menu_ok;                  /* Men� vorhanden ? */
GLOBAL BOOLEAN  menu_fits;                /* Men� pa�t in Men�zeile ? */
GLOBAL FUNCINFO funcmenus [MAX_FUNC];     /* Men�s auf den Funktionstasten */
GLOBAL SET      menus;                    /* W�hlbare Men�s vor Zustandswechsel */

/****** FUNCTIONS ************************************************************/

GLOBAL VOID    updt_menu _((WINDOWP window));
GLOBAL VOID    hndl_menu _((WINDOWP window, WORD title, WORD item));

GLOBAL VOID    mabout    _((WORD title));

GLOBAL BOOLEAN init_menu _((VOID));
GLOBAL BOOLEAN term_menu _((VOID));

#endif /* __MENU__ */

