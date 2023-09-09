;
;		True if TOS < HL
;
		.export __ccltequ
		.code

__ccltequ:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		or	a
		sbc	hl,de
		jp	c,__true
		jp	z,__true
		jp	__false
