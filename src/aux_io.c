/*
 * Name:		aux_io.c
 * Projekt:		Power Glove Anbindung an RS-232 (c't-POGLI)
 * Verzeichnis:	//overscan1/pj/p:/pogli/bgi_hand/aux_io.c
 * Autor:		Patrick Jerchel / OverScan GbR Berlin
 * eMail:		Subnetz:  pj@zelator.in-berlin.de
 *				Mausnetz: pj@b
 * Mailbox:		Siehe HAND.TXT.
 * Copyright:	(c)1992 c't , Patrick Jerchel / OverScan Berlin
 * Bermerkung:	In diesem Modul befinden sich alle Funktionen, die
 *				die Kommunikation Åber die serielle Schnittstelle
 *				des Wirtsrechners erlauben. Dies ist stark von
 *				vorhandenem Rechner und Betriebssystem abhÑngig.
 *				Deshalb muû vor dem öbersetzen eine der Konstanten
 *				(z.Zt.) DOS, TOS oder UNIX gesetzt werden. Nur die 
 *				zugehîrigen Teile des Quellcode werden dann 
 *				Åberstetzt.
 * Literatur:	[1] Manfred Krauss, Virtuality now!, 
 *				    Rechner-Interface fÅr den Nintendo PowerGlove, 
 *					Teil 1, c't 9/92, S. 158
 *				[2] Patrick Jerchel, Virtuality now!, 
 *					Rechner-Interface fÅr den Nintendo PowerGlove, 
 *					Teil 2, c't 10/92, S. 250,
 *					Teil 3, c't 11/92, S. 274,
 *					Teil 4, c't 12/92
 *				[3] W.Hartung, M.Felsmann, A.Stiller, PC-Bausteine,
 *					Der UART 8250 als Tor zur seriellen Welt, 
 *					c't 5/88, S.204ff
 *				[4] Handbuch zu Pure-C (TOS)
 *				[5] Handbuch zu Borland-C++ (DOS)
 *				[6] Handbuch zu Turbo-C (DOS)
 * Tabsize:		4
 */
#include "aux_io.h"

/* ==================================================================
 * === System-abhÑngiger Code fÅr TOS ===============================
 */
#define TOS  /* BD */

#ifdef TOS

#include <ext.h>
#include <tos.h>

#define RSBAUD 		19200L,9600L,4800L,3600L,2400L,2000L,1800L,\
					1200L,600L,300L,200L,150L,134L,110L,75L,50L
#define RSCTR 		0		/* Kein Handshake */
#define RSUCRMASK 	0x81	/* Maske */
#define RSUCRDATA	0x08	/* 8,N,1 */

/*
 * Modulglobale Variablen.
 */
static int actualaux;		/* Aktuelle Schnittstelle	*/


/* ------------------------------------------------------------------
 * Name:		AuxInit()
 * Parameter:	aux:  Nr. der Schnittstelle,
 *				baud: Einzustellende Baud-Rate,
 *				res:  Reserviert fÅr Handshake.
 * Return:		1 wenn Initialisierung ohne Fehler klappt, 0 wenn
 *				nicht.
 * Bemerkung:	Initialisiert die gewÅnschte serielle Schnittstelle
 *				mit der gewÅnschten Baudrate.
 */
int AuxInit(int aux, long baud, int res)
{
	BCONMAP *bcmap;
	long    rsbaud[] = {RSBAUD, 0L};
	int     baudindex = -1;
	int     i = 0;
	union   {long l; char c[4];} rsreturn;
	
	/*
	 * Feststellen, ob die gewÅnschte Schnittstellen existiert und 
	 * Ablegen der Information in actualaux (modulglobal).
	 */
	if ((long)(bcmap = (BCONMAP*)Bconmap(-2)) == 44L)
		actualaux = 1;
	else
	{
		if (aux <= bcmap->maptabsize)
		{
			actualaux = aux;
			Bconmap(actualaux + 5);
		}
		else
			actualaux = 1;
	}
	if (actualaux != aux)
		return 0;

	/*
	 * Ist die Baudrate verfÅgbar?
	 */
	do {
		if (rsbaud[i] == baud)
			baudindex = i;
	} while ((baudindex < 0) && rsbaud[i++]);

	if (baudindex < 0)
		return 0;

	/*
	 * Einstellen der Parameter (kein Handshake, 8,N,1).
	 * Der dritte Parameter bei Rsconf() ist etwas kritisch.
	 * Nur die Bits 1..6 dÅrfen verÑndert werden.
	 */
	rsreturn.l = Rsconf(-1, -1, -1, -1, -1, -1);
	rsreturn.c[0] &= RSUCRMASK;
	rsreturn.c[0] |= RSUCRDATA;
	Rsconf(baudindex, RSCTR, (int)rsreturn.c[0], -1, -1, -1);

	i = res;	/* Dummy gegen Compiler-Warnung */

	return 1;
}

