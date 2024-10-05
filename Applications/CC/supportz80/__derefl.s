;
;		HL holds the pointer
;
		.export __derefl
		.export __dereflsp
		.code

__dereflsp:
		add	hl,sp
__derefl:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	hl
		push	de
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		ld	(__hireg),de
		pop	hl
		ret
