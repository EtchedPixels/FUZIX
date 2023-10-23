			.export __shleql
			.setcpu 8080
			.code

__shleql:
	mov	a,l
	pop	h
	xthl
	; HL is now the lval, A is the shift
	ani	31
	jz	done
	push	h
	push	psw
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a
	pop	psw
loop:
	xchg
	dad	h
	xchg
	jc	slide1
	dad	h
	dcr	a
	jnz	loop
done:
	; our value is now in HLDE
	shld	__hireg		; save the upper half result
	pop	h		; lval back
	mov	m,e
	inx	h
	mov	m,d
	inx	h
	push	d
	xchg
	lhld	__hireg
	xchg
	mov	m,e		; write back the upper word
	inx	h
	mov	m,d
	pop	h		; get low word back for result
	ret
slide1:
	dad	h
	inr	l		; set low bit
	dcr	a
	jnz	loop
	jmp	done
