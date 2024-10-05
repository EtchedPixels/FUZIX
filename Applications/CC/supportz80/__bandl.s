;
;		Long or, values are on stack and in hireg:h
;
		.export __bandl
		.code

__bandl:
		ex	de,hl		; pointer into de
		ld	hl,2
		add	hl,sp		; so m is the memory pointer, de the value
		ld	a,(hl)
		and	e
		ld	e,a
		inc	hl
		ld	a,(hl)
		and	d
		ld	d,a
		inc	hl
		push	de
		ld	de,(__hireg)
		ld	a,(hl)
		and	e
		ld	e,a
		inc	hl
		ld	a,(hl)
		and	d
		ld	d,a
		ld	(__hireg),de
		pop	hl		; result
		pop	de
		pop	af
		pop	af
		push	de
		ret
