		.export	__booll

		.code

__booll:
		ld	a,h
		or	l
		ld	hl,(__hireg)
		or	h
		or	l
		ld	hl,0
		ret	z
		inc	l		; NZ
		ret
