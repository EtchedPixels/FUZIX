		.code
		.export	_isblank
_isblank:	pop bc
		pop hl
		push hl
		push bc
		ld a,l
		; Space
		cp 32
		jr z, ret1
		cp 9
		jr z, ret1
ret0:		ld hl,0
		ret
		; No need to handle -1 specially
ret1:		ld hl,1
		ret
