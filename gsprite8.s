| Assembly Source File
| Created 2020/6/16; 2:39:55

| Glacc 8x8 sprite function (Non-grayscale)

.xdef	gsprite8

gsprite8:
	movem.w	%D1-%D5,-(%SP)
	move.l	%A0,-(%SP)
	move.l	%A1,-(%SP)
	
	move.b	%D0,%D5
	andi.b	#7,%D5
	move.b	#8,%D4
	sub.b	%D5,%D4
	andi.w	#0xFF,%D4
	
	cmpi.w	#8,%D1
	bge	skip1
	
	move.w	#8,%D5
	sub.w	%D1,%D5
	adda.l	%D5,%A0
	move.w	%D1,%D5
	clr.w	%D1
	bra	skip2
skip1:
	move.w	%D1,%D5
	subi.b	#8,%D1
skip2:
	move.w	%D1,%D3
	andi.w	#0xFF,%D3
	mulu	#30,%D3
	move.w	%D0,%D2
	cmpi.w	#8,%D2
	sub.w	#8,%D2
	bge	skip2_2
	clr.w	%D2
skip2_2:
	lsr.w	#3,%D2
	add.w	%D2,%D3
	adda.l	%D3,%A1
	cmpi.b	#108,%D1
	ble	skip3
	move.b	#108,%D1
skip3:
	andi.w	#0x00FF,%D0
	cmpi.w	#8,%D0
	blt	loopL
	cmpi.w	#160,%D0
	blt	loopM
	
	move.b	#8,%D3
	sub.b	%D4,%D3
	move.b	%D3,%D4
loopR:
	move.b	(%A0)+,%D2
	lsr.b	%D4,%D2
	move.b	(%A1),%D3
	or.b	%D2,%D3
	move.b	%D3,(%A1)
	adda.l	#30,%A1
	addi.b	#1,%D1
	cmp.w	%D5,%D1
	bne	loopR
	bra	endf
loopL:
	move.b	(%A0)+,%D2
	lsl.b	%D4,%D2
	move.b	(%A1),%D3
	or.b	%D2,%D3
	move.b	%D3,(%A1)
	adda.l	#30,%A1
	addi.b	#1,%D1
	cmp.w	%D5,%D1
	bne	loopL
	bra	endf
loopM:
	clr.w	%D2
	move.b	(%A0)+,%D2
	lsl.w	%D4,%D2
	
	move.b	(%A1)+,%D3
	lsl.w	#8,%D3
	move.b	(%A1)+,%D3
	or.w	%D2,%D3
	
	move.b	%D3,-(%A1)
	lsr.w	#8,%D3
	move.b	%D3,-(%A1)
	adda.l	#30,%A1
	
	addi.b	#1,%D1
	cmp.w	%D5,%D1
	bne	loopM
	
endf:
	move.l	(%SP)+,%A1
	move.l	(%SP)+,%A0
	movem.w	(%SP)+,%D1-%D5
	rts
