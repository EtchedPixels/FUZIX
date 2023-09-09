;
;	TOS is the lval, hl is the shift amount
;
;
		.export __shleq
		.setcpu 8080
		.code

__shleq:
	mov	a,l
	pop	h		; return
	xthl			; HL is now the lval ptr
	mov	e,m		; get the value into DE
	inx	h
	mov	d,m
	xchg			; value is in HL, ptr DE-1

	ani	15
	rz
	cpi	8
	jc	notquick
	mov	h,l
	mvi	l,0
	sbi	8
	jz	done
notquick:
	dad	h
	dcr	a
	jnz	notquick
done:
	; Store HL into DE-1
	xchg
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	ret
