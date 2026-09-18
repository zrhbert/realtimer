/*****************************************************************************/
/*                                                                           */
/* Modul: MROS.C                                                             */
/* Datum: 05/11/92                                                           */
/*                                                                           */
/* M-ROS Funktionen fÅr RTM                                               	  */
/*****************************************************************************/

#include "import.h"
#include "global.h"
#include "ext.h"

#include "export.h"
#include "mros.h"

/****** FUNCTIONS ************************************************************/

GLOBAL MROS_STRUCT	*create_mros	_((VOID))
{
	MROS_STRUCT *ms;
	
	ms = (MROS_STRUCT *)mem_alloc(sizeof(MROS_STRUCT));
	
	return ms;
}

GLOBAL BOOLEAN 		init_mros		_((VOID))
{
	struct ffblk fblock;
	char path[128];
	char name[14];
	     
	if(Dgetpath (path,0)==0);
	{
		strcpy (path,"\\");
		Dsetpath ("MROS\\");
		     
		if (findfirst ("MROS*.",&fblock,0) == 0)
		{
			strcpy (name,fblock.ff_name);
			if (Pexec (0,name,NULL,NULL) == 0)
				return(0);
		}
	}
	printf("MROS not found! \n");
	return -1;
}


GLOBAL BOOLEAN 		term_mros		_((VOID))
{
	return TRUE;
}
