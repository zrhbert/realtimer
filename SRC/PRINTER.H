/*****************************************************************************/
/*                                                                           */
/* Modul: PRINTER.H                                                          */
/* Datum: 21/07/89                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef __PRINTER__
#define __PRINTER__

/****** DEFINES **************************************************************/

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

GLOBAL BOOLEAN prn_ready    _((WORD port));
GLOBAL BOOLEAN prn_check    _((WORD port));
GLOBAL WINDOWP crt_printer  _((OBJECT *obj, OBJECT *menu, WORD icon));

GLOBAL BOOLEAN open_printer _((WORD icon));
GLOBAL BOOLEAN info_printer _((WINDOWP window, WORD icon));
GLOBAL BOOLEAN help_printer _((WINDOWP window, WORD icon));

GLOBAL BOOLEAN init_printer _((VOID));
GLOBAL BOOLEAN term_printer _((VOID));

#endif /* __PRINTER__ */

