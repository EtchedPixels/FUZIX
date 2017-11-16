		.code
		.export _memset

_memset:	push ix
		push bc
		ld ix,0
		add ix,sp
		ld h,(ix+4)
		ld l,(ix+5)
		ld e,(ix+6)
		ld c,(ix+8)
		ld b,(ix+9)
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
_memset_none:	ld l,(ix+4)
		ld h,(ix+5)
		pop bc
		pop ix
		ret
