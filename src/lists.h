/*****************************************************************************/
/*                                                                           */
/* Modul: LISTS.H                                                            */
/* Datum: 25/02/94                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef __LISTS__
#define __LISTS__

/****** DEFINES **************************************************************/
#define list_insert list_rear_insert

/****** TYPES ****************************************************************/
typedef int (COMPARE_F) (VOID*, VOID*);
typedef VOID* KEYTYPE;

typedef struct list *LIST_P;
typedef struct list
{
	LIST_P	next;
	LIST_P	prev;
	KEYTYPE	key;
} LIST;

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

GLOBAL VOID   list_front_insert	(LIST_P header, LIST_P element);GLOBAL VOID   list_rear_insert	(LIST_P header, LIST_P element);GLOBAL VOID   list_remove 			(LIST_P element);GLOBAL LIST_P list_sort 			(LIST_P header, COMPARE_F compare);GLOBAL LIST_P list_filter 			(LIST_P header, LIST_P filter, COMPARE_F compare);GLOBAL LIST_P list_unite 			(LIST_P header1, LIST_P header2, COMPARE_F compare);GLOBAL LIST_P list_intersect 		(LIST_P header1, LIST_P header2, COMPARE_F compare);GLOBAL LIST_P list_concat 			(LIST_P header1, LIST_P header2);GLOBAL LIST_P list_new_el 			(KEYTYPE key);GLOBAL VOID   list_destroy_el 	(LIST_P element);GLOBAL VOID   list_empty   		(LIST_P header);GLOBAL VOID   list_destroy 		(LIST_P header);GLOBAL VOID   list_delete			(LIST_P element);GLOBAL LIST_P list_search  		(LIST_P header, KEYTYPE key);GLOBAL LIST_P list_search_cmp		(LIST_P header, KEYTYPE key, COMPARE_F compare);GLOBAL LIST_P list_create  		(VOID);
GLOBAL LIST_P	list_prev	 		(LIST_P element);
GLOBAL LIST_P	list_next	 		(LIST_P element);

#endif /* __LISTS__ */

