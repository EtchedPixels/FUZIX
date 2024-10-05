;
;		TOS = lval of object HL = mask
;
		.export __oreq
		.export __orequ

		.setcpu 8080
		.code
__oreq:
__orequ:
		xchg
		pop	h
		xthl
		; Now DE is the mask and HL the pointer
		mov	a,m
		ora	e
		mov	m,a
		mov	e,a
		inx	h
		mov	a,m
		ora	d
		mov	m,a
		mov	l,e
		mov	h,a
		ret
