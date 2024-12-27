/*****************************************************************************/
/*                                                                           */
/* Modul: DISPOBJ.C                                                         */
/*                                                                           */
/* Display-Objects                                                           */
/*****************************************************************************/

#define DISPOBJVERSION "V 0.05"
#define DISPOBJDATE "20.02.95"

/*****************************************************************************
V 0.05
- var und text fÅr Bar, 20.02.95
- ClickPushObject eingebaut, 18.02.95
- Quadro repariert, 09.02.95
- Bug in KeyValueObject beseitigt, 03.02.95
- Update-Bug in ClickValueObject beseitigt, 03.02.95
- Bar modifiziert, 15.01.95
- bug in ClickTimeObject beseitigt, 08.01.95
- delay in ClickValueObject und ClickTimeObject eingefÅhrt
- dzahl in ClickTimeObject eingebaut, 23.12.94
- Setup-Class in dsetup auf CLASS_xxx umgestellt
- Volume in Space eingebaut
- Bug in öbernahme der display Flags bereinigt
- KeyValueObject modifiziert und eingebaut
V 0.04 28.08.94
- modified some FLOATS to INTs
V 0.03
- added GEM-Object functions
- Restructured objects to automatically take values from the VAR's
V 0.02
- Bar eingebaut
- set_area eingebaut
*****************************************************************************/

#ifndef XRSC_CREATE
/* #define XRSC_CREATE TRUE                    /* X-Resource-File im Code */ */
#endif

#include "import.h"
#include "global.h"
#include "windows.h"
#include "math.h"

#include "realtspc.h"
#include "objects.h"		/* wg. var_module */
#include "var.h"			/* wg. var_get_value */
#include "ed4.h"			/* wg. soundobjekten */
#include <msh_unit.h>		/* Deklarationen fÅr MidiShare Library */
#include "msh.h"			/* wg. Time-Funktionen */
#include "ext.h"			/* wg. delay() */

#include "export.h"
#include "dispobj.h"

/****** DEFINES **************************************************************/
#define DMAXDOKOOR 2*MAXDOKOOR
#define DMAXKOOR 2*MAXKOOR

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/
LOCAL CONST LONG	aspect_x  = 120L,		/* Default-Werte fÅr Space */
						aspect_y  =-120L;		/* Verzerrung x/y/z */
LOCAL CONST LONG	persp 	 = 12L;		/* Perspektive */

LOCAL POLYGON spaceform[NumSpaceForms][2];

/****** PROTOTYPES ***********************************************************/

LOCAL DISPOBJP CreateDispobj (WINDOWP parent, RECT *area);
LOCAL VOID	DeleteDispobj	(DISPOBJP dispobj);
LOCAL DISPOBJP	CopyDispobj	(DISPOBJP dispobj);
LOCAL VOID	MessageDispobj	(DISPOBJP module, WORD type, VOID *msg);
LOCAL	VOID ResetDispobj 	(DISPOBJP dispobj);
LOCAL	VOID TextDefaultDispobj (DISPOBJP dispobj);
LOCAL BOOL KeyDispobj 		(DISPOBJP dispobj, MKINFO *mk);
LOCAL VOID MovedDispobj 	(DISPOBJP dispobj);
LOCAL	BOOL SetWorkDispobj	(DISPOBJP dispobj, RECT *work);
LOCAL	BOOL SetRotDispobj	(DISPOBJP dispobj, POS_3DP rotation);
LOCAL	BOOL SetTextDispobj 	(DISPOBJP dispobj, INT id, CHAR *text);
LOCAL	BOOL SetRangeDispobj	(DISPOBJP dispobj, INT id, INT range);
LOCAL	BOOL SetTypeDispobj 	(DISPOBJP dispobj, INT id, INT type);
LOCAL	BOOL SetSizeDispobj 	(DISPOBJP dispobj, INT id, INT size);
LOCAL	BOOL SetVarDispobj	(DISPOBJP dispobj, INT id, UINT var, BOOL reactive);
LOCAL	BOOL SetUniDispobj	(DISPOBJP dispobj, INT id, LONG uni);
LOCAL VOID StartDispobj		(DISPOBJP dispobj);
LOCAL VOID Start1Dispobj 	(DISPOBJP dispobj);
LOCAL VOID LineDispobj (DISPOBJP dispobj, POS_2DP p1, POS_2DP p2);

LOCAL VOID	MessageObject		(DISPOBJP dispobj, WORD type, VOID *msg);
LOCAL BOOL	SetObject			(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer);
LOCAL BOOL	SetWorkObject 		(DISPOBJP dispobj, RECT *work);
LOCAL VOID	MovedObject		 	(DISPOBJP dispobj);
LOCAL VOID	RecalcWorkObject	(DISPOBJP dispobj);
LOCAL BOOL 	KeyValueObject 	(DISPOBJP dispobj, MKINFO *mk);
LOCAL BOOL 	KeyCheckObject 	(DISPOBJP dispobj, MKINFO *mk);
LOCAL BOOL 	KeyPushObject 		(DISPOBJP dispobj, MKINFO *mk);
LOCAL VOID  StartValueObject	(DISPOBJP dispobj);
LOCAL VOID	StartTimeObject	(DISPOBJP dispobj);
LOCAL VOID  StartCheckObject	(DISPOBJP dispobj);
LOCAL VOID	FinishObject		(DISPOBJP dispobj);
LOCAL VOID	DrawObject			(DISPOBJP dispobj);
LOCAL VOID  TextDefaultObject (DISPOBJP dispobj);
LOCAL VOID	ClickValueObject	(DISPOBJP dispobj, MKINFO *mk);
LOCAL VOID	ClickTimeObject	(DISPOBJP dispobj, MKINFO *mk);
LOCAL VOID	ClickCheckObject	(DISPOBJP dispobj, MKINFO *mk);
LOCAL VOID	ClickPushObject	(DISPOBJP dispobj, MKINFO *mk);
LOCAL VOID	TimerObject			(DISPOBJP dispobj);

LOCAL BOOL	SetBar		(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer);
LOCAL BOOL	SetUniBar	(DISPOBJP dispobj, INT id, LONG uni);
LOCAL VOID	ResetBar		(DISPOBJP dispobj);
LOCAL VOID	StartBar		(DISPOBJP dispobj);
LOCAL VOID	FinishBar	(DISPOBJP dispobj);
LOCAL VOID 	DrawBar		(DISPOBJP dispobj);
LOCAL VOID	line_1d		(DISPOBJP dispobj, POS_1D point1, POS_1D point2);
LOCAL POINT_2D ProjBar 	(DISPOBJP dispobj, POS_1D point);

LOCAL BOOL	SetText		(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer);
LOCAL VOID	StartText	(DISPOBJP dispobj);
LOCAL VOID	StartCMIText	(DISPOBJP dispobj);
LOCAL VOID	FinishText	(DISPOBJP dispobj);
LOCAL VOID	DrawText		(DISPOBJP dispobj);
LOCAL VOID	TextDefaultText (DISPOBJP dispobj);

LOCAL BOOL	SetSO		(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer);
LOCAL	BOOL	SetWorkSO(DISPOBJP dispobj, RECT *work);
LOCAL BOOL	SetUniSO	(DISPOBJP dispobj, INT id, LONG uni);
LOCAL BOOL	SetTypeSO(DISPOBJP dispobj, INT id, INT type);
LOCAL VOID	ResetSO	(DISPOBJP dispobj);
LOCAL VOID	TextDefaultSO (DISPOBJP dispobj);
LOCAL VOID	StartSO	(DISPOBJP dispobj);
LOCAL VOID	FinishSO	(DISPOBJP dispobj);
LOCAL VOID 	DrawSO	(DISPOBJP dispobj);

LOCAL VOID	CreateArea	(DISPOBJP dispobj, WORD type, WORD mode);
LOCAL VOID 	DrawArea		(DISPOBJP dispobj);

LOCAL VOID	DeleteSpace	(DISPOBJP dispobj);
LOCAL BOOL	SetSpace		(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer);
LOCAL	BOOL	SetWorkSpace(DISPOBJP dispobj, RECT *work);
LOCAL BOOL	SetUniSpace	(DISPOBJP dispobj, INT id, LONG uni);
LOCAL BOOL	SetTypeSpace(DISPOBJP dispobj, INT id, INT type);
LOCAL BOOL	SetTypeCMOSpace(DISPOBJP dispobj, INT id, INT type);
LOCAL VOID	ResetSpace	(DISPOBJP dispobj);
LOCAL VOID	TextDefaultSpace (DISPOBJP dispobj);
LOCAL VOID	StartSpace	(DISPOBJP dispobj);
LOCAL VOID	StartCMOSpace	(DISPOBJP dispobj);
LOCAL VOID	FinishSpace	(DISPOBJP dispobj);
LOCAL VOID 	DrawSpace	(DISPOBJP dispobj);
LOCAL VOID 	DrawED4Space	(DISPOBJP dispobj);
LOCAL VOID DrawED4Cursors (DISPOBJP dispobj);
LOCAL VOID VolSpace 		(DISPOBJP dispobj);

LOCAL	BOOL compute_persp_lookups (DISPOBJP dispobj);

LOCAL  ProjectFn Project3D;
LOCAL  ProjectFn Project3DNeu;

LOCAL  VOID draw_polygon	(DISPOBJP dispobj, POLY_P poly);
LOCAL  VOID line_3d 			(DISPOBJP dispobj, POS_3D *point1, POS_3D *point2);
LOCAL  VOID text_3d 			(DISPOBJP dispobj, POS_3D *point, STRING text);
LOCAL  VOID multiline_3d	(DISPOBJP dispobj, POS_3D *point[], WORD points);
LOCAL  VOID crosshairs		(DISPOBJP dispobj);

LOCAL  VOID mono				(DISPOBJP dispobj);
LOCAL  VOID stereo			(DISPOBJP dispobj);
LOCAL  VOID quadro			(DISPOBJP dispobj);
LOCAL  VOID okto				(DISPOBJP dispobj);

LOCAL  VOID wuerfel_innen 	(POLY_P space);
LOCAL  VOID arrows			(DISPOBJP dispobj);

LOCAL VOID time_str			(STRING timestr, LONG time);
LOCAL LONG str_time			(STRING timestr);

LOCAL  VOID define_forms 			(VOID);
LOCAL  VOID define_sechskanal		(VOID);
LOCAL  VOID define_wuerfel			(VOID);
LOCAL  VOID define_wuerfel_hoch	(VOID);
LOCAL  VOID define_wuerfel_lang	(VOID);
LOCAL  VOID define_wuerfel_mitte	(VOID);
LOCAL  VOID define_tetraeder		(VOID);
LOCAL  VOID define_quadrophon		(VOID);

LOCAL VOID SetEdgeF (EDGE_P e, WORD from, WORD to, WORD style, WORD begin, WORD end, WORD color, WORD width);

/****** FUNCTIONS ************************************************************/

LOCAL DISPOBJP CreateDispobj (WINDOWP parent, RECT *work)
{
	/* Create a new Dispobj object, initialize default functions */
	
	DISPOBJP dispobj;
	
	dispobj = new (DISPOBJ);
	mem_set (dispobj, 0, (UWORD) sizeof (DISPOBJ));
	 
	dispobj->parent	= parent;
	dispobj->status	= new(DISPOBJSTATUS);
	dispobj->message	= MessageDispobj;
	/* dispobj->key	 	= KeyDispobj; */
	dispobj->reset 	= ResetDispobj;
	dispobj->moved 	= MovedDispobj;
	dispobj->delete 	= DeleteDispobj;
	dispobj->copy 		= CopyDispobj;
	dispobj->text_default = TextDefaultDispobj;
	dispobj->set_work = SetWorkDispobj;
	dispobj->set_text = SetTextDispobj;
	dispobj->set_range= SetRangeDispobj;
	dispobj->set_size = SetSizeDispobj;
	dispobj->set_type = SetTypeDispobj;
	dispobj->set_var  = SetVarDispobj;
	dispobj->set_uni  = SetUniDispobj;
	
	mem_set (dispobj->status, 0, (UWORD) sizeof (DISPOBJSTATUS));

	/* Define the object's workspace */
	dispobj->set_work (dispobj, work);

	return dispobj;	
} /* CreateDispobj */

LOCAL VOID	DeleteDispobj	(DISPOBJP dispobj)
{
	mem_free(dispobj->status);
	mem_free(dispobj);
	
} /* DeleteDispobj */

LOCAL DISPOBJP	CopyDispobj	(DISPOBJP dispobj)
{
	DISPOBJP new;
	
	new = new(DISPOBJ);
	new->status	= new(DISPOBJSTATUS);

	/* Kopieren der Information */
	*new = *dispobj;
	*new->status = *dispobj->status;
	
	return new;

} /* CopyDispobj */

