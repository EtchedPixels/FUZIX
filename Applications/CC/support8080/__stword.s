;
;	Save word from further off stack
;
		.export __stword

		.setcpu 8080
		.code

__stword:
	xthl		; tos is now value, hl is return
	mov	e,m
	inx	h
	mvi	d,0
	xthl		; tos is now return hl is value
	xchg		; de is value hl is offset
	dad	sp
	mov	m,e
	inx	h
	mov	m,d
	xchg		; value back into hl
	ret
