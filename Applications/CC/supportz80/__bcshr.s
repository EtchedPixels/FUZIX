		.export __bcshr
		.export __bcshru
		.code

__bcshr:
		; shift BC right by E
		ld	l,c
		ld	h,b
		call	__shrde
		ld	c,l
		ld	b,h
		ret

__bcshru:
		; shift BC right by E
		ld	l,c
		ld	h,b
		call	__shrdeu
		ld	c,l
		ld	b,h
		ret
