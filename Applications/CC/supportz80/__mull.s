;
;		working = working * TOS
;
;	TODO: rework this Z80 style rather than the direct 8080 conversion
;	it is at the moment
;
		.export __mull
		.export __muleql
		.export __copy4
		.code

__mull:
		ld	(__tmp),hl	; hireg:tmp is now one half of the sum
		pop	de		; d is the return address
		pop	hl
		ld	(__tmp2),hl	; tmp2 holds the other
		pop	hl
		ld	(__tmp2 + 2),hl
		push	de		; return address back

__domull:
		push	bc		; save BC


		ld	hl,0
		ld	(__tmp3),hl	; tmp3 our working result
		ld	(__tmp3+2),hl
		ld	bc,0

nextbyte:
		ld	hl,__tmp	; work through tmp into hireg
		add	hl,bc		; at this point B is 0 and C is byte count
		ld	a,(hl)		; get next byte of multiplier
		ld	b,8		; work through the byte

nextbit:
		rra
		jr	nc,noadd
		ld	hl,(__tmp2)	; 32bit add of the product
		ex	de,hl
		ld	hl,(__tmp3)
		add	hl,de
		ld	(__tmp3),hl
		ld	hl,(__tmp2+2)
		jr	nc,nocarry
		inc	l		; we know l is even so this can't carry
nocarry:
		ex	de,hl
		ld	hl,(__tmp3+2)
		add	hl,de		; TODO -use adc hl,de ??
		ld	(__tmp3+2),hl
noadd:
		ld	hl,(__tmp2)	; 32bit left shift using dad
		add	hl,hl
		ld	(__tmp2),hl
		ld	hl,(__tmp2+2)
		jr	nc,noshiftc
		add	hl,hl
		inc	l		; we know l is even so this can't carry
		jr	shifted
noshiftc:
		add	hl,hl
shifted:
		ld	(__tmp2+2),hl

		; Complete the byte
		djnz	nextbit

		; Now move on to the next word
		inc	c
		ld	a,c
		cp	4
		jr	nz,nextbyte

		; At this point tmp3 holds the 32bit result
		ld	hl,( __tmp3+2)
		ld	(__hireg),hl
		ld	hl,(__tmp3)
		pop	bc
		ret

__muleql:
		ld	(__tmp),hl	; working into hireg:tmp
		pop	hl		; return address
		ex	(sp),hl		; swap back in for lval pointer
		push	hl
		ld	de,__tmp2	; copy lval into tmp2
		call	__copy4
		call	__domull	; result is now in hireg;tmp
		pop	de
		ld	(__tmp),hl
		ld	hl,__tmp
		call	__copy4		; stick it back in the register
		ld	hl,(__tmp)	; set up HL correctly for return
		ret

__copy4:			; copy 4 bytes from HL to DE
		push	bc
		ldi
		ldi
		ldi
		ldi
		pop	bc
		ret
