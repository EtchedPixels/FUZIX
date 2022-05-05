;
;	The tricky bit here is the stacked return X
;
;
	.export __setjmp
	.setcpu 6
	.code

__setjmp:
	lda	2(s)		; buffer
	stx	4(x)
	lda	(s)		; saved old X
	sta	2(x)
	xfr	s,a
	sta	(x)
	cla
	rsr
