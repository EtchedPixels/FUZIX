		.export	__boolc
		.export __cmpne0c

		.code
;
;	Unlike most operations we require the bool ones to set the flags
;	for the jumps so the code stays a bit cleaner.
;
__cmpne0c:	; a compare to non zero is a bool op
__boolc:
		ld	a,l
		or	a	; Z if zero
		ld	hl,0
		ret	z
		inc	l	; force NZ
		ret
