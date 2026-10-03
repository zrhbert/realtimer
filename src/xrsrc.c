/******************************************************************************
 * XRSRC.C
 *
 *			Extended Resource-Manager. RSC-Files can now have up to
 *			4294967295 bytes length.
 *			You can modify this source to handle more than one RSC-File
 *			by calling the MLOCAL-Functions
 *
 *				rs_load(pglobal, re_lpfname);
 *				rs_free(pglobal);
 *				rs_gaddr(pglobal, re_gtype, re_gindex, re_gaddr);
 *				rs_sadd(pglobal, re_stype, re_sindex, re_saddr);
 *
 *			with an integer-pointer to a 15 int array which will be
 *			handled as single global-arrays for each RSC-File.
 *
 *			This Source is copyrighted material by
 *					Oliver Groeger
 *					Graf-Konrad-Str.25
 *					8000 Munich 40
 *					Germany
 *
 * Version  :  1.00
 * Date     :  Aug 15th 1991
 * Author   :  Oliver Groeger
 *
 * Anm.     : Um diese Routinen benutzen zu drfen, muž man im
 *            Besitz eines Interface Originals sein!
 *            
 * Žnderungen :  2 + Umbau auf einzelne Module 
 * Date     :  Oct 29th 1992
 * Author   :  Bertram Dunskus
 *
 * Anm.     : Um diese Routinen benutzen zu k”nnen, muž man erst
 *            einiges am Original „ndern!
 *            
 ******************************************************************************/
#include "import.h"
#include "global.h"

#include "export.h"
#include "xrsrc.h"


/****** VARIABLES ************************************************************/

LOCAL WORD		pglobal[15];
LOCAL WORD		*rs_global;
LOCAL RSXHDR  *rs_hdr;
LOCAL RSXHDR	hdr_buf;

/* Extern deklarierte Variablen */
extern RECT  desk;					/* Gr”že des Desktops             */
extern WORD   gl_wbox, gl_hbox;		/* Breite und H”he eines Zeichens */


/****** FUNCTIONS ************************************************************/

LOCAL VOID rs_obfix      _((OBJECT *rs_otree, WORD rs_oobject));
LOCAL VOID rs_sglobal    _((WORD *base));
LOCAL VOID *get_address  _((WORD type, WORD index));
LOCAL VOID *get_sub      _((WORD index, LONG offset, WORD size));
LOCAL WORD rs_read       _((WORD *global, CONST BYTE *fname));
LOCAL VOID rs_fixindex   _((WORD *global));
LOCAL VOID do_rsfix      _((RSXHDR *rs_hdr, ULONG rs_size));
LOCAL VOID fix_treeindex _((VOID));
LOCAL VOID fix_object    _((VOID));
LOCAL VOID fix_tedinfo   _((VOID));
LOCAL VOID fix_nptr      _((LONG index, WORD ob_type));
LOCAL WORD fix_ptr       _((WORD type, LONG index));
LOCAL WORD fix_long      _((LONG *lptr));
LOCAL VOID fix_chp       _((WORD *pcoord, WORD flag));
LOCAL WORD fix_long      _((LONG *lptr));
GLOBAL WORD rs_gaddr (WORD *base, WORD re_gtype, WORD re_gindex, OBJECT **re_gaddr);
GLOBAL WORD rs_sadd (WORD *base, WORD rs_stype, WORD rs_sindex, OBJECT *re_saddr);

/*****************************************************************************/

GLOBAL WORD xrsrc_load (CONST BYTE *re_lpfname)
{
	return (rs_load (pglobal, re_lpfname));
}

/*****************************************************************************/

GLOBAL WORD xrsrc_free (VOID)
{
	return (rs_free (pglobal));
}

/*****************************************************************************/

GLOBAL WORD xrsrc_gaddr (WORD re_gtype, WORD re_gindex, VOID *re_gaddr)
{
	return (rs_gaddr (pglobal, re_gtype, re_gindex, (OBJECT **)re_gaddr));
}

/*****************************************************************************/

GLOBAL WORD xrsrc_saddr (WORD re_stype, WORD re_sindex, VOID *re_saddr)
{
	return (rs_sadd (pglobal, re_stype, re_sindex, (OBJECT *)re_saddr));
}

/*****************************************************************************/

