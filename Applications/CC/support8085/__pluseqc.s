;
;		TOS = lval of object L = amount
;
		.export __pluseqc

		.setcpu 8080
		.code
__pluseqc:
		xchg
		pop	h
		xthl
		mov	a,m
		add	e
		mov	m,a
		mov	l,a
		ret
