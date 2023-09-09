;
;	Subtract de from (HL), return the result
;
			.export __minuseqde
			.setcpu 8080
			.code
__minuseqde:
	push	b
	mov	b,d	; save subtraction value into BC
	mov	c,e
	mov	a,m
	inx	h
	mov	d,m
	sub	c
	mov	e,a
	mov	a,d
	sbb	b
	mov	d,a
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	pop	b	; restore BC
	ret
