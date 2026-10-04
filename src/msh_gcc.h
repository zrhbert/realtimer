/*****************************************************************************/
/*                                                                           */
/* Modul: MSH_GCC.H                                                          */
/*                                                                           */
/* MidiShare entry points for the GNU C cross compiler.                      */
/*                                                                           */
/* The Pure C client library reaches the MidiShare kernel by pushing the     */
/* arguments right to left (16-bit words for short/Boolean, 32-bit for       */
/* long and pointers) followed by the function number as a word, and then    */
/* jumping to the kernel through trap #6. GCC's int is 32 bits and its       */
/* variadic calls promote everything, so the Pure C macro trick cannot be    */
/* used. Instead every entry point is an inline function with an explicit    */
/* stack frame that enters the kernel through trap #5, whose handler expects */
/* the function number directly on the caller's stack (the trap #6 handler  */
/* expects a return address in front of it). The kernel returns in d0 and   */
/* treats d0-d2/a0-a2 as scratch, like a Pure C cdecl callee.               */
/*                                                                           */
/* Generated from the macro table in msh_unit.h; keep both in sync.          */
/*                                                                           */
/*****************************************************************************/

#ifndef __MSH_GCC__
#define __MSH_GCC__

#define MSH_CLOBBERS "d1", "d2", "a0", "a1", "a2", "memory", "cc"

/* MidiGetVersion (void) => short ; function 0x0 */
static inline short MidiGetVersion (void)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w #0,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #2,%%sp"
		: "=r" (__r)
		:
		: MSH_CLOBBERS);
	return (short) __r;
} /* MidiGetVersion */

/* MidiCountAppls (void) => short ; function 0x1 */
static inline short MidiCountAppls (void)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w #1,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #2,%%sp"
		: "=r" (__r)
		:
		: MSH_CLOBBERS);
	return (short) __r;
} /* MidiCountAppls */

/* MidiGetIndAppl (short index) => short ; function 0x2 */
static inline short MidiGetIndAppl (short index)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #2,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (index)
		: MSH_CLOBBERS);
	return (short) __r;
} /* MidiGetIndAppl */

/* MidiGetNamedAppl (MidiName name) => short ; function 0x3 */
static inline short MidiGetNamedAppl (const void * name)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #3,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (name)
		: MSH_CLOBBERS);
	return (short) __r;
} /* MidiGetNamedAppl */

/* MidiGetSyncInfo (SyncInfoPtr p) => void ; function 0x39 */
static inline void MidiGetSyncInfo (const void * p)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #57,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (p)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiGetSyncInfo */

/* MidiSetSyncMode (unsigned short mode) => void ; function 0x3a */
static inline void MidiSetSyncMode (unsigned short mode)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #58,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (mode)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSetSyncMode */

/* MidiGetExtTime (void) => long ; function 0x3e */
static inline long MidiGetExtTime (void)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w #62,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #2,%%sp"
		: "=r" (__r)
		:
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiGetExtTime */

/* MidiInt2ExtTime (long time) => long ; function 0x3f */
static inline long MidiInt2ExtTime (long time)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #63,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (time)
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiInt2ExtTime */

/* MidiExt2IntTime (long time) => long ; function 0x40 */
static inline long MidiExt2IntTime (long time)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #64,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (time)
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiExt2IntTime */

/* MidiTime2Smpte (long time, short format, SmpteLocPtr loc) => void ; function 0x41 */
static inline void MidiTime2Smpte (long time, short format, const void * loc)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %3,-(%%sp)\n\t"
		"move.w %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #65,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 12(%%sp),%%sp"
		: "=r" (__r)
		: "r" (time), "r" (format), "r" (loc)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiTime2Smpte */

/* MidiSmpte2Time (SmpteLocPtr loc) => long ; function 0x42 */
static inline long MidiSmpte2Time (const void * loc)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #66,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (loc)
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiSmpte2Time */

/* MidiOpen (MidiName applName) => short ; function 0x4 */
static inline short MidiOpen (const void * applName)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #4,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (applName)
		: MSH_CLOBBERS);
	return (short) __r;
} /* MidiOpen */

