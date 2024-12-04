;
;	Setjmp is nice and simple on the 6800. X is scratch, D is
;	return so only the program counter and S matter
;
	.export __setjmp
	.code

__setjmp:
	tsx
	ldab	,x		; return address
	ldaa	1,x
	ldx	2,x		; get the jmp buffer
	sts	,x		; remember the stack pointer
	stab	2,x		; return address
	staa	3,x
	clra
	clrb
	jmp	__popret2	; clear the argument
