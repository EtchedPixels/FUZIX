;
;	Restore BC IX and IY as well as the stack pointer and then
;	return appropriately
;

		.export _longjmp
		.code

_longjmp:
	ld	hl,4
	add	hl,sp
	; We can destroy BC here - any old register variable state is
	; lost in the longjmp
	ld	c,(hl)
	inc	hl
	ld	b,(hl)		; BC is now return code
	ld	a,b
	or	c
	jr	nz, retok
	inc	c		; force return code non zero
retok:
	dec	hl
	dec	hl		; now points to end of buffer arg
	ld	d,(hl)
	dec	hl		; and start
	ld	e,(hl)		; DE is now buffer address
	ex	de,hl		; into HL

	ld	e,(hl)
	inc	hl
	ld	d,(hl)		; DE is the SP to go to
	inc	hl		; now points to return copy
	ex	de,hl
	inc	hl
	inc	hl		; move the SP to be past the old return
	ld	sp,hl		; set stack pointer
	ex	de,hl		; HL is now back to the return addr save
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	de		; return address onto stack
	push	bc		; push the return value
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	inc	hl		; BC back
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	de
	pop	ix		; IX back
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	push	de
	pop	iy		; IY back
	pop	hl		; return value
	ret
