;
;		TOS = lval of object HL = mask
;
		.export __xoreq
		.export __xorequ

		.setcpu 8080
		.code
__xoreq:
__xorequ:
		xchg
		pop	h
		xthl
		; Mask now in DE pointer in HL
		mov	a,m
		xra	e
		mov	m,a		; xor byte and store
		mov	e,a
		inx	h		; high byte
		mov	a,m
		xra	d
		mov	m,a		; same again
		mov	l,e		; recover value for caller
		mov	h,a
		ret
