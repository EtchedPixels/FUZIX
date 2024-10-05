;
;		32bit complement
;
			.export __cpll
			.setcpu 8080
			.code
__cpll:
	mov	a,h
	cma
	mov	h,a
	mov	a,l
	cma
	mov	l,a
	push	h
	lhld	__hireg
	mov	a,h
	cma
	mov	h,a
	mov	a,l
	cma
	mov	l,a
	shld	__hireg
	pop	h
	ret