/* MidiClose (short refNum) => void ; function 0x5 */
static inline void MidiClose (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #5,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiClose */

/* MidiGetName (short refNum) => MidiName ; function 0x6 */
static inline char * MidiGetName (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #6,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	return (char *) __r;
} /* MidiGetName */

/* MidiSetName (short refNum, MidiName applName) => void ; function 0x7 */
static inline void MidiSetName (short refNum, const void * applName)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #7,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #8,%%sp"
		: "=r" (__r)
		: "r" (refNum), "r" (applName)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSetName */

/* MidiGetInfo (short refNum) => Ptr ; function 0x8 */
static inline Ptr MidiGetInfo (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #8,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	return (Ptr) __r;
} /* MidiGetInfo */

/* MidiSetInfo (short refNum, Ptr infoZone) => void ; function 0x9 */
static inline void MidiSetInfo (short refNum, const void * infoZone)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #9,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #8,%%sp"
		: "=r" (__r)
		: "r" (refNum), "r" (infoZone)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSetInfo */

/* MidiGetFilter (short refNum) => FilterPtr ; function 0xa */
static inline FilterPtr MidiGetFilter (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #10,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	return (FilterPtr) __r;
} /* MidiGetFilter */

/* MidiSetFilter (short refNum, FilterPtr filter) => void ; function 0xb */
static inline void MidiSetFilter (short refNum, const void * filter)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #11,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #8,%%sp"
		: "=r" (__r)
		: "r" (refNum), "r" (filter)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSetFilter */

/* MidiGetRcvAlarm (short refNum) => RcvAlarmPtr ; function 0xc */
static inline RcvAlarmPtr MidiGetRcvAlarm (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #12,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	return (RcvAlarmPtr) __r;
} /* MidiGetRcvAlarm */

/* MidiSetRcvAlarm (short refNum, RcvAlarmPtr alarm) => void ; function 0xd */
static inline void MidiSetRcvAlarm (short refNum, const void * alarm)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #13,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #8,%%sp"
		: "=r" (__r)
		: "r" (refNum), "r" (alarm)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSetRcvAlarm */

/* MidiGetApplAlarm (short refNum) => ApplAlarmPtr ; function 0xe */
static inline ApplAlarmPtr MidiGetApplAlarm (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #14,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	return (ApplAlarmPtr) __r;
} /* MidiGetApplAlarm */

/* MidiSetApplAlarm (short refNum, ApplAlarmPtr alarm) => void ; function 0xf */
static inline void MidiSetApplAlarm (short refNum, const void * alarm)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #15,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #8,%%sp"
		: "=r" (__r)
		: "r" (refNum), "r" (alarm)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSetApplAlarm */

/* MidiConnect (short src, short dest, Boolean state) => void ; function 0x10 */
static inline void MidiConnect (short src, short dest, Boolean state)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %3,-(%%sp)\n\t"
		"move.w %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #16,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #8,%%sp"
		: "=r" (__r)
		: "r" (src), "r" (dest), "r" (state)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiConnect */

/* MidiIsConnected (short src, short dest) => Boolean ; function 0x11 */
static inline Boolean MidiIsConnected (short src, short dest)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #17,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (src), "r" (dest)
		: MSH_CLOBBERS);
	return (Boolean) __r;
} /* MidiIsConnected */

/* MidiGetPortState (short port) => Boolean ; function 0x12 */
static inline Boolean MidiGetPortState (short port)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #18,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (port)
		: MSH_CLOBBERS);
	return (Boolean) __r;
} /* MidiGetPortState */

/* MidiSetPortState (short port, Boolean state) => void ; function 0x13 */
static inline void MidiSetPortState (short port, Boolean state)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #19,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (port), "r" (state)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSetPortState */

/* MidiFreeSpace (void) => long ; function 0x14 */
static inline long MidiFreeSpace (void)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w #20,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #2,%%sp"
		: "=r" (__r)
		:
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiFreeSpace */

/* MidiNewCell (void) => MidiEvPtr ; function 0x33 */
static inline MidiEvPtr MidiNewCell (void)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w #51,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #2,%%sp"
		: "=r" (__r)
		:
		: MSH_CLOBBERS);
	return (MidiEvPtr) __r;
} /* MidiNewCell */

/* MidiFreeCell (MidiEvPtr ev) => void ; function 0x34 */
static inline void MidiFreeCell (const void * ev)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #52,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (ev)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiFreeCell */

/* MidiNewEv (short typeNum) => MidiEvPtr ; function 0x15 */
static inline MidiEvPtr MidiNewEv (short typeNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #21,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (typeNum)
		: MSH_CLOBBERS);
	return (MidiEvPtr) __r;
} /* MidiNewEv */

/* MidiCopyEv (MidiEvPtr ev) => MidiEvPtr ; function 0x16 */
static inline MidiEvPtr MidiCopyEv (const void * ev)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #22,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (ev)
		: MSH_CLOBBERS);
	return (MidiEvPtr) __r;
} /* MidiCopyEv */

/* MidiFreeEv (MidiEvPtr ev) => void ; function 0x17 */
static inline void MidiFreeEv (const void * ev)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #23,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (ev)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiFreeEv */

/* MidiSetField (MidiEvPtr ev, short f, long v) => void ; function 0x18 */
static inline void MidiSetField (const void * ev, short f, long v)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %3,-(%%sp)\n\t"
		"move.w %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #24,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 12(%%sp),%%sp"
		: "=r" (__r)
		: "r" (ev), "r" (f), "r" (v)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSetField */

