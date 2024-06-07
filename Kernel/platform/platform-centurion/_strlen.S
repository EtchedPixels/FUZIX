
	.export	_strlen
	.code

_strlen:
	stx	(-s)
	ldx	2(s)
	cla
sl:	ldab	(x+)
	bz	strlen_done
	ina
	bra	sl
strlen_done:
	xab
	ldx	(s+)
	rsr
