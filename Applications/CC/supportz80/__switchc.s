;
;	Switch. We use the tables as is, nothing clever like
;	binary searching yet
;
		.export __switchc
		.export __switchcu
		.code

__switchcu:
__switchc:
		push	bc
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
		ld	a,c
		cp	(hl)
		inc	hl		; Move on to address
		jr	z,match
		inc	hl		; Skip address low
		dec	de
		ld	a,d
		or	e
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