LOCAL BYTE *rs_dupstr (BYTE *s)
{
	/* Private, writable copy of a resource string */
	BYTE *copy;

	if (s == NULL) return (NULL);
	copy = (BYTE *) mem_alloc ((LONG) strlen (s) + 1);
	if (copy == NULL) return (s);
	strcpy (copy, s);
	return (copy);
} /* rs_dupstr */

GLOBAL WORD xrsrc_obfix (OBJECT *re_otree, WORD re_oobject)
{
	/* Fix one object of a compiled-in resource (.RSH): convert the
		character coordinates and give the object its own string storage.
		The .RSH initialises many objects with identical placeholder
		literals (e.g. all lines of the alert box); the compiler merges
		those into a single read-only string, so writing one object's text
		would change them all. */
	OBJECT	*obj = &re_otree [re_oobject];
	TEDINFO	*ted;
	ICONBLK	*icon;

	rs_obfix (re_otree, re_oobject);

	switch (obj->ob_type & 0xFF)
	{
		case G_STRING:
		case G_BUTTON:
		case G_TITLE:
			obj->ob_spec.free_string = rs_dupstr (obj->ob_spec.free_string);
			break;
		case G_TEXT:
		case G_BOXTEXT:
		case G_FTEXT:
		case G_FBOXTEXT:
			ted = obj->ob_spec.tedinfo;
			ted->te_ptext  = rs_dupstr (ted->te_ptext);
			ted->te_ptmplt = rs_dupstr (ted->te_ptmplt);
			ted->te_pvalid = rs_dupstr (ted->te_pvalid);
			break;
		case G_ICON:
			icon = obj->ob_spec.iconblk;
			icon->ib_ptext = rs_dupstr (icon->ib_ptext);
			break;
	} /* switch */

	return (TRUE);
}

/*****************************************************************************/

LOCAL VOID rs_obfix (OBJECT *rs_otree, WORD rs_oobject)
{
	WORD *coord;
	WORD tmp = FALSE;
	WORD count = 0;

	coord = &rs_otree[rs_oobject].ob_x;

	while (count++ < 4)
	{
		fix_chp (coord++, tmp);
		tmp = tmp ? FALSE : TRUE;
	}

	return;
}

/*****************************************************************************/

LOCAL VOID rs_sglobal (WORD *base)
{
	rs_global = base;
	rs_hdr = (RSXHDR *)*(LONG *)&rs_global[7];

	return;
}

/*****************************************************************************/

GLOBAL WORD rs_free (WORD *base)
{
	rs_global = base;

	mem_free ((RSXHDR *)*(LONG *)&rs_global[7]);
/*	if (mem_free ((VOID *) rs_hdr))
		return (FALSE); */

	return (TRUE);
}

/*****************************************************************************/

GLOBAL WORD rs_gaddr (WORD *base, WORD re_gtype, WORD re_gindex, OBJECT **re_gaddr)
{
	rs_sglobal (base);

	*re_gaddr = (OBJECT *) get_address (re_gtype, re_gindex);

	if (*re_gaddr == (OBJECT *)NULL)
		return (FALSE);

	return (TRUE);
}

/*****************************************************************************/

GLOBAL WORD rs_sadd (WORD *base, WORD rs_stype, WORD rs_sindex, OBJECT *re_saddr)
{
	OBJECT *old_addr;

	rs_sglobal (base);

	old_addr = (OBJECT *) get_address (rs_stype, rs_sindex);

	if (old_addr == (OBJECT *)NULL)
		return (FALSE);

	*old_addr = *re_saddr;

	return (TRUE);
}

/*****************************************************************************/

GLOBAL WORD rs_load (WORD *global, CONST BYTE *fname)
{
	if (!rs_read (global, fname))
		return (FALSE);

	rs_fixindex (global);

	return (TRUE);
}

/*****************************************************************************/

