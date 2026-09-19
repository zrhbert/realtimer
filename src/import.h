/*****************************************************************************/
/*                                                                           */
/* IMPORT.H                                                                  */
/* Datum: 02.02.95                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
- minmax eingebaut, 02.02.95
*****************************************************************************/
#ifndef __IMPORT__
#define __IMPORT__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <portab.h>
/* #include <aes.h> */
/* #include <vdi.h> */
#define rc_copy gemlib_rc_copy
#define rc_equal gemlib_rc_equal
#define rc_intersect gemlib_rc_intersect
#include <gem.h>

/* math.h only defines M_PI for BSD/XOPEN/PureC feature-test macros, which
   -std=c99 disables; provide it directly since it's the only constant used. */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#undef rc_copy
#undef rc_equal
#undef rc_intersect

#ifdef STRING
#undef STRING
#endif

#ifndef WF_CXYWH
#define WF_CXYWH WF_CURRXYWH
#endif

#define WF_WXYWH WF_WORKXYWH
#define WF_PXYWH WF_PREVXYWH
#define WF_FXYWH WF_FULLXYWH

#ifndef OB_TYPE
#define OB_TYPE(tree, id) ((tree)[id].ob_type & 0x00FF)
#define OB_FLAGS(tree, id) ((tree)[id].ob_flags)
#define OB_STATE(tree, id) ((tree)[id].ob_state)
#endif

/* Old-style raw access to the (now union) ob_spec field; treat it as a
   string pointer, matching the realtimer call sites which use it that way. */
#define OB_SPEC(tree, id) ((tree)[id].ob_spec.free_string)

#define OB_EXTYPE(tree, id) ((tree)[id].ob_type >> 8)
#define OW_NOCHANGE 255
#define SQUARED SQUARE

/* vs_mute() action codes (from the old Pure-C VDI.H; not in the SDK) */
#define MUTE_ENABLE  0
#define MUTE_DISABLE 1
#define ALI_LEFT 0
#define ALI_TOP 5

#define mp fd_addr
#define fwp fd_w
#define fh fd_h
#define fww fd_wdwidth
#define np fd_nplanes
#define ff fd_stand

#include "winclass.h"	/* BD */

#if GEMDOS
#if TURBO_C
#include <tos.h>
#else
#include <osbind.h>
#endif
typedef _DTA DTA;
#define d_attrib dta_attribute
#define d_time dta_time
#define d_date dta_date
#define d_length dta_size
#define d_fname dta_name
#define Mavail() (LONG)Malloc (-1L)
typedef _BCONMAP BCONMAP;
#endif

#if MSDOS | OS2 | FLEXOS
#include <gemdos.h>
#include <dosbind.h>
#endif

#if UNIX
#include <sys/types.h>
#define Mavail() (64 * 1024L)                 /* In UNIX ist immer Speicher frei */
#endif

#if ANSI
#include <stdlib.h>
#else
#define abs(x)      ((x) <  0  ? -(x) : (x))  /* Absolut-Wert */
#define labs(x)     abs (x)                   /* Langer Absolut-Wert */
#define fabs(x)     abs (x)                   /* Double Absolut-Wert */
#endif

/****** DEFINES **************************************************************/

#ifdef GLOBAL
#undef GLOBAL
#endif

#define GLOBAL EXTERN

#if LASER_C
#define strchr  index
#define strrchr rindex
#endif

#if HIGH_C
#ifdef NULL
#undef NULL
#define NULL 0L
#endif
#endif

/* math.h's own max/min macros use bare "typeof", which isn't available
   under -std=c99; undefine them and use the plain ternary versions instead. */
#ifdef max
#undef max
#undef min
#endif
#define max(a,b)    (((a) > (b)) ? (a) : (b)) /* Maximum-Funktion */
#define min(a,b)    (((a) < (b)) ? (a) : (b)) /* Minimum Funktion */

#ifndef minmax
#define minmax(value, minimum, maximum)	max(minimum, min(value, maximum))
#endif

#define odd(i)      ((i) & 1)                 /* ungerade */

/*****************************************************************************/

#ifdef PASCAL_DEF
#define and         &&                        /* F�r Pascal-Programmierer */
#define or          ||
#define xor         ^^
#define not         !
#define div         /
#define mod         %

#define bitand      &
#define bitor       |
#define bitxor      ^
#define bitnot      ~

#define loop        for (;;)
#define exitloop(e) if (e) break
#define nextloop(e) if (e) continue

#define repeat      do {
#define until(e)    } while (! (e))

#define begin       {
#define end         }

#define then

#define boolean     BOOLEAN
#define integer     WORD
#define longint     LONG
#define real        FLOAT
#define longreal    DOUBLE

#define type        typedef
#endif /* PASCAL_DEF */

/*****************************************************************************/

#ifdef MODULA_DEF
#define AND         &&                        /* F�r Modula-Programmierer */
#define OR          ||
#define XOR         ^^
#define NOT         !
#define DIV         /
#define MOD         %

#define BITAND      &
#define BITOR       |
#define BITXOR      ^
#define BITNOT      ~

#define LOOP        for (;;)
#define EXITLOOP(e) if (e) break
#define NEXTLOOP(e) if (e) continue

#define REPEAT      do {
#define UNTIL(e)    } while (! (e))

#define BEGIN       {
#define END         }

#define WHILE(e)    while (e) {
#define IF(e)       if (e) {
#define THEN
#define ELSE        } else {
#define ELSIF(e)    } else if (e) {

#define CASE(e)     switch (e) {
#define OF

#define RETURN      return

#define INTEGER     WORD
#define LONGINT     LONG
#define CARDINAL    UWORD
#define LONGCARD    ULONG
#define REAL        FLOAT
#define LONGREAL    DOUBLE
#define BITSET      UWORD
#define LONGBITSET  ULONG

#define TYPE        typedef
#endif /* MODULA_DEF */

/*****************************************************************************/
/* Turbo-C's ext.h provided delay(); the modern SDK has no equivalent, so we */
/* implement it here via evnt_timer() (the only symbol actually used from   */
/* ext.h by the realtimer sources).                                        */
/*****************************************************************************/

#define delay(ms) evnt_timer ((LONG)(ms))

/*****************************************************************************/

#endif /* __IMPORT__ */

