		.export	__bool
		.export __cmpne0
		.export __cmpne0b
		.export __cmpgt0u
		.export __cmpgt0ub

		.code

__cmpgt0ub:
__cmpne0b:
		ld	h,0
__cmpgt0u:
__cmpne0:	; a compare to non zero is a bool op
__bool:
		ld	a,h
		or	l
		ld	hl,0
		ret	z
		inc	l		; NZ
		ret
