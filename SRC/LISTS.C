/*****************************************************************************/
/*                                                                           */
/* Modul: LISTS.C                                                            */
/*                                                                           */
/* List Management                                                           */
/*****************************************************************************/
#define LISTVERSION "V 1.01"
#define LISTDATE "18.12.94"

/*****************************************************************************
- local definition of 'new'
- added some comments
24.02.94	0.02
- list_insert now as a makro for list_rear_insert
- list_new_el now with NULL-check
- list_xxxx_insert now with NULL-check
*****************************************************************************/

#include "import.h"

#include "export.h"
#include "lists.h"

/****** DEFINES **************************************************************/
#define DEBUG(level, x)  #define new(type) (type*) malloc (sizeof(type))

/****** TYPES ****************************************************************/

/****** VARIABLES ************************************************************/

/****** PROTOTYPES ***********************************************************/

/****** FUNCTIONS ************************************************************/

GLOBAL VOID list_front_insert (LIST_P header, LIST_P element){	/* insert an element at the beginning of the list */		DEBUG(3, printf("list: list_front_insert\n"););		if (element == NULL || header == NULL) return;
	
	element->next = header->next;	element->prev = header;	element->next->prev = element;	header->next  = element;
} /* list_front_insert */GLOBAL VOID list_rear_insert (LIST_P header, LIST_P element){	/* insert an element at the end of a list */	DEBUG(3, printf("list: list_rear_insert\n"););		if (element == NULL || header == NULL) return;

	element->prev = header->prev;	element->next = header;	element->prev->next = element;	header->prev  = element;} /* list_rear_insert */GLOBAL VOID list_remove (LIST_P element){  /* remove an element from a list without destroying it */  DEBUG(3, printf("list: list_remove\n"););  element->prev->next = element->next;  element->next->prev = element->prev;  element->next = NULL;  element->prev = NULL;} /* list_remove */GLOBAL LIST_P list_search_cmp (LIST_P header, KEYTYPE key, COMPARE_F compare){  /* find a list member using the given key and comparison function */  LIST_P element;  DEBUG(3, printf("list: list_search\n"););  if (header) {    element = header->next;    while (compare (element->key,  key) !=0 && element != header) {      element = element->next;    } /* while */  } /* if header */  return element;} /* list_search_cmp */GLOBAL LIST_P list_filter (LIST_P header, LIST_P filter, COMPARE_F compare){  /* Remove from the list all the elements that     don't match the given filter element and comparison function */  LIST_P element, tmp;  DEBUG(3, printf("list: list_filter\n"););  if (compare && header) {    element = header->next;    while (element != header) {      tmp = element;      element = element->next;      /* if the comparison returns != 0 then the element is not ok */       /* remove that element */      if (compare (tmp->key,  filter->key) !=0 )	list_destroy_el (tmp);    } /* while */  } /* if */  return header;} /* list_filter */
