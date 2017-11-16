		.code
		.export	_isgraph
_isgraph:	pop de
		pop hl
		push hl
		push de
		ld a,l
		cp 33
		jr c, ret0
		cp 127
		jr nc, ret1
ret0:		ld hl,0
		ret
		; No need to handle -1 specially
ret1:		ld hl,1
		ret
