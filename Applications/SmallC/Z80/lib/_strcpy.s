		.code
		.export _strcpy

_strcpy:	pop bc		;ret
		pop de		;dest
		pop hl		;src
		push hl
		push de
		push bc
		push de		; for return code
_strcpy_1:	ld a,(hl)
		ld (de),a
		or a
		jr z _strcpy_2
		inc hl
		inc de
		jr _strcpy_1
_strcpy_2:
		pop hl
		ret
