		.code
		.export	_isxdigit
_isxdigit:	pop de
		pop hl
		push hl
		push de
		ld a,l
		; Space
		cp '0'
		jr c, ret0
		cp 'A'
		jr nc,ascii1
		cp 'a'
		jr nc,ascii2
		cp '9'+1
		jr nc, ret0
		jr ret1
ascii1:		add a,32
ascii2:
		cp 'a'
		jr c, ret0
		cp 'z'+1
		jr c, ret1
ret0:		ld hl,0
		ret
		; No need to handle -1 specially
ret1:		ld hl,1
		ret
