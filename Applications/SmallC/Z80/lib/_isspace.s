		.code
		.export	_isspace
_isspace:	pop de
		pop hl
		push hl
		push de
		ld a,l
		; Space
		cp 32
		jr z, ret1
		; 9-13
		cp 9
		jr c, ret0
		cp 13
		jr nc, ret0
		; No need to handle -1 specially
ret1:		ld hl,1
		ret
ret0:		ld hl,0
		ret
