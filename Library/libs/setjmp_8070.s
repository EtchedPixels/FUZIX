;
;	p1 is the stack pointer
;	p2/p3/ea are scratch so simples
;	(:__tmp etc are also scratch)
;
	.export __setjmp
	.code

__setjmp:
	ld	ea,2,p1		; pointer to buffer
	ld	p2,ea
	ld	ea,0,p1		; return address
	st	ea,0,p2
	ld	ea,p1		; stack pointer
	st	ea,2,p2
	ld	ea,=0
	ret
