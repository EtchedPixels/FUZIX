			.export __andeql
			.setcpu 8080
			.code
__andeql:
	xchg
	pop	h
	xthl
	mov	a,e
	ana	m
	mov	e,a
	mov	m,a
	inx	h
	mov	a,d
	ana	m
	mov	d,a
	mov	m,a
	inx	h
	push	d		; save the lower result
	; Upper word
	xchg
	lhld	__hireg
	xchg
	mov	a,e
	ana	m
	mov	e,a
	mov	m,a
	inx	h
	mov	a,d
	ana	m
	mov	m,a
	mov	h,a
	mov	l,e
	shld	__hireg
	pop	h
	ret
