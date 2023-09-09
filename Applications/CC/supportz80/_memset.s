;
;	Memset
;
;	TODO: rewrite into Z80 style with a set and copy
;
		.export _memset
		.code
_memset:
		push	bc
		ld	hl,4		; Allow for the push of BC
		add	hl,sp
		ld	e,(hl)
		inc	hl
		ld	d,(hl)		; Pointer
		push	de		; Return is the passed pointer
		inc	hl
		ld	a,(hl)		; fill byte
		inc	hl		; skip fill high
		inc	hl
		ld	c,(hl)
		inc	hl
		ld	b,(hl)		; length into BC

		ld	l,a		; We need to free up A for the loop check
		ex	de,hl		; now have HL as the pointer and E as the fill byte
		jp	loopin

loop:
		ld	(hl),e
		inc	hl
		dec	bc
loopin:
		ld	a,b
		or	c
		jr	nz,loop
		pop	hl		; Address passed in
		pop	bc		; Restore BC
		ret
