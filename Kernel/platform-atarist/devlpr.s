
;;; gcc for m6809 : Oct  1 2014 11:32:32
;;; 4.6.4 (gcc6809lw)
;;; ABI version 1
;;; -mint16
	.module	devlpr.c
	.area .text2
	.globl _lpr_open
_lpr_open:
	pshs	u
	leau	,s
	ldx	#0
	puls	u,pc
	.globl _lpr_close
_lpr_close:
	pshs	u
	leau	,s
	ldx	#0
	puls	u,pc
	.globl _lpr_write
_lpr_write:
	pshs	y,u
	leas	-6,s
	leau	,s
	ldy	_udata+96
	ldx	_udata+94
	stx	2,u
	cmpy	#0	;cmphi:
	ble	L8
	leay	-1,y
	sty	4,u
L14:
	ldb	[_lpstat]
	clra		;zero_extendqihi: R:b -> R:d
	std	,u
	anda #0
	andb #2
	cmpd	#0	;cmphi:
	beq	L16
	ldy	_udata
	ldd	#3
	std	36,y
	ldb	13,u
	ldx	#0
	jsr	__far_call_handler	;old style
		.dw	_psleep_flags
		.db	
	cmpx	#0	;cmphi:
	beq	L14
	ldy	_udata+96
	beq	L8
	ldd	#0
	std	_udata+12
L8:
	leax	,y
	leas	6,s
	puls	y,u,pc
L16:
	ldy	_lpdata
	ldx	2,u
	leax	1,x
	stx	,u
	ldx	2,u
	jsr	__far_call_handler	;old style
		.dw	_ugetc
		.db	
	tfr	x,d
	stb	,y	;movlsbqihi: R:d -> ,y
	ldx	4,u
	leax	-1,x
	stx	4,u
	cmpx	#-1	;cmphi:
	beq	L17
	ldx	,u
	stx	2,u
	jmp	L14
L17:
	ldy	_udata+96
	bra	L8
	.globl _lpdata
	.area .data
_lpdata:
	.word	-255
	.globl _lpstat
_lpstat:
	.word	-256
