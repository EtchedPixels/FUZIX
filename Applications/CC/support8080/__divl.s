; 32 bits integer divide and remainder routine
; Bit 0 of a-reg is set iff quotient has to be delivered
; Bit 7 of a-reg is set iff the operands are signed, so:
; Expects in a-reg:	0 if called by rmu 4
;			1 if called by dvu 4
;			128 if called by rmi 4
;			129 if called by dvi 4
;
; Based on the ACK 8080 routine but with a bit less stack shuffling
;

			.export	__divl
			.export	__divul
			.export __reml
			.export __remul
			.export	__diveql
			.export	__divequl
			.export __remeql
			.export __remequl

			.setcpu 8080
			.code

; Calculate TOS / hireg:hl

__divl:
	mvi	a,129
	jmp	__divlop
__reml:
	mvi	a,128
	jmp	__divlop
__divul:
	mvi	a,1
	jmp	__divlop
__remul:
	xra	a		; remainder 32bit
__divlop:
	shld	__tmp		; 32bit divisor into hireg:tmp
	pop	d

	pop	h		; 32bit dividend into tmp2
	shld	__tmp2
	pop	h
	shld	__tmp2+2

	push	d		; Save return address

	;
	;	Entry point for assignment forms
	;
divldo:
	push	b		; Save BC

	lxi	h,0			; store initial value of remainder
	shld	__tmp3
	shld	__tmp3+2

	mvi	b,0

	push	psw
	ral
	jnc	nosignmod		; jump if unsigned

	;
	;	Do the preparatory juggling for signed maths
	;

	lda	__tmp2+3
	ral
	jnc	nocomp
	mvi	b,129
	lxi	h,__tmp2
	call	compl		; dividend is positive now

nocomp:
	lda	__tmp+3
	ral
	jnc	nosignmod
	inr	b
	lxi	h,__tmp
	call	compl		; divisor is positive now

	;
	;	Unsigned 32bit divide. Signed maths fixups happen after this
	;

nosignmod:
	push	b		; save the status of the signs


	;
	;	Standard divide algorithm 32 cycles
	;
	mvi	b,32

	;
	;	Shift
	;
divloop:
	lxi h,	__tmp2		; 64bit left shift through tmp2 tmp3
	mvi	c,8
	xra	a
shiftl:
	mov	a,m
	ral
	mov	m,a
	inx	h
	dcr	c
	jnz	shiftl

	;
	;	32bit compare
	;
	lxi	h,__tmp3+3
	lxi	d,__tmp+3
	mvi	c,4

nextcmp:			; 1b
	ldax	d
	cmp	m
	jz	same		; 0f
	jnc 	smaller		; 3f
	jmp	larger		; 4f
same:	dcx	d		; 0f
	dcx	h
	dcr	c
	jnz	nextcmp

	; Equal - fall through

larger:		; 4f
	lxi	d,__tmp3		; remainder is larger or equal: subtract divisor
	lxi	h,__tmp
	mvi	c,4
	xra	a
nextsub:
	ldax	d		; 1b
	sbb	m
	stax	d
	inx	d
	inx	h
	dcr	c
	jnz	nextsub		; 1b
	lxi	h,__tmp2
	inr	m

smaller:			; 3f
	dcr	b
	jnz	divloop		; keep looping

	;
	;	The main work is done.
	;

	pop	b		; state of signs
	pop	psw		; A holds the operation flags
	rar
	jnc	remainder
	;
	;	Division
	;
	mov	a,b
	rar
	lxi	h,__tmp2	; complement quotient if divisor
	cc	compl		; and dividend have different signs
	lhld	__tmp2+2	; quotient high
	shld	__hireg		; into hireg
	lhld	__tmp2
	jmp	cleanup
	;
	;	Remainder
	;
remainder:
	mov	a,b
	ral
	lxi 	h,__tmp3
	cc	compl		; negate remainder if dividend was negative
	lhld	__tmp3+2
	shld	__hireg
	lhld	__tmp3
cleanup:
	pop	b
	ret

; make 2's complement of 4 bytes pointed to by hl.
compl:	push	b
	mvi	c,4
	xra	a
complp:
	mvi	a,0		; preserve carry
	sbb	m
	mov	m,a
	inx	h
	dcr	c
	jnz	complp
	pop	b
	ret
;
;	Helpers for assign forms
;
;	(TOS) = (TOS) / hireg:hl
;
__diveql:
	mvi	a,129		; select operation
dodiveq:
	shld	__tmp		; so we can use hireg:tmp
	pop	h		; return address
	xthl			; swap for lval
	push	h		; save address
	lxi	d,__tmp2
	push	psw
	call	__copy4		; copy value into tmp2/tmp3
	pop	psw
	call	divldo		; result in hireg:tmp
	pop	d
	push	h
	lxi	h,__tmp2
	call	__copy4		; copy it back
	pop	h
	ret
__divequl:
	mvi	a,1
	jmp	dodiveq
__remeql:
	mvi	a,128
	jmp	dodiveq
__remequl:
	xra	a
	jmp	dodiveq
