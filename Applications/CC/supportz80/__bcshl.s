		.export __bcshl
		.code

__bcshl:
		; shift BC right by E
		ld	l,c
		ld	h,b
		call	__shlde
		ld	c,l
		ld	b,h
		ret
