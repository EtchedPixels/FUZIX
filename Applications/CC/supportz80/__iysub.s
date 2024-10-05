		.export __iysub
		.code


__iysub:	ex	de,hl
		push	iy
		pop	hl
		or	a
		sbc	hl,de
		push	hl
		pop	iy
		ret
