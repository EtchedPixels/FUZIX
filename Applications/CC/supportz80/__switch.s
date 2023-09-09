;
;	Switch. We use the tables as is, nothing clever like
;	binary searching yet
;
		.export __switch
		.export __switchu
		.code

__switchu:
__switch:
		push	bc
		ld	b,h
		ld	c,l

		; DE points to the table in the format
		; Length
		; value, label
		; default label
		ex	de,hl
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
next:
		inc	hl		; Move on to value to check
		ld	a,(hl)
		cp	c
		inc	hl		; Move on to address
		ld	a,(hl)
		inc	hl
		jr	nz,nomatch
		cp	b
		jr	z,match
nomatch:
		inc	hl		; Skip address low
		dec	de
		ld	a,e
		or	d
		jr	nz, next
		inc	hl
		; We are pointing at the address
match:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		ex	de,hl
		pop	bc
		jp	(hl)
