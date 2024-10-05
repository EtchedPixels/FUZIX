
	.export _memset
	.code

;
;	memset has an int sized pattern that is byte written.
;
;
_memset:
	stx	(-s)
	ldx	2(s)
	ldb	4(s)
	lda	6(s)
	jz	nowork

setl:
	stbb	(x+)
	dca
	jnz	setl
nowork:
	ldb	2(s)
	ldx	(s+)
	rsr