LOCAL VOID	MessageDispobj	(DISPOBJP dispobj, WORD type, VOID *msg)
{
	UWORD 	variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	WORD		x;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			/* Find variable slot */
			for (x = 0; x < MAXPOS; x++) {
				if (dispobj->var[x] == variable)
					if (value != dispobj->var_value[x])
					{
						dispobj->var_value[x] = value;
						dispobj->new = TRUE;
						/* Request redraw */
						dispobj->parent->milli = 1;
					} /* if */
			} /* for */
			break;
	} /* switch type */
} /* message */

LOCAL BOOL KeyDispobj (DISPOBJP dispobj, MKINFO *mk)
{
	return FALSE;
} /* KeyDispobj */

LOCAL VOID ResetDispobj (DISPOBJP dispobj)
{
	dispobj->new = TRUE;
} /* ResetDispobj */

LOCAL VOID MovedDispobj (DISPOBJP dispobj)
{
} /* MovedDispobj */

LOCAL VOID TextDefaultDispobj (DISPOBJP dispobj)
{
	text_default (vdi_handle);
} /* TextDefaultDispobj */

LOCAL	BOOL SetWorkDispobj (DISPOBJP dispobj, RECT *work)
{
	dispobj->work = *work;
	dispobj->new	= TRUE;	

	return TRUE;
} /* SetWorkDispobj */

LOCAL	BOOL SetTextDispobj (DISPOBJP dispobj, INT id, CHAR *text)
{
	/* Return old memory */
	if (dispobj->text[id]) mem_free (dispobj->text[id]);
	dispobj->text[id] = mem_alloc ((LONG)strlen(text)+1);

	/* Copy the new string into the new location */
	strcpy (dispobj->text[id], text);

	dispobj->new	= TRUE;	
	return TRUE;
} /* SetTextDispobj */

LOCAL	BOOL SetRangeDispobj (DISPOBJP dispobj, INT id, INT range)
{
	dispobj->range[id] = range;
	dispobj->new	= TRUE;	
	return TRUE;
} /* SetRangeDispobj */

LOCAL	BOOL SetSizeDispobj (DISPOBJP dispobj, INT id, INT size)
{
	dispobj->size[id] = size;
	dispobj->new	= TRUE;	
	return TRUE;
} /* SetSizeDispobj */

LOCAL	BOOL SetTypeDispobj (DISPOBJP dispobj, INT id, INT type)
{
	dispobj->type[id] = type;
	dispobj->new	= TRUE;	
	return TRUE;
} /* SetTypeDispobj */

LOCAL	BOOL SetVarDispobj (DISPOBJP dispobj, INT id, UINT var, BOOL reactive)
{
	dispobj->var[id] = var;

	if (reactive && (var > 0 && var < MAXSYSVARS))
		add_rcv (var, (RTMCLASSP) dispobj);

	return TRUE;
} /* SetVarDispobj */

LOCAL	BOOL SetUniDispobj (DISPOBJP dispobj, INT id, LONG uni)
{
	if (dispobj->uni[id]	!= uni)
	{
		dispobj->uni[id]	= uni;
		dispobj->new		= TRUE;	
	} /* if not equal */
	
	return TRUE;
} /* SetUniDispobj */

LOCAL	BOOL SetRotDispobj (DISPOBJP dispobj, POS_3DP rotation)
{
	dispobj->rotation	= *rotation;
	dispobj->new		= TRUE;	
	return TRUE;
} /* SetRotDispobj */

LOCAL VOID StartDispobj (DISPOBJP dispobj)
{
	REG WORD			x;	
	REG UINT			var;
	REG LONG			val;
	
	for (x = 0; x < MAXPOS; x++)
	{
		if (dispobj->var[x] > 0)
		{
			/* Update VAR value */
			if (dispobj->text[x])
			{
				val = var_get_value(var_module, dispobj->var[x]);
				if (val != dispobj->var_value[x])
				{
					dispobj->var_value[x] = val;
					dispobj->new = TRUE;
				} /* if */
			} /* if */
		} /* if */
	} /* for */
	
} /* StartDispobj */

LOCAL VOID Start1Dispobj (DISPOBJP dispobj)
{
	REG LONG			val;

	/* Start, for single-var objects */
	if (dispobj->var[0] > 0)
	{
		/* Update VAR value */
		if (dispobj->text[0])
		{
			val = var_get_value(var_module, dispobj->var[0]);
			if (val != dispobj->var_value[0])
			{
				dispobj->var_value[0] = val;
				dispobj->new = TRUE;
			} /* if */
		} /* if */
	} /* if */
	
} /* Start1Dispobj */

LOCAL  VOID LineDispobj (DISPOBJP dispobj, POS_2DP p1, POS_2DP p2)
{
	SPACESTATP	status = &dispobj->status->space;
	RECT			*work = &dispobj->work;
	WORD			xo = 	work->x, yo = work->y, w = work->w, h = work->h; 
	INT			pxyarray[4];
	
	/* Umrechnen von x und y auf Objektgroesse */
	
	pxyarray[0] = xo + (p1->x * w)/100;
	pxyarray[1] = yo + (p1->y * h)/100;

	pxyarray[2] = xo + (p2->x * w)/100;
	pxyarray[3] = yo + (p2->y * h)/100;
	
	v_pline( vdi_handle, 2, pxyarray );
} /* LineDispobj */

/****** GEM-Object ************************************************************/
GLOBAL DISPOBJP CreateObjectDispobj (WINDOWP parent, WORD object, UINT variable, WORD type, CHAR key)
{
	OBJECT *tree = parent->object;
	OBJECT *obj = &tree[object];
	RECT	 work;
	DISPOBJP dispobj;
	OBJECTSTATP	status;
	STRING s;

	if (tree)
	{
		work.x = obj->ob_x;
		work.y = obj->ob_y;
		work.w = obj->ob_width;
		work.h = obj->ob_height;
			
		dispobj = CreateDispobj (parent, &work);
			
		dispobj->draw			= DrawObject;
		dispobj->finished		= FinishObject;
		dispobj->timer			= TimerObject;
		dispobj->set_work		= SetWorkObject;
		dispobj->moved			= MovedObject;
		dispobj->message		= MessageObject;
		
		switch (type)
		{
			case ObjectTime:
				dispobj->start		= StartTimeObject;
				dispobj->click 	= ClickTimeObject;
				break;
			case ObjectValue:
				dispobj->start		= StartValueObject;
				dispobj->click 	= ClickValueObject;
				dispobj->key	 	= KeyValueObject;
				break;
			case ObjectCheck:
				dispobj->start		= StartCheckObject;
				dispobj->click 	= ClickCheckObject;
				if (key)
					dispobj->key	 	= KeyCheckObject;
				break;
			case ObjectPush:
				dispobj->start		= StartCheckObject;
				dispobj->click 	= ClickPushObject;
				if (key)
					dispobj->key	 	= KeyPushObject;
				break;
		} /* switch */

		dispobj->set_var (dispobj, 0, variable, TRUE);
		dispobj->set_text (dispobj, 0, "%ld");	
	 	status = &dispobj->status->object;

		if (object)
		{
			status->object =  object;
			status->key		=  key;
		}
		else
		{
			var_get_name (var_module, variable, s);
			printf ("Error, Window %s, VAR #%d '%s', Object = 0!\n", 
						dispobj->parent->name, variable, s);
		} /* else */
	} /* if */
	return dispobj;
} /* CreateObjectDispobj */

LOCAL VOID	MessageObject	(DISPOBJP dispobj, WORD type, VOID *msg)
{
	UWORD 	variable = ((MSG_SET_VAR *)msg)->variable;
	LONG		value		= ((MSG_SET_VAR *)msg)->value;
	
	switch(type)
	{
		case SET_VAR:				/* Systemvariable auf neuen Wert setzen */
			if (dispobj->var[0] == variable)
				if (value != dispobj->var_value[0])
				{
					dispobj->var_value[0] = value;
					dispobj->new = TRUE;
					/* Request redraw */
					dispobj->parent->milli = 1;
				} /* if */
			break;
	} /* switch type */
} /* MessageObject */

LOCAL BOOL SetWorkObject (DISPOBJP dispobj, RECT *work)
{
	SetWorkDispobj (dispobj, work);
	RecalcWorkObject (dispobj);
	
	return TRUE;
} /* SetWorkObject */

LOCAL VOID MovedObject	(DISPOBJP dispobj)
{
	RecalcWorkObject (dispobj);
} /* MovedObject */

LOCAL VOID RecalcWorkObject (DISPOBJP dispobj)
{
	OBJECTSTATP	status = &dispobj->status->object;
	RECT *work = &dispobj->work;
	
	objc_offset (dispobj->parent->object, status->object, &work->x, &work->y);
	
} /* RecalcWorkObject */

LOCAL BOOL KeyValueObject (DISPOBJP dispobj, MKINFO *mk)
{
	WINDOWP 		window = dispobj->parent;
	OBJECTSTATP	status = &dispobj->status->object;
	STRING		s, format = "%ld";
	LONG			x;
	
	/* Focus/Edit dieses Objektes ? */
	if (window->edit_obj == status->object)
	{
		switch (mk->scan_code)
		{
			case	UP	:
			case	LEFT	:
			case	RIGHT	:
			case	DOWN	:
			case	UNDO	:
			/* Alten Wert wieder Anzeigen */
				dispobj->new = TRUE;
				window->milli = 1;
				return TRUE;
			case	RETURN:
			case	ENTER	:
				/* Eingegebenen Wert rausschicken */
				GetPLong (window->object, status->object, &x);
				send_variable (dispobj->var[0], x);
				window->milli = 1;
				return TRUE;
			case	MINUS	:
				/* Wert um 1 verringern und rausschicken */
				GetPLong (window->object, status->object, &x);
				x--;
				send_variable (dispobj->var[0], x);
				window->milli = 1;
				return TRUE;
			case	PLUS	:
				/* Wert um 1 vergrîûern und rausschicken */
				GetPLong (window->object, status->object, &x);
				x++;
				send_variable (dispobj->var[0], x);
				window->milli = 1;
				return TRUE;
		} /* switch */
	} /* if edit_obj */
	
	window->milli = 1;

	return FALSE;
} /* KeyValueObject */

LOCAL BOOL KeyCheckObject (DISPOBJP dispobj, MKINFO *mk)
{
	WINDOWP 		window = dispobj->parent;
	OBJECTSTATP	status = &dispobj->status->object;
	LONG			x;
		
	if (mk->scan_code == status->key)
	{
		x = get_checkbox (window->object, status->object);
		/* Invert variable value */
		x = !x;
		set_checkbox (window->object, status->object, x);
		dispobj->new = TRUE;
		send_variable (dispobj->var[0], x);
		window->milli = 1;
		return TRUE;
	} /* if */

	return FALSE;
} /* KeyCheckDispobj */

LOCAL BOOL KeyPushObject (DISPOBJP dispobj, MKINFO *mk)
{
	WINDOWP 		window = dispobj->parent;
	OBJECTSTATP	status = &dispobj->status->object;
	LONG			x;
		
	if (mk->scan_code == status->key)
	{
		set_checkbox (window->object, status->object, TRUE);
		dispobj->new = TRUE;
		resend_variable (dispobj->var[0], TRUE);
		window->milli = 1;
		return TRUE;
	} /* if */

	return FALSE;
} /* KeyPushObject */

LOCAL VOID StartValueObject (DISPOBJP dispobj)
{
	OBJECTSTATP	status = &dispobj->status->object;
	OBJECT		*object = dispobj->parent->object;
	WORD			obj = status->object;
	
	/* Call the default function */
	Start1Dispobj (dispobj);
	
	if (dispobj->new)
	{
		/* Auffrischen der Objekt-Position */
		RecalcWorkObject (dispobj);
		/* Einsetzen des Textes */
		sprintf (status->text, dispobj->text[0], dispobj->var_value[0]);
		SetPText (object, obj, status->text);
	} /* if */
} /* StartValueObject */

LOCAL VOID StartTimeObject (DISPOBJP dispobj)
{
	OBJECTSTATP	status = &dispobj->status->object;
	OBJECT		*object = dispobj->parent->object;
	WORD			obj = status->object;
	
	/* Call the default function */
	Start1Dispobj (dispobj);
	
	if (dispobj->new)
	{
		/* Auffrischen der Objekt-Position */
		RecalcWorkObject (dispobj);
		
		/* Einsetzen des Textes */
		SetPTime (object, obj, dispobj->var_value[0]);
	} /* if */
} /* StartTimeObject */

