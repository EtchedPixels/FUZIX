		.code
		.export _strcmp

_strcmp:
		pop bc
		pop de
		pop hl
		push hl
		push de

_strcmp_1:
		ld a,(de)
		cp (hl)
		jr nz, strcmp_2
		or a
		jr z, strcmp_4
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
