;
;	Add de to (HL), return the result
;
		.export __pluseqde
		.export __pluseqe
		.export __pluseq1
		.export __pluseq2
		.export __pluseq3
		.export __pluseq4
		.code

__pluseq4:
		ld	e,4
		jr	__pluseqe
__pluseq3:
		ld	e,3
		jr	__pluseqe
__pluseq1:
		ld	e,1
		jr	__pluseqe
__pluseq2:
		ld	e,2		; most common case
__pluseqe:
		ld	d,0
__pluseqde:
		push	bc
		ld	c,(hl)
		inc	hl
		ld	b,(hl)
		ex	de,hl
		add	hl,bc	; do the math
		ex	de,hl
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		pop	bc	; restore BC
		ret
