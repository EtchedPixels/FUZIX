		.export __bcsub
		.setcpu 8080
		.code


__bcsub:	mov	a,c
		sub	l
		mov	c,a
		mov	a,b
		sbb	h
		mov	b,a
		ret
