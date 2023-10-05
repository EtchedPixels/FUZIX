
			.export _memset
			.code

;
;	memset has an int sized pattern that is byte written.
;
;
_memset:
		lda	6(s)
		xay
		ldb	4(s)
		lda	2(s)
		jz	nowork

setl:
		stbb	(y+)
		dca
		jnz	setl
nowork:
		lda	6(s)
		rsr
