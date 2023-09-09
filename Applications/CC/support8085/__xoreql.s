			.export __xoreql
			.setcpu 8080
			.code
__xoreql:
	xchg
	pop	h
	xthl
	mov	a,e
	xra	m
	mov	e,a
	mov	m,a
	inx	h
	mov	a,d
	xra	m
	mov	d,a
	mov	m,a
	inx	h
	push	d		; save the lower result
	; Upper word
	xchg
	lhld	__hireg
	xchg
	mov	a,e
	xra	m
	mov	e,a
	mov	m,a
	inx	h
	mov	a,d
	xra	m
	mov	m,a
	mov	h,a
	mov	l,e
	shld	__hireg
	pop	h
	ret