LOCAL VOID StartCheckObject (DISPOBJP dispobj)
{
	OBJECTSTATP	status = &dispobj->status->object;
	
	/* Call the default function */
	Start1Dispobj (dispobj);
	
	if (dispobj->new)
	{
		/* Auffrischen der Objekt-Position */
		RecalcWorkObject (dispobj);

		set_checkbox (dispobj->parent->object, status->object, (BOOL)dispobj->var_value[0]);
	} /* if */
} /* StartCheckObject */

LOCAL VOID DrawObject (DISPOBJP dispobj)
{
	OBJECTSTATP	status = &dispobj->status->object;

	if (dispobj->new)
		draw_object (dispobj->parent, status->object);
} /* DrawObject */

LOCAL VOID TimerObject (DISPOBJP dispobj)
{
	OBJECTSTATP	status = &dispobj->status->object;

	/* Check if something is new */
	Start1Dispobj (dispobj);

	if (dispobj->new)
		redraw_window(dispobj->parent, &dispobj->work);
} /* TimerObject */

LOCAL VOID FinishObject (DISPOBJP dispobj)
{
	dispobj->new = FALSE;
} /* FinishObject */;

PRIVATE VOID ClickValueObject (DISPOBJP dispobj, MKINFO *mk)
{
	WINDOWP 		window = dispobj->parent;
	OBJECTSTATP	status = &dispobj->status->object;
	LONG			x, step;
	WORD			ret;
	BOOL 			repeat = FALSE;
		
	undo_state (dispobj->parent->object, status->object, SELECTED);
	GetPLong (window->object, status->object, &x);
	if (mk->breturn < 2)
	{
		do{
			step = 1;
			if (mk->alt)	step *= 10;
			if (mk->shift)	step *= 10;
			if (mk->ctrl)	step *= 10;
			if (mk->momask == 0x001)		/* linke Taste */
				x-= step;
			else if (mk->momask == 0x002)	/* rechte Taste */
				x+= step;
			dispobj->new = TRUE;
			send_variable (dispobj->var[0], x);
			timer_all(1);
			/* PrÅfen ob Taste gedrÅckt */
			mkstate(mk);
			if (mk->mobutton>0 && mk->momask>0)
				delay (150);
			/* PrÅfen ob Taste immernoch gedrÅckt */
			mkstate(mk);
			repeat =  (mk->mobutton>0 && mk->momask>0);
		} while (repeat);
	} /* if */
	else
	{
		dispobj->new = TRUE;
		resend_variable (dispobj->var[0], x);
	} /* else */

	window->milli = 1;
} /* ClickValueObject */

PRIVATE VOID ClickTimeObject (DISPOBJP dispobj, MKINFO *mk)
{
	WINDOWP 		window = dispobj->parent;
	OBJECTSTATP	status = &dispobj->status->object;
	LONG			x, step, unit;
	WORD			ret;
	STRING s;
	BOOL 			repeat = FALSE;
	
	GetPTime (window->object, status->object, &x);

	switch (window->exit_obj - status->object)
	{
		case 1:
			unit = 3600000L;	/* Stunden */
			break;
		case 2:
			unit = 60000L;		/* Minuten */
			break;
		case 3:
			unit = 1000;			/* Sekunden */
			break;
		case 4:
			unit = 1;				/* Milli-Sekunden */
			break;
		default:
			unit = 1;				/* */
	} /* switch */
	
	switch (mk->breturn)
	{
		case 0:
		case 1:
			undo_state (dispobj->parent->object, status->object, SELECTED);
			do{
				step = unit;
				if (mk->shift)	step = unit*10;
				if (mk->ctrl)	step = unit*100;
				if (mk->alt)	step = QUANT;
				if (mk->momask == 0x001)		/* linke Taste */
					x-= step;
				else if (mk->momask == 0x002)	/* rechte Taste */
					x+= step;
				dispobj->new = TRUE;
				send_variable (dispobj->var[0], x);
				dispobj->timer (dispobj);
				timer_all(1);
				/* PrÅfen ob Taste gedrÅckt */
				mkstate(mk);
				if (mk->mobutton>0 && mk->momask>0)
					delay (200);
				/* PrÅfen ob Taste immernoch gedrÅckt */
				mkstate(mk);
				repeat =  (mk->mobutton>0 && mk->momask>0);
			} while (repeat);
			break;
		case 2:	
			dzeit (&x, dispobj->work.x, dispobj->work.y, "SMPTE");
			undo_state (dispobj->parent->object, status->object, SELECTED);
			dispobj->new = TRUE;
			send_variable (dispobj->var[0], x);
			window->milli = 1;
			break;
	} /* switch */

} /* ClickTimeObject */

PRIVATE VOID ClickCheckObject (DISPOBJP dispobj, MKINFO *mk)
{
	WINDOWP 		window = dispobj->parent;
	OBJECTSTATP	status = &dispobj->status->object;
	BOOL			x;
		
	x = get_checkbox (window->object, status->object);

	dispobj->new = TRUE;
	send_variable (dispobj->var[0], x);

	window->milli = 1;
} /* ClickCheckObject */

PRIVATE VOID ClickPushObject (DISPOBJP dispobj, MKINFO *mk)
{
	WINDOWP 		window = dispobj->parent;
	OBJECTSTATP	status = &dispobj->status->object;
	BOOL			x;
		
	x = get_checkbox (window->object, status->object);

	/* Objekt kann nur eingedrÅckt werden */
	if (!x)
	{
		x = TRUE;
		set_checkbox (window->object, status->object, x);
	} 
	
	dispobj->new = TRUE;
	resend_variable (dispobj->var[0], x);

	window->milli = 1;
} /* ClickPushObject */

/****** Text ************************************************************/

GLOBAL DISPOBJP CreateTextDispobj (WINDOWP parent, WORD type, WORD mode, RECT *area, UWORD var, STRING text)
{
	DISPOBJP dispobj;

	/* Zusammenfassung der Create-Routinen */
	dispobj = CreateDispobj (parent, area);
	if (dispobj)
	{
		dispobj->set			= SetText;
		if (type == TextCMI)
			dispobj->start		= StartCMIText;
		else
			dispobj->start		= StartText;
		dispobj->draw			= DrawText;
		dispobj->finished		= FinishText;
		dispobj->text_default= TextDefaultText;

		dispobj->set_text (dispobj, 0, text);
		dispobj->set_var (dispobj, 0, var, FALSE);
	} /* if */
	return dispobj;
} /* CreateTextDispobj */

LOCAL BOOL	SetText	(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer)
{
	BOOL		ok = FALSE;

	switch (type)
	{
		default:	
			break;
	} /* switch type */

	dispobj->new |= ok;
	return ok;
} /* SetText */

LOCAL VOID StartText (DISPOBJP dispobj)
{
	TEXTSTATP	status = &dispobj->status->text;
	WORD			x;	
	
	/* Call the default function */
	StartDispobj (dispobj);
	
	if (dispobj->new)
	{
		for (x = 0; x < MAXPOS; x++)
		{
			/* Print VAR into Text */
			if (dispobj->text[x])
			{
				if (dispobj->var[x] > 0)
					sprintf (status->text[x], dispobj->text[x], dispobj->var_value[x]);
				else
					strcpy (status->text[x], dispobj->text[x]);
			} /* if */
		} /* for */
	} /* if */
} /* StartText */

LOCAL VOID StartCMIText (DISPOBJP dispobj)
{
	/* Modified to display only one value 
		and to add 1 to the variable in the display */
		
	TEXTSTATP	status = &dispobj->status->text;
	
	/* Call the default function */
	StartDispobj (dispobj);
	
	if (dispobj->new)
	{
		sprintf (status->text[0], dispobj->text[0], dispobj->var_value[0]+1);
	} /* if */
} /* StartCMIText */
	
LOCAL VOID FinishText (DISPOBJP dispobj)
{
	dispobj->new = FALSE;
} /* FinishText */;

LOCAL VOID DrawText (DISPOBJP dispobj)
{
	WORD			x;	
	POINT_2D		p;
	TEXTSTATP	status = &dispobj->status->text;
	LONGSTR		s;
	
	if (dispobj->new)
	{
		/* clr_area (&dispobj->work); */
		dispobj->text_default (dispobj);
	
		p.x = dispobj->parent->work.x + dispobj->work.x;
		p.y = dispobj->parent->work.y + dispobj->work.y;
	
		/* Concatenate all valid strings */
		sprintf (s, "");
		for (x = 0; x < MAXPOS; x++)
		{
			if (dispobj->text[x])
				strcat (s, status->text[x]);
		} /* for */

		/* Print the long string */
		v_gtext( vdi_handle, p.x, p.y, s);
	} /* if */

} /* DrawText */

LOCAL VOID TextDefaultText (DISPOBJP dispobj)
{
	WORD	fontsize, ret;

	/* Call standard function */
	TextDefaultDispobj (dispobj);

	if (desk.h > 400)
		fontsize = 14;
	else
		fontsize = 6;

	vst_height (vdi_handle, fontsize, &ret, &ret, &ret, &ret);
	
} /* TextDefaultText */

/****** Bar ************************************************************/

GLOBAL DISPOBJP CreateBarDispobj (WINDOWP parent, WORD type, WORD mode, RECT *area, UWORD var, STRING text)
{
	DISPOBJP		dispobj;
	SPACESTATP  status;
	RECT			work = {0,0,0,0};
		
	dispobj = CreateDispobj (parent, &work);
		
	status = &dispobj->status->space;
	if (dispobj)
	{
		dispobj->draw			= DrawBar;
		dispobj->start			= StartBar;
		dispobj->finished		= FinishBar;
		dispobj->set			= SetBar;
		dispobj->set_uni		= SetUniBar;
				
		/* Default Values */
		dispobj->set_uni (dispobj, DOUniRotationZ, mode);

		/* Jetzt erst die Groesse einstellen */
		dispobj->set_work (dispobj, area);
			
		dispobj->set_var (dispobj, 0, var, FALSE);
	} /* if dispobj */
	
	return dispobj;
} /* CreateBarDispobj */

LOCAL BOOL	SetBar	(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer)
{
	BOOL			ok = FALSE;
	BARSTATP  	status = &dispobj->status->bar;

	switch (type)
	{
		default:	
			break;
	} /* switch type */

	dispobj->new |= ok;

	return ok;
} /* SetBar */

LOCAL BOOL	SetUniBar	(DISPOBJP dispobj, INT id, LONG uni)
{
	BARSTATP  	status = &dispobj->status->bar;
	WORD			x;
	BOOL			ok = FALSE, flag;
	
	switch (id)
	{
		default:
			/* Continue with default function */
			ok = SetUniDispobj (dispobj, id, uni);
	} /* switch id */
	
	return ok;
} /* SetUniBar */

LOCAL VOID StartBar (DISPOBJP dispobj)
{
	BARSTATP		status = &dispobj->status->bar;
	
	/* Compute the % value from the absolute position and the range */
	status->position = var_get_relvalue(var_module, dispobj->var[0]) * MAXDOKOOR;
} /* StartBar */

LOCAL VOID DrawBar (DISPOBJP dispobj)
{
	BARSTATP		status = &dispobj->status->bar;
	BOOL			new = dispobj->new;
		
	if (new)
	{
		if (dispobj->reset) dispobj->reset (dispobj);
		clr_area (&dispobj->work);
		/* Text zeichnen  */
		/* DrawBarText (dispobj); */
	} /* if */

	line_default(vdi_handle);	

	/* Linien zeichnen  */
	vsf_interior (vdi_handle, FIS_SOLID);
	vsf_color (vdi_handle, WHITE);
	vsl_color (vdi_handle, WHITE);
	vsl_width (vdi_handle, 5);
	line_1d (dispobj, 10 + status->position*100/80, 90);
	vsf_color (vdi_handle, BLACK);
	vsl_color (vdi_handle, BLACK);
	line_1d (dispobj, 10, 10 + status->position*100/80);
	vsl_width (vdi_handle, 1);
	
} /* DrawBar */

LOCAL  POINT_2D ProjBar (DISPOBJP dispobj, POS_1D point)
{
	POINT_2D		proj;
	BARSTATP		status = &dispobj->status->bar;
	FLOAT 		rotation = dispobj->rotation.z;
	RECT			work = dispobj->work;
	WORD			xmin = work.x;
	WORD			ymin = work.y;
	WORD			xmax = work.x + work.w;
	WORD			ymax = work.y + work.h;

	/* Berechnung der Koordinaten im Fenster */
	proj.x = point * work.w * aspect_x / 2 * sin (M_PI*rotation/180);
	proj.y = point * work.h * aspect_y / 2 * cos (M_PI*rotation/180);
	
	proj.x += status->xoffset;
	proj.y += status->yoffset;

	/* Clipping */
	if (proj.x > xmax) proj.x = xmax;
	else if (proj.x < xmin) proj.x = xmin;

	if (proj.y > ymax) proj.y = ymax;
	else if (proj.y < ymin) proj.y = ymin;

	return proj;
} /* ProjBar */

