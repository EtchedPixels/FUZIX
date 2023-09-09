;
;		highreg:hl = TOS - hireg:hl
;
		.export __minusl
		.setcpu 8080
		.code

__minusl:
	xchg		; low half of value into DE
	lxi	h,2
	dad	sp	; pointer into stack

	mov	a,m
	sub	e
	mov	e,a
	inx	h
	mov	a,m
	sbb	d
	mov	d,a
	inx	h

	push	d	; save low word

	xchg
	lhld	__hireg	; high word
	xchg

	mov	a,m
	sbb	e
	mov	e,a
	inx	h
	mov	a,m
	sbb	d
	mov	d,a

	xchg
	shld	__hireg

	pop	h	; our result
	pop	d	; our return address
	pop	psw	; throw the argument
	pop	psw
	push	d
	ret

