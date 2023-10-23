	.export __shreql
	.export __shrequl
	.setcpu 8080
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
	mov	a,l
	pop	h
	xthl
	push	b		; save BC
	push	h
	; HL is now the lval, A is the shift
	ani	31
	jz	done
	mov	b,a		; count
	call	setup4		; HLDE is now the data
	mov	a,h
	ora	a
	jp	shftu		; Sign bit positive - do unsigned shift
shftn:
	mov	a,h
	stc			; Set top bit as we are shifting a negative number
	rar
	mov	h,a
	mov	a,l
	rar
	mov	l,a
	mov	a,d
	rar
	mov	d,a
	mov	a,e
	rar
	mov	e,a
	dcr	b
	jnz	shftn

	; Result is now in HL:DE, TOS is the pointer
store:
	shld	__hireg		; save high word
	pop	h
	mov	m,e		; Write low word back
	inx	h
	mov	m,d
	inx	h
	push	d		; Save low word
	xchg
	lhld	__hireg		; Get high word in DE
	xchg
	mov	m,e		; Write high word
	inx	h
	mov	m,d
	pop	h		; get low back
	pop	b		; recover register values
	ret

; No shift but still need to load it
done:
	call	setup4
	jmp	store

; Just load the value on a 0 shfit
nowork:
	call	setup4
	xchg
	pop	b
	ret
;
;	Shift through A on the 8080
;
__shrequl:
	mov	a,l		; save shift value
	pop	h		; return
	xthl			; swap with pointer
	push	b		; save BC
	; HL is now the lval, A is the shift
	ani	31
	jz	nowork
	push	h		; save pointer
	mov	b,a		; count
	call	setup4		; HLDE is now the data
shftu:
	mov	a,h		; Shift 32bits HLDE right through A
	ora	a
	rar
	mov	h,a
	mov	a,l
	rar
	mov	l,a
	mov	a,d
	rar
	mov	d,a
	mov	a,e
	rar
	mov	e,a
	dcr	b
	jnz	shftu

	jmp	store
;
;	Load DEHL from (HL). Destroys A
;
setup4:
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a
	ret
