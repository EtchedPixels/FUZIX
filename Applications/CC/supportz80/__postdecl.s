;
;		TOS = lval of object HL = amount
;		Amount is always within the size of a pointer so is not
;		turned long on 8085
;
		.export __postdecl
		.code

__postdecl:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,(hl)
		ld	(__tmp),a
		sub	e
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__tmp+1),a
		sbc	a,d
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__hireg),a
		sbc	a,0
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__hireg+1),a
		sbc	a,0
		ld	(hl),a
                ld	hl,(__tmp)
		ret
