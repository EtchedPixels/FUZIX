;
;		TOS = lval of object L = mask
;
		.export __xoreqc

		.setcpu 8080
		.code
__xoreqc:
		xchg
		pop	h		; return address
		xthl			; swap it for the lval
		mov	a,e
		xra	m
		mov	m,a
		mov	l,a
		ret
