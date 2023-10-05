
		.export _memcpy

		.code

_memcpy:
		xfr	z,a
		sta	(-s)
		lda	8(s)
		xay
		lda	6(s)
		xaz
		lda	4(s)
		bz	nowork
memcpyl:
		ldb	(z+)
		stb	(y+)
		dca
		bnz	memcpyl
		lda	8(s)
nowork:
		lda	(s+)
		xfr	a,z
		rsr
