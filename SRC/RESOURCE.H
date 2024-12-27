/*****************************************************************************/
/*                                                                           */
/* Modul: RESOURCE.H                                                         */
/* Datum: 04.01.95                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef __RESOURCE__
#define __RESOURCE__

/****** DEFINES **************************************************************/

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

GLOBAL OBJECT *popup;
GLOBAL OBJECT *userimg;
GLOBAL OBJECT *icons;
GLOBAL OBJECT *settings;
GLOBAL OBJECT *settinghelp;
GLOBAL OBJECT *anzmenu;
GLOBAL OBJECT *infmeta;
GLOBAL OBJECT *alert;
GLOBAL OBJECT *selfont;
GLOBAL OBJECT *fonthelp;
GLOBAL OBJECT *clipmenu;

/****** FUNCTIONS ************************************************************/

GLOBAL VOID    fix_objs      _((OBJECT *tree, BOOLEAN is_dialog));
GLOBAL BOOLEAN init_resource _((VOID));
GLOBAL BOOLEAN term_resource _((VOID));

#endif /* __RESOURCE__ */

