;
;	Load byte from 8bit range of stack
;
		.export __ldbyte
		.code

__ldbyte:
		pop	hl
		ld	e,(hl)
		inc	hl
		ld	d,0
		push	hl
		add	hl,sp
		ld	l,(hl)
		ret
