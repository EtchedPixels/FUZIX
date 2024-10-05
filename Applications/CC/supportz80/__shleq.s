;
;	TOS is the lval, hl is the shift amount
;
;
		.export __shleq
		.code

__shleq:
		ld	a,l
		pop	hl		; return
		ex	(sp),hl		; HL is now the lval ptr
		ld	e,(hl)		; get the value into DE
		inc	hl
		ld	d,(hl)
		ex	de,hl		; value is in HL, ptr DE-1

		and	15
		ret	z
		cp	8
		jr	c,notquick
		ld	h,l
		ld	l,0
		sub	8
		jr	z,done
notquick:
		add	hl,hl
		dec	a
		jr	nz,notquick
done:
		; Store HL into DE-1
		ex	de,hl
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		ret
