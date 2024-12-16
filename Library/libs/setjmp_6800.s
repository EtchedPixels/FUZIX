;
;	Setjmp is nice and simple on the 6800. X is scratch, D is
;	return so only the program counter and S matter
;
	.export __setjmp
	.code

__setjmp:
	tsx
	ldaa	,x		; return address
	ldab	1,x
	ldx	2,x		; get the jmp buffer
	sts	,x		; remember the stack pointer
	staa	2,x		; return address
	stab	3,x
	clra
	clrb
	jmp	__cleanup2	; clear the argument
