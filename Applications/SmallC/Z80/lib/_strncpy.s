		.code
		.export _strncpy

_strncpy:
		push	ix
		ld	ix,#0
		add	ix,sp
		ld	l,4(ix)
		ld	h,5(ix)
		ld	e,6(ix)
		ld	d,7(ix)
		ld	c,8(ix)
		ld	b,9(ix)
		; Copy bytes until limit or \0
_strncpy_1:
		ld	a,b
		or	c
		jr	z, _strncpy_2
		ld	a,(de)
		ld	(hl),a
		or	a
		jr	z, _strncpy_3
		inc	de
		inc	hl
		dec	bc
		jr	_strncpy_1
_strncpy_2:	ld	l,4(ix)
		ld	h,5(ix)
		pop	ix
		ret
		; Copy \0 until limit
_strncpy_3:	inc	hl
		dec	bc
		ld	a,b
		or	c
		jr	z,_strncpy_2
		ld	(hl),0
		jr	_strncpy_3
