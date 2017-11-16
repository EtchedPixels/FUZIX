		.code
		.export _memset

_memset:	push ix
		ld ix,0
		add ix,sp
		ld h,4(ix)
		ld l,5(ix)
		ld e,6(ix)
		ld c,8(ix)
		ld b,9(ix)
		ld a,b
		or c
		jr z, _memset_none
		ld (hl),e
		dec bc
		ld a,b
		or c
		jr z,_memset_none
		ld e,l
		ld d,h
		inc de
		ldir
_memset_none:	ld l,4(ix)
		ld h,5(ix)
		ret
