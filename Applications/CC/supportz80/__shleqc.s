;
;	Shift TOS left by HL
;
		.export __shleqc
		.code

__shleqc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,e
		and	7
		ld	e,a
		ld	a,(hl)
		jr	z,noop
loop:
		add	a,a
		dec	e
		jr	nz,loop
		ld	(hl),a
noop:
		ld	l,a
		ret


