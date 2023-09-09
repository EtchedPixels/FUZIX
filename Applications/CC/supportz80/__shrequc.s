;
;	Right shift TOS by HL
;
		.export __shrequc
		.code

__shrequc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,e
		and	7
		ld	e,a
		ld	a,(hl)
		jr	z, noop
loop:
		srl	a
		dec	e
		jr	nz, loop
		ld	(hl),a
noop:
		ld	l,a
		ret
