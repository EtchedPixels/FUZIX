		.code
		.export	_isalnum

_isalnum:	pop de
		pop hl
		push hl
		push de
		ld a,l
		cp '9'
		jr nc, isalnum1
		cp '0'
		jr c, ret0
ret1:		ld hl,1
		ret
isalnum1:
		cp 'a'
		jr c, isalnum2
		sub 32
isalnum2:
		cp 'A'
		jr c, ret0
		cp 'Z'+1
		jr c, ret1
ret0:		ld hl,0
		ret
