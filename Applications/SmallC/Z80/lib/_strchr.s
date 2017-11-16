		.code
		.export	_strchr

_strchr:
		pop de		; char
		pop hl		; ptr
		pop bc
		push bc
		push hl
		push de
_strchr_1:	ld a,(hl)	; "The terminting nul is considered part
		cp e		;  of the string"
		ret z
		or a
		inc hl
		jr nz,_strchr_1
		ld hl,0
		ret

		