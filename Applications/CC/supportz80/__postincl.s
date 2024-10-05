;
;		TOS = lval of object HL = amount. Amount is never long
;		on an 8085. The compiler generates this knowing that
;		int is safe (largest possible sizeof()).
;
		.export __postincl
		.code

__postincl:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		; HL is now the pointer, hireg:DE the amount
		ld	a,(hl)
		ld	(__tmp),a
		add	a,e
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__tmp+1),a
		adc	a,d
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__hireg),a
		adc	a,0
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__hireg+1),a
		adc	a,0
		ld	(hl),a
                ld	hl,(__tmp)
		ret
