		.export	__bool
		.export __cmpne0
		.export __cmpne0b
		.export __cmpgt0u
		.export __cmpgt0ub

		.setcpu 8080
		.code

__cmpgt0ub:
__cmpne0b:
		mvi	h,0
__cmpgt0u:
__cmpne0:	; a compare to non zero is a bool op
__bool:
		mov	a,h
		ora	l
		lxi	h,0
		rz
		inr	l		; NZ
		ret
