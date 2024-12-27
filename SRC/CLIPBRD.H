/*****************************************************************************/
/*                                                                           */
/* Modul: CLIPBRD.H                                                          */
/* Datum: 06/11/89                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef __CLIPBRD__
#define __CLIPBRD__

/****** DEFINES **************************************************************/

#define CLASS_CLIPBRD 17                /* Klasse Klemmbrettfenster */

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

GLOBAL WORD    scrap_read      _((BYTE *pscrap));
GLOBAL WORD    scrap_write     _((BYTE *pscrap));
GLOBAL WORD    scrap_clear     _((VOID));

GLOBAL VOID    get_clipxywh    _((WORD obj, RECT *border));
GLOBAL VOID    print_clipfiles _((WINDOWP window, SET objs));

GLOBAL BOOLEAN icons_clipbrd   _((WORD src_obj, WORD dest_obj));

GLOBAL WINDOWP crt_clipbrd     _((OBJECT *obj, OBJECT *menu, WORD icon));

GLOBAL BOOLEAN open_clipbrd    _((WORD icon));
GLOBAL BOOLEAN info_clipbrd    _((WINDOWP window, WORD icon));
GLOBAL BOOLEAN help_clipbrd    _((WINDOWP window, WORD icon));

GLOBAL BOOLEAN init_clipbrd    _((VOID));
GLOBAL BOOLEAN term_clipbrd    _((VOID));

#endif /* __CLIPBRD__ */

