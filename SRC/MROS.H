/*****************************************************************************/
/*                                                                           */
/* Modul: MROS.H                                                             */
/* Datum: 05/11/92                                                           */
/*                                                                           */
/* M-ROS Deklarationen fÅr RTM                                               */
/*****************************************************************************/


/****** DEFINES **************************************************************/

#define RESOL 4             /* resolution= 4 1/16 frames. Simply changing
                               this will vari the resolution */
#define TIKS 20             /* if smpte based, this should be
                               RESOL*5 (1/16 frame=5 subframes) */ 
#define TIMEMODE 1          /* SMPTE based. Changing this will
                               make the program TEMPO based */

/*----------------------------------------------------------------*/
/*                    Entry points                                */
/*----------------------------------------------------------------*/

extern long IOM(void,...);
extern long TM(void,...);
extern long MEM(void,...);

/*------------------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*                     MEM - Functions                            */
/*----------------------------------------------------------------*/

#define Open_mros(a)        MEM(0,a)
#define Close_mros(a)       MEM(1,a)
#define Snd_message(a,b,c)  MEM(2,a,b,c)
#define Appl_req(a,b)       MEM(3,a,b)
#define Get_vectab()        MEM(4)
#define Terminal_req(a,b)   MEM(5,a,b)
#define Switch(a)           MEM(6,a)

/*----------------------------------------------------------------*/
/*              Message Manager Application Structure             */
/*----------------------------------------------------------------*/

typedef struct mem_struct  {

   void (*ms_message)();              /* message receiver         */
   int ms_topped;                     /* <>0: application active  */
   int ms_id;
   int ms_count;  
   int ms_handle;
   int ms_intern; 
   int ms_active;
   char ms_undef[16];
   char ms_name[16];                  /* write name in here       */  
} MEM_STRUCT;

/*----------------------------------------------------------------*/
/*                     Switch Struct                              */
/*----------------------------------------------------------------*/

typedef struct sw_struct {

int sw_mode;                 /* mode (0=mros,1=comp mode,else none) */
int sw_undef;                /* reserved                            */
struct mem_struct *sw_memp;  /* &mem_struct                         */
long sw_basep;               /* memory start                        */
long sw_mend;                /* memory end                          */
int sw_indev;                /* input device                        */
int sw_input;                /* input                               */
struct mmstruct *sw_inrec;   /* &mm_struct (input)                  */
int sw_outdev;               /* output device                       */              
int sw_output;               /* output                              */
struct mmstruct *sw_outrec;  /* &mm_struct (output)                 */
char sw_resvd[16];           /* reserved                            */

} SW_STRUCT;


/*----------------------------------------------------------------*/
/*              Message manager opcodes                           */
/*----------------------------------------------------------------*/

#define MEM_START               0
#define MEM_STOP                1
#define MEM_CONT                2
#define MEM_POS                 3
#define MEM_PLAY                4
#define MEM_CYCSET              5
#define MEM_CYCSTART            6
#define MEM_CYCONOFF            7
#define MEM_COUNTIN             8
#define MEM_FRAME               24
#define MEM_SYNC                25
#define MEM_TEMPO               26
#define MEM_MASTER              27
#define MEM_OFFSET              28
#define MEM_SWITCH              0x7D
#define MEM_PRIVAT              0x7E

/*----------------------------------------------------------------*/
/*                     MEM - Parameters                           */
/*----------------------------------------------------------------*/

typedef struct mempara {

        void *mem_mrtab[32];        /* mem_struct table (32 long)    */
        int mem_mrcount;            /* mros application counter      */
        int mem_mrhand;             /* mros handle counter           */
        int mem_undef1;
        int mem_apidcnt;            /* appl identifier count         */
        int mem_otid;               /* own terminal id               */
        int mem_noterms;            /* no of connected terminals     */
        int mem_updreq;             /* update request flag           */
        int mem_msgact;             /* message send active           */
        int mem_extflag;            /* ext communication active      */
        int mem_msglock;            /* ext msg forbidden             */
        int mem_versno;             /* version no                    */
        int mem_mecho;              /* ext msg echo                  */
        long mem_tinfbuf;           /* *buffer for terminal info     */
        struct sw_struct *actappl;  /* *sw_struct of active appl.    */
} MEMPARA;

