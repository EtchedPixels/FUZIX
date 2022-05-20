
		.export	_strlen
		.code

_strlen:
		clr	y
		lda	2(s)
sl:		ldb	(a+)
		bz	strlen_done
		inr	y
		bra	sl
strlen_done:
		xfr	y,a
		rsr
