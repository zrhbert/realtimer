/*****************************************************************************/
/*                                                                           */
/* Modul: MROS.S                                                             */
/* Datum: 05/11/92                                                           */
/*                                                                           */
/* M-ROS Assembler-Funktionen fÅr RTM                                        */
/*****************************************************************************/

; if you write more assembler, you should INCLUDE 'mrosdef.s'

IMPORT mevout
EXPORT MEM,IOM,TM,eventout

MEM:
        move.l savmemp,a0
        move.l (sp)+,(a0)+           ; save return adress
        move.l a0,savmemp
        trap #8
        move.l savmemp,a0
        move.l -(a0),-(sp)
        move.l a0,savmemp
        rts

savmem: dcb.l 32,0
savmemp: dc.l savmem

IOM:
        move.l saviomp,a0
        move.l (sp)+,(a0)+           ; save return adress
        move.l a0,saviomp
        trap #10
        move.l saviomp,a0
        move.l -(a0),-(sp)
        move.l a0,saviomp
        rts

saviom: dcb.l 32,0
saviomp: dc.l saviom

TM:
        move.l savtmp,a0
        move.l (sp)+,(a0)+           ; save return adress
        move.l a0,savtmp
        trap #9
        move.l savtmp,a0
        move.l -(a0),-(sp)
        move.l a0,savtmp
        rts

savtm: dcb.l 32,0
savtmp: dc.l savtm

eventout:
	    movem.l a2/a5,-(sp)
	    move.l mevout,a0             ; fast iom midi event out
	    jsr (a0)
	    movem.l (sp)+,a2/a5
	    rts