/*------------------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*      IOM - Functions                                           */
/*----------------------------------------------------------------*/

#define Dev_req(a)              IOM(0,a) 
#define Ssnd_msingl(a,b,c)      IOM(1,a,b,c)
#define Snd_mevent(a,b,c,d,e)   IOM(2,a,b,c,d,e)
#define Snd_mmult(a,b,c,d)      IOM(3,a,b,c,d)
#define Out_mreq(a,b)           IOM(4,a,b)
#define Mget(a,b)               IOM(5,a,b)
#define Inp_mreq(a,b)           IOM(6,a,b)
#define Remdat(a,b,c)           IOM(7,a,b,c)
#define Set_mbuf(a,b,c,d,e)     IOM(8,a,b,c,d,e)
#define Rst_mbuf(a,b,c)         IOM(9,a,b,c)
#define Dev_reset(a)            IOM(10,a)
#define Run_stat(a,b,c)         IOM(11,a,b,c)
#define Get_iorec(a,b)          IOM(15,a,b)

#define Open_io(a,b)            IOM(16,a,b)
#define Close_io(a)             IOM(17,a)
#define Open_mout(a,b,c,d,e)    IOM(18,a,b,c,d,e)
#define Open_device(a)          IOM(20,a)
#define Get_minmask(a,b)        IOM(21,a,b)
#define Next_device(a,b)        IOM(22,a,b)
#define Get_device(a)           IOM(23,a)
#define Iotrapvec()             IOM(24)
#define Iovec()                 IOM(25)
#define Get_iopar()             IOM(26)

/*----------------------------------------------------------------*/
/*                     io_struct                                  */
/*----------------------------------------------------------------*/

typedef struct io_struct {

        long inpmask;                /* input mask (32 inputs)    */
        char ioreserved1[12];        /* reserved                  */
        void (*inpvec)();            /* midi in receive routine   */
        int ioppqsmp;                /* time stamp mode (0=ppq)   */
        int io_hand;                 /* handle (set by mros)      */
        int mros_hand;               /* mros handle (set by mros) */
        char ioreserved2[6];         /* reserved                  */
} IO_STRUCT; 

/*----------------------------------------------------------------*/
/*                      IOM - Parameters                          */
/*----------------------------------------------------------------*/

typedef struct iopara {

        int io_nodevs;               /* no of devices                  */
        void *io_sysmrecp;           /* *system midi record            */
        long io_motreq;              /* midi out request bits register */
        long io_resvd1;
        long io_ofreelp;             /* *next free element out task list */
        int io_iotskc;               /* iom task counter               */
        int io_iohc;                 /* handle count                   */
        int io_amotrbit;             /* actual out request bit         */
        long io_amotmask;            /* actual out request bit mask    */
        long io_aminmask;            /* actual in bit mask             */
} IOPARA;

/*----------------------------------------------------------------*/
/*                       Devices etc                              */
/*----------------------------------------------------------------*/


typedef struct vec_tab {

  struct mempara *memglob;           /* MEM variables           */
  struct iopara *ioglob;             /* IOM variables           */
  struct tmpara *tmglob;             /* TM variables            */
  void (*evlmin)();                  /* midi in evaluation      */
  void (*evlmout)();                 /* midi out evaluation     */
  void (*evlmclock)();               /* midi clock evaluation   */
  void (*evlmtc)();                  /* mtc evaluation          */
  void (*mevout)();                  /* midi event out vector   */
  struct dev_struct *devp[32];       /* devices list            */
  void *getclockv;                   /* get smpte clocks vector */ 
} VEC_TAB;                           /* to be continued...      */


