		.code
		.export _strcmp

_strcmp:
		pop af
		pop de
		pop hl
		push hl
		push de
		push af

_strcmp_1:
		ld a,(de)
		cp (hl)
		jr nz, _strcmp_2
		or a
		jr z, _strcmp_4
		inc hl
		inc de
		jr _strcmp_1
_strcmp_2:	jr c,_strcmp_3
		ld hl,1
		ret
_strcmp_3:
		ld hl,0xFFFF
		ret
_strcmp_4:
		ld hl,0
		ret
