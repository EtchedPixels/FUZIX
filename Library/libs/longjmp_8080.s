;
;	Not a lot to do because we don't have to save much. We save BC ready
;	for future use
;

		.export _longjmp
		.setcpu 8080
		.code

_longjmp:
	lxi	h,4
	dad	sp
	mov	c,m		; return code
	inx	h
	mov	b,m		; and high half
	mov	a,b
	ora	c
	jnz	retok
	inr	c		; force return code non zero
retok:
	dcx	h
	dcx	h		; now points to end of buffer arg
	mov	d,m
	dcx	h		; and start
	mov	e,m		; DE is now buffer address
	xchg			; into HL
	mov	e,m
	inx	h
	mov	d,m
	inx	h		; points to return addr copy
	xchg
	inx	h		; discard old return address
	inx	h
	sphl			; restore SP
	xchg			; HL is now pointing to return addr save
	mov	e,m
	inx	h
	mov	d,m
	inx	h		; points to saved bc
	push	d		; return address
	push	b		; return code
	mov	c,m
	inx	h
	mov	b,m
	pop	h		; retunr code
	ret
