;
;		TOS = lval of object HL = mask
;
		.export __andeq
		.export __andequ

		.setcpu 8080
		.code
__andeq:
__andequ:
		xchg
		pop	h
		xthl
		; HL is now the pointer, DE the mask
		mov	a,m
		ana	e
		mov	m,a
		mov	e,a
		inx	h
		mov	a,m
		ana	d
		mov	m,a
		mov	l,e
		mov	h,a
		ret
