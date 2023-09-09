;
;	Save word from further off stack
;
		.export __stwordw

		.setcpu 8080
		.code

__stwordw:
	xthl		; tos is now value, hl is return
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	xthl		; tos is now return hl is value
	xchg		; de is value hl is offset
	dad	sp
	mov	m,e
	inx	h
	mov	m,d
	xchg		; value back into hl
	ret