/* MidiGetField (MidiEvPtr ev, short f) => long ; function 0x19 */
static inline long MidiGetField (const void * ev, short f)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #25,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #8,%%sp"
		: "=r" (__r)
		: "r" (ev), "r" (f)
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiGetField */

/* MidiAddField (MidiEvPtr ev, long v) => void ; function 0x1a */
static inline void MidiAddField (const void * ev, long v)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #26,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 10(%%sp),%%sp"
		: "=r" (__r)
		: "r" (ev), "r" (v)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiAddField */

/* MidiCountFields (MidiEvPtr ev) => short ; function 0x1b */
static inline short MidiCountFields (const void * ev)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #27,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (ev)
		: MSH_CLOBBERS);
	return (short) __r;
} /* MidiCountFields */

/* MidiNewSeq (void) => MidiSeqPtr ; function 0x1d */
static inline MidiSeqPtr MidiNewSeq (void)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w #29,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #2,%%sp"
		: "=r" (__r)
		:
		: MSH_CLOBBERS);
	return (MidiSeqPtr) __r;
} /* MidiNewSeq */

/* MidiAddSeq (MidiSeqPtr s, MidiEvPtr ev) => void ; function 0x1e */
static inline void MidiAddSeq (const void * s, const void * ev)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #30,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 10(%%sp),%%sp"
		: "=r" (__r)
		: "r" (s), "r" (ev)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiAddSeq */

/* MidiFreeSeq (MidiSeqPtr s) => void ; function 0x1f */
static inline void MidiFreeSeq (const void * s)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #31,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (s)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiFreeSeq */

/* MidiClearSeq (MidiSeqPtr s) => void ; function 0x20 */
static inline void MidiClearSeq (const void * s)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #32,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (s)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiClearSeq */

/* MidiApplySeq (MidiSeqPtr s, ApplyProcPtr ProcPtr) => void ; function 0x21 */
static inline void MidiApplySeq (const void * s, const void * ProcPtr)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #33,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 10(%%sp),%%sp"
		: "=r" (__r)
		: "r" (s), "r" (ProcPtr)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiApplySeq */

/* MidiGetTime (void) => long ; function 0x22 */
static inline long MidiGetTime (void)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w #34,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #2,%%sp"
		: "=r" (__r)
		:
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiGetTime */

/* MidiSendIm (short refNum, MidiEvPtr ev) => void ; function 0x23 */
static inline void MidiSendIm (short refNum, const void * ev)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #35,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #8,%%sp"
		: "=r" (__r)
		: "r" (refNum), "r" (ev)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSendIm */

/* MidiSend (short refNum, MidiEvPtr ev) => void ; function 0x24 */
static inline void MidiSend (short refNum, const void * ev)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #36,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #8,%%sp"
		: "=r" (__r)
		: "r" (refNum), "r" (ev)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSend */

/* MidiSendAt (short refNum, MidiEvPtr ev, long d) => void ; function 0x25 */
static inline void MidiSendAt (short refNum, const void * ev, long d)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %3,-(%%sp)\n\t"
		"move.l %2,-(%%sp)\n\t"
		"move.w %1,-(%%sp)\n\t"
		"move.w #37,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 12(%%sp),%%sp"
		: "=r" (__r)
		: "r" (refNum), "r" (ev), "r" (d)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiSendAt */

/* MidiCountEvs (short refNum) => long ; function 0x26 */
static inline long MidiCountEvs (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #38,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiCountEvs */

/* MidiGetEv (short refNum) => MidiEvPtr ; function 0x27 */
static inline MidiEvPtr MidiGetEv (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #39,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	return (MidiEvPtr) __r;
} /* MidiGetEv */

/* MidiAvailEv (short refNum) => MidiEvPtr ; function 0x28 */
static inline MidiEvPtr MidiAvailEv (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #40,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	return (MidiEvPtr) __r;
} /* MidiAvailEv */

/* MidiFlushEvs (short refNum) => void ; function 0x29 */
static inline void MidiFlushEvs (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #41,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiFlushEvs */

/* MidiReadSync (Ptr adrMem) => Ptr ; function 0x2a */
static inline Ptr MidiReadSync (const void * adrMem)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #42,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (adrMem)
		: MSH_CLOBBERS);
	return (Ptr) __r;
} /* MidiReadSync */

/* MidiWriteSync (Ptr adrMem, Ptr val) => Ptr ; function 0x2b */
static inline Ptr MidiWriteSync (const void * adrMem, const void * val)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #43,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 10(%%sp),%%sp"
		: "=r" (__r)
		: "r" (adrMem), "r" (val)
		: MSH_CLOBBERS);
	return (Ptr) __r;
} /* MidiWriteSync */

