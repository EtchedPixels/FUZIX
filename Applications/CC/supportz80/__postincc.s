;
;		TOS = lval of object HL = amount
;
		.export __postincc
		.code

__postincc:
		ex	de,hl		; E is now amount to add
		pop	hl		; Return address
		ex	(sp),hl		; Swap with pointer
		ld	a,(hl)		; Get old value
		ld	d,a		; Old value into D
		add	a,e		; Plus E
		ld	(hl),a		; Save to pointer
		ld	l,d		; into return
		ld	h,0		; clear upper byte of working value
		ret
