;
;		32bit negate
;
			.export __negatel
			.setcpu 8080
			.code
__negatel:
	mov	a,h
	cma
	mov	h,a
	mov	a,l
	cma
	mov	l,a
	inx	h
	mov	a,h
	ora	a
	push	h
	push	psw
	lhld	__hireg
	mov	a,h
	cma
	mov	h,a
	mov	a,l
	cma
	mov	l,a
	pop	psw
	jnz	nocarry
	inx	h
nocarry:
	shld	__hireg
	pop	h
	ret