/* MidiCall (TaskPtr proc, long date, short refNum, long a1, long a2, long a3) => void ; function 0x2c */
static inline void MidiCall_ (const void * proc, long date, short refNum, long a1, long a2, long a3)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %6,-(%%sp)\n\t"
		"move.l %5,-(%%sp)\n\t"
		"move.l %4,-(%%sp)\n\t"
		"move.w %3,-(%%sp)\n\t"
		"move.l %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #44,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 24(%%sp),%%sp"
		: "=r" (__r)
		: "r" (proc), "r" (date), "r" (refNum), "r" (a1), "r" (a2), "r" (a3)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiCall_ */
#define MidiCall(proc, date, refNum, a1, a2, a3)	MidiCall_ ((proc), (date), (refNum), (long) (a1), (long) (a2), (long) (a3))

/* MidiTask (TaskPtr proc, long date, short refNum, long a1, long a2, long a3) => MidiEvPtr ; function 0x2d */
static inline MidiEvPtr MidiTask_ (const void * proc, long date, short refNum, long a1, long a2, long a3)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %6,-(%%sp)\n\t"
		"move.l %5,-(%%sp)\n\t"
		"move.l %4,-(%%sp)\n\t"
		"move.w %3,-(%%sp)\n\t"
		"move.l %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #45,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 24(%%sp),%%sp"
		: "=r" (__r)
		: "r" (proc), "r" (date), "r" (refNum), "r" (a1), "r" (a2), "r" (a3)
		: MSH_CLOBBERS);
	return (MidiEvPtr) __r;
} /* MidiTask_ */
#define MidiTask(proc, date, refNum, a1, a2, a3)	MidiTask_ ((proc), (date), (refNum), (long) (a1), (long) (a2), (long) (a3))

/* MidiDTask (TaskPtr proc, long date, short refNum, long a1, long a2, long a3) => MidiEvPtr ; function 0x2e */
static inline MidiEvPtr MidiDTask_ (const void * proc, long date, short refNum, long a1, long a2, long a3)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %6,-(%%sp)\n\t"
		"move.l %5,-(%%sp)\n\t"
		"move.l %4,-(%%sp)\n\t"
		"move.w %3,-(%%sp)\n\t"
		"move.l %2,-(%%sp)\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #46,-(%%sp)\n\t"
		"trap #5\n\t"
		"lea 24(%%sp),%%sp"
		: "=r" (__r)
		: "r" (proc), "r" (date), "r" (refNum), "r" (a1), "r" (a2), "r" (a3)
		: MSH_CLOBBERS);
	return (MidiEvPtr) __r;
} /* MidiDTask_ */
#define MidiDTask(proc, date, refNum, a1, a2, a3)	MidiDTask_ ((proc), (date), (refNum), (long) (a1), (long) (a2), (long) (a3))

/* MidiForgetTask (MidiEvPtr * ev) => void ; function 0x2f */
static inline void MidiForgetTask (const void * ev)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #47,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (ev)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiForgetTask */

/* MidiCountDTasks (short refNum) => long ; function 0x30 */
static inline long MidiCountDTasks (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #48,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiCountDTasks */

/* MidiFlushDTasks (short refNum) => void ; function 0x31 */
static inline void MidiFlushDTasks (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #49,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiFlushDTasks */

/* MidiExec1DTask (short refNum) => void ; function 0x32 */
static inline void MidiExec1DTask (short refNum)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w %1,-(%%sp)\n\t"
		"move.w #50,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #4,%%sp"
		: "=r" (__r)
		: "r" (refNum)
		: MSH_CLOBBERS);
	(void) __r;
} /* MidiExec1DTask */

/* MidiTotalSpace (void) => long ; function 0x35 */
static inline long MidiTotalSpace (void)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w #53,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #2,%%sp"
		: "=r" (__r)
		:
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiTotalSpace */

/* MidiGetStatPtr (void) => MidiStatPtr ; function 0x36 */
static inline MidiStatPtr MidiGetStatPtr (void)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.w #54,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #2,%%sp"
		: "=r" (__r)
		:
		: MSH_CLOBBERS);
	return (MidiStatPtr) __r;
} /* MidiGetStatPtr */

/* MidiGrowSpace (long space) => long ; function 0x37 */
static inline long MidiGrowSpace (long space)
{
	register long __r __asm__ ("d0");
	__asm__ __volatile__ (
		"move.l %1,-(%%sp)\n\t"
		"move.w #55,-(%%sp)\n\t"
		"trap #5\n\t"
		"addq.l #6,%%sp"
		: "=r" (__r)
		: "r" (space)
		: MSH_CLOBBERS);
	return (long) __r;
} /* MidiGrowSpace */

#endif /* __MSH_GCC__ */
