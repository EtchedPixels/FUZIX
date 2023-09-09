;
;	Add de to (HL), return the result
;
			.export __pluseqde
			.setcpu 8080
			.code
__pluseqde:
	push	b
	mov	c,m
	inx	h
	mov	b,m
	xchg
	dad	b	; do the math
	xchg
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	pop	b	; restore BC
	ret
