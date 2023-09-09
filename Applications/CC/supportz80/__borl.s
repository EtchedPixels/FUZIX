;
;		Long or, values are on stack and in hireg:h
;
		.export __orl
		.code

__orl:
		ex	de,hl		; low word into DE
		ld	hl,2
		add	hl,sp		; so hl is the memory pointer, de the value
		ld	a,(hl)
		or	e
		ld	e,a
		inc	hl
		ld	a,(hl)
		or	d
		ld	d,a
		inc	hl
		push	de		; save result low
		ld	de,(__hireg)	; get high reg
		ld	a,(hl)		; do upper half
		or	e
		ld	e,a
		inc	hl
		ld	a,(hl)
		or	d
		ld	d,a
		ld	(__hireg),de	; upper into hireg
		pop	hl		; result
		pop	de		; return addr
		pop	af		; remove arg
		pop	af
		push	de		; and return
		ret