#if 0		/* geht noch nicht */GLOBAL LIST_P list_sort (LIST_P header, COMPARE_F compare){  LONG counter = 0;  LIST_P element;  KEYTYPE  keys;  DEBUG(3, printf("list: list_sort\n"););  if (header && compare) {    /* count elements */    element = header->next;    while (element != header) {      counter++;      element = element->next;    } /* while */    /* check that list contains something */    if (counter > 0) {          keys = (KEYTYPE*) malloc(sizeof(KEYTYPE) * counter);      /* count elements */      /* copy the key pointers into an array for qsort */      counter = 0;      element = header->next;      while (element != header) {			keys[counter] = element->key;			element = element->next;			counter++;      } /* while */      /* call qsort with the compare function */      qsort (keys, counter, sizeof(KEYTYPE), compare);      /* copy the key fields back from the array into the list */      counter = 0;      element = header->next;      while (element != header) {			element->key = keys[counter];			element = element->next;			counter++;      }  /* while */      /* free up the array */      free (keys);    } /* if */  } /* if header */  return element;} /* list_sort */#endif
GLOBAL LIST_P list_unite (LIST_P header1, LIST_P header2, COMPARE_F compare){  /* unite two lists, throwing out doubles. compare() is used to check     for identity of two list elements */  LIST_P element1, element2, junk, header;  BOOLEAN equal;  DEBUG(3, printf("list: list_unite\n"););  if (header1 && header2) {	/* build a new list containing the old two lists */	header = list_concat(header1, header2);		/* weed out doubles */	element1 = header->next;	while (element1 != header) {		equal = FALSE;		element2 = header->next;		/* compare to all the other elements in the list */		while (element2 != header && !equal) {		/* only compare distinct elements */		if (element1 != element2)		equal = compare (element1->key, element2->key);		element2 = element2->next;	} /* while */	junk = element1;	element1 = element1->next;	if (equal) {		/* kill that element */		list_destroy_el (junk);	} /* if */  } /* while */  } /* if header */  return header;} /* list_unite */GLOBAL LIST_P list_intersect (LIST_P header1, LIST_P header2, COMPARE_F compare){  /* intersect two lists. compare() is used to check     for identity of two list elements */  LIST_P element1, element2, header;  BOOLEAN equal;  DEBUG(3, printf("list: list_intersect\n"););  if (header1 && header2) {  /* build a new list containing the old two lists */  header = list_create();  /* weed out doubles */  element1 = header1->next;  while (element1 != header) {    equal = FALSE;    element2 = header2->next;    /* compare to all the other elements in the list */    while (element2 != header2 && !equal) {      /* only compare distinct elements */      equal = compare (element1->key, element2->key);      element2 = element2->next;    } /* while */    if (equal) {      /* add that element */      list_rear_insert (header, list_new_el(element1->key));    } /* if */    element1 = element1->next;  } /* while */  } /* if header */  return header;} /* list_intersect */GLOBAL LIST_P list_concat (LIST_P header1, LIST_P header2){  /* concatenate two lists by copying the elements to a new list */  LIST_P header, element;    DEBUG(3, printf("list: list_concat\n"););  if (header1 && header2) {  /* generate a new list */  header = list_create();  /* copy first list */  element = header1;  while (element->next != header1) {    list_rear_insert (header, list_new_el (element->key));    element = element->next;  } /* while */  /* copy second list */  element = header2;  while (element->next != header2) {    list_rear_insert (header, list_new_el (element->key));    element = element->next;  } /* while */  } /* if header */  return header;} /* list_concat */GLOBAL LIST_P list_new_el (KEYTYPE key){	/* Generate a new list element and copy the information into it */		LIST_P element;		DEBUG(3, printf("list: list_new_el\n"););		if (key == NULL) return NULL;

	element = new(LIST);	element->key = key;	return element;} /* list_new_el */GLOBAL VOID list_destroy_el (LIST_P element){  /* destroy and remove from memory      one list element and free its data area */   DEBUG(3, printf("list: list_destroy\n"););  list_remove (element);  free(element);} /* list_destroy_el */GLOBAL LIST_P list_create (){  /* create a new list, with a header element */  LIST_P header;   DEBUG(3, printf("list: list_create\n"););  header = new(LIST);  header->key = 0;  header->next = header;  header->prev = header;  return header;} /* list_create */GLOBAL VOID list_empty (LIST_P header){  /* destroy and remove from memory     all the elements of a list */  LIST_P element;  DEBUG(3, printf("list: list_empty\n"););  if (header) {	  while (header->next != header) {	    element = header->next;	    list_destroy_el (element);	  } /* while */  } /* if header */} /* list_empty */GLOBAL VOID list_destroy (LIST_P header){  /* destroy and remove from memory      an entire list, including the header */  DEBUG(3, printf("list: list_destroy\n"););  if (header) {    list_empty (header);    list_destroy (header);  } /* if header */} /* list_destroy */GLOBAL LIST_P	list_search 	(LIST_P header, KEYTYPE key)
{
	/* Find an element in the list, searching by key */
	LIST_P	element;
	
	element = header->next;
	while ((element != header) && (element->key != key))
	{
		element = element->next;
	} /* while */
	
	return element;
} /* list_search */

GLOBAL VOID	list_delete 	(LIST_P element)
{
	/* Delete an element in the list */
	element->prev->next	= element->next;
	element->next->prev	= element->prev;
} /* list_delete */

GLOBAL LIST_P	list_prev 	(LIST_P element)
{
	/* Get the previous element in the list */
	return element->prev;
} /* list_prev */

GLOBAL LIST_P	list_next 	(LIST_P element)
{
	/* Get the next element in the list */
	return element->next;
} /* list_next */
