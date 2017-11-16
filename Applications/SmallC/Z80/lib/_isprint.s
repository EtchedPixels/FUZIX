		.code
		.export	_isprint
_isprint:	pop bc
		pop hl
		push hl
		push bc
		ld a,l
		cp 32
		jr c, ret0
		cp 128
		jr c, ret1
ret0:		ld hl,0
		ret
		; No need to handle -1 specially
ret1:		ld hl,1
		ret