LOCAL  VOID line_1d (DISPOBJP dispobj, POS_1D point1, POS_1D point2)
{
	POINT_2D		p1, p2;
	INT			pxyarray[4];
	BARSTATP		status = &dispobj->status->bar;
	WORD			width = dispobj->uni[DOSizeWidth];
	FLOAT 		rotation = dispobj->rotation.z;
	
	p1 = ProjBar (dispobj, point1);
	p2 = ProjBar (dispobj, point2);
	
	if (width == 1)
	{
		pxyarray[0] = p1.x;
		pxyarray[1] = p1.y;
		pxyarray[2] = p2.x;
		pxyarray[3] = p2.y;
		v_pline( vdi_handle, 2, pxyarray );
	} /* if width */
	else
	{
		/* Fette Balken nur bei glatten Winkeln */
		if (rotation == 0 || rotation == 180)
		{
			/* Horizontaler Balken */
			pxyarray[0] = p1.x;
			pxyarray[1] = p1.y - width/2;
			pxyarray[2] = p2.x;
			pxyarray[3] = p2.y + width/2;
		}
		else	if (rotation == 90 || rotation == 270)
		{
			/* Vertikaler Balken */
			pxyarray[0] = p1.x - width/2;
			pxyarray[1] = p1.y;
			pxyarray[2] = p2.x + width/2;
			pxyarray[3] = p2.y;
		}
		v_bar( vdi_handle, pxyarray );
	} /* else width */
		
} /* line_1d */

LOCAL VOID FinishBar (DISPOBJP dispobj)
{
	BARSTATP	status = &dispobj->status->bar;

	/* Neue Koordinaten Åbernehmen */
	mem_move(&status->pos_alt, &status->position,(UWORD)sizeof(POS_1D) * MAXPOS); 

	dispobj->new = FALSE;
} /* FinishBar */;

/****** Area ************************************************************/

LOCAL VOID CreateArea (DISPOBJP dispobj, WORD type, WORD mode)
{
	dispobj->draw = DrawArea;
} /* CreateArea */

LOCAL VOID DrawArea (DISPOBJP dispobj)
{
} /* DrawArea */

/****** SoundObjects ************************************************************/

GLOBAL DISPOBJP CreateSODispobj (WINDOWP parent, WORD type, WORD mode, RECT *area)
{
	DISPOBJP		dispobj;
	SOSTATP  status;
	RECT			work = {0,0,0,0};
		
	dispobj = CreateDispobj (parent, &work);
		
	status = &dispobj->status->sound;
	if (dispobj)
	{
		dispobj->draw			= DrawSO;
		dispobj->start			= StartSO;
		dispobj->finished		= FinishSO;
		dispobj->set			= SetSO;
		dispobj->set_work		= SetWorkSO;
		dispobj->set_uni		= SetUniSO;
		dispobj->set_type		= SetTypeSO;
		dispobj->reset			= ResetSO;
				
		/* Default Values */
			
	} /* if dispobj */
	
	return dispobj;
} /* CreateSODispobj */

LOCAL BOOL	SetSO	(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer)
{
	BOOL			ok = FALSE;
	SOSTATP 	status = &dispobj->status->sound;

	switch (type)
	{
		default:	
			break;
	} /* switch type */

	dispobj->new |= ok;

	return ok;
} /* SetSO */

LOCAL	BOOL SetWorkSO (DISPOBJP dispobj, RECT *work)
{
	SOSTATP  status = &dispobj->status->sound;

	/* Call the default function first */
	SetWorkDispobj (dispobj, work);

	if (work->h > 200)
		status->fontsize = 14;
	else if (work->h < 100)
		status->fontsize = 4;
	else
		status->fontsize = 6;
	
	return TRUE;
} /* SetWorkSO */

LOCAL BOOL	SetUniSO	(DISPOBJP dispobj, INT id, LONG uni)
{
	SOSTATP  	status = &dispobj->status->sound;
	CHAR 			*text = status->text;
	WORD			x;
	BOOL			ok = FALSE, flag;
	SO_P			event = (SO_P) uni;
	
	switch (id)
	{
		case DOUniSOEvent:
			/* Kopiere Objekt Info */
			status->event = *event;
			dispobj->new = TRUE;
			break;
	} /* switch id */
	
	return ok;
} /* SetUniSO */

LOCAL BOOL	SetTypeSO	(DISPOBJP dispobj, INT id, INT type)
{
	BOOL			ok = FALSE;
	SOSTATP  status = &dispobj->status->sound;
	
	switch (id)
	{
	}
	return ok;
} /* SetTypeSO */

LOCAL VOID ResetSO (DISPOBJP dispobj)
{
	SOSTATP		status = &dispobj->status->sound;
	
	dispobj->new = TRUE;
} /* ResetSO */

LOCAL VOID StartSO (DISPOBJP dispobj)
{
	SOSTATP		status = &dispobj->status->sound;
	SO_P			event = &status->event;
	WORD			signal;
	CHAR 			*text = status->text;
	LONGSTR 		s;
	
	/* Call the default function */
	StartDispobj (dispobj);
	
	timstr (event->cue_time, s);
	sprintf (text, "%s  ", s);
	
	timstr (event->entry_time, s);
	strcat (text, s);
	strcat (text, "  ");
	
	timstr (event->exit_time, s);
	strcat (text, s);
	strcat (text, "  ");
	
	sprintf (s, "%3d ", event->speed.z);
	strcat (text, s);

	sprintf (s, "%2d ", event->input_ch[0]+1);
	strcat (text, s);

	sprintf (s, "%3d ", event->volume);
	strcat (text, s);

	sprintf (s, "%4.0f %4.0f ",
				event->position[0].x,
				event->position[0].y);
	strcat (text, s);
	
} /* StartSO */

LOCAL VOID FinishSO (DISPOBJP dispobj)
{
	SOSTATP	status = &dispobj->status->sound;

	dispobj->new = FALSE;
} /* FinishSO */;

LOCAL VOID DrawSO (DISPOBJP dispobj)
{
	SOSTATP	status = &dispobj->status->sound;
	BOOL			new = dispobj->new;

	if (new)
	{
		dispobj->text_default (dispobj);

		/* Print the long string */
		v_gtext( vdi_handle, dispobj->work.x, dispobj->work.y, status->text);
	} /* if */

} /* DrawSO */

/****** Space ************************************************************/

GLOBAL DISPOBJP CreateSpaceDispobj (WINDOWP parent, WORD type, WORD mode, RECT *area)
{
	DISPOBJP		dispobj;
	SPACESTATP  status;
	RECT			work = {0,0,0,0};
		
	dispobj = CreateDispobj (parent, &work);
		
	status = &dispobj->status->space;
	if (dispobj)
	{
		dispobj->delete		= DeleteSpace;
		dispobj->draw			= DrawSpace;
		dispobj->start			= StartSpace;
		dispobj->finished		= FinishSpace;
		dispobj->set			= SetSpace;
		dispobj->set_work		= SetWorkSpace;
		dispobj->set_uni		= SetUniSpace;
		dispobj->set_type		= SetTypeSpace;
		dispobj->reset			= ResetSpace;
		dispobj->text_default = TextDefaultSpace;
		status->perspective	= persp;
		status->projection	= Project3D;
		status->numpoints 	= MAXPOS;
		status->base 			= 1;
		status->vol_draw 		= VolSpace; 
				
		/* Default Values */
		dispobj->set_uni (dispobj, DOUniSpaceInside, FALSE);
		dispobj->set_uni (dispobj, DOUniSpaceDisplay, 0x1FE);
		dispobj->set_uni (dispobj, DOUniSpaceCrosshair, 0x1);
		dispobj->set_uni (dispobj, DOUniSpaceArrows, FALSE);
		dispobj->set_uni (dispobj, DOUniSpaceNumbers, TRUE);
		dispobj->set_type (dispobj, DOTypeSpaceForm, WUERFEL);
		dispobj->set_type (dispobj, DOTypeSpaceMode, STEREO);

		status->shared_persp = (mode & SpaceModeSharedPersp);
		/* Lookuptables init */
		if (!status->shared_persp)
		{
			status->xwin   = mem_alloc (sizeof (WORD)*MAXDOKOOR*4);
			status->ywin   = mem_alloc (sizeof (WORD)*MAXDOKOOR*4);
			status->xpersp = mem_alloc (sizeof (WORD)*MAXDOKOOR*4);
			status->ypersp = mem_alloc (sizeof (WORD)*MAXDOKOOR*4);
			status->xzoff  = mem_alloc (sizeof (WORD)*MAXDOKOOR*4);
			status->yzoff  = mem_alloc (sizeof (WORD)*MAXDOKOOR*4);
		} 
			
		/* Lookup update verhindern */
		status->initializing = TRUE;
		dispobj->set_uni (dispobj, DOUniRotationX, 30);
		dispobj->set_uni (dispobj, DOUniRotationY, 20);
		dispobj->set_uni (dispobj, DOUniRotationZ, 200);
		/* Lookup update erlauben */
		status->initializing = FALSE;
		compute_persp_lookups (dispobj);
		
		/* Jetzt erst die Groesse einstellen */
		dispobj->set_work (dispobj, area);
			
	} /* if dispobj */
	
	return dispobj;
} /* CreateSpaceDispobj */

GLOBAL DISPOBJP CreateCMOSpaceDispobj (WINDOWP parent, WORD type, WORD mode, RECT *area, WORD input_base)
{
	DISPOBJP		dispobj;
	SPACESTATP  status;
	RECT			work = {0,0,0,0};
	
	/* Objekt auf Groesse 0 initialisieren */
	dispobj = CreateSpaceDispobj(parent, type, mode, &work);

	status = &dispobj->status->space;
	dispobj->start = StartCMOSpace;
	dispobj->set_type = SetTypeCMOSpace;
	/* Jetzt erst die Groesse einstellen */
	dispobj->set_work (dispobj, area);
	dispobj->set_uni (dispobj, DOUniSpaceInputBase, input_base);
	dispobj->set_uni (dispobj, DOUniSpaceNumbers, FALSE);
	dispobj->set_uni (dispobj, DOUniSpaceCrosshair, 0x0);
	dispobj->set_uni (dispobj, DOUniSpaceVolume, 0xFF);
	status->base = 0;
	return dispobj;
		
} /* CreateCMOSpaceDispobj */

GLOBAL DISPOBJP CreateED4SpaceDispobj (WINDOWP parent, WORD type, WORD mode, RECT *area)
{
	DISPOBJP		dispobj;
	SPACESTATP  status;
	RECT			work = {0,0,0,0};
	
	/* Objekt auf Groesse 0 initialisieren */
	dispobj = CreateSpaceDispobj(parent, type, mode, &work);

	dispobj->draw = DrawED4Space;
	dispobj->set_type (dispobj, DOTypeSpaceMode, MONO);
	
	return dispobj;
		
} /* CreateED4SpaceDispobj */

LOCAL VOID	DeleteSpace	(DISPOBJP dispobj)
{
	SPACESTATP 	status = &dispobj->status->space;

	if (!status->shared_persp)
	{
		mem_free (status->xpersp);
		mem_free (status->ypersp);
		mem_free (status->xzoff);
		mem_free (status->yzoff);
	} /* if not shared */

	DeleteDispobj (dispobj);
	
} /* DeleteSpace */

LOCAL BOOL	SetSpace	(DISPOBJP dispobj, WORD type, LONG index, VOID *pointer)
{
	BOOL			ok = FALSE;
	SPACESTATP 	status = &dispobj->status->space;

	switch (type)
	{
		default:	
			break;
	} /* switch type */

	dispobj->new |= ok;

	return ok;
} /* SetSpace */

