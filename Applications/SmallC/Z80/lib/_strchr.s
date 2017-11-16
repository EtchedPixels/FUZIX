		.code
		.export	_strchr

_strchr:
		pop af
		pop hl		; ptr
		pop de		; char
		push de
		push hl
		push af
_strchr_1:	ld a,(hl)	; "The terminting nul is considered part
		cp e		;  of the string"
		ret z
		or a
		inc hl
		jr nz,_strchr_1
		ld hl,0
		ret
