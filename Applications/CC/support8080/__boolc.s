		.export	__boolc
		.export __cmpne0c
		.export __boolc_a

		.setcpu 8080
		.code
;
;	Unlike most operations we require the bool ones to set the flags
;	for the jumps so the code stays a bit cleaner.
;
__cmpne0c:	; a compare to non zero is a bool op
__boolc:
		mov	a,l
__boolc_a:
		ora	a	; Z if zero
		lxi	h,0
		rz
		inr	l	; force NZ
		ret
