		.export __cceq

		.code

__cceq:		ex	de,hl
		pop	hl
		ex	(sp),hl
		or	a
		sbc	hl,de
		jp	nz,__false
		jp	__true
