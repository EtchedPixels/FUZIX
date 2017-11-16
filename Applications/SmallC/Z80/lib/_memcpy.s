		.code
		.export _memcpy

_memcpy:	push ix
		ld ix,#0
		add ix,sp
		ld e,4(ix)
		ld d,5(ix)
		ld l,6(ix)
		ld h,7(ix)
		ld c,8(ix)
		ld b,9(ix)
		ld a,b
		or c
		jr z, _memcpy_none
		ldir
_memcpy_none:	ld l,4(ix)
		ld h,5(ix)
		ret
