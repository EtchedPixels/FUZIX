;
;	strlen
;
		.export _strlen
		.code

_strlen:
		pop	hl
		pop	de
		push	de
		push	hl
		ld	hl,0
loop:
		ld	a,(de)
		inc	de
		or	a
		ret	z
		inc	hl
		jr	loop
