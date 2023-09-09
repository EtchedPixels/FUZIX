		.export	__booll

		.setcpu 8080
		.code

__booll:
		mov	a,h
		ora	l
		lhld	__hireg
		ora	h
		ora	l
		lxi	h,0
		rz
		inr	l		; NZ
		ret
