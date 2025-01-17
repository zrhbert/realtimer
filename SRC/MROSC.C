/*****************************************************************************/
/*                                                                           */
/* Modul: MROSC.C                                                            */
/* Datum: 13/01/93                                                           */
/*                                                                           */
/* M-ROS Funktionen fÅr RTM                                               	  */
/*****************************************************************************/

#include "import.h"
#include "global.h"
#include "ext.h"
#include "realtspc.h"

#include "export.h"
#include "mrosc.h"

#pragma warn -par
#pragma warn -sig

/****** FUNCTIONS ************************************************************/

/* MEM Functions */

GLOBAL	int CDECL	open_mros(MEM_STRUCT *mem_struct)
{
	return MEM(0);
}

GLOBAL	int	CDECL		close_mros(int mros_hand)
{
	return MEM(1);
}

GLOBAL	long CDECL	snd_message(int appl_no,long opcode,long msg2,long msg3)
{
	return MEM(2);
}

GLOBAL	long CDECL	appl_req(int appl_code,int appl_count)
{
	return MEM(3);
}

GLOBAL	long CDECL	get_vectab(void, ...)
{
	return MEM(4);
}

GLOBAL	long CDECL	terminal_req(void, ...)
{
	return MEM(5);
}

GLOBAL	long CDECL	swtch(void, ...)
{
	return MEM(6);
}

/* IOM Functions */

GLOBAL	int CDECL		dev_req(int dev)
{
	return IOM(0);
}

GLOBAL	int CDECL		snd_msingl(int dev,int out,int data)
{
	return IOM(1);
}

GLOBAL	int CDECL		snd_mevent(int dev,int out,int data1,int data2,int data3)
{
char	evnt[4];
void	*stack;
void	mevntout(long data);
long	Super(void *stack);

	stack = (void *)Super(0);
	evnt[0] = data1;
	evnt[1] = data2;
	evnt[2] = data3;
	evnt[3] = (dev << 4) | out;
	mevntout(*(long *)evnt);
	Super(stack);
	return 0;
}

GLOBAL	int CDECL		snd_mmult(int dev,int out,unsigned char *data,int lenght,int opt)
{
	return IOM(3);
}

GLOBAL	int CDECL		out_mreq(int dev,int out)
{
	return IOM(4);
}

GLOBAL	int CDECL		mget(int dev,int inp)
{
	return IOM(5);
}

GLOBAL	int CDECL		inp_mreq(int dev,int inp)
{
	return IOM(6);
}

GLOBAL	long CDECL	remdat(int dev,int io,int ioswitch)
{
	return IOM(7);
}

GLOBAL	int CDECL		set_mbuf(int dev,int io,int ioswitch,void *buffer,int lenght)
{
	return IOM(8);
}

GLOBAL	int CDECL		rst_mbuf(int dev,int io,int ioswitch)
{
	return IOM(9);
}

GLOBAL	int CDECL		dev_reset(int dev)
{
	return IOM(10);
}

GLOBAL	int CDECL		run_stat(int dev,int out,int flag)
{
	return IOM(11);
}

GLOBAL	long CDECL	get_iorec(void, ...)
{
	return MEM(15);
}

GLOBAL	int CDECL		open_io(IO_STRUCT *io_struct,int mros_hand)
{
	return IOM(16);
}

GLOBAL	int CDECL		close_io(int io_hand)
{
	return IOM(17);
}

GLOBAL	long CDECL	open_mout(void, ...)
{
	return IOM(18);
}

GLOBAL	long CDECL	open_device(void, ...)
{
	return IOM(20);
}

GLOBAL	long CDECL	get_minmask(void, ...)
{
	return IOM(21);
}

GLOBAL	long CDECL	n_device(void, ...)	/* next_device */
{
	return IOM(22);
}

GLOBAL	long CDECL	get_device(void, ...)
{
	return IOM(23);
}

GLOBAL	void	*iotrapvec(void)
{
	return (void *)IOM(24);
}

GLOBAL	void	*iovec(void)
{
	return (void *)IOM(25);
}

GLOBAL	void	*get_iopar(void)
{
	return (void *)IOM(26);
}


/* TM Functions */

GLOBAL	int	tm_start(void)
{
	return TM(0);
}

GLOBAL	void	tm_stop(void)
{
	TM(1);
}

GLOBAL	int 	tm_cont(void)
{
	return TM(2);
}

GLOBAL	void CDECL	tm_pos(long pos)
{
	TM(3);
}

GLOBAL	int CDECL		tm_play(unsigned long lenght)
{
	return TM(4);
}

