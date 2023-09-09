;
;		Long xor, values are on stack and in hireg:h
;
		.export __xorl
		.code

__xorl:
		ex	de,hl		; value into de
		ld	hl,2		; base of value to xor
		add	hl,sp
		ld	a,(hl)
		xor	e
		ld	e,a
		inc	hl
		ld	a,(hl)
		xor	d
		ld	d,a
		inc	hl
		push	de
		ld	de,(__hireg)
		ld	a,(hl)
		xor	e
		ld	e,a
		inc	hl
		ld	a,(hl)
		xor	d
		ld	d,a
		ld	(__hireg),de
		pop	hl		; result
		pop	de
		pop	af
		pop	af
		push	de
		ret