LOCAL	BOOL compute_persp_lookups (DISPOBJP dispobj)
{
	SPACESTATP  status = &dispobj->status->space;
	WORD			*xpersp = status->xpersp,
					*ypersp = status->ypersp,
					*xzoff = status->xzoff,
					*yzoff = status->yzoff;
	REG WORD		koor, kindex;
	INT			zxkoeff = 70;	/* sin(44)*100 */
	INT			zykoeff = 71;	/* sin(44)*100 */
	INT			perspect;

	if (!status->shared_persp)
	{
		if (xpersp && ypersp && xzoff && yzoff)
		{
			zxkoeff = 100 * sin(dispobj->uni[DOUniRotationX]*M_PI/180);
			zykoeff = 100 * sin(dispobj->uni[DOUniRotationY]*M_PI/180);
			perspect= (WORD)dispobj->uni[DOUniSpacePerspective];
			
			for (koor = -DMAXDOKOOR; koor < DMAXDOKOOR; koor++)
			{
				kindex = koor+DMAXDOKOOR;
				xzoff[kindex]  = -((koor * zxkoeff)>>7);
				yzoff[kindex]  = -((koor * zykoeff)>>7);
				xpersp[kindex] = 50L + ((koor*perspect)>>7);
				/* ypersp[kindex] = 50L + ((koor*perspect)>>7); */
				ypersp[kindex] = xpersp[kindex];
			} /* for koor */
		} /* if not shared */
	} /* if not shared */

	return TRUE;
} /* compute_persp_lookups */

LOCAL	BOOL SetWorkSpace (DISPOBJP dispobj, RECT *work)
{
	SPACESTATP  status = &dispobj->status->space;
	WORD			*xwin   = status->xwin;
	WORD			*ywin   = status->ywin;
	LONG			koor;
	LONG			width = (LONG)work->w,
					height = (LONG)work->h;
	BOOL			new_w = width != dispobj->work.w;
	BOOL			new_h = height != dispobj->work.h;

	/* Call the default function first */
	SetWorkDispobj (dispobj, work);

	status->xoffset = work->x + width*0.50;
	status->yoffset = work->y + height*0.50;
	
	if (work->h > 200)
		status->fontsize = 14;
	else if (work->h < 100)
		status->fontsize = 4;
	else
		status->fontsize = 6;
	
	if (dispobj->work.h < 80)
		status->line_width = 1;
	else
		status->line_width = 1;		/* Immer 1, wg. GEM Zeichenproblemen */

	if (!status->shared_persp)
	{
		if (new_w)
			for (koor = -DMAXDOKOOR; koor < DMAXDOKOOR; koor++)
				xwin[koor+DMAXDOKOOR]  = (koor * width * aspect_x / 20000L);
		
		if (new_h)
			for (koor = -DMAXDOKOOR; koor < DMAXDOKOOR; koor++)
				ywin[koor+DMAXDOKOOR]  = (koor * height * aspect_y / 20000L);
	} /* if not shared */
	
	return TRUE;
} /* SetWorkDispobj */

LOCAL BOOL	SetUniSpace	(DISPOBJP dispobj, INT id, LONG uni)
{
	SPACESTATP  status = &dispobj->status->space;
	WORD			x;
	BOOL			ok = FALSE, flag;

	switch (id)
	{
		case DOUniSpaceArrows:
			status->arrows = uni;
			ok = TRUE;
			dispobj->new = TRUE;
			break;
		case DOUniSpaceInside:
			if (status->inside != uni) {
				status->inside = uni;
				dispobj->new = TRUE;
			} /* if */
			ok = TRUE;
			break;
		case DOUniSpaceNumbers:
			if (status->numbers != uni) {
				status->numbers = uni;
				dispobj->new = TRUE;
			} /* if */
			ok = TRUE;
			break;
		case DOUniSpaceCrosshair:
			for (x = 0; x < MAXPOS; x++)
			{
				/* Get the flags through bitshifting */
				flag = uni & (1<<x);
				if (status->crosshair[x] != flag) {
					status->crosshair[x] = flag;
					dispobj->new = TRUE;
				} /* if */ 
			} /* for */ 
			ok = TRUE;
			break;
		case DOUniSpaceDisplay:
			for (x = 0; x < MAXPOS; x++)
			{
				flag = uni & (1<<x);
				/* Get the flags through bitshifting */
				if (status->display[x] != flag) {
					status->display[x] = flag;
					dispobj->new = TRUE;
				} /* if */ 
			} /* for */ 
			ok = TRUE;
			break;
		case DOUniSpaceVolume:
			for (x = 0; x < MAXPOS; x++)
			{
				flag = uni & (1<<x);
				/* Get the flags through bitshifting */
				if (status->vol_line[x] != flag) {
					status->vol_line[x] = flag;
					dispobj->new = TRUE;
				} /* if */ 
			} /* for */ 
			ok = TRUE;
			break;
		default:
			/* Continue with default function */
			ok = SetUniDispobj (dispobj, id, uni);
			switch (id)
			{
				case DOUniRotationX:
				case DOUniRotationY:
				case DOUniRotationZ:
				case DOUniSpacePerspective:
					/* Hat sich die Persp geÑndert? */
					if (dispobj->new && 	!status->initializing)
						compute_persp_lookups (dispobj);
					break;
			} /* switch */
			break;
	} /* switch type */

	return ok;
} /* SetUniSpace */

LOCAL BOOL	SetTypeSpace	(DISPOBJP dispobj, INT id, INT type)
{
	BOOL			ok = FALSE;
	SPACESTATP  status = &dispobj->status->space;

	switch (id)
	{
		case DOTypeSpaceForm:
			/* Zeiger auf Polygon einsetzen */
			status->spaceform = &spaceform[type][status->inside];
			ok = TRUE;
			dispobj->new = TRUE;
			break;
		case DOTypeSpaceMode:
			/* Zeichenroutine fÅr Linien festlegen */
			status->numpoints = 8;
			switch (type)
			{
				case MONO:			
					status->line_draw = mono; 
					break;
				case STEREO:		
					status->line_draw = stereo;
					break;
				case QUADRO:		
					status->line_draw = quadro;
					break;
				case OKTO:			
					status->line_draw = okto;
					break;
			} /* switch */
			ok = TRUE;
			dispobj->new = TRUE;
			break;
		default:
			/* Continue with default function */
			SetTypeDispobj (dispobj, id, type);
			break;
	} /* switch type */

	return ok;
} /* SetTypeSpace */

LOCAL BOOL	SetTypeCMOSpace	(DISPOBJP dispobj, INT id, INT type)
{
	BOOL			ok = FALSE;
	SPACESTATP  status = &dispobj->status->space;

	switch (id)
	{
		case DOTypeSpaceForm:
			/* Zeiger auf Polygon einsetzen */
			status->spaceform = &spaceform[type][status->inside];
			ok = TRUE;
			dispobj->new = TRUE;
			break;
		case DOTypeSpaceMode:
			/* Zeichenroutine fÅr Linien festlegen */
			switch (type)
			{
				case MONO:			
					status->line_draw = mono; 
					status->numpoints = 1;
					break;
				case STEREO:		
					status->line_draw = stereo;
					status->numpoints = 2;
					break;
				case QUADRO:		
					status->line_draw = quadro;
					status->numpoints = 4;
					break;
				case OKTO:			
					status->line_draw = okto;
					status->numpoints = 8;
					break;
			} /* switch */
			ok = TRUE;
			dispobj->new = TRUE;
			break;
		default:
			/* Continue with default function */
			SetTypeDispobj (dispobj, id, type);
			break;
	} /* switch type */

	return ok;
} /* SetTypeCMOSpace */

LOCAL VOID ResetSpace (DISPOBJP dispobj)
{
	SPACESTATP		status = &dispobj->status->space;
	
	dispobj->new = TRUE;
} /* ResetSpace */

LOCAL VOID TextDefaultSpace (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	WORD			ret;
	
	TextDefaultDispobj (dispobj);
	vst_height (vdi_handle, status->fontsize, &ret, &ret, &ret, &ret);

} /* TextDefaultSpace */

LOCAL VOID DrawSpace (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	BOOL			new = dispobj->new;
	WORD			x;
	STRING		s;	
	
	if (new)
	{
		clr_area (&dispobj->work);
		/* Raumform zeichnen  */
		draw_polygon (dispobj, status->spaceform);
		if (status->arrows) arrows (dispobj);
	} /* if */

	vswr_mode(vdi_handle, MD_XOR);			/* Immer EXOR */
	vsl_ends  (vdi_handle, SQUARED, SQUARED);
	vsl_type(vdi_handle, SOLID);
	vsl_width (vdi_handle, status->line_width);

	if (new || status->kneu)
	{
		/* Fadenkreuze */
		if (status->crosshair != 0)
			crosshairs(dispobj);
	
		/* Linien zeichnen  */
		if (status->display != 0)
			status->line_draw (dispobj);

		/* Volume zeichnen  */
		if (status->vol_line != 0)
			status->vol_draw (dispobj);

		if(status->numbers)
		{
			/* Nummern */
			dispobj->text_default (dispobj);
			for (x = 0; x < status->spaceform->num_numbers; x++)
			{
				sprintf (s, "%d", x+1);
				text_3d (dispobj, &status->spaceform->vertex[x], s);
			} /* for */
		} /* if numbers */
	} /* if new */

} /* DrawSpace */

LOCAL VOID DrawED4Space (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	STRING		s;
	WORD			x;
	BOOL			new = dispobj->new;
	
	if (new)
	{
		clr_area (&dispobj->work);
		/* Raumform zeichnen  */
		draw_polygon (dispobj, status->spaceform);
		if (status->arrows) arrows (dispobj);
	} /* if */

	vswr_mode(vdi_handle, MD_XOR);			/* Immer EXOR */
	vsl_ends  (vdi_handle, SQUARED, SQUARED);
	vsl_type(vdi_handle, SOLID);
	vsl_width (vdi_handle, status->line_width);

	if (new || status->kneu)
	{
		/* Fadenkreuze */
		crosshairs(dispobj);

		if (status->display[0])
			DrawED4Cursors (dispobj);
		
		if(status->numbers)
		{
			/* Nummern */
			dispobj->text_default (dispobj);
			for (x = 0; x < status->spaceform->num_numbers; x++)
			{
				sprintf (s, "%d", x+1);
				text_3d (dispobj, &status->spaceform->vertex[x], s);
			} /* for */
		} /* if numbers */

	} /* if new */

} /* DrawED4Space */

LOCAL VOID StartSpace (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	POS_3DP		position = status->position;
	WORD			signal;
	
	/* Call the default function */
	StartDispobj (dispobj);
	
	/* Direkt die Koordinaten aus den VAR's in den Positionsspeicher
		Åbertragen. */
	for (signal = 0; signal < MAXSIGNALS; signal++)
	{
		position[signal].x = var_get_value(var_module, VAR_PUF_KOORX0 + signal) * 100 / MAXKOOR;
		position[signal].y = var_get_value(var_module, VAR_PUF_KOORY0 + signal) * 100 / MAXKOOR;
		position[signal].z = var_get_value(var_module, VAR_PUF_KOORZ0 + signal) * 100 / MAXKOOR;
		status->kneu = TRUE;
	} /* for */
} /* StartSpace */

LOCAL VOID StartCMOSpace (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	POS_3DP		position = status->position;
	WORD			signal;
	
	/* Call the default function */
	StartDispobj (dispobj);
	
	/* Koordinaten werden direkt vom CMO Modul in die Status-Register 
		Åbertragen. */

} /* StartCMOSpace */
	
LOCAL VOID FinishSpace (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;

	/* Neue Koordinaten Åbernehmen */
	mem_move(&status->pos_alt, &status->position,(UWORD)sizeof(POS_3D) * MAXPOS); 
	mem_move(&status->vol_alt, &status->volume,(UWORD)sizeof(WORD) * MAXPOS); 

	status->kneu = FALSE;
	dispobj->new = FALSE;
} /* FinishSpace */;

LOCAL  VOID Project3D (DISPOBJP dispobj, POS_3DP point, POINT_2DP proj)
{
	SPACESTATP	status = &dispobj->status->space;
	WORD			xz, yz, *x = &proj->x, *y = &proj->y;
	RECT			*work = &dispobj->work;
	WORD			xmin = work->x;
	WORD			ymin = work->y;
	WORD			xmax = xmin + work->w;
	WORD			ymax = ymin + work->h;
	WORD			z = point->z + DMAXDOKOOR;
	
	/* Verschiebung von X und Y durch Z */
	xz = point->x + status->xzoff[z];
	yz = point->y + status->yzoff[z];
	
	/* Tiefenverzerrung durch Z */
	xz *= status->xpersp[z];
	xz = xz >> 7;
	yz *= status->ypersp[z];
	yz = yz >> 7;
	
	/* Darstellung nur des Innenraumes, Zoom*3 */
	if (status->inside) {
		xz *= 3;
		yz *= 3;
	}

	*x = status->xoffset + status->xwin[xz+DMAXDOKOOR];
	*y = status->yoffset + status->ywin[yz+DMAXDOKOOR];

	/* Clipping */
	if (*x > xmax) *x = xmax;
	else if (*x < xmin) *x = xmin;

	if (*y > ymax) *y = ymax;
	else if (*y < ymin) *y = ymin;

} /* Project3D */

