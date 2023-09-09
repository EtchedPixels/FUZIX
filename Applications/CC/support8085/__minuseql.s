		.export __minuseql
		.code
		.setcpu	8080

__minuseql:
	xchg
	pop	h
	xthl

	; HL is pointer, hireg:de amount to sub

	mov	a,m
	sub	e
	mov	m,a
	mov	e,a
	inx	h
	mov	a,m
	sbb	d
	mov	m,a
	mov	d,a
	inx	h
	push	d

	xchg
	lhld	__hireg
	xchg

	mov	a,m
	sbb	e
	mov	m,a
	mov	e,a
	inx	h
	mov	a,m
	sbb	d
	mov	m,a
	mov	d,a

	xchg
	shld	__hireg

	pop	h
	ret
