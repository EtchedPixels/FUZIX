;
;		True if TOS < HL
;
		.export __ccgtu
		.code

__ccgtu:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		or	a
		sbc	hl,de
		jp	c,__false
		jp	z,__false
		jp	__true