typedef struct dev_struct {

  struct mmstruct *devrecp;          /* pointer to first mmstruct        */
  int devid;                         /* device id                        */
  int devmidi;                       /* <>0, if device has midi ports    */
  int devppq;                        /* if device can produce ppq clocks */
  int devsmpte;                      /* if device has smpte capabilities */
  void (*devframev)();               /* frame change vector              */
  void (*devwritev)();               /* write smpte vector               */
  void (*devpsyncv)();               /* ppq sync on/off vector           */
  void (*devssyncv)();               /* smpte sync on/off vector         */
  int devact;                        /* <>0, if device active            */
  int devnum;                        /* device no.                       */
  int devmins;                       /* no. of midi inputs (0=no input)  */
  int devmouts;                      /* midi outputs                     */
  void (*devinitv)();                /* re-initialisation vector         */
  void (*devtermv)();                /* terminate vector                 */
  void (*devcallv)();                /* for direct call to the driver    */
  int devwrcont;                     /* can write smpte in the background*/
  char devresvd[4];                  /* reserved                         */
  int devvers;                       /* version no.                      */
  char devname[8];                   /* 8 ascii device name              */

} DEV_STRUCT;

/*----------------------------------------------------------------*/
/*             Midi Manager (MM) Midi record                      */
/*----------------------------------------------------------------*/

typedef struct mmstruct {

        void *mm_minin;              /* midi in buffer in         */
        void *mm_minstr;             /* midi in buffer start      */
        void *mm_minend;             /* midi in buffer end        */
        void *mm_minout;             /* midi in buffer out        */
        long mm_minmask;             /* in bit mask               */
        int mm_devin;                /* device,input              */
        long mm_msop;                /* actual songpointer        */
        long mm_rtim;                /* actual time               */
        int mm_resvd1;

        void *mm_moutin;             /* midi out buffer in        */
        void *mm_moutstr;            /* midi out buffer start     */
        void *mm_moutend;            /* midi out buffer end       */
        void *mm_moutout;            /* midi out buffer out       */
        long mm_motmask;             /* out bit mask              */
        int mm_sysflg;               /* system exclusive active   */
        int mm_syscnt;               /* sysex data count          */
        int mm_mooccup;              /* output occupied           */
        int mm_devout;               /* device,out                */
        int mm_molock;               /* bulk send active          */
        int mm_resvd2;

        long mm_status;              /* status + max 2 chars      */
        long mm_event;               /* mevout event latch        */
        long mm_resvd3;
        int mm_resvd4;
        int mm_minc;                 /* midi in data counter      */
        int mm_tmic;                 /* temporary                 */
        void *mm_moibstr;            /* own in buffer start       */
        void *mm_moibend;            /* own in buffer end         */
        void *mm_moobstr;            /* own out buffer start      */
        void *mm_moobend;            /* own out buffer end        */

        void *mm_emsgin;             /* ext message buffer in     */
        int mm_rstat;                /* running status on/off     */
        int mm_astat;                /* actual status             */
        int mm_mrsys;                /* mros sysex active         */
        void *mm_evmvec;             /* actual midi in vector     */
        void *mm_oevmvec;            /* save                      */
        void *mm_bmoutin;            /* out buffer in while mmsend */
        char mm_sysbuf[8];           /* sysex buffer 8 byt        */

        void *mm_olistp;             /* *out task list            */
        void *mm_olilast;            /* *last task in list        */
        int mm_motrbit;              /* motreq bit no. for this output */
        int mm_mtcact;               /* this input mtc master     */
        int mm_mclact;               /* this input midi clock master */
        int mm_resvd5;
        void *mm_outtrigv;           /* out trigger vector        */
        char mm_undef[44];           /* not used yet              */
} MMSTRUCT;



/*------------------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*                    TM - functions                              */
/*----------------------------------------------------------------*/

