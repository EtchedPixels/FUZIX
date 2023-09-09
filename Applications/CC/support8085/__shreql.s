			.export __shreql
			.export __shrequl
			.setcpu 8085
			.code
;
;	On 8085 we have ARHL so right arithmetic isn't too hard
;
__shreql:
	mov	a,l
	pop	h
	xthl
	push	b
	; HL is now the lval, A is the shift
	ani	31
	jz	done
	push	h
	call	setup4
loop:
	arhl
loop2:
	xchg
	jc	slide1
	arhl
	xchg
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
	pop	b
	ret
slide1:
	arhl
	dad	b		; set top bit
	xchg
	dcr	a
	jnz	loop
	jmp	done

;
;	We have no right shift logical
;
__shrequl:
	mov	a,l
	pop	h
	xthl
	push	b
	; HL is now the lval, A is the shift
	ani	31
	jz	done
	push	h
	call	setup4

	;	Do one slow shift to get a 0 bit top then fall into the
	;	signed version
	arhl
	push	psw
	mov	a,h
	ani	0x7F
	mov	h,a
	pop	psw
	jmp	loop2

setup4:
	push	psw
	lxi	b,0x8000	; used to do the shift carry
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a
	pop	psw
	ret
