/*****************************************************************************/
/*                                                                           */
/* Modul: DISK.C                                                             */
/* Datum: 27/11/94                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- printer, edit, meta, clipboard, image und trash rausgenommen
06.07.90
*****************************************************************************/

#include "import.h"
#include "global.h"
#include "windows.h"

#include "realtim4.h"
#include "errors.h"

#include "dialog.h"
#include "edit.h"
#include "meta.h"
#include "image.h"

#include "export.h"
#include "disk.h"

/****** DEFINES **************************************************************/

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

/*****************************************************************************/
/* Kreieren eines Fensters                                                   */
/*****************************************************************************/

GLOBAL WINDOWP crt_disk (obj, menu, icon)
OBJECT *obj, *menu;
WORD   icon;

{
  return (NULL);                        /* Fenster zurÅckgeben */
} /* crt_disk */

/*****************************************************************************/
/* ôffnen des Objekts                                                        */
/*****************************************************************************/

GLOBAL BOOLEAN open_disk (icon)
WORD icon;

{
  BOOLEAN ok;
  STR128  filename;
  BYTE    ext [4];

  ok = TRUE;

  if (select_file (NULL, NULL, NULL, "", filename))
  {
    file_split (filename, NULL, NULL, NULL, ext);

/*
    if (strcmp (ext, "IMG") == 0)
      ok = open_image (NIL, filename);
    else
      if (strcmp (ext, "GEM") == 0)
        ok = open_meta (NIL, filename);
      else
        ok = open_edit (NIL, filename);
*/
  } /* if */

  return (ok);
} /* open_disk */

/*****************************************************************************/
/* Info des Objekts                                                          */
/*****************************************************************************/

GLOBAL BOOLEAN info_disk (window, icon)
WINDOWP window;
WORD    icon;

{
  hndl_alert (ERR_INFDISK);
  return (TRUE);
} /* info_disk */

/*****************************************************************************/
/* Hilfe des Objekts                                                         */
/*****************************************************************************/

GLOBAL BOOLEAN help_disk (window, icon)
WINDOWP window;
WORD    icon;

{
  hndl_alert (ERR_HELPDISK);
  return (TRUE);
} /* help_disk */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOLEAN init_disk ()

{
  return (TRUE);
} /* init_disk */

/*****************************************************************************/
/* Terminieren des Moduls                                                    */
/*****************************************************************************/

GLOBAL BOOLEAN term_disk ()

{
  return (TRUE);
} /* term_disk */

