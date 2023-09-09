;
;	Shift the top of stack right by HL (arithemtic)
;
		.export __shr
		.code
__shr:
		ld	a,l
		pop	hl
		ex	(sp),hl
		and	15		; A is shift, HL is value
		ret	z
		cp	8
		jr	c,shift
		ld	l,h
		bit	7,h
		ld	h,0
		jr	z,pve
		dec	h		; FF for shifed signed negative 8 bits
pve:
		sub	8
shift:		sra	h
		rr	l
		dec	a
		jr	nz,shift
		ret
