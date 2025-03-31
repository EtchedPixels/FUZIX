
	.export	_longjmp

	.code

_longjmp:
	ld	ea,2,p1		; return value
	ld	t,ea
	or	a,e
	bnz	retok
	ld	t,=1		; return 1 if 0 is passed
retok:
	ld	ea,4,p1		; get the setjmp buffer
	ld	p2,ea		; pointer to buffer
	ld	ea,2,p2		; stack pointer recovery
	ld	p1,ea		; stack fixedx
	ld	ea,0,p2		; return address
	push	ea
	ld	ea,t		; get return value to use
	ret