#if 0
LOCAL  POINT_2D Project3D (DISPOBJP dispobj, POS_3DP point)
{
	POINT_2D		proj;
	SPACESTATP	status = &dispobj->status->space;
	FLOAT			persp = status->perspective;
	RECT			*work = &dispobj->work;
	WORD			xmin = work->x;
	WORD			ymin = work->y;
	WORD			xmax = work->x + work->w;
	WORD			ymax = work->y + work->h;
	LONG			z = (point->z * aspect_z)/100;
	LONG			perspk = (1 + (point->z * persp) /10000L);
	CONST INT	zxkoeff = 70;	/* sin(44)*100 */
	CONST INT	zykoeff = 71;	/* sin(44)*100 */

	/* Berechnung der Koordinaten im Fenster */
	proj.x = (point->x - (z * zxkoeff)/100) * perspk * work.w * aspect_x /20000L;
	proj.y = (point->y - (z * zykoeff)/100) * perspk * work.h * aspect_y /20000L;
	
	if (status->inside) {
		proj.x *= 3;
		proj.y *= 3;
	} /* if inside */

	proj.x += status->xoffset;
	proj.y += status->yoffset;

	/* Clipping */
	if (proj.x > xmax) proj.x = xmax;
	else if (proj.x < xmin) proj.x = xmin;

	if (proj.y > ymax) proj.y = ymax;
	else if (proj.y < ymin) proj.y = ymin;

	return proj;
} /* Project3D */
#endif

LOCAL  VOID Project3DNeu (DISPOBJP dispobj, POS_3DP point, POINT_2DP proj)
{
	FLOAT			xz, yz, xp = point->x, yp = point->y;
	WORD			*x = &proj->x, *y = &proj->y;
	SPACESTATP	status = &dispobj->status->space;
	RECT			*work = &dispobj->work;
	WORD			xmin = work->x;
	WORD			ymin = work->y;
	WORD			xmax = work->x + work->w;
	WORD			ymax = work->y + work->h;
	FLOAT	dx = dispobj->uni[DOUniRotationX];
	FLOAT	dy = dispobj->uni[DOUniRotationY];
	FLOAT	d = dispobj->uni[DOUniRotationZ];
	FLOAT	a = dispobj->uni[DOUniSpacePerspective];
	FLOAT	z = -point->z;

	if ((d+z) && a)
	{
		xz = (xp * a/(d+z)) - (dx * (d+z)/a);
		yz = (yp * a/(d+z)) - (dy * (d+z)/a);
/*
		xz = (point->x * a/(d+z)) - (dx * d/a);
		yz = (point->y * a/(d+z)) - (dy * d/a);
*/

		/* Darstellung nur des Innenraumes, Zoom*3 */
		if (status->inside) {
			xz *= 3;
			yz *= 3;
		}
	
/*
	if (xz > DMAXDOKOOR) 
		xz = DMAXDOKOOR;
	else if (xz < DMAXDOKOOR)
		xz = -DMAXDOKOOR;
	if (yz > DMAXDOKOOR)
		yz = DMAXDOKOOR;
	else if (yz < DMAXDOKOOR)
		yz = -DMAXDOKOOR;

		/* Umrechnen von xz und yz auf Objektgrîûe */
		*x = status->xoffset + xz * work->w * aspect_x / 20000L;
		*y = status->xoffset + yz * work->h * aspect_y / 20000L;
*/
		/* Umrechnen von xz und yz auf Objektgroesse */
		*x = status->xoffset + status->xwin[xz+DMAXDOKOOR];
		*y = status->yoffset + status->ywin[yz+DMAXDOKOOR];
	}
	/* Clipping */
	if (*x > xmax) *x = xmax;
	else if (*x < xmin) *x = xmin;

	if (*y > ymax) *y = ymax;
	else if (*y < ymin) *y = ymin;

} /* Project3DNeu */

LOCAL  VOID line_3d (DISPOBJP dispobj, POS_3D *point1, POS_3D *point2)
{
	POINT_2D		p1, p2;
	INT			pxyarray[4];
	SPACESTATP	status = &dispobj->status->space;
	ProjectFn	*projection = status->projection;
	
	projection (dispobj, point1, &p1);
	projection (dispobj, point2, &p2);
	
	pxyarray[0] = p1.x;
	pxyarray[1] = p1.y;
	pxyarray[2] = p2.x;
	pxyarray[3] = p2.y;
	
	v_pline( vdi_handle, 2, pxyarray );
} /* line_3d */

LOCAL  VOID text_3d (DISPOBJP dispobj, POS_3D *point, STRING text)
{
	POINT_2D		p;
	SPACESTATP	status = &dispobj->status->space;
	ProjectFn	*projection = status->projection;
	
	projection (dispobj, point, &p);

	v_gtext( vdi_handle, p.x, p.y, text );
} /* text_3d */

LOCAL  VOID multiline_3d (DISPOBJP dispobj, POS_3D *point[], WORD points)
{
	/* Zeichnet einen 3-D Linienzug mit maximal 64 Eckpunkten */
	/* *point[] enthÑlt Zeiger auf die 3D-Koordinaten, ab *point[0]! */
	POINT_2D		proj;					/* Merker fÅr Projektionsdaten */
	WORD 			count;					/* Laufender ZÑhler */
	INT			pxyarray[128];			/* VDI-öbergabe-Array */
	SPACESTATP	status = &dispobj->status->space;
	ProjectFn	*projection = status->projection;
	
	for (count=0; count < points; count ++){
		projection (dispobj, point[count], &proj);
		pxyarray[count*2] = proj.x;
		pxyarray[1+count*2] = proj.y;
	}
	
	v_pline( vdi_handle, points, pxyarray );
} /* multiline_3d */

LOCAL  VOID draw_polygon	(DISPOBJP dispobj, POLY_P poly)
{
	WORD		v, e, 
				num_vertices = poly->num_vertices,
				num_edges = poly->num_edges;
	POS_3D	*vertex = poly->vertex;
	EDGE		*edge = poly->edge;
	SPACESTATP	status = &dispobj->status->space;
	ProjectFn	*projection = status->projection;
	POINT_2D		proj[MAXVERTEX];
	INT			pxyarray[4];
	WORD			line_style = -1,
					begin_style = -1,
					end_style = -1,
					color = -1,
					width = -1;
					
	/* Alle Punkte projizieren */
	for (v=0; v < num_vertices; v++) {
		projection (dispobj, vertex++, &proj[v]);
	} /* for vertex */
	
	for (e = 0; e < num_edges; e++) {

		/* Linien-Attribute setzen, wenn nîtig */
		if (line_style != edge->line_style)
			vsl_type (vdi_handle, edge->line_style);
		if ((begin_style != edge->begin_style) ||
				(end_style != edge->end_style))
			vsl_ends (vdi_handle, edge->begin_style, end_style);
		if (color != edge->color)
			vsl_color (vdi_handle, edge->color);
		if (width != edge->width)
			vsl_width (vdi_handle, edge->width);

		/* Punkte einsetzen fÅr v_pline */
		pxyarray[0] = proj[edge->from].x;
		pxyarray[1] = proj[edge->from].y;
		pxyarray[2] = proj[edge->to].x;
		pxyarray[3] = proj[edge->to].y;
		
		v_pline( vdi_handle, 2, pxyarray );
		edge++;	/* NÑchste Kante */
	} /* for edge */
	
} /* draw_polygon */

#define z2line(z, breite) ((INT)z+2*breite)%(2*breite) - breite

LOCAL VOID DrawED4Cursors (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	BOOL			xneu,  yneu,  zneu;		/* Merker fÅr Koor-énderung */
	POS_3DP		pos_alt = status->pos_alt,
					position = status->position;
	REG POS_3DP palt, pneu;				/* Zeiger fÅr alte Werte */
	POS_3D 		p1, p2;					/* Variablen fÅr Koor-öbergabe */
	WORD			signal, breite, numpoints = status->numpoints;
	BOOL			new = dispobj->new;
	WORD			z, zalt;
	WORD			z2, z2alt;
	
	if(status->inside)
		breite = MAXDOKOOR / 3;
	else
		breite = MAXDOKOOR;

	pneu = &position[0];		/* Hilfszeiger */
	palt = &pos_alt[0];		/* Hilfszeiger */
	
	xneu = (pneu->x != palt->x);			/* neu?->merken */
	yneu = (pneu->y != palt->y);			/* usw. */
	zneu = (pneu->z != palt->z);


	/* X-Linie neu zeichnen */
	if (xneu || new)	
	{			
		/* Punkte festlegen */
		SetPoint( p1, pneu->x, -breite, -breite);
		SetPoint( p2, pneu->x,  breite, -breite);
		
		line_3d (dispobj,  &p1, &p2);	/* Neue Linie zeichnen */
		
		if (!new)
		{
			/* Alte Linie lîschen */
			SetPoint( p1, palt->x, -breite, -breite);
			SetPoint( p2, palt->x,  breite, -breite);

			line_3d (dispobj,  &p1, &p2);	/* Alte Linie lîschen */
		} /* if */
	} /* if */

	/* Y-Linie neu zeichnen */
	if (yneu || new)
	{
		SetPoint( p1, -breite, pneu->y, -breite);
		SetPoint( p2,  breite, pneu->y, -breite);

		line_3d (dispobj,  &p1, &p2);
		

		if (!new)
		{
			/* Alte Linie lîschen */
			SetPoint( p1, -breite, palt->y, -breite);
			SetPoint( p2,  breite, palt->y, -breite);

			line_3d (dispobj,  &p1, &p2);
		} /* if */
	} /* if */

	vsl_type(vdi_handle, DOT);

	/* Z-Linie neu zeichnen */
	if( zneu || new)
	{	
		z = pneu->z;
		zalt = palt->z;
		z2 = z2line (z, breite);
		z2alt = z2line (zalt, breite);
		
		/* 1. Linie */
		SetPoint( p1, -breite, -breite, z);
		SetPoint( p2, -breite,  breite, z);
		line_3d (dispobj,  &p1, &p2);

		/* 2. Linie */
		SetPoint( p1, -breite, -breite, z2);
		SetPoint( p2, -breite,  breite, z2);
		line_3d (dispobj,  &p1, &p2);
		
		/* 3. Linie */
		SetPoint( p1,  breite, -breite, z);
		SetPoint( p2,  breite,  breite, z);
		line_3d (dispobj,  &p1, &p2);
		
		/* 4. Linie */
		SetPoint( p1,  breite, -breite, z2);
		SetPoint( p2,  breite,  breite, z2);
		line_3d (dispobj,  &p1, &p2);

		if (!new)
		{
			/* Alte Linie lîschen */
			SetPoint( p1, -breite, -breite, zalt);
			SetPoint( p2, -breite,  breite, zalt);
			line_3d (dispobj,  &p1, &p2);

			SetPoint( p1, -breite, -breite, z2alt);
			SetPoint( p2, -breite,  breite, z2alt);
			line_3d (dispobj,  &p1, &p2);

			SetPoint( p1,  breite, -breite, zalt);
			SetPoint( p2,  breite,  breite, zalt);
			line_3d (dispobj,  &p1, &p2);

			SetPoint( p1,  breite, -breite, z2alt);
			SetPoint( p2,  breite,  breite, z2alt);
			line_3d (dispobj,  &p1, &p2);
		} /* if */

	} /* if zneu */

} /* DrawED4Cursors */

