;
;	Switch. We use the tables as is, nothing clever like
;	binary searching yet
;
		.export __switchl
		.export __switchlu
		.code

__switchl:
__switchlu:
		push	bc
		ld	b,h
		ld	c,l

		; DE points to the table in the format
		; Length
		; value.32, label.16
		; default label
		ex	de,hl
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		; DE is the number of records to scan

next:
		; Low byte compare to C
		inc	hl		; Move on to value 0
		ld	a,(hl)
		cp	c
		jr	nz,nomatch		; Value 0 didn't match

		; Second byte compare to B
		inc	hl		; Move on to value 1
		ld	a,(hl)
		cp	b		; Value 1 didn't match
		jr	nz,nomatch1

		; Third byte compare to __hireg
		inc	hl
		ld	a,(__hireg)
		cp	(hl)
		jr	nz,nomatch2

		; Final byte compare to __hireg+1
		inc	hl
		ld	a,(__hireg+1)
		cp	(hl)
		jr	nz,nomatch3
match:
		inc	hl		; Point to the address
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		ex	de,hl
		pop	bc
		jp	(hl)

nomatch:
		inc	hl
nomatch1:
		inc	hl		; Skip value 1
nomatch2:
		inc	hl		; Skip value 2
nomatch3:
		inc	hl		; Skip value 3
		inc	hl		; Skip to end of address
		dec	de
		ld	a,e
		or	d
		jr	nz, next
		jp	match
