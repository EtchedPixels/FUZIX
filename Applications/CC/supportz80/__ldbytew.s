;
;	Load byte from further off stack
;
	.export __ldbytew
	.code

__ldbytew:
	pop	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	hl
	add	hl,sp
	ld	l,(hl)
	ret

