;
;	memcpy
;
		.export _memcpy
		.code
_memcpy:
		push	bc
		ld	hl,9
		add	hl,sp
		ld	b,(hl)	; Count
		dec	hl
		ld	c,(hl)
		dec	hl
		ld	d,(hl)	; Source
		dec	hl
		ld	e,(hl)
		dec	hl
		ld	a,(hl)	; Destination
		dec	hl
		ld	l,(hl)
		ld	h,a

		ld	a,b
		or	c
		jr	z,done

		push	hl
		ex	de,hl
		ldir
		pop	hl
done:
		pop	bc
		ret
