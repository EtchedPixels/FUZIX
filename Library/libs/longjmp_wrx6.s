
	.export	_longjmp

	.setcpu 4
	.code

;
;	We don't have much work to do. X is scratch, D is the return
;	code so really we only have S and the return address to worry
;	about
;
_longjmp:
	ldb	2(s)		; retval
	bnz	retok
	inr	b		; retval 1 if 0 requested
retok:
	ldx	4(s)
	lda	(x)
	xas
	lda	2(x)		; lost X save
	sta	(-s)
	ldx	4(x)
	lda	6(x)
	xay
	lda	8(x)
	xaz
	rsr
