		.export __shrul
		.export __shrl_p
		.setcpu 8080
		.code


__shrul:
		;	Shift top of stack by amount in HL
		mov	a,l		; shift amount
		pop	h		; return address
		pop	d		; lower half of value
		xthl			; swap return addr with lower half

		; value is now HL:DE

		ani	31		; nothing to do ?
		jz	done
;
;	Shortcut, do the bytes by register swap
;
__shrl_p:			; Positive side of __shrl joins here
		cpi	24
		jc	not3byte
		mov	e,h
		mvi	d,0
		mov	h,d
		mov	l,d
		sui	24
		jmp	leftover

not3byte:
		cpi	16
		jc	not2byte
		xchg			; HL into DE
		mvi	h,0
		mov	l,h
		sui	16
		jmp	leftover
not2byte:
		cpi	8
		jc	leftover
		mov	e,d
		mov	d,l
		mov	l,h
		mvi	h,0
		sui	8
;
;	Do any remaining work
;
leftover:
		jz	done
		push	b
		mov	c,a		; count into C
shloop:
		mov	a,h
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
		dcr	c
		jnz	shloop
		pop	b
done:
		shld	__hireg
		xchg
		ret
