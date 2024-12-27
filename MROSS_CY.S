export	mem,iom,tm

;************** MROS TRAPHANDLER *************************************

mem:						lea			mem_stke(pc),a0
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

iom:						lea			iom_stke(pc),a0
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


tm:							lea			tm_stke(pc),a0
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
