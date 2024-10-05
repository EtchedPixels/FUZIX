;
;	Load word from further off stack
;
		.export __ldwordw
		.code

__ldwordw:
		pop	hl
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	hl
		push	hl
		ex	de,hl
		add	hl,sp
		ld	a,(hl)
		inc	hl
		ld	h,(hl)
		ld	l,a
		ret

