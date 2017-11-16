		.code
		.export	_isascii
_isascii:	pop de
		pop hl
		push hl
		push de
		ld a,l
		cp 32
		jr c, ret0
		rlca
		jr c, ret0
		; No need to handle -1 specially
ret1:		ld hl,1
		ret
ret0:		ld hl,0
		ret