LOCAL  VOID crosshairs (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	BOOL			xneu,  yneu,  zneu;		/* Merker fÅr Koor-énderung */
	POS_3DP		pos_alt = status->pos_alt,
					position = status->position;
	REG POS_3DP palt, pneu;				/* Zeiger fÅr alte Werte */
	POS_3D 		p1, p2;					/* Variablen fÅr Koor-öbergabe */
	WORD			signal, breite, numpoints = status->numpoints;
	BOOL			new = dispobj->new;
	
	if(status->inside)
		breite = MAXDOKOOR / 3;
	else
		breite = MAXDOKOOR;
		
	for (signal=0; signal < numpoints; signal++)	/* Alle Signale durchgehen */
	{
		if (status->crosshair[signal])			/* Fadenkreuz zeichnen? */
		{
			pneu = &position[signal];		/* Hilfszeiger */
			palt = &pos_alt[signal];		/* Hilfszeiger */
			
			xneu = (pneu->x != palt->x);			/* neu?->merken */
			yneu = (pneu->y != palt->y);			/* usw. */
			zneu = (pneu->z != palt->z);


			if (xneu || yneu || new)	/* Wenn X oder Y pneu sind, Z pneu zeichnen */
			{
				SetPoint (p1, pneu->x, pneu->y, -breite);
				SetPoint (p2, pneu->x, pneu->y, breite);
				line_3d (dispobj,  &p1, &p2);	/* Neue Linie zeichnen */
				
				if (!new)
				{
					/* Alte Linie lîschen */
					SetPoint (p1, palt->x, palt->y, -breite);
					SetPoint (p2, palt->x, palt->y, breite);
					line_3d (dispobj,  &p1, &p2);	/* Alte Linie lîschen */
				} /* if */
			} /* if */
		
			if (yneu || zneu || new)
			{
				SetPoint (p1, -breite, pneu->y, pneu->z);
				SetPoint (p2,  breite, pneu->y, pneu->z);
				line_3d (dispobj,  &p1, &p2);
				
				if (!new)
				{
					/* Alte Linie lîschen */
					SetPoint (p1, -breite, palt->y, palt->z);
					SetPoint (p2,  breite, palt->y, palt->z);
					line_3d (dispobj,  &p1, &p2);
				} /* if */
			} /* if */
		
			if(xneu || zneu || new)
			{
				SetPoint (p1, pneu->x, -breite, pneu->z);
				SetPoint (p2, pneu->x, breite, pneu->z);
				line_3d (dispobj,  &p1, &p2);
				
				if (!new)
				{
					/* Alte Linie lîschen */
					SetPoint (p1, palt->x, -breite, palt->z);
					SetPoint (p2, palt->x, breite, palt->z);
					line_3d (dispobj,  &p1, &p2);
				} /* if */
			} /* if */
		} /* if */
	} /* for */
} /* crosshairs */

LOCAL  VOID VolSpace (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	WORD 			*volume = status->volume,
			 		*vol_alt = status->vol_alt,
					vneu, valt;
	POS_2D		orig, pos;
	WORD			signal, numpoints = status->numpoints;
	WORD			base = status->base;
	BOOL			new = dispobj->new;
	BOOL 			neu_v = FALSE;
	WORD			vwidth = 80;
	WORD			vheight;
	
	vheight = status->line_width * 2;
	
	orig.x = 10;

	for (signal = 0; signal < numpoints; signal++)
	{
		if (status->vol_line[signal+base])
		{
			/* Hîhe berechnen */
			pos.y = MAXDOKOOR - (numpoints - signal) * vheight;
			orig.y = pos.y;
			
			vneu = volume[signal+base];
			if (!new)
			{
				valt = vol_alt[signal+base];
				neu_v  = vneu != valt;		/* Feststellen, ob neu gezeichnet */

				if (neu_v)
				{
					pos.x = orig.x + valt*vwidth/127;
					LineDispobj(dispobj, &orig, &pos);
				}
			} /* if */
			/* Zeichnen falls noch keine Linie da, oder neue Koor */
			pos.x = orig.x + vneu*vwidth/127;
			if(neu_v || new)	LineDispobj(dispobj, &orig, &pos);
		} /* if */
	} /* for */

} /* VolSpace */

LOCAL  VOID mono (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	POS_3DP 		position = status->position,
			 		pos_alt = status->pos_alt,
					pneu, palt;
	POS_3D	 	null = {0,0,0};
	WORD			signal, numpoints = status->numpoints;
	WORD			base = status->base;
	BOOL		new = dispobj->new;
	BOOL 		neu_k = FALSE;

	for (signal = 0; signal < numpoints; signal++)
	{
		if (status->display[signal+base])
		{
			pneu = &position[signal+base];
			if (!new)
			{
				palt = &pos_alt[signal+base];
				neu_k  = pneu->x != palt->x;		/* Feststellen, ob neu gezeichnet */
				neu_k |= pneu->y != palt->y;		/* werden muû */
				neu_k |= pneu->z != palt->z;
				if (neu_k) 		line_3d(dispobj, &null, palt);
			} /* if */
			/* Zeichnen falls noch keine Linie da, oder neue Koor */
			if(neu_k || new)	line_3d(dispobj, &null, pneu);
		} /* if */
	} /* for */
} /* mono */

LOCAL  VOID stereo (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	POS_3DP 		position = status->position,
			 		pos_alt = status->pos_alt,
				 	akt1, alt1, akt2, alt2;
	WORD			signal, numpoints = status->numpoints;
	WORD			base = status->base;
	BOOL		new = dispobj->new;
	BOOL 		neu_k = FALSE;

	for (signal = 0; signal < numpoints; signal+=2)
	{
		if (status->display[signal+base] || status->display[signal+base+1] || new)
		{
			akt1 = &position[signal+base];
			akt2 = &position[signal+base+1];
			if (!new)
			{
				alt1 = &pos_alt[signal+base];
				alt2 = &pos_alt[signal+base+1];
				neu_k  = akt1->x != alt1->x;	/* Feststellen, ob neu gezeichnet */
				neu_k |= akt1->y != alt1->y;	/* werden muû wg. 1. Signal */
				neu_k |= akt1->z != alt1->z;
				neu_k |= akt2->x != alt2->x;	/* Feststellen, ob neu gezeichnet */
				neu_k |= akt2->y != alt2->y;	/* werden muû wg. 2. Signal*/
				neu_k |= akt2->z != alt2->z;
				if (neu_k)		line_3d(dispobj, alt1, alt2);
			} /* if */
			/* Zeichnen falls noch keine Linie da, oder neue Koor */
			if(neu_k || new)	line_3d(dispobj, akt1, akt2);
		} /* if */
	} /* for */
} /* stereo */

LOCAL  VOID quadro (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	POS_3DP 		position = status->position,
		 			pos_alt = status->pos_alt,
					points_akt[MAXPOS], /* Zeiger-Array fÅr öbergabe */
					points_alt[MAXPOS]; /* Zeiger-Array fÅr öbergabe */
	WORD			signal, offset, numpoints = status->numpoints;
	WORD			base = status->base;
	BOOL		new = dispobj->new;
	BOOL 		neu = FALSE;

	for (offset = 0; offset < numpoints; offset+=4)
	{
		neu = FALSE;
		for (signal = 0; signal < numpoints; signal++)
		{
			/* PrÅfen, ob irgendeine Koordinate gezeichnet werden muû */
			neu = neu || status->display[offset+signal+base];
			/* Koordinaten immer in den Puffer kopieren */
			points_akt[signal]= &position[offset+signal+base];
			points_alt[signal]= &pos_alt[offset+signal]+base;
		}/* for */
		if (neu || new)
		{
			multiline_3d(dispobj, points_akt, 4);
			if (!new)
				multiline_3d(dispobj, points_alt, 4);
		} /* if */
	} /* for */
} /* quadro */

LOCAL  VOID okto (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	POS_3DP 	position = status->position,
				pos_alt = status->pos_alt,
				points_akt[MAXPOS], /* Zeiger-Array fÅr öbergabe */
				points_alt[MAXPOS]; /* Zeiger-Array fÅr öbergabe */
	WORD		signal, numpoints = status->numpoints;
	WORD			base = status->base;
	BOOL	new = dispobj->new;
	BOOL 	neu = FALSE;
	
	for (signal = 0; signal < numpoints; signal++)
	{
		neu = neu || status->display[signal+base];
		points_akt[signal]= &position[signal+base];
		points_alt[signal]= &pos_alt[signal+base];
	} /* for */
	if (neu || new)
	{
		multiline_3d(dispobj, points_akt, 8);
		if (!new)
			multiline_3d(dispobj, points_alt, 8);
	} /* if */
} /* okto */

LOCAL  VOID arrows (DISPOBJP dispobj)
{
	SPACESTATP	status = &dispobj->status->space;
	INT 			p, m;
	POS_3D 		w[6];

	if(status->inside)
		p = MAXDOKOOR / 3;
	else
		p = MAXDOKOOR;
	
	m	= -p;
	
	/* Dimensions-Pfeile */
	vsl_ends  (vdi_handle, SQUARED, ARROWED);
	SetPoint(w[0], m,  0, 0);
	SetPoint(w[1], p,  0, 0);
	line_3d (dispobj, &w[0], &w[1]);
	SetPoint(w[2], 0,  m, 0);
	SetPoint(w[3], 0,  p, 0);
	line_3d (dispobj, &w[2], &w[3]);
	SetPoint(w[4], 0,  0, m);
	SetPoint(w[4], 0,  0, p);
	line_3d (dispobj, &w[4], &w[5]);
	vsl_type(vdi_handle, SOLID);
	/* Texte */
	dispobj->text_default (dispobj);
	text_3d (dispobj, &w[0], "L");
	text_3d (dispobj, &w[1], "R");
	text_3d (dispobj, &w[2], "U");
	text_3d (dispobj, &w[3], "O");
	text_3d (dispobj, &w[4], "V");
	text_3d (dispobj, &w[5], "H");

} /* arrows */

LOCAL  VOID define_sechskanal ()
{	
	POLY_P	space;
	WORD		innen, p = MAXDOKOOR,
				m = -p;

	for (innen = 0; innen<2; innen++)
	{
	
		if (innen)
			p = MAXDOKOOR / 3;	/* Innenraum-Form */
		
		m=-p;
		
		space = &spaceform[SECHSKANAL][innen];
	
		/*
		   2------3
		  /|     /|
		 4------5 |
		  \|     \|
			0------1
		*/
		
		AddVertex(space, m,  m, m);
		AddVertex(space, p,  m, m);
		
		AddVertex(space, m,  p, m);
		AddVertex(space, p,  p, m);
	
		AddVertex(space, m,  p, p);
		AddVertex(space, p,  p, p);

		space->num_numbers = 6;
		
		AddEdge (space, 0, 1, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 0, 2, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 0, 4, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 3, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 5, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 3, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 4, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 5, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 4, 5, SOLID, SQUARED, SQUARED, 1, 1);
	} /* for innen */
		
} /* define_sechskanal */

#define Add4Edges(space, v0)\
	AddEdge (space, v0+0, v0+1, DOT, SQUARED, SQUARED, 1, 1);\
	AddEdge (space, v0+1, v0+2, DOT, SQUARED, SQUARED, 1, 1);\
	AddEdge (space, v0+2, v0+3, DOT, SQUARED, SQUARED, 1, 1);\
	AddEdge (space, v0+3, v0+0, DOT, SQUARED, SQUARED, 1, 1);


LOCAL  VOID wuerfel_innen (POLY_P space)
{	
	WORD		v0, p = MAXDOKOOR, p3 = p/3,
				m = -p, m3 = -p3;
	
	/* Innen-Markierung */
	v0 = space->num_vertices;
	AddVertex(space, m3,  m, m3);
	AddVertex(space, p3,  m, m3);
	AddVertex(space, p3,  m, p3);
	AddVertex(space, m3,  m, p3);
	Add4Edges(space, v0);

	v0 = space->num_vertices;
	AddVertex(space, m3,  p, m3);
	AddVertex(space, p3,  p, m3);
	AddVertex(space, p3,  p, p3);
	AddVertex(space, m3,  p, p3);
	Add4Edges(space, v0);

	v0 = space->num_vertices;
	AddVertex(space, m,  m3, m3);
	AddVertex(space, m,  p3, m3);
	AddVertex(space, m,  p3, p3);
	AddVertex(space, m,  m3, p3);
	Add4Edges(space, v0);

	v0 = space->num_vertices;
	AddVertex(space, p,  m3, m3);
	AddVertex(space, p,  p3, m3);
	AddVertex(space, p,  p3, p3);
	AddVertex(space, p,  m3, p3);
	Add4Edges(space, v0);

	v0 = space->num_vertices;
	AddVertex(space, m3,  m3, m);
	AddVertex(space, p3,  m3, m);
	AddVertex(space, p3,  p3, m);
	AddVertex(space, m3,  p3, m);
	Add4Edges(space, v0);

	v0 = space->num_vertices;
	AddVertex(space, m3,  m3, p);
	AddVertex(space, p3,  m3, p);
	AddVertex(space, p3,  p3, p);
	AddVertex(space, m3,  p3, p);
	Add4Edges(space, v0);

} /* wuerfel_innen */