LOCAL VOID *get_address (WORD type, WORD index)
{
	VOID *the_addr = (VOID *)NULL;
	union
	{
		VOID		*dummy;
		BYTE		*string;
		OBJECT	**dpobject;
		OBJECT	*object;
		TEDINFO	*tedinfo;
		ICONBLK	*iconblk;
		BITBLK	*bitblk;
	} all_ptr;

	switch (type)
	{
		case R_TREE:
			all_ptr.dpobject = (OBJECT **)(*(long **)&rs_global[5]);
			the_addr = all_ptr.dpobject[index];
			break;

		case R_OBJECT:
			the_addr = get_sub (index, rs_hdr->rsh_object, sizeof(OBJECT));
			break;

		case R_TEDINFO:
		case R_TEPTEXT:
			the_addr = get_sub (index, rs_hdr->rsh_tedinfo, sizeof(TEDINFO));
			break;

		case R_ICONBLK:
		case R_IBPMASK:
			the_addr = get_sub (index, rs_hdr->rsh_iconblk, sizeof(ICONBLK));
			break;

		case R_BITBLK:
		case R_BIPDATA:
			the_addr = get_sub (index, rs_hdr->rsh_bitblk, sizeof(BITBLK));
			break;

		case R_OBSPEC:
			all_ptr.object = (OBJECT *)get_address(R_OBJECT, index);
			the_addr = &all_ptr.object->ob_spec;
			break;

		case R_TEPVALID:
		case R_TEPTMPLT:
			all_ptr.tedinfo = (TEDINFO *)get_address(R_TEDINFO, index);
			if (type == R_TEPVALID)
				the_addr = &all_ptr.tedinfo->te_pvalid;
			else
				the_addr = &all_ptr.tedinfo->te_ptmplt;
			break;

		case R_IBPDATA:
		case R_IBPTEXT:
            all_ptr.iconblk = (ICONBLK *)get_address(R_ICONBLK, index);
			if (type == R_IBPDATA)
				the_addr = &all_ptr.iconblk->ib_pdata;
			else
				the_addr = &all_ptr.iconblk->ib_ptext;
			break;

		case R_STRING:
			the_addr = get_sub (index, rs_hdr->rsh_frstr, sizeof (BYTE *));
			the_addr = (VOID *)*(BYTE *)the_addr;
			break;

		case R_IMAGEDATA:
			the_addr = get_sub (index, rs_hdr->rsh_imdata, sizeof (BYTE *));
			the_addr = (VOID *)*(BYTE *)the_addr;
			break;

		case R_FRIMG:
			the_addr = get_sub (index, rs_hdr->rsh_frimg, sizeof (BYTE *));
			the_addr = (VOID *)*(BYTE *)the_addr;
			break;

		case R_FRSTR:
			the_addr = get_sub (index, rs_hdr->rsh_frstr, sizeof (BYTE *));
			break;
	}

	return (the_addr);
}

/*****************************************************************************/

LOCAL VOID *get_sub (WORD index, LONG offset, WORD size)
{
	UBYTE *ptr = (UBYTE *)rs_hdr;
	
	/* Žnderung BD, vorher keine LONG-Multiplikation ! */
	LONG size_temp, index_temp;
	
	size_temp  = (LONG) size;
	index_temp = (LONG) index;
	
	ptr += offset;
	ptr += (index_temp * size_temp);

	return ((VOID *)ptr);
}

/*****************************************************************************/

LOCAL WORD rs_read (WORD *global, CONST BYTE *fname)
{
	WORD fh;
	BYTE tmpnam[128];

	strcpy ((char*)tmpnam, (char*)fname);

	if (!shel_find (tmpnam))
		return (FALSE);

	rs_global = global;

	if ((fh = Fopen ((char*)tmpnam, 0)) < 0)
		return (FALSE);

	if (Fread (fh, sizeof(RSXHDR), &hdr_buf) != sizeof (RSXHDR))
	{
		Fclose (fh);
		return (FALSE);
	}
	if ((rs_hdr = (RSXHDR *)mem_alloc (hdr_buf.rsh_rssize)) == NULL)
	{
		Fclose (fh);
		return (FALSE);
	}

	Fseek (0L, fh, 0);

	if (Fread (fh, hdr_buf.rsh_rssize, rs_hdr) != hdr_buf.rsh_rssize)
	{
		Fclose (fh);
		return (FALSE);
	}

	do_rsfix (rs_hdr, hdr_buf.rsh_rssize);

	Fclose (fh);

	return (TRUE);
}

/*****************************************************************************/

LOCAL VOID rs_fixindex (WORD *global)
{
	rs_sglobal (global);

	fix_object ();
}

/*****************************************************************************/

