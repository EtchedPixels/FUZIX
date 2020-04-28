;
;	Setjmp is nice and simple on the 6803/6303. X is scratch, D is
;	return so only the program counter and S matter
;
	.export __setjmp
	.setcpu 6803
	.code

__setjmp:
	tsx
	ldd	1,x		; return address
	ldx	3,x		; get the jmp buffer
	sts	,x		; remember the stack pointer
	std	2,x		; return address
	clra
	clrb
	rts
