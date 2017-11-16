		.code
		.export	_iscntrl
_iscntrl:	pop de
		pop hl
		push hl
		push de
		ld a,l
		; Space
		cp 32
		jr c, ret1
		cp 127
		jr z, ret1
ret0:		ld hl,0
		ret
		; No need to handle -1 specially
ret1:		ld hl,1
		ret