/* ------------------------------------------------------------------
 * Name:		AuxExit()
 * Parameter:	-
 * Return:		-
 * Bemerkung:	Unter TOS passiert hier nichts.
 */
void AuxExit(void)
{
	return;
}

/* ------------------------------------------------------------------
 * Name:		AuxIn()
 * Parameter:	*in: Zeiger auf ein Byte (char).
 * Return:		Immer 1.
 * Bemerkung:	Liest ein Byte aus dem seriellen Schnittstellenpuffer.
 *				Timeout muû noch realisiert werden...
 */
int AuxIn(char *in)
{
	int retry = AUXRETRY;

	/*
	 * Warten, bis Zeichen da sind. Dann Einlesen.
	 */
	while(Bconstat(actualaux) == 0L)
	{
		delay(1);	/* Eine Millisekunde warten */
		if (!retry--)
			return 0;
	}

	*in = (char)(Bconin(actualaux) & 0xffL);

	return 1;
}

/* ------------------------------------------------------------------
 * Name:		AuxOut()
 * Parameter:	out: Das auszugebende Zeichen.
 * Return:		Immer 1.
 * Bemerkung:	Schreibt ein Byte zur seriellen Schnittstelle.
 */
int AuxOut(char out)
{
	int retry = AUXRETRY;
	
	/*
	 * Warten, solange Schnittstelle belegt. Dann Ausgabe.
	 */
	while(Bcostat(actualaux) == 0L)
	{
		delay(1);	/* Eine Millisekunde warten */
		if (!retry--)
			return 0;
	}

	Bconout(actualaux, (int)out);

	return 1;
}

/* ------------------------------------------------------------------
 * Name:		AuxFree()
 * Parameter:	-
 * Return:		Immer 1.
 * Bemerkung:	Liest alle vorliegenden Zeichen aus dem seriellen
 *				Schnittstellenpuffer.
 */
int AuxFree(void)
{
	while(Bconstat(actualaux) == -1)
		Bconin(actualaux);

	return 1;
}

/* ------------------------------------------------------------------
 * Name:		AuxIs()
 * Parameter:	-
 * Return:		1, wenn mindestens ein Zeichen im Eingabepuffer
 *				vorliegt, sonst 0.
 */
int AuxIs(void)
{
	return (Bconstat(actualaux) == -1);
}

#endif /*TOS*/


/* ==================================================================
 * === System-abhÑngiger Code fÅr DOS ===============================
 */
#ifdef DOS

#include <stdlib.h>
#include <dos.h>
#include <bios.h>

/* ------------------------------------------------------------------
 * Serielle Service-Routinen nach [3] (s.o.). Namen von Funktionen
 * und Variablen wurden weitgehend beibehalten.
 * --- Nur fÅr dieses Modul gÅltig ---
 */

#define RSBAUD 		19200L,9600L,4800L,2400L,1200L,600L,300L,150L,110L
#define RSDATAB		3			/* 8 Daten-Bits */
#define RSPARITY	0			/* No Parity */
#define RSSTOPB		0			/* 1 Stop-Bit */
#define RSNCOM 		2			/* Max. Anzahl der Schnittstellen */
#define RSCOMU1		0x3f8		/* Basisadresse COM1-Portchip */
#define RSCOMU2		0x2f8		/* Basisadresse COM2-Portchip */
#define RSINTU1		0x0c
#define RSINTU2		0x0b
#define RSBUFSIZ	10000L		/* Puffergîûe in Bytes */
#define RSINTCON	0x20		/* Comm.-Register Inter.-Controller */
#define RSIRQMSK	0x21		/* Mask-Register Inter.-Cotroller */
#define RSEOI		0x20		/* MUSS IRQ-Handler abschlieûen! */
#define RSSTATUS	0x20		/* Maske fÅr Ausgabestatus */

/*
 * Modulglobale Variablen.
 */
static int  comnr;				/* Aktuelle Schnittstelle */
static int	com;				/* Port-ID */
static char ComInt;				/* Interrupt-Nr. */
static char *InBuff;			/* Zeiger auf Eingabepuffer */
static int	EinZeiger = 0;		/* High-/Low- water mark */
static int	AusZeiger = 0;

/* Zeiger auf ursprÅngliche Interrupt-Routine.
 */
static void interrupt (*altcom)();	


