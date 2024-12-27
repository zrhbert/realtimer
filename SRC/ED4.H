/*****************************************************************************/
/*                                                                           */
/* Modul: ED4.H                                                              */
/* Datum: 21/12/94                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef __ED4__
#define __ED4__

/****** DEFINES **************************************************************/

/****** TYPES ****************************************************************/

/* Message-Kommunikation */

typedef struct{
	SO_P		in1;			/* Input fr Message-Handler */
	SO_P		in2;			/* Input fr Message-Handler */
	SO_P		out1;			/* Output fr Message-Handler */
	SO_P		out2;			/* Output fr Message-Handler */
	LONG		long_in;		/* Input fr Message-Handler */
	LONG		long_out;	/* Output fr Message-Handler */
}	ED4_MSG;

/* Message-Types */
enum {
		GET_FIRST_EVENT	= CLASS_ED4<<8,		/* CUE-List abfragen */
		GET_NEXT_EVENT,
		GET_PREV_EVENT,
		MODIFY_EVENT,
		DELETE_EVENT,
		INSERT_EVENT,
		GET_NUM_EVENTS
};

/****** VARIABLES ************************************************************/

/****** FUNCTIONS ************************************************************/

GLOBAL BOOLEAN init_ed4  _((VOID));

#endif /* __ED4__ */

