		.code
		.export	_isascii
_isascii:	pop bc
		pop hl
		push hl
		push bc
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
