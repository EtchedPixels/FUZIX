
	.export	_longjmp

	.setcpu 6
	.code

;
;	We don't have much work to do. X is scratch, D is the return
;	code so really we only have S and the return address to worry
;	about
;
_longjmp:
	lda	2(s)		; retval
	bnz	retok
	ina			; retval 1 if 0 requested
retok:
	ldx	4(s)
	ldb	(x)
	xfr	b,s
	ldb	2(x)		; lost X save
	stb	(-s)
	ldx	4(x)
	rsr
