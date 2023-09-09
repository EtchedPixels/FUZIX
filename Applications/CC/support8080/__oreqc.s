;
;		TOS = lval of object L = mask
;
		.export __oreqc

		.setcpu 8080
		.code
__oreqc:
		xchg
		pop	h		; return address
		xthl			; swap it for the lval
		mov	a,e
		ora	m
		mov	m,a
		mov	l,a
		ret
