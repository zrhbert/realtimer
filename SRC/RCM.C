/***************************************************************************/
/*                                                                         */
/* Modul: RCM.C                                                            */
/* Datum: 17/03/91                                                         */
/*                                                                         */
/***************************************************************************/

#include <portab.h>
#include <aes.h>

#include "rcm.h"

/****** DEFINES **************************************************************/

#define DESK 0

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

/***************************************************************************/
/* Vorw„rtsreferenzen                                                      */
/***************************************************************************/

LOCAL VOID fix_tree    _((WORD gl_hbox, WORD gl_wbox, WORD n_tree, OBJECT **rs_trindex, OBJECT *rs_object));
LOCAL VOID fix_tedinfo _((WORD object, OBJECT *rs_object, TEDINFO *rs_tedinfo, BYTE **rs_strings));
LOCAL VOID fix_bitblk  _((WORD object, OBJECT *rs_object, BITBLK *rs_bitblk, RS_IMDOPE *rs_imdope));
LOCAL VOID fix_string  _((WORD object, OBJECT *rs_object, BYTE **rs_strings));
LOCAL VOID fix_iconblk _((WORD object, OBJECT *rs_object, ICONBLK *rs_iconblk, RS_IMDOPE *rs_imdope, BYTE **rs_strings));
LOCAL VOID fix_frimg   _((WORD object, LONG *rs_frimg, BITBLK *rs_bitblk, RS_IMDOPE *rs_imdope));

/***************************************************************************/

LOCAL VOID fix_tree (gl_wbox, gl_hbox, n_tree, rs_trindex, rs_object)
REG WORD   gl_wbox, gl_hbox, n_tree;
REG OBJECT **rs_trindex;
REG OBJECT *rs_object;

{
  REG WORD   tree;     /* index for trees */
  REG WORD   object;   /* index for objects */
  REG WORD   help;
  REG OBJECT *pobject, *p;
  WORD       xdesk, ydesk, wdesk, hdesk;

  wind_get (DESK, WF_WXYWH, &xdesk, &ydesk, &wdesk, &hdesk);

  for (tree = 0; tree < n_tree; tree++) /* fix trees */
  {
    help = (WORD)(LONG)rs_trindex [tree];
    rs_trindex [tree] = (OBJECT *)&rs_object [help];

    object  = 0;
    pobject = rs_trindex [tree];

    do
    {
      p            = &pobject [object];
      p->ob_x      = (p->ob_x & 0x00FF) * gl_wbox + ((p->ob_x >> 8) & 0x00FF);
      p->ob_y      = (p->ob_y & 0x00FF) * gl_hbox + ((p->ob_y >> 8) & 0x00FF);
      p->ob_width  = (p->ob_width & 0x00FF) * gl_wbox + ((p->ob_width >> 8) & 0x00FF);
      p->ob_height = (p->ob_height & 0x00FF) * gl_hbox + ((p->ob_height >> 8) & 0x00FF);

      if ((p->ob_x + p->ob_width) / gl_wbox == 80) p->ob_width = xdesk + wdesk - p->ob_x;
      if ((p->ob_y + p->ob_height) / gl_hbox == 25) p->ob_height = ydesk + hdesk - p->ob_y;
    } while (! (pobject [object++].ob_flags & LASTOB));
  } /* for */
} /* fix_tree */

/***************************************************************************/

LOCAL VOID fix_tedinfo (object, rs_object, rs_tedinfo, rs_strings)
REG WORD    object;
REG OBJECT  *rs_object;
REG TEDINFO *rs_tedinfo;
REG BYTE    **rs_strings;

{
  REG WORD index;

  index                      = (WORD)rs_object [object].ob_spec;
  rs_object [object].ob_spec = (LONG)&rs_tedinfo [index];

  rs_tedinfo [index].te_ptext  = rs_strings [(WORD)(LONG)rs_tedinfo [index].te_ptext];
  rs_tedinfo [index].te_ptmplt = rs_strings [(WORD)(LONG)rs_tedinfo [index].te_ptmplt];
  rs_tedinfo [index].te_pvalid = rs_strings [(WORD)(LONG)rs_tedinfo [index].te_pvalid];
} /* fix_tedinfo */

/***************************************************************************/

LOCAL VOID fix_bitblk (object, rs_object, rs_bitblk, rs_imdope)
REG WORD      object;
REG OBJECT    *rs_object;
REG BITBLK    *rs_bitblk;
REG RS_IMDOPE *rs_imdope;

{
  REG WORD index1;
  REG WORD index2;

  index1 = (WORD)rs_object [object].ob_spec;
  index2 = (WORD)rs_bitblk [index1].bi_pdata;

  rs_bitblk [index1].bi_pdata = rs_imdope [index2].image;
  rs_object [object].ob_spec  = (LONG)&rs_bitblk [index1];
} /* fix_bitblk */

