;
;	Add de to (HL), return the result
;
		.export __pluseqde
		.code

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
