export	MEM,IOM,TM,AdSpeed16,AdSpeed8
export activity,actadr,scradr,vuwert,screenbreite



;************** MROS TRAPHANDLER *************************************

MEM:						lea			mem_stke(pc),a0
								move.l	(sp)+,-(a0)		;2. R…ksprungadresse sichern
								move.w	(sp)+,mem_opcode	;Funktionsnummer speichern
								move.l	(sp)+,-(a0)		;1. R…ksprungadresse
								move.w	mem_opcode(pc),-(sp) ;und wieder auf den Stack
								trap #8
								lea			mem_stk(pc),a0
								addq.l	#2,sp
								move.l	(a0)+,-(sp)		;1. R…ksprungadresse holen
								subq.l	#2,sp					;da war der Opcode
								move.l	(a0)+,-(sp)		;zweite Adresse
								rts
mem_stk:				DS.L		2
mem_stke:
mem_opcode:			DC.W		0

IOM:						lea			iom_stke(pc),a0
								move.l	(sp)+,-(a0)		;2. R…ksprungadresse sichern
								move.w	(sp)+,iom_opcode
								move.l	(sp)+,-(a0)		;1. R…ksprungadresse
								move.w	iom_opcode(pc),-(sp)
								trap #10
								lea			iom_stk(pc),a0
								addq.l	#2,sp
								move.l	(a0)+,-(sp)		;1. R…ksprungadresse holen
								subq.l	#2,sp					;da war der Opcode
								move.l	(a0)+,-(sp)		;zweite Adresse
								rts
iom_stk:				DS.L		2
iom_stke:
iom_opcode:			DC.W		0


TM:							lea			tm_stke(pc),a0
								move.l	(sp)+,-(a0)		;2. R…ksprungadresse sichern
								move.w	(sp)+,tm_opcode
								move.l	(sp)+,-(a0)		;1. R…ksprungadresse
								move.w	tm_opcode(pc),-(sp)
								trap #9
								lea			tm_stk(pc),a0
								addq.l	#2,sp
								move.l	(a0)+,-(sp)		;1. R…ksprungadresse holen
								subq.l	#2,sp					;da war der Opcode
								move.l	(a0)+,-(sp)		;zweite Adresse
								rts
tm_stk:					DS.L		2
tm_stke:
tm_opcode:			DC.W		0


* set AdSpeed ST to 16 MHz
AdSpeed16:
        move.w  #0,$f00000
        rts



* set AdSpeed ST to 8 MHz
AdSpeed8:
        move.w  #0,$f10000
        rts


/* Anzeige f〉 steuern() */
activity:			move.w	vuwert(pc),d0
							move.l	actadr(pc),a0
					 		eori.b	#0xff,(a0)
					 		addq.w	#1,d0
					 		cmpi.b	#16,d0
					 		beq			actv_reset
					 		move.w	d0,vuwert
					 		move.w	screenbreite(pc),d1
					 		ext.l		d1
					 		add.l		d1,a0
					 		move.l	a0,actadr
					 		rts
actv_reset:		move.l	scradr(pc),actadr
							move.w	#0,vuwert
							rts

actadr:				dc.l		0
scradr:				dc.l		0
vuwert:				dc.w		0
screenbreite:	dc.w		0


