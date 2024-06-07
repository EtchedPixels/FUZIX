
	.export _memcpy

	.code

_memcpy:
	stx	(-s)
	xfr	y,a
	sta	(-s)

	ldx	6(s)
	lda	8(s)
	xay
	lda	10(s)
	bz	nowork
memcpyl:
	ldb	(x+)
	stb	(y+)
	dca
	bnz	memcpyl
	ldb	6(s)
nowork:
	lda	(s+)
	xay
	ldx	(s+)
	rsr
