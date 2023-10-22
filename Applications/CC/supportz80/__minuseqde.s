;
;	Subtract de from (HL), return the result
;
		.export __minuseqde
		.export __minuseqe
		.export __minuseq1
		.export __minuseq2
		.export __minuseq3
		.export __minuseq4
		.code

__minuseq4:
		ld	e,4
		jr	__minuseqe
__minuseq3:
		ld	e,3
		jr	__minuseqe
__minuseq1:
		ld	e,1
		jr	__minuseqe
__minuseq2:
		ld	e,2
__minuseqe:
		ld	d,0
__minuseqde:
		push	bc
		ld	b,d	; save subtraction value into BC
		ld	c,e
		ld	a,(hl)
		inc	hl
		ld	d,(hl)
		sub	c
		ld	e,a
		ld	a,d
		sbc	a,b
		ld	d,a
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		pop	bc	; restore BC
		ret