/* ..................................................................
 * Name:		ReadCom()
 * Parameter:	-
 * Return:		-
 * Bemerkung:	Interrupt-Rountine zum Lesen von der Seriellen.
 */
static void interrupt ReadCom(void)
{
	if (++EinZeiger >= RSBUFSIZ)			/* öberlauf? */
		EinZeiger = 0;
	
	InBuff[EinZeiger] = (char)inportb(com);	/* Zeichen in Puffer */
	
	outportb(RSINTCON, RSEOI);				/* Meldung: fertig */
}

/* ..................................................................
 * Name:		InByte()
 * Parameter:	-
 * Return:		Gelesenes Zeichen.
 * Bemerkung:	Ein Zeichen aus dem Puffer holen. Im Unterschied zu 
 *				der gleichnamigen Routine in [3] wartet diese 
 *				Implementierung, bis wirklich ein neues Zeichen im 
 *				Puffer vorhanden ist (vorher kann das mit InStatus() 
 *				ÅberprÅft werden).
 */
static char InByte(void)
{
	while (AusZeiger == EinZeiger)			/* Nichts neues? */
		/*nothing*/;

	if (++AusZeiger > RSBUFSIZ)				/* öberlauf? */
		AusZeiger = 0;

	return InBuff[AusZeiger];				/* RÅckgabe */
}

/* ..................................................................
 * Name:		InStatus()
 * Parameter:	-
 * Return:		1 wenn mindestens ein neues Zeichen vorliegt, 
 * 				0 wenn nicht. 
 * Bemerkung:	Feststellen, ob mindestens ein neues Zeichen im Puffer
 *				vorliegt. Diese Funktion gibt es in [3] nicht. Sie 
 *				sollte vor Benutzen von InByte() aufgerufen werden.
 */
static int InStatus(void)
{
	return (AusZeiger != EinZeiger);
}

/* ..................................................................
 * Name:		OutByte()
 * Parameter:	out: Auszugebendes Zeichen.
 * Return:		-
 * Bemerkung:	Ein Zeichen auf Schnittstelle ausgeben.
 */
static void OutByte(char out)
{
	outportb(com, out);
}

/* ..................................................................
 * Name:		OutStatus()
 * Parameter:	-
 * Return:		1 klar zum senden, 
 * 				0 wenn nicht.
 * Bemerkung:	Status des Ausgabeports erfragen.
 */
static int OutStatus(void)
{
	return ((inportb(com+5) & RSSTATUS) != 0);
}

/* ..................................................................
 * Name:		SioInt_aus()
 * Parameter:	-
 * Return:		-
 * Bemerkung:	Interrupt ausschalten.
 */
static void SioInt_aus(void)
{
	outportb(RSIRQMSK, (inportb(RSIRQMSK) | 0x18));
}

/* ..................................................................
 * Name:		SioInt_ein()
 * Parameter:	-
 * Return:		-
 * Bemerkung:	Interrupt einschalten.
 */
static void SioInt_ein(void)
{
	outportb(RSIRQMSK, (inportb(RSIRQMSK) & 0xe7));
}

/* ..................................................................
 * Name:		initcom()
 * Parameter:	BasisPort: Port-ID,
 *				baud:      Baud-Rate.
 * Return:		-
 * Bemerkung:	Schnittstelle initialisieren.
 */
static void initcom(int BasisPort, long Baud)
{
	Baud = 115200L / Baud;
	outportb(BasisPort+3, 0x80);		/* divisor latch enable */
	outport(BasisPort, (int)(Baud));	/* Baudrate setzen */
	outportb(BasisPort+3, RSDATAB|RSPARITY|RSSTOPB);
										/* 8,N,1 */
	outportb(BasisPort+1, 1);			/* nur Empfangsinterrupt */
	outportb(BasisPort+4, 0x0b);		/* 000:Keine Funktion
										 *    0:keine Test-Loop
										 *     1:OUT2 on: f.IRQ-Enable
										 *      0:OUT1 off, egal
										 *       1:RTS on
										 *        1:DTR on
										 */
	inportb(BasisPort);					/* Nach [3] (s.o.) */
}	

/* ------------------------------------------------------------------
 * Name:		AuxInit()
 * Parameter:	aux:  Nr. der Schnittstelle,
 *				baud: Einzustellende Baud-Rate,
 *				res:  Reserviert fÅr Handshake.
 * Return:		1 wenn Initialisierung ohne Fehler klappt, 0 wenn
 *				nicht.
 * Bemerkung:	Initialisiert die gewÅnschte serielle Schnittstelle
 *				mit der gewÅnschten Baudrate und Installation der 
 *				Interrupt-Service-Routinen fÅr einen gepufferten
 *				Empfang.
 */
