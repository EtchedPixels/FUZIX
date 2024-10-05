;
;		(TOS) /= L
;

		.export __remeqc
		.code
__remeqc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		; Now we are doing (HL) * DE
		push	de
		ld	e,(hl)
		ex	(sp),hl		; swap address with stacked value
		ex	de,hl		; swap them back as we divide by DE
		; We are now doing HL / DE and the address we want is TOS
		call	__sex
		ex	de,hl
		call	__sex
		ex	de,hl
		call	__remde
		; Return is in HL
		pop	de
		ld	a,l
		ld	(de),a
		ret