LOCAL  VOID define_wuerfel ()
{	
	POLY_P	space;
	WORD		innen, p = MAXDOKOOR,
				m = -p;

	for (innen = 0; innen<2; innen++)
	{
	
		if (innen)
			p = MAXDOKOOR / 3;	/* Innenraum-Form */
		
		m=-p;
		
		space = &spaceform[WUERFEL][innen];
		
		/*
			 	4------5
		     /|     /|	
			 / 0----/-1
			7------6 /
			|/     |/
			3------2
		*/
	
		/* Unten */
		AddVertex (space, m,m,m);
		AddVertex (space, p,m,m);
		AddVertex (space, p,m,p);
		AddVertex (space, m,m,p);
	
		/* Oben */
		AddVertex (space, m,p,m);
		AddVertex (space, p,p,m);
		AddVertex (space, p,p,p);
		AddVertex (space, m,p,p);
	
		space->num_numbers = 8;
		
		/* Unten */
		AddEdge (space, 0, 1, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 2, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 3, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 0, SOLID, SQUARED, SQUARED, 1, 1);
		
		/* Oben */
		AddEdge (space, 4, 5, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 5, 6, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 6, 7, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 7, 4, SOLID, SQUARED, SQUARED, 1, 1);
	
		/* Senkrechte */
		AddEdge (space, 0, 4, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 5, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 6, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 7, SOLID, SQUARED, SQUARED, 1, 1);
	
		if (!innen)
		{
			wuerfel_innen (space);
		} /* if not innen */
	} /* for innen */
	
} /* define_wuerfel */


LOCAL  VOID define_wuerfel_hoch ()
{	
	POLY_P	space;
	WORD		innen, x, p = MAXDOKOOR,
				m = -p;

	for (innen = 0; innen<2; innen++)
	{
	
		if (innen)
			p = MAXDOKOOR / 3;	/* Innenraum-Form */
		
		m=-p;
		
		space = &spaceform[WUERFELHOCH][innen];

	
		/*		8------9
			  /|		/|
			 /	4----/-5
		  11------10/|	
			|/ 0---|/-1
			7------6 /
			|/     |/
			3------2
		*/
		
		AddVertex (space, m,m,m);
		AddVertex (space, p,m,m);
		AddVertex (space, p,m,p);
		AddVertex (space, m,m,p);
	
		for (x = 4; x < 8; x++)
		{
			AddVertex (space, space->vertex[x-4].x, 0, space->vertex[x-4].z);
		} /* for */
	
		for (x = 8; x < 12; x++)
		{
			AddVertex (space, space->vertex[x-4].x, p, space->vertex[x-4].z);
		} /* for */

		space->num_numbers = 12;
		
		/* Unten */
		AddEdge (space, 0, 1, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 2, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 3, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 0, SOLID, SQUARED, SQUARED, 1, 1);
		
		/* Mitte */
		AddEdge (space, 4, 5, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 5, 6, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 6, 7, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 7, 4, SOLID, SQUARED, SQUARED, 1, 1);
	
		/* Oben */
		AddEdge (space, 8, 9, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 9,10, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space,10,11, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space,11, 8, SOLID, SQUARED, SQUARED, 1, 1);

		/* Senkrechte */
		AddEdge (space, 0, 4, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 5, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 6, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 7, SOLID, SQUARED, SQUARED, 1, 1);
	
		AddEdge (space, 4, 8, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 5, 9, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 6, 10, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 7, 11, SOLID, SQUARED, SQUARED, 1, 1);

		if (!innen)
		{
			wuerfel_innen (space);
		} /* if not innen */
	} /* for innen */	

} /* define_wuerfel_hoch */

LOCAL  VOID define_wuerfel_lang ()
{	
	POLY_P	space;
	WORD		innen, x, p = MAXDOKOOR,
				m = -p;

	for (innen = 0; innen<2; innen++)
	{
	
		if (innen)
			p = MAXDOKOOR / 3;	/* Innenraum-Form */
		
		m=-p;
		
		space = &spaceform[WUERFELLANG][innen];
		
		/*		 6-------7
				/|      /|
			 11 |     8 |
			 /| 0 ---/| 1
		  10 ------9 |/
		   | 5-----|-2
			|/      |/
			4-------3
		
		*/
				
		AddVertex (space, m,m,m);
		AddVertex (space, p,m,m);
		AddVertex (space, p,m,0);
	
		AddVertex (space, p,m,p);
		AddVertex (space, m,m,p);
		AddVertex (space, m,m,0);
	
		space->num_numbers = 12;
		
		for (x = 6; x < 12; x++)
		{
			AddVertex (space, space->vertex[x-6].x, p, space->vertex[x-6].z);
		} /* for */
		
		/* Waagerechte */
		/* Unten */
		AddEdge (space, 0, 1, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 2, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 3, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 4, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 4, 5, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 5, 0, SOLID, SQUARED, SQUARED, 1, 1);

		/* Oben */
		AddEdge (space, 6, 7, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 7, 8, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 8, 9, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 9,10, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space,10,11, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space,11, 6, SOLID, SQUARED, SQUARED, 1, 1);

		/* Senkrechte */
		AddEdge (space, 0, 6, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 7, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 8, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 9, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 4,10, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 5,11, SOLID, SQUARED, SQUARED, 1, 1);

		if (!innen)
		{
			wuerfel_innen (space);
		} /* if innen */
	} /* for not innen */	

} /* define_wuerfel_lang */

LOCAL  VOID define_wuerfel_mitte ()
{	
	POLY_P	space;
	WORD		innen, p = MAXDOKOOR,
				m = -p;

	for (innen = 0; innen<2; innen++)
	{
	
		if (innen)
			p = MAXDOKOOR / 3;	/* Innenraum-Form */
		
		m=-p;
		
		space = &spaceform[WUERFELMITTE][innen];
		
		/*
			 	 4------5
		      /|  8  /|	
			  / 0-12-/-1
			 /11 13 /9/
			7------6 /
			|/ 10  |/
			3------2
		*/
	
		AddVertex (space, m,m,m);
		AddVertex (space, p,m,m);
		AddVertex (space, p,m,p);
		AddVertex (space, m,m,p);
	
		AddVertex (space, m,p,m);
		AddVertex (space, p,p,m);
		AddVertex (space, p,p,p);
		AddVertex (space, m,p,p);
	
		/* Mittelpunkte */
		AddVertex (space, 0,0,m);
		AddVertex (space, p,0,0);
		AddVertex (space, 0,0,m);
		AddVertex (space, m,0,0);

		space->num_numbers = 12;
		
		/* Unten */
		AddEdge (space, 0, 1, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 2, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 3, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 0, SOLID, SQUARED, SQUARED, 1, 1);
		
		/* Oben */
		AddEdge (space, 4, 5, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 5, 6, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 6, 7, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 7, 4, SOLID, SQUARED, SQUARED, 1, 1);
	
		/* Senkrechte */
		AddEdge (space, 0, 4, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 5, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 6, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 7, SOLID, SQUARED, SQUARED, 1, 1);

		if (!innen)
		{
			wuerfel_innen (space);
		} /* if not innen */
	} /* for innen */	
		
} /* define_wuerfel_mitte */

LOCAL  VOID define_tetraeder ()
{	
	POLY_P	space;
	WORD		innen, p = MAXDOKOOR, p3 = p/3,
				m = -p, m3 = m/3;

	for (innen = 0; innen<2; innen++)
	{
	
		if (innen)
			p = MAXDOKOOR / 3;	/* Innenraum-Form */
		
		m=-p;
		
		space = &spaceform[TETRAEDER][innen];
	
		AddVertex (space, m,m,m);
		AddVertex (space, p,m,m);
		AddVertex (space, 0,m,p);
		AddVertex (space, 0,p,0);
		space->num_numbers = 4;
		
		/* Kanten */
		AddEdge (space, 0, 1, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 2, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 0, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 0, 3, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 3, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 3, SOLID, SQUARED, SQUARED, 1, 1);

		if (!innen)
		{
			AddVertex (space, m3,m3,m3);
			AddVertex (space, p3,m3,m3);
			AddVertex (space,  0,m3,p3);
			AddVertex (space,  0,p3, 0);
	
			/* Kanten */
			AddEdge (space, 4, 4, DOT, SQUARED, SQUARED, 1, 1);
			AddEdge (space, 5, 6, DOT, SQUARED, SQUARED, 1, 1);
			AddEdge (space, 6, 4, DOT, SQUARED, SQUARED, 1, 1);
			AddEdge (space, 4, 7, DOT, SQUARED, SQUARED, 1, 1);
			AddEdge (space, 5, 7, DOT, SQUARED, SQUARED, 1, 1);
			AddEdge (space, 6, 7, DOT, SQUARED, SQUARED, 1, 1);
		} /* if not innen */

	} /* for innen */

} /* define_tetraeder */

LOCAL  VOID define_quadrophon ()
{	
	POLY_P	space;
	WORD		innen, p = MAXDOKOOR, p3 = p/3,
				m = -p, m3 = m/3;

	for (innen = 0; innen<2; innen++)
	{
	
		if (innen)
			p = MAXDOKOOR / 3;	/* Innenraum-Form */
		
		m=-p;
		
		space = &spaceform[QUADROPHON][innen];

		AddVertex (space, m,0,m);
		AddVertex (space, p,0,m);
		AddVertex (space, p,0,p);
		AddVertex (space, m,0,p);
	
		/* Unten */
		AddEdge (space, 0, 1, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 1, 2, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 2, 3, SOLID, SQUARED, SQUARED, 1, 1);
		AddEdge (space, 3, 0, SOLID, SQUARED, SQUARED, 1, 1);
		space->num_numbers = 4;
		
		if (!innen)
		{

			AddVertex (space, m3,0,m3);
			AddVertex (space, p3,0,m3);
			AddVertex (space, p3,0,p3);
			AddVertex (space, m3,0,p3);
		
			/* Unten */
			AddEdge (space, 4, 5, DOT, SQUARED, SQUARED, 1, 1);
			AddEdge (space, 5, 6, DOT, SQUARED, SQUARED, 1, 1);
			AddEdge (space, 6, 7, DOT, SQUARED, SQUARED, 1, 1);
			AddEdge (space, 7, 4, DOT, SQUARED, SQUARED, 1, 1);

		} /* if not innen */

	} /* for innen */

} /* define_quadrophon */

/********************************************************************/

LOCAL VOID time_str (STRING timestr, LONG time)
{
	/* Wandelt einen Long-Wert fÅr die Anzahl der Millisekunden */
	/* in eine Kette von Zahlen der Form hh:mm:ss:ll um */
	REG LONG temp;
	REG WORD hh, mm, ss, ll;
	
	temp = time /1000;						/* Millisekunden abschneiden */
	ll = (WORD) (time - temp*1000);			/* ms merken */
	time /= 1000;
	temp /= 60;
	ss = (WORD) (time - temp*60);
	time /= 60;
	temp /= 60;
	mm = (WORD) (time - temp*60);
	time /= 60;
	temp /= 24;
	hh = (WORD) (time - temp*24);
	time /= 24;
	/*
	hh = time / 3600000L;
	mm = (time - hh * 3600000L) / 60000L;
	ss = (time - hh * 3600000L - mm * 60000L) / 1000;
	*/
	sprintf(timestr, "%2d:%2d:%2d:%3d", hh, mm, ss, ll);
} /* time_str */

LOCAL LONG str_time (STRING timestr)
{
	/* Wandelt eine Kette von Zahlen der Form hhmmssll */
	/* in einen Long-Wert fÅr die Anzahl der Millisekunden um */
	REG LONG time;
	LONG hh = 0L, mm = 0L, ss = 0L, ll = 0L;
	STRING s, trenn = ":. ";
	char *ts;
	LONG t[4], count = 0L;
	
	strcpy(s, timestr);
	
	ts = strtok(s, trenn);
	while (ts)
	{
		sscanf(ts, "%ld", &t[count]);
		count++;
		ts = strtok(NULL, trenn);
	} /* while */

	if (count>0) ll = t[--count];
	if (count>0) ss = t[--count];
	if (count>0) mm = t[--count];
	if (count>0) hh = t[--count];
	
	time = hh;
	time *= 60;
	time += mm;
	time *= 60;
	time += ss;
	time *= 1000;
	time += ll;

	return (time);
} /* str_time */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

LOCAL VOID define_forms ()
{
	/* Init der Raumformen */

	define_sechskanal ();
	define_wuerfel ();
	define_wuerfel_lang ();
	define_wuerfel_hoch ();
	define_wuerfel_mitte ();
	define_quadrophon ();
	define_tetraeder ();
} /* define_forms */

/*****************************************************************************/
/* Initialisieren des Moduls                                                 */
/*****************************************************************************/

GLOBAL BOOL init_dispobj ()
{
	BOOL	ok = TRUE;

	define_forms ();

	return ok;
} /* init_dispobj */

GLOBAL BOOL term_dispobj ()
{
	BOOL	ok = TRUE;

	return ok;
} /* term_dispobj */

