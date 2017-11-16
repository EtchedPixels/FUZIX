		.code
		.export	_isalpha
_isalpha:	pop de
		pop hl
		push hl
		push de
		ld a,l
		cp 'a'
		jr c, isalpha1
		sub 32
isalpha1:
		cp 'A'
		jr c, ret0
		cp 'Z'+1
		jr c, ret1
ret0:		ld hl,0
		ret
		; No need to handle -1 specially
ret1:		ld hl,1
		ret
