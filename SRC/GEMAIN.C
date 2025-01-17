/*****************************************************************************/
/* Skelettprogramm zur Programmierung unter GEM                              */
/*                                                                           */
/* Version [2.2]                                                             */
/* von Dieter & Jrgen Gei                                                  */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Modul: GEMAIN.C                                                           */
/* Datum: 05/06/89                                                           */
/*                                                                           */
/*****************************************************************************/

#include "import.h"
#include "global.h"

#include "initerm.h"
#include "event.h"

#include "export.h"
#include "gemain.h"


/****** DEFINES **************************************************************/

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

#if GEMDOS
#if MW_C
LONG _stksize = 12288;            /* 12 KBytes Stack fr Mark Williams C */
#endif
#endif

#if MSDOS
#if TURBO_C
UWORD _stklen = 12288;            /* 12 KBytes Stack fr Turbo C */
#endif
#endif

/****** FUNCTIONS ************************************************************/

#if GEM & XGEM

GLOBAL WORD GEMAIN ()
{
  if (init_initerm (0, NULL)) hndl_events ();    /* Alles ok => multi */
  term_initerm ();                               /* Terminierungsschritte */
  return (0);                                    /* Exit-Code (kein Fehler) */
} /* GEMAIN */

#else

/*****************************************************************************/

GLOBAL WORD main (argc, argv)
INT  argc;
char *argv [];

{

  if (init_initerm (0, NULL)) hndl_events (); /* Alles ok => multi */
  term_initerm ();                               /* Terminierungsschritte */
  return (0);                                    /* Exit-Code (kein Fehler) */
} /* main */

#endif
