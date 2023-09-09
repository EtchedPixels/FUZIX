		.export __shrl
		.code

__shrl:
		; Shift top of stack by amount in HL - signed shift
		ld	a,l		; shift amount
		pop	hl		; return address
		pop	de		; upper half of value
		ex	(sp),hl		; swap return addr with lower half

		; value is now HL:DE

		and	31		; nothing to do ?
		jr	z,done

		push	af

		ld	a,h
		or	a
		jp	m,shift_neg

		pop	af
		jp	__shrl_p

shift_neg:
		pop	af
;
;	Shortcut, do the bytes by register swap
;
		cp	24
		jr	c,not3byte
		ld	e,h
		ld	d,255
		ld	h,d
		ld	l,d
		sub	24
		jr	leftover

not3byte:
		cp	16
		jr	c,not2byte
		ex	de,hl		; HL into DE
		ld	h,255
		ld	l,d
		sub	16
		jr	leftover
not2byte:
		cp	8
		jr	c,leftover
		ld	e,d
		ld	d,l
		ld	l,h
		ld	h,255
		sub	8
;
;	Do any remaining work
;
leftover:
		jr	z,done
		push	bc
		ld	b,a		; count into B
shloop:
		sra	h
		rr	l
		rr	d
		rr	e
		djnz	shloop
		pop	bc
done:
		ld	(__hireg),hl
		ex	de,hl
		ret
