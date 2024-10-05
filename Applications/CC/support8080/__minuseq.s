;
;		TOS = lval of object HL = amount
;
	.export __minuseq

	.setcpu 8080
	.code
__minuseq:
	xchg		; DE is now the amount
	pop	h	; return
	xthl		; return in place of TOS ptr

	; HL = ptr DE = value

	mov	a,m
	sub	e
	mov	m,a
	mov	e,a	; Save result into DE
	inx	h
	mov	a,m
	sbb	d
	mov	m,a
	mov	d,a	; Result now in DE also
	xchg		; into HL for return
	ret