/***************************************************************************/

LOCAL VOID fix_string (object, rs_object, rs_strings)
REG WORD    object;
REG OBJECT  *rs_object;
REG BYTE    **rs_strings;

{
  rs_object [object].ob_spec = (LONG)rs_strings [(WORD)rs_object [object].ob_spec];
} /* fix_string */

/***************************************************************************/

LOCAL VOID fix_iconblk (object, rs_object, rs_iconblk, rs_imdope, rs_strings)
REG WORD      object;
REG OBJECT    *rs_object;
REG ICONBLK   *rs_iconblk;
REG RS_IMDOPE *rs_imdope;
REG BYTE      **rs_strings;

{
  REG WORD index1;
  REG WORD index2;

  index1 = (WORD)rs_object [object].ob_spec;

  index2                       = (WORD)(LONG)rs_iconblk [index1].ib_pmask;
  rs_iconblk [index1].ib_pmask = rs_imdope [index2].image;

  index2                       = (WORD)(LONG)rs_iconblk [index1].ib_pdata;
  rs_iconblk [index1].ib_pdata = rs_imdope [index2].image;

  index2                       = (WORD)(LONG)rs_iconblk [index1].ib_ptext;
  rs_iconblk [index1].ib_ptext = rs_strings [index2];

  rs_object [object].ob_spec = (LONG)&rs_iconblk [index1];
} /* fix_iconblk */

/***************************************************************************/

LOCAL VOID fix_frimg (object, rs_frimg, rs_bitblk, rs_imdope)
REG WORD      object;
REG LONG      *rs_frimg;
REG BITBLK    *rs_bitblk;
REG RS_IMDOPE *rs_imdope;

{
  REG WORD index1;
  REG WORD index2;

  index1 = (WORD)rs_frimg [object];
  index2 = (WORD)(LONG)rs_bitblk [index1].bi_pdata;

  rs_bitblk [index1].bi_pdata = rs_imdope [index2].image;
  rs_frimg [object]           = (LONG)&rs_bitblk [index1];
} /* fix_frimg */

/***************************************************************************/

GLOBAL VOID rsc_create (gl_wbox, gl_hbox, n_tree, n_obs, n_frstr, n_frimg,
                        rs_strings, rs_frstr, rs_bitblk, rs_frimg, rs_iconblk,
                        rs_tedinfo, rs_object, rs_trindex, rs_imdope)
WORD      gl_wbox, gl_hbox;
WORD      n_tree, n_obs, n_frstr, n_frimg;
BYTE      **rs_strings;
LONG      *rs_frstr;
BITBLK    *rs_bitblk;
LONG      *rs_frimg;
ICONBLK   *rs_iconblk;
TEDINFO   *rs_tedinfo;
OBJECT    *rs_object;
OBJECT    **rs_trindex;
RS_IMDOPE *rs_imdope;

{
  REG WORD object; /* index for objects */

  fix_tree (gl_wbox, gl_hbox, n_tree, rs_trindex, rs_object);

  for (object = 0; object < n_obs; object ++) /* fix objects */
  {
    switch (rs_object [object].ob_type & 0xFF)
    {
      case G_BOX      : break;
      case G_TEXT     : fix_tedinfo (object, rs_object, rs_tedinfo, rs_strings);            break;
      case G_BOXTEXT  : fix_tedinfo (object, rs_object, rs_tedinfo, rs_strings);            break;
      case G_IMAGE    : fix_bitblk (object, rs_object, rs_bitblk, rs_imdope);               break;
      case G_PROGDEF  : break;
      case G_IBOX     : break;
      case G_BUTTON   : fix_string (object, rs_object, rs_strings);                         break;
      case G_BOXCHAR  : break;
      case G_STRING   : fix_string (object, rs_object, rs_strings); break;
      case G_FTEXT    : fix_tedinfo (object, rs_object, rs_tedinfo, rs_strings);            break;
      case G_FBOXTEXT : fix_tedinfo (object, rs_object, rs_tedinfo, rs_strings);            break;
      case G_ICON     : fix_iconblk (object, rs_object, rs_iconblk, rs_imdope, rs_strings); break;
      case G_TITLE    : fix_string (object, rs_object, rs_strings);                         break;
    } /* switch */
  } /* for */

  for (object = 0; object < n_frstr; object++) rs_frstr [object] = (LONG)rs_strings [rs_frstr [object]]; /* fix free strings */
  for (object = 0; object < n_frimg; object++) fix_frimg (object, rs_frimg, rs_bitblk, rs_imdope); /* fix free images */
} /* rsc_create */

/***************************************************************************/

