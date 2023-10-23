;
;	TODO: register movement optimizations for 8 16 and 24
;

			.export __shleql
			.setcpu 8080
			.code

__shleql:
	mov	a,l		; Save the shift value
	pop	h		; Return address
	xthl			; Swap with the pointer
	; HL is now the lval, A is the shift
	push	h		; Save pointer
	push	psw		; Save amount to shift
	mov	e,m		; Load HLDE with the 32bit value
	inx	h
	mov	d,m
	inx	h
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a
	pop	psw		; Get back shift amount
	ani	31
	jz	done		; No work
loop:
	xchg			; Shift low word left one
	dad	h
	xchg
	jc	slide1		; Bump as needed if cary
	dad	h		; No carry so just shift high left one
	dcr	a		; Count down
	jnz	loop		; Are we there yet ?
done:
	; our value is now in HLDE
	shld	__hireg		; save the upper half result
	pop	h		; lval back
	mov	m,e		; Write back value
	inx	h
	mov	m,d		; Low word
	inx	h
	push	d		; Save the low word (we need it for return)
	xchg
	lhld	__hireg		; Get high word into DE
	xchg
	mov	m,e		; write back the upper word
	inx	h
	mov	m,d
	pop	h		; get low word back for result
	ret
slide1:				; Shift with carry case
	dad	h		; Double 
	inr	l		; set low bit
	dcr	a		; Next shift
	jnz	loop
	jmp	done