GLOBAL	void CDECL	cycle_set(long cyc_start,long cyc_end)
{
	TM(5);
}

GLOBAL	void			cycle_start(void)
{
	TM(6);
}

GLOBAL	void CDECL	cycle_onoff(int onoff)
{
	TM(7);
}

GLOBAL	int CDECL	count_in(unsigned long tempo,int bars,int time_signature)
{
	return TM(8);
}

GLOBAL	void CDECL	tm_forward(unsigned long lenght)
{
	TM(9);
}

GLOBAL	void CDECL	tm_rewind(unsigned long lenght)
{
	TM(10);
}

GLOBAL	void CDECL	timstr(unsigned long pos,char *string)
{
	TM(16);
}

GLOBAL	void CDECL	curtimstr(char *string)
{
	TM(17);
}

GLOBAL	void CDECL	songstr(unsigned long pos,char *string)
{
	TM(18);
}

GLOBAL	void CDECL	cursngstr(char *string)
{
	TM(19);
}

GLOBAL	void CDECL	curtmpstr(char *string)
{
	TM(20);
}

GLOBAL	void CDECL	curtsgstr(char *string)
{
	TM(21);
}

GLOBAL	long CDECL	strtim(void, ...)
{
	return TM(22);
}

GLOBAL	int CDECL		set_frame(int frame)
{
	return TM(24);
}

GLOBAL	int CDECL		tm_sync(int psync,int ssync,int link,int pdev,int pinp,int sdev,int sinp)
{
	return TM(25);
}

GLOBAL	void CDECL	set_tempo(unsigned long tempo)
{
	TM(26);
}

GLOBAL	int CDECL		set_master(void *mastertrack)
{
	/*  /*evtl. Referenz von HP */
	extern char stop_ignore;

	stop_ignore = 1;
	*/
	return TM(27);
}

GLOBAL	int CDECL		mtc_onoff(int dev,int out,int onoff)
{
	return TM(28);
}

GLOBAL	int CDECL		mcl_onoff(int dev,int out,int onoff)
{
	return TM(29);
}

GLOBAL	int CDECL		mclick(long event1,long event2)
{
	return TM(30);
}

GLOBAL	int CDECL		wrt_smpte(int dev,int onoff,unsigned long start_time)
{
	return TM(31);
}

GLOBAL	int CDECL		open_tm(TM_STRUCT *tm_struct,int mros_hand)
{
	return TM(32);
}

GLOBAL	int CDECL		close_tm(int tm_hand)
{
	return TM(33);
}

GLOBAL	int CDECL		store_loc(int locno,long pos)
{
	return TM(34);
}

GLOBAL	int CDECL		get_loc(int locno)
{
	return TM(35);
}

GLOBAL	void				*get_tmpar(void)
{
	return (void *)TM(40);
}

GLOBAL	void				*get_tmtrap(void)
{
	return (void *)TM(41);
}

GLOBAL	void				*get_tmvec(void)
{
	return (void *)TM(42);
}

GLOBAL	void CDECL	set_smpoffs(unsigned long smpoffs)
{
	TM(43);
}

#define Tm_masterconv(a)         TM(49,a)

GLOBAL	void CDECL	humpar(HUMANBLK *humstruct)
{
	TM(50);
}


GLOBAL MROS_STRUCT	*create_mros	_((VOID))
{
	MROS_STRUCT *ms;
	
	ms = (MROS_STRUCT *)mem_alloc(sizeof(MROS_STRUCT));
	
	if (ms)
	{
		ms->memstruct = (MEM_STRUCT *)mem_alloc(sizeof(MEM_STRUCT));
		ms->io_struct = (IO_STRUCT *)mem_alloc(sizeof(IO_STRUCT));
		ms->tm_struct = (TM_STRUCT *)mem_alloc(sizeof(TM_STRUCT));
	}
	
	return ms;
}

GLOBAL BOOLEAN 		init_mros		_((VOID))
{
	BOOLEAN ok = FALSE;
	struct ffblk fblock;
	char name[14];
	     
	if (findfirst ("MROS\\MROS*.",&fblock,0) == 0)
	{
		strcpy (name,"MROS\\");
		strcat (name,fblock.ff_name);
		if (Pexec (0,name,NULL,NULL) == 0)
		{
			ok = TRUE;
			mevout=(void*) *(long*)iovec();           /* fast IOM event out */
		} /* if */
		else 
		{
			printf("MROS not found! \n");
		} /* else */
	} /* if */
	return ok;
}


GLOBAL BOOLEAN 		term_mros		_((VOID))
{
	return TRUE;
}
