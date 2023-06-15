;
;	Not a lot to do because we don't have to save much. We save BC ready
;	for future use
;

		.export __setjmp
		.setcpu 8080
		.code

__setjmp:
	lxi	h,2
	dad	sp
	mov	e,m
	inx	h
	mov	d,m		; Buffer into DE
	lxi	h,0
	dad	sp		; get SP
	xchg
	mov	e,m
	inx	h
	mov	d,m
	inx	h		; now points after saved SP
	xchg
	pop	d		; get return address
	mov	e,m
	inx	h
	mov	d,m
	inx	h		; now points after saved return
	push	d		; put return back
	mov	m,c		; Save BC
	inx	h
	mov	m,b
	lxi	h,0		; Return 0
	ret
