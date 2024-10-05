;
;		(TOS) /= L
;

		.export __diveqc
		.code

__diveqc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		; Now we are doing (HL) / E
		push	hl
		ld	l,(hl)
		; We are now doing HL / DE and the address we want is TOS
		call	__sex
		ex	de,hl
		call	__sex
		ex	de,hl
		call	__divdeu
		; Return is in HL
		pop	de
		ld	a,l
		ld	(de),a
		ret
