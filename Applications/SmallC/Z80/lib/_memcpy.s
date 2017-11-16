		.code
		.export _memcpy

_memcpy:	push ix
		ld ix,#0
		add ix,sp
		ld e,(ix+4)
		ld d,(ix+5)
		ld l,(ix+6)
		ld h,(ix+7)
		ld c,(ix+8)
		ld b,(ix+9)
		ld a,b
		or c
		jr z, _memcpy_none
		ldir
_memcpy_none:	ld l,(ix+4)
		ld h,(ix+5)
		pop ix
		ret
