;
;		TOS = lval of object HL = amount
;
		.export __postincc

		.setcpu 8080
		.code
__postincc:
		xchg			; E is now amount to add
		pop	h		; Return address
		xthl			; Swap with pointer
		mov	a,m		; Get old value
		mov	d,a		; Old value into D
		add	e		; Plus E
		mov	m,a		; Save to pointer
		mov	l,d		; into return
		mvi	h,0		; clear upper byte of working value
		ret
