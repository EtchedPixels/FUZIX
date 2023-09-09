		.export __shll
		.setcpu 8080
		.code

__shll:
		mov	a,l		; shift amount
		pop	h
		shld	__retaddr	; save return address
		pop	h
		pop	d

		ani	31
		jz	done

		; Shift DEHL left by A

		cpi	24
		jc	not3byte
		mov	d,l
		mvi	e,0
		mov	h,e
		mov	l,e
		sui	24
		jmp	leftover
not3byte:	cpi	16
		jc	not2byte
		xchg
		lxi	h,0
		sui	16
		jmp	leftover
not2byte:	cpi	8
		jc	remainder
		mov	d,e
		mov	e,h
		mov	h,l
		mvi	l,0
		sui	8
leftover:	jz	done
remainder:
		push	b
		mov	c,a
shiftloop:
		mov	a,l
		ora	a
		ral
		mov	l,a
		mov	a,h
		ral
		mov	h,a
		mov	a,e
		ral
		mov	e,a
		mov	a,d
		ral
		mov	d,a
		dcr	c
		jnz	shiftloop
		pop	b
done:
		xchg
		shld	__hireg
		xchg
		jmp	__ret
