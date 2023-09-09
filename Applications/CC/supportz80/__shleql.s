		.export __shleql
		.code

__shleql:
		ld	a,l
		pop	hl
		ex	(sp),hl
		; HL is now the lval, A is the shift
		and	31
		jr	z,done
		push	af
		push	hl
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	hl
		ld	a,(hl)
		inc	hl
		ld	h,(hl)
		ld	l,a
		pop	af
loop:
		ex	de,hl
		add	hl,hl
		ex	de,hl
		adc	hl,hl
		dec	a
		jr	nz,loop
done:
		; our value is now in HLDE
		ld	(__hireg),hl	; save the upper half result
		pop	hl		; lval back
		ld	(hl),e
		inc	hl
		ld	(hl),d
		inc	hl
		push	de
		ld	de,(__hireg)
		ld	(hl),e		; write back the upper word
		inc	hl
		ld	(hl),d
		pop	hl		; get low word back for result
		ret
