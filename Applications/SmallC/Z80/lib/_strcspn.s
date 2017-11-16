		.code
		.export _strcspn

_strcspn:
		pop bc
		pop hl		;	string
		pop de		;	match
		push de
		push hl
		push bc

_strcspn_1:
		ld a,(hl)
		or a
		jr z,_strcspn_4
		ld c,a

		push de
_strcspn_2:
		ld a,(de)
		or a
		jr z, _strcspn_3
		cp c
		ret z		;	Matched HL ponts to right spot
		inc de
		jr _strcspn_2
_strcpsn_3:	pop de
		inc hl
		jr _strcspn_1
_strcspn_4:	ld h,a		;	A always 0 here
		ld l,a
		ret