#define Tm_start()               TM(0)
#define Tm_stop()                TM(1)
#define Tm_cont()                TM(2)
#define Tm_pos(a)                TM(3,a)
#define Tm_play(a)               TM(4,a)
#define Cycle_set(a,b)           TM(5,a,b)
#define Cycle_start()            TM(6)
#define Cycle_onof(a)            TM(7,a)
#define Count_in(a,b,c)          TM(8,a,b,c)
#define Tm_forward(a)            TM(9,a)
#define Tm_rewind(a)             TM(10,a)

#define Timstr(a,b)              TM(16,a,b)
#define Curtimstr(a)             TM(17,a)
#define Songstr(a,b)             TM(18,a,b)
#define Cursngstr(a)             TM(19,a)
#define Curtmpstr(a)             TM(20,a)
#define Curtsgstr(a)             TM(21,a)
#define Strtim(a)                TM(22,a)

#define Set_frame(a)             TM(24,a)
#define Tm_sync(a,b,c,d,e,f,g)   TM(25,a,b,c,d,e,f,g)
#define Set_tempo(a)             TM(26,a)
#define Set_master(a)            TM(27,a)
#define Mtc_onoff(a,b,c)         TM(28,a,b,c)
#define Mcl_onoff(a,b,c)         TM(29,a,b,c)
#define Mclick(a,b)              TM(30,a,b)
#define Wrt_smpte(a,b,c)         TM(31,a,b,c)

#define Open_tm(a,b)             TM(32,a,b)
#define Close_tm(a)              TM(33,a)
#define Store_loc(a)             TM(34,a)
#define Get_loc(a)               TM(35,a)

#define Get_tmpar()              TM(40)
#define Get_tmtrap()             TM(41)
#define Get_tmvec()              TM(42)
#define Smpte_offset(a)          TM(43,a)

#define Tm_masterconv(a)         TM(49,a)
#define Human_clock(a)           TM(50,a)

/*----------------------------------------------------------------*/
/*                         tm_struct                              */
/*----------------------------------------------------------------*/

typedef struct tm_struct {
        void (*tmq)();               /* task (called in run mode) */
        void (*startq)();            /* start message task        */
        void (*stopq)();             /* stop message task         */
        void (*posq)();              /* position message task     */
        void (*freezeq)();           /* cycle freeze task         */
        void (*restq)();             /* cycle restore task        */
        void (*stillq)();            /* task (called during stop) */
        long tmreserved1;

        int tiks;                    /* task called every n tiks  */
        int tmppqsmp;                /* either 1/384 ppq or 1/16 frame */
        int prio;                    /* priority                  */
        char tmreserved2[8];
        int tmhand;                  /* handle (set by mros)      */
        int tmmroshand;              /* mros handle (set by mros) */
        char tmreserved3[18];
} TM_STRUCT;

/*----------------------------------------------------------------*/
/*              Time Manager Parameters                           */
/*----------------------------------------------------------------*/