int AuxInit(int aux, long baud, int res)
{
	long    rsbaud[] = {RSBAUD, 0L};	/* VerfÅgbare Baudraten */
	int     baudindex = -1;
	int     i = 0;
	
	static int installed = 0;

	/*
	 * Ist die Interrupt-Service-Routine schon installiert?
	 * Wenn ja, mittels AuxInit() deinstallieren und weitermachen,
	 * als wenn es das erste Mal wÑre...
	 */
	if (installed)
	{
		AuxExit();
		installed = 0;
	}

	/*
	 * Schnittstellen-Interrupt abschalten, Schnittstelle festlegen
	 * und Port-ID modulglobal machen.
	 */
	SioInt_aus();

	if (aux > RSNCOM)
		return 0;
	comnr = aux;

	switch(comnr)
	{
		case 1:	com    = RSCOMU1;
				ComInt = RSINTU1;
				break;
		case 2: com    = RSCOMU2;
				ComInt = RSINTU2;
				break;
		default:
				com    = -1;
				break;
	}
	if (com == -1)
		return 0;

	/*
	 * Speicher anfordern fÅr den Eingabepuffer InBuffer
	 * (modulglobale Variable).
	 */
	if ((InBuff = (char*)calloc((size_t)RSBUFSIZ, 1)) == NULL)
		return 0;

	/*
	 * Alten Interrupt-Vektor retten.
	 */
	altcom = getvect(ComInt);

	/*
	 * Ist die Baudrate verfÅgbar?
	 */
	do {
		if (rsbaud[i] == baud)
			baudindex = i;
	} while ((baudindex < 0) && rsbaud[i++]);

	if (baudindex < 0)
		return 0;

	/*
	 * Einstellen der Parameter (kein Handshake, 8,N,1) und
	 * Installieren der Interrupt-Routine.
	 */
	initcom(com, baud);
	setvect(ComInt, ReadCom);
	installed = 1;

	/*
	 * Wassermarken Null setzen und Interrupt zulassen.
	 */
	EinZeiger = AusZeiger = 0;
	SioInt_ein();

	i = res;	/* Dummy gegen Compiler-Warnung */

	return 1;
}

/* ------------------------------------------------------------------
 * Name:		AuxExit()
 * Parameter:	-
 * Return:		-
 * Bemerkung:	Nach Myexit() aus [3] (s.o.).
 */
void AuxExit(void)
{
	SioInt_aus();
	inportb(com);	/* Nach [3] (s.o.) */
	/*
	 * Alten Vektor restaurieren.
	 */
	setvect(ComInt, altcom);
}

/* ------------------------------------------------------------------
 * Name:		AuxIn()
 * Parameter:	*in: Zeiger auf ein Byte (char).
 * Return:		1, wenn Zeichen gelesen wurde, 0 bei Timeout.
 * Bemerkung:	Liest ein Byte aus dem seriellen Schnittstellenpuffer.
 */
int AuxIn(char *in)
{
	int retry = AUXRETRY;
	/*
	 * Warten, bis Zeichen da sind. Dann Einlesen.
	 */
	while (InStatus() == 0)
	{
		delay(1);	/* Eine Millisekunde warten */
		if (!retry--)
			return 0;
	}

	*in = InByte();

	return 1;
}

/* ------------------------------------------------------------------
 * Name:		AuxOut()
 * Parameter:	out: Das auszugebende Zeichen.
 * Return:		1 wenn Zeichen geschrieben wurde, 0 bei Timeout.
 * Bemerkung:	Schreibt ein Byte zur seriellen Schnittstelle.
 */
int AuxOut(char out)
{
	int retry = AUXRETRY;
	/*
	 * Warten, solange Schnittstelle belegt. Dann Ausgabe.
	 */
	while (OutStatus() == 0)
	{
		delay(1);	/* Eine Millisekunde warten */
		if(!retry--)
			return 0;
	}

	OutByte(out);

	return 1;
}

/* ------------------------------------------------------------------
 * Name:		AuxFree()
 * Parameter:	-
 * Return:		Immer 1.
 * Bemerkung:	Liest alle vorliegenden Zeichen aus dem seriellen
 *				Schnittstellenpuffer.
 */
int AuxFree(void)
{
	while(InStatus())
		InByte();

	return 1;
}

/* ------------------------------------------------------------------
 * Name:		AuxIs()
 * Parameter:	-
 * Return:		1, wenn mindestens ein Zeichen im Eingabepuffer
 *				vorliegt, sonst 0.
 */
int AuxIs(void)
{
	return InStatus();
}

#endif /*DOS*/