LOCAL VOID do_rsfix (RSXHDR *hdr, ULONG size)
{
	rs_global[7] = ((LONG)hdr >> 16) & 0xFFFF;
	rs_global[8] = (LONG)hdr & 0xFFFF;
	rs_global[9] = size;

	fix_treeindex ();
	fix_tedinfo ();

	fix_nptr (hdr->rsh_nib - 1, R_IBPMASK);
	fix_nptr (hdr->rsh_nib - 1, R_IBPDATA);
	fix_nptr (hdr->rsh_nib - 1, R_IBPTEXT);

	fix_nptr (rs_hdr->rsh_nbb - 1, R_BIPDATA);
	fix_nptr (rs_hdr->rsh_nstring - 1, R_FRSTR);
	fix_nptr (rs_hdr->rsh_nimages - 1, R_FRIMG);
}

/*****************************************************************************/

LOCAL VOID fix_treeindex (VOID)
{
	OBJECT **adr;
	LONG   count;

	count = rs_hdr->rsh_ntree - 1L;

	adr = (OBJECT **)get_sub (0, rs_hdr->rsh_trindex, sizeof (OBJECT *));

	rs_global[5] = ((LONG)adr >> 16) & 0xFFFF;
	rs_global[6] = (LONG)adr & 0xFFFF;

	while (count >= 0)
	{
		fix_long ((LONG *)(count * sizeof (OBJECT *) + (LONG)adr));
		count--;
	}
}

/*****************************************************************************/

LOCAL VOID fix_object (VOID)
{
	WORD 	 count;
	OBJECT *obj;

	count = rs_hdr->rsh_nobs - 1;

	while (count >= 0)
	{
		obj = (OBJECT *)get_address (R_OBJECT, count);
		rs_obfix (obj, 0);
		if ((obj->ob_type & 0xff) != G_BOX && (obj->ob_type & 0xff) != G_IBOX && (obj->ob_type & 0xff) != G_BOXCHAR)
			fix_long ((LONG *)&obj->ob_spec);

		count--;
	}
}

/*****************************************************************************/

LOCAL VOID fix_tedinfo(VOID)
{
	LONG		count;
	TEDINFO *tedinfo;

	count = rs_hdr->rsh_nted - 1;

	while (count >= 0)
	{
		tedinfo = (TEDINFO *)get_address (R_TEDINFO, count);

		if (fix_ptr (R_TEPTEXT, count))
			tedinfo->te_txtlen = strlen ((char*)tedinfo->te_ptext) + 1;

		if (fix_ptr (R_TEPTMPLT, count))
			tedinfo->te_tmplen = strlen ((char*)tedinfo->te_ptmplt) + 1;

		fix_ptr (R_TEPVALID, count);

		count--;
	}

	return;
}

/*****************************************************************************/

LOCAL VOID fix_nptr (LONG index, WORD ob_type)
{
	while (index >= 0)
		fix_long ((LONG*) get_address(ob_type, index--));
}

/*****************************************************************************/

LOCAL WORD fix_ptr (WORD type, LONG index)
{
	return (fix_long ((LONG*) get_address (type, index)));
}

/*****************************************************************************/

LOCAL WORD fix_long (LONG *lptr)
{
	LONG base;

	/* Žnderung BD : */
	if (lptr == 0L)
		return (FALSE);
		/* damit kein Zugriff auf 0 entsteht ! */

	base = *lptr;
	if (base == 0L)
		return (FALSE);

	base += (LONG)rs_hdr;

	*lptr = base;

	return (TRUE);
}

/*****************************************************************************/

LOCAL VOID fix_chp (WORD *pcoord, WORD flag)
{
	WORD ncoord;

	ncoord = *pcoord & 0xff;

	if (!flag && ncoord == 0x50)
		ncoord = desk.w;										/* desk.g_w = Breite des Bildschirms in Pixel */
	else 
		ncoord *= (flag ? gl_hbox : gl_wbox);	/* gl_wbox, gl_hbox = Zeichenbreite, Zeichenh”he in Pixel */

	if (((*pcoord >> 8) & 0xff) > 0x80)
		ncoord += (((*pcoord >> 8) & 0xff) | 0xff00);
	else
		ncoord += ((*pcoord >> 8) & 0xff);

	*pcoord = ncoord;
}
