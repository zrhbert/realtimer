/*****************************************************************************/
/*                                                                           */
/* Modul: MODULE.H                                                           */
/* Datum: 18/10/92                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef __MODULE__
#define __MODULE__

/****** DEFINES **************************************************************/

#define CLASS_MODULE 18                 /* Klasse Modulefenster */

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

GLOBAL BOOLEAN icons_module _((WORD src_obj, WORD dest_obj));

GLOBAL WINDOWP crt_module   _((OBJECT *obj, OBJECT *menu, WORD icon));

GLOBAL BOOLEAN open_module  _((WORD icon));
GLOBAL BOOLEAN info_module  _((WINDOWP window, WORD icon));
GLOBAL BOOLEAN help_module  _((WINDOWP window, WORD icon));

GLOBAL BOOLEAN init_module  _((VOID));
GLOBAL BOOLEAN term_module  _((VOID));

#endif /* __MODULE__ */