typedef struct tmpara {
        long t_runclock;             /* smpte-clock                   */
        long t_songpos;              /* actual songp                  */
        void *t_nxtmas;              /* next master tempo entry       */
        long t_nxtpos;               /* next tempo change position    */
        long t_tempo;                /* tempo                         */
        long t_humt;                 /* human runclk algned to qurter */
        int t_ttsden;                /* tsm denom temp                */
        int t_tsden;                 /* tsm denom(4=4,8=2,16=1)       */
        int t_tsnum;                 /* tsm numerator                 */
        int t_abar;                  /* actual bar                    */
        int t_numer;                 /* numerator                     */
        int t_denom;                 /* denominator                   */
        int t_mtcc;                  /* mtc count                     */
        char t_rfrm;                 /* mtc frames                    */
        char t_rsec;
        char t_rmin;
        char t_rhor;
        long t_ttemp;                /* exebit tempo counter          */
        long t_humqua;               /* human quarter count (24 ppq)  */
        char t_unused_0[14];
                                     /* first block swapped on cycle  */
        int t_runflag;               /* system running                */
        int t_frame;                 /* frame type 0=24,1=25 etc      */
        int t_beepflg;               /* beep on/off                   */
        char t_bfre1;                /* beeper frequency lo           */
        char t_bfre2;                /* hi                            */
        int t_ahrs;                  /* actual hours                  */
        int t_amin;                  /* minutes                       */
        int t_asec;                  /* seconds                       */
        int t_afrm;                  /* frames                        */
        int t_abit;                  /* subfrms                       */
        int t_mtco;                  /* mtc out on/off                */
        int t_frms;                  /* frames(real)                  */
        long t_cclock;               /* calc clock                    */
        void *t_master;              /* mastertrack adress            */
        int t_ssync;                 /* smpte sync mode               */
        int t_maston;                /* mastertrack on/off            */
        int t_psync;                 /* ppq sync mode                 */
        long t_cycstr;               /* cycle start pos               */
        long t_cycend;               /* cycle end pos                 */
        int t_cycon;                 /* cycle on/off                  */
        long t_cycflg;               /* cycle start req               */
        int t_tmlock;                /* system lock                   */
        int t_jobcnt;                /* jobs counter                  */
        void *t_jobend;              /* jobs list end                 */
        int t_cycchg;                /* cycle start change            */
        long t_timclock;             /* lock clock                    */

        long t_stopclk;              /* stop clock                    */
        long t_tmcount;              /* tm overflow counter           */
        int t_tmhandc;               /* next tm handle                */
        int t_mcthand;               /* midi clock out tm handle      */
        int t_sfcnt;                 /* 1/16 frame counter            */
        int t_smpclock;              /* mtc subframes                 */
        int t_stic;                  /* mtc flag                      */
        int t_unused_1;
        char t_mtcdat[8];            /* mtc (8 bytes)                 */
        long t_stopsop;              /* stop songpos                  */
        int t_tioffs;                /* internal offset               */
        long t_smpoffs;              /* smpte offset (mtc sync)       */
        int t_sycc;                  /* sync watchdog count           */
        long t_tcibpb;               /* count in bpb                  */
        int t_cinum;                 /* numerator                     */
        int t_tcinum;                /* temp                          */
        int t_cibar;                 /* bars                          */
        int t_cippq;                 /* ppq                           */
        int t_tcippq;                /* temp                          */
        int t_ciflag;                /* count in flag                 */
        int t_slzero;                /* leading zero flag songpos     */
        int t_tlzero;                /* time                          */
        char t_tsep;                 /* time string separator         */
        char t_ssep;                 /* song string separator         */
        int t_sscal;                 /* songstr remainder scaling     */
        int t_cabar;                 /* calc paras actual bar         */
        int t_cnumer;                /* numerator                     */
        int t_cdenom;                /* denominator                   */
        int t_cppbar;                /* ppq per bar                   */
        int t_cppden;                /* ppq per denominator           */
        int t_carem;                 /* actual remainder              */
        int t_canum;                 /* actual numerator              */
        long t_ctempo;               /* tempo                         */
        int t_ciden;                 /* count in denominator          */
        int t_mtcon;                 /* mtc out on                    */
        int t_mclon;                 /* midi clock out on/off         */
        struct mmstruct *t_mtcodev;  /* mtc out device record         */
        struct mmstruct *t_mclodev;  /* midi clock out device record  */
        int t_tmflag;                /* tm overflow flag              */
        int t_tmtc;                  /* tm applications count         */
        int t_ptskc;                 /* ppq tasks count(-1)           */
        int t_stskc;                 /* smpte tasks count(-1)         */
        int t_strtc;                 /* start tasks count             */
        int t_stopc;                 /* stop tasks count              */
        int t_posc;                  /* position tasks count          */
        int t_freezec;               /* cycle freeze tasks count      */
        int t_restc;                 /* cycle restore tasks count     */
        int t_stillc;                /* still tasks count             */
        int t_psfcnt;                /* stillq 1/16 frame counter     */
        int t_mclkon;                /* midi click on/off             */
        long t_mclkevt[2];           /* 2 events                      */

        struct mmstruct *t_mclkdev;  /* midi clock in record          */
        int t_pstiqc;                /* stillq count ppq temp         */
        int t_sstiqc;                /* stillq count smpte temp       */
        int t_asmpsyn;               /* smpte sync source int/ext     */
        int t_appqsyn;               /* ppq sync source               */
        int t_misync;                /* midi sync status              */
        int t_ppqclock;              /* ext sync ppq clocks           */
        int t_mclcnt;                /* ext ppq clock subframe cnt    */
        int t_lckoffs;               /* lock time offset in subframes */
        long t_lockt;                /* * frames                                                                             */
        int t_settle;                /* settle time (* ys/subframe)   */
        long alltim;                 /* timer clock                   */
        struct mmstruct *t_tmsinrec; /* mtc in record                 */
        struct mmstruct *t_tmpinrec; /* midi clock in record          */
        void *t_humtab;              /* &human clock quantize table   */
        int t_hbpb;                  /* bits (subframes) per beat     */
        int t_humtmhand;             /* tm handle                     */
        int t_humiohand;             /* io handle                     */
        int t_hevtcnt;               /* event repeat count            */
        long t_tscale;               /* (frames*80*60*1000)/384       */
        long t_bpbscal;              /* frames*80*60*1000             */
        long t_humscal;              /* (frames*80*60*1000)/8         */
        void *t_jobin;               /* jobs buffer in                */
        void *t_jobout;              /* out                           */
        int t_timsem;                /* tasks entry/exit semaphore    */
        long t_locktim;              /* lock time mtc                 */
        long t_tmsp;                 /* tm stackpointer save          */
        long t_mclscal;              /* midi clock tempo scaler       */
        long t_citempo;              /* still queue,count in tempo    */
        int t_tmsindev;              /* mtc device                    */
        int t_tmsinput;              /* input                         */
        int t_tmpindev;              /* midi clock device             */
        int t_tmpinput;              /* input                         */
        int t_tevcnt;                /* tempo evaluation counter      */
        long t_twidth;               /* width (in ppq)                */
        int t_unused_2;
        int t_atwcnt;                /* actual width count            */
        long t_atfunc;               /* actual entry                  */
        long t_otempo;               /* tempo offset                  */
        long t_arem;                 /* csmpte remainder temporary    */
        int t_humcibar;              /* human count in bars           */
        long t_humcit;               /* last trigger time             */
        long t_timdoffs;             /* time display offset           */
        int t_timtim;                /* time divisor                  */
        long t_cyccnt;               /* cycle count                   */
        long t_tcyccnt;              /* temporary                     */
        int t_tmpreq;                /* tempo change request          */
        long t_rtempo;               /* tempo                         */
        long t_rtmppos;              /* position                      */
        int t_emaster;               /* external master               */
        int t_eslave;                /* external slave                */
        int tstress;                 /* overflow clocks latch (displ) */
        long t_cpos;                 /* calculated time after sngstr */
} TMPARA;

/*---------------------------------------------------------------------*/
/*                   Human Sync struct                                 */
/*---------------------------------------------------------------------*/

typedef struct humanblk {
        int hm_algo[24];
        long hm_offset;
        int hm_lag;
        long hm_maxdiff;
        long hm_mindiff;
        long hm_maxtempo;
        long hm_mintempo;
        char hm_unused[10];
} HUMANBLK;


typedef struct mrosstruct {
	MEM_STRUCT	*memstruct;
	IO_STRUCT	*io_struct;
	TM_STRUCT	*tm_struct;
	TMPARA		*tmpar;
	int mroshandle;          /* MROS handle */
	int iohandle;            /* IOM handle */
	int tmhandle;            /* TM handle */
} MROS_STRUCT;

/****** FUNCTIONS ************************************************************/

GLOBAL MROS_STRUCT	*create_mros	_((VOID));
GLOBAL BOOLEAN 		init_mros		_((VOID));
GLOBAL BOOLEAN 		term_mros		_((VOID));
