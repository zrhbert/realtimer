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
 * Anm.     : Um diese Routinen benutzen zu dÅrfen, muû man im
 *            Besitz eines Interface Originals sein!
 *            
 * énderungen :  2 + Umbau auf einzelne Module 
 * Date     :  Oct 29th 1992
 * Author   :  Bertram Dunskus
 *
 * Anm.     : Um diese Routinen benutzen zu kînnen, muû man erst
 *            einiges am Original Ñndern!
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
extern RECT  desk;					/* Grîûe des Desktops             */
extern WORD   gl_wbox, gl_hbox;		/* Breite und Hîhe eines Zeichens */


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
	return (rs_gaddr (pglobal, re_gtype, re_gindex, re_gaddr));
}

/*****************************************************************************/

GLOBAL WORD xrsrc_saddr (WORD re_stype, WORD re_sindex, VOID *re_saddr)
{
	return (rs_sadd (pglobal, re_stype, re_sindex, re_saddr));
}

/*****************************************************************************/

GLOBAL WORD xrsrc_obfix (OBJECT *re_otree, WORD re_oobject)
{
	rs_obfix (re_otree, re_oobject);

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

	*re_gaddr = get_address (re_gtype, re_gindex);

	if (*re_gaddr == (OBJECT *)NULL)
		return (FALSE);

	return (TRUE);
}

/*****************************************************************************/

GLOBAL WORD rs_sadd (WORD *base, WORD rs_stype, WORD rs_sindex, OBJECT *re_saddr)
{
	OBJECT *old_addr;

	rs_sglobal (base);

	old_addr = get_address (rs_stype, rs_sindex);

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
			all_ptr.object = get_address(R_OBJECT, index);
			the_addr = &all_ptr.object->ob_spec;
			break;

		case R_TEPVALID:
		case R_TEPTMPLT:
			all_ptr.tedinfo = get_address(R_TEDINFO, index);
			if (type == R_TEPVALID)
				the_addr = &all_ptr.tedinfo->te_pvalid;
			else
				the_addr = &all_ptr.tedinfo->te_ptmplt;
			break;

		case R_IBPDATA:
		case R_IBPTEXT:
			all_ptr.iconblk = get_address(R_ICONBLK, index);
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
	
	/* énderung BD, vorher keine LONG-Multiplikation ! */
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

	strcpy (tmpnam, fname);

	if (!shel_find (tmpnam))
		return (FALSE);

	rs_global = global;

	if ((fh = Fopen (tmpnam, 0)) < 0)
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

	adr = get_sub (0, rs_hdr->rsh_trindex, sizeof (OBJECT *));

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
		obj = get_address (R_OBJECT, count);
		rs_obfix (obj, 0);
		if ((obj->ob_type & 0xff) != G_BOX && (obj->ob_type & 0xff) != G_IBOX && (obj->ob_type & 0xff) != G_BOXCHAR)
			fix_long ((LONG *)&obj->ob_spec);

		count--;
	}
}

/*****************************************************************************/

LOCAL VOID fix_tedinfo()
{
	LONG		count;
	TEDINFO *tedinfo;

	count = rs_hdr->rsh_nted - 1;

	while (count >= 0)
	{
		tedinfo = get_address (R_TEDINFO, count);

		if (fix_ptr (R_TEPTEXT, count))
			tedinfo->te_txtlen = strlen (tedinfo->te_ptext) + 1;

		if (fix_ptr (R_TEPTMPLT, count))
			tedinfo->te_tmplen = strlen (tedinfo->te_ptmplt) + 1;

		fix_ptr (R_TEPVALID, count);

		count--;
	}

	return;
}

/*****************************************************************************/

LOCAL VOID fix_nptr (LONG index, WORD ob_type)
{
	while (index >= 0)
		fix_long (get_address(ob_type, index--));
}

/*****************************************************************************/

LOCAL WORD fix_ptr (WORD type, LONG index)
{
	return (fix_long (get_address (type, index)));
}

/*****************************************************************************/

LOCAL WORD fix_long (LONG *lptr)
{
	LONG base;

	/* énderung BD : */
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
		ncoord *= (flag ? gl_hbox : gl_wbox);	/* gl_wbox, gl_hbox = Zeichenbreite, Zeichenhîhe in Pixel */

	if (((*pcoord >> 8) & 0xff) > 0x80)
		ncoord += (((*pcoord >> 8) & 0xff) | 0xff00);
	else
		ncoord += ((*pcoord >> 8) & 0xff);

	*pcoord = ncoord;
}
