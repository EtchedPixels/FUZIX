		.code
		.export	_isupper
_isupper:	pop bc
		pop hl
		push hl
		push bc
		ld a,l
		; Space
		cp 'A'
		jr c, ret0
		cp 'Z'+1
		jr c, ret1
ret0:		ld hl,0
		ret
		; No need to handle -1 specially
ret1:		ld hl,1
		ret
