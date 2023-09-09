;
;	Save byte from further off stack
;
		.export __stbytew

		.setcpu 8080
		.code

__stbytew:
	xthl		; tos is now value, hl is return
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	xthl		; tos is now return hl is value
	xchg		; de is value hl is offset
	dad	sp
	mov	m,e
	xchg		; value back into hl
	ret
