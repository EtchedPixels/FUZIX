;
;	Right shift TOS left by HL
;
		.export __shreqc
		.code

__shreqc:
		ex	de,hl
		pop	hl
		ex	de,hl
		ld	a,e
		and	7
		ld	e,a
		ld	a,(hl)
		jr	z,noop
loop:
		sra	a
		dec	e
		jr	nz,loop
		ld	(hl),a
noop:
		ld	l,a
		ret


