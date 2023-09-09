		.export __pluseql
		.code
		.setcpu	8080

__pluseql:
	xchg
	pop	h
	xthl

	; HL is pointer, hireg:de amount to add

	mov	a,m
	add	e
	mov	m,a
	mov	e,a
	inx	h
	mov	a,m
	adc	d
	mov	m,a
	mov	d,a
	inx	h
	push	d

	xchg
	lhld	__hireg
	xchg

	mov	a,m
	adc	e
	mov	m,a
	mov	e,a
	inx	h
	mov	a,m
	adc	d
	mov	m,a
	mov	d,a

	xchg
	shld	__hireg

	pop	h
	ret
