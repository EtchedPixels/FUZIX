;
;		TOS = lval of object L = amount
;
		.export __minuseqc
		.export __minusequc

		.setcpu 8080
		.code
__minuseqc:
__minusequc:
		xchg
		pop	h
		xthl
		mov	a,m
		sub	e
		mov	m,a
		mov	l,a
		ret

