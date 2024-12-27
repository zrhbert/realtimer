/*****************************************************************************/
/*                                                                           */
/* Modul: EDIT.H                                                             */
/* Datum: 06/11/89                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef __EDIT__
#define __EDIT__

/****** DEFINES **************************************************************/

#define CLASS_EDIT    12                /* Klasse Editierfenster */

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

GLOBAL VOID    print_edit _((BYTE *filename));
GLOBAL WINDOWP crt_edit   _((OBJECT *obj, OBJECT *menu, WORD icon, BYTE *filename));

GLOBAL BOOLEAN open_edit  _((WORD icon, BYTE *filename));
GLOBAL BOOLEAN info_edit  _((WINDOWP window, WORD icon));
GLOBAL BOOLEAN help_edit  _((WINDOWP window, WORD icon));

GLOBAL BOOLEAN init_edit  _((VOID));
GLOBAL BOOLEAN term_edit  _((VOID));

#endif /* __EDIT__ */

