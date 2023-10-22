;
;		highreg:hl = TOS - hireg:hl
;
		.export __minusl
		.code

__minusl:
		ex	de,hl	; low half of value into DE
		ld	hl,2
		add	hl,sp	; pointer into stack

		ld	a,(hl)
		sub	e
		ld	e,a
		inc	hl
		ld	a,(hl)
		sbc	a,d
		ld	d,a
		inc	hl

		push	de	; save low word

		ld	de,(__hireg)

		ld	a,(hl)
		sbc	a,e
		ld	e,a
		inc	hl
		ld	a,(hl)
		sbc	a,d
		ld	d,a

		ld	(__hireg),de

		pop	hl	; our result
		pop	de	; our return address
		pop	af	; throw the argument
		pop	af
		push	de
		ret
