	.export __shreql
	.export __shrequl
	.code
;
;	Has to be done the hard way
;
;	TOS holds the lval, HL the shift
;
;
;	We could optimize 8,16,24 bit shift slices with register swaps TODO
;
__shreql:
		ld	a,l
		pop	hl
		ex	(sp),hl
		push	bc		; save BC
		push	hl
		; HL is now the lval, A is the shift
		and	31
		jr	z,done
		ld	b,a		; count
		call	setup4		; HLDE is now the data
		ld	a,h
		or	a
		; When we do the optimized path his will help
		jp	p,shftu		; Sign bit positive - do unsigned shift
shftn:
		sra	h
		rr	l
		rr	d
		rr	e
		djnz	shftn

		; Result is now in HL:DE
store:
		ld	(__hireg),hl
		pop	hl
		ld	(hl),e
		inc	hl
		ld	(hl),d
		inc	hl
		push	de		; save low half
		ld	de,(__hireg)
		ld	(hl),d
		inc	hl
		ld	(hl),e
		pop	hl
		pop	bc
		ret

; No shift but still need to load it
done:
		call	setup4
		jr	store

__shrequl:
		ld	a,l
		pop	hl
		ex	(sp),hl
		push	bc		; save BC
		push	hl
		; HL is now the lval, A is the shift
		and	31
		jr	z,done
		ld	b,a		; count
		call	setup4		; HLDE is now the data
shftu:
		srl	h
		rr	l
		rr	d
		rr	e
		djnz	shftu

		jr	store

setup4:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	hl
		ld	a,(hl)
		inc	hl
		ld	h,(hl)
		ld	l,a
		ret
