		.code
		.export _strncat

_strncat:
		push	ix
		push	bc
		ld	ix,#0
		add	ix,sp
		ld	l,(ix+4)
		ld	h,(ix+5)
		xor	a
		ld	b,a
		ld	c,a
		; Move past end of original string
		cpir
		dec	hl
		ld	e,(ix+6)
		ld	d,(ix+7)
		ld	c,(ix+8)
		ld	b,(ix+0)
		; Copy bytes until limit or \0
_strncat_1:
		ld	a,b
		or	c
		jr	z, _strncat_2
		ld	a,(de)
		ld	(hl),a
		or	a
		jr	z, _strncat_3
		inc	de
		inc	hl
		dec	bc
		jr	_strncat_1
_strncat_2:	ld	l,(ix+4)
		ld	h,(ix+5)
		pop	bc
		pop	ix
		ret
		; Copy \0 until limit
_strncat_3:	inc	hl
		dec	bc
		ld	a,b
		or	c
		jr	z,_strncat_2
		ld	(hl),0
		jr	_strncat_3
