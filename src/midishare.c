/*****************************************************************************/
/*                                                                           */
/* Modul: MIDISHARE.C                                                        */
/*                                                                           */
/* MidiShare client glue for the GNU C cross compiler, replacing the Pure C  */
/* library LIBMIDI.LIB. The entry points themselves are inline functions in  */
/* msh_gcc.h; this file holds the two routines that are not kernel calls.    */
/*                                                                           */
/* MidiShare (MIDSHARE.PRG) hooks the trap #5 and trap #6 vectors ($94 and   */
/* $98) and places the signature "MIDISHARE" 18 bytes in front of each      */
/* handler. MidiShare() checks that signature behind vector $94.             */
/*                                                                           */
/* TCMidiRestore() re-installs both vectors from the file "midisave" that    */
/* MIDISAVE.PRG writes after MIDSHARE.PRG was started by Pexec() from the    */
/* application (see init_msh in msh.c); the OS may reset the vectors when    */
/* that child process terminates.                                            */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <mint/osbind.h>

#include "import.h"
#include "msh_unit.h"

#define MSH_SIGNATURE     "MIDISHARE"
#define MSH_SIG_LEN       9
#define MSH_SIG_OFFSET    18                 /* Signatur liegt 18 Bytes vor dem Handler */
#define MSH_VEC_TRAP5     ((long volatile *) 0x94L)
#define MSH_VEC_TRAP6     ((long volatile *) 0x98L)

static long msh_saved_vectors [2];           /* Inhalt der Datei "midisave": $94, $98 */

/*****************************************************************************/
/* Signaturpruefung, nur im Supervisor-Modus aufrufen                        */
/*****************************************************************************/

static long msh_signature_at (long handler)
{
	if (handler < MSH_SIG_OFFSET) return (0L);
	return (memcmp ((const char *) handler - MSH_SIG_OFFSET, MSH_SIGNATURE, MSH_SIG_LEN) == 0);
} /* msh_signature_at */

static long msh_probe (void)
{
	return (msh_signature_at (*MSH_VEC_TRAP5));
} /* msh_probe */

static long msh_restore (void)
{
	if (! msh_signature_at (msh_saved_vectors [0])) return (0L);
	*MSH_VEC_TRAP5 = msh_saved_vectors [0];
	*MSH_VEC_TRAP6 = msh_saved_vectors [1];
	return (1L);
} /* msh_restore */

/*****************************************************************************/
/* Ist MidiShare installiert?                                                */
/*****************************************************************************/

Boolean MidiShare (void)
{
	return ((Boolean) Supexec (msh_probe));
} /* MidiShare */

/*****************************************************************************/
/* Vektoren aus "midisave" wiederherstellen; Rueckgabe 1 bei Erfolg          */
/*****************************************************************************/

short TCMidiRestore (void)
{
	const char *name = (Drvmap () & 4) ? "C:\\midisave" : "A:\\midisave";
	FILE       *f;
	size_t     n;

	f = fopen (name, "rb");
	if (f == NULL)
	{
		printf ("MidiRestore: error opening %s.\n", name);
		return (0);
	} /* if */
	n = fread (msh_saved_vectors, sizeof (long), 2, f);
	fclose (f);
	if (n != 2)
	{
		printf ("MidiRestore: error reading %s.\n", name);
		return (0);
	} /* if */
	if (! Supexec (msh_restore))
	{
		printf ("MidiRestore: error bad pointer for MidiShare.\n");
		return (0);
	} /* if */
	return (1);
} /* TCMidiRestore */
