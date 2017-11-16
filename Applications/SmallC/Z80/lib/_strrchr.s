		.code
		.export	_strrchr

_strrchr:
		pop de		; char
		pop hl		; ptr
		pop bc
		push bc
		push hl
		push de
		ld bc, 0
_strrchr_1:	ld a,(hl)
		or a
		jr z, _strrchr_2
		cp e
		jr z, _strrchr_3
		inc hl
		jr _strrchr_1
_strrchr_2:
		ld h,b
		ld l,c
		ret
_strrchr_3:
		ld b,h
		ld c,l
		jr _strrchr_1
