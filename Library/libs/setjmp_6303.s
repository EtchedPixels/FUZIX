;
;	Setjmp is nice and simple on the 6803/6303. X is scratch, D is
;	return so only the program counter and S matter
;
_setjmp:
	ldx	3,s		; get the jmp buffer
	sts	,x		; remember the stack pointer
	std	2,x		; return address
	ldd	1,s		; return address
	std	0,x		; save it
	clra
	clrb
	rts
