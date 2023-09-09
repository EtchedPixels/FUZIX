;
;		TOS = lval of object L = mask
;
		.export __andeqc

		.setcpu 8080
		.code
__andeqc:
		xchg
		pop	h		; return address
		xthl			; swap it for the lval
		mov	a,e
		ana	m
		mov	m,a
		mov	l,a
		ret
