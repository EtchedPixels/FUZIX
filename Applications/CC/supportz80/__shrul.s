		.export __shrul
		.export __shrl_p
		.code


__shrul:
		;	Shift top of stack by amount in HL
		ld	a,l		; shift amount
		pop	hl		; return address
		pop	de		; lower half of value
		ex	(sp),hl		; swap return addr with lower half

		; value is now HL:DE

		and	31		; nothing to do ?
		jr	z,done
;
;	Shortcut, do the bytes by register swap
;
__shrl_p:			; Positive side of __shrl joins here
		cp	24
		jr	c,not3byte
		ld	e,h
		ld	d,0
		ld	h,d
		ld	l,d
		sub	24
		jr	leftover

not3byte:
		cp	16
		jr	c,not2byte
		ex	de,hl			; HL into DE
		ld	h,0
		ld	l,h
		sub	16
		jr	leftover
not2byte:
		cp	8
		jr	c,leftover
		ld	e,d
		ld	d,l
		ld	l,h
		ld	h,0
		sub	8
;
;	Do any remaining work
;
leftover:
		jr	z,done
		push	bc
		ld	b,a		; count into B
shloop:
		srl	h
		rr	l
		rr	d
		rr	e
		djnz	shloop
		pop	bc
done:
		ld	(__hireg),hl
		ex	de,hl
		ret
