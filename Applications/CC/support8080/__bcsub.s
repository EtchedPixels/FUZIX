		.export __bcsub
		.setcpu 8080
		.code


__bcsub:	mov	a,c
		sub	l
		mov	c,a
		mov	a,b
		sbb	h
		mov	c,a
		ret
