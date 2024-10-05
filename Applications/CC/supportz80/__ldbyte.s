;
;	Load byte from 8bit range of stack
;
		.export __ldbyte
		.code

__ldbyte:
		pop	hl
		ld	a,(hl)
		inc	hl
		push	hl
		ld	l,a
		ld	h,0
		add	hl,sp
		ld	l,(hl)
		ret
