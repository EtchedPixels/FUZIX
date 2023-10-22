		.export __bcsub
		.code


__bcsub:	ld	a,c
		sub	l
		ld	c,a
		ld	a,b
		sbc	a,h
		ld	b,a
		ret
