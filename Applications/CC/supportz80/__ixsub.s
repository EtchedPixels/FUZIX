		.export __ixsub
		.code


__ixsub:	ex	de,hl
		push	ix
		pop	hl
		or	a
		sbc	hl,de
		push	hl
		pop	ix
		ret
