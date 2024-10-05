		.export __ccne
		.code

__ccne:		ex	de,hl
		pop	hl
		ex	(sp),hl
		or	a
		sbc	hl,de
		ret	z
		jp	__true
