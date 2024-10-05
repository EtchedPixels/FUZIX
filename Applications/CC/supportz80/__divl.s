; 32 bits integer divide and remainder routine
; Bit 0 of a-reg is set iff quotient has to be delivered
; Bit 7 of a-reg is set iff the operands are signed, so:
; Expects in a-reg:	0 if called by rmu 4
;			1 if called by dvu 4
;			128 if called by rmi 4
;			129 if called by dvi 4
;
; Based on the ACK 8080 routine but with a bit less stack shuffling
;

		.export	__divl
		.export	__divul
		.export __reml
		.export __remul
		.export	__diveql
		.export	__divequl
		.export __remeql
		.export __remequl

		.code

; Calculate TOS / hireg:hl

__divl:
		ld	a,129
		jr	__divlop
__reml:
		ld	a,128
		jr	__divlop
__divul:
		ld	a,1
		jr	__divlop
__remul:
		xor	a		; remainder 32bit
__divlop:
		ld	(__tmp),hl	; 32bit divisor into hireg:tmp
		pop	de

		pop	hl		; 32bit dividend into tmp2
		ld	(__tmp2),hl
		pop	hl
		ld	(__tmp2+2),hl

		push	de		; Save return address

		;
		;	Entry point for assignment forms
		;
divldo:
		push	bc		; Save BC

		ld	hl,0			; store initial value of remainder
		ld	(__tmp3),hl
		ld	(__tmp3+2),hl

		ld	b,0

		push	af
		add	a,a
		jr	nc,nosignmod		; jump if unsigned

		;
		;	Do the preparatory juggling for signed maths
		;

		ld	a,(__tmp2+3)
		add	a,a
		jr	nc,nocomp
		ld	b,129
		ld	hl,__tmp2
		call	compl		; dividend is positive now

nocomp:
		ld	a,(__tmp+3)
		add	a,a
		jr	nc,nosignmod
		inc	b
		ld	hl,__tmp
		call	compl		; divisor is positive now

		;
		;	Unsigned 32bit divide. Signed maths fixups happen after this
		;

nosignmod:
		push	bc		; save the status of the signs


		;
		;	Standard divide algorithm 32 cycles
		;
		ld	b,32

		;
		;	Shift
		;
divloop:
		ld	hl,__tmp2		; 64bit left shift through tmp2 tmp3
		ld	c,8
		xor	a
shiftl:
		rl	(hl)
		inc	hl
		dec	c
		jr	nz,shiftl

		;
		;	32bit compare
		;
		ld	hl,__tmp3+3
		ld	de,__tmp+3
		ld	c,4

nextcmp:			; 1b
		ld	a,(de)
		cp	(hl)
		jr	z,same		; 0f
		jr	nc,smaller		; 3f
		jr	larger		; 4f
same:		dec	de		; 0f
		dec	hl
		dec	c
		jr	nz,nextcmp

		; Equal - fall through

larger:		; 4f
		ld	de,__tmp3		; remainder is larger or equal: subtract divisor
		ld	hl,__tmp
		ld	c,4
		xor	a
nextsub:
		ld	a,(de)		; 1b
		sbc	a,(hl)
		ld	(de),a
		inc	de
		inc	hl
		dec	c
		jr	nz,nextsub		; 1b
		ld	hl,__tmp2
		inc	(hl)

smaller:			; 3f
		dec	b
		jr	nz,divloop		; keep looping

		;
		;	The main work is done.
		;

		pop	bc		; state of signs
		pop	af		; A holds the operation flags
		rra
		jr	nc,remainder
		;
		;	Division
		;
		ld	a,b
		rra
		ld	hl,__tmp2	; complement quotient if divisor
		call	c,compl		; and dividend have different signs
		ld	hl,(__tmp2+2)	; quotient high
		ld	(__hireg),hl		; into hireg
		ld	hl,(__tmp2)
		jr	cleanup
		;
		;	Remainder
		;
remainder:
		ld	a,b
		add	a,a
		ld 	hl,__tmp3
		call	c,compl		; negate remainder if dividend was negative
		ld	hl,(__tmp3+2)
		ld	(__hireg),hl
		ld	hl,(__tmp3)
cleanup:
		pop	bc
		ret

; make 2's complement of 4 bytes pointed to by hl.
compl:		push	bc
		ld	c,4
		xor	a
complp:
		ld	a,0		; preserve carry
		sbc	a,(hl)
		ld	(hl),a
		inc	hl
		dec	c
		jr	nz,complp
		pop	bc
		ret
;
;	Helpers for assign forms
;
;	(TOS) = (TOS) / hireg:hl
;
__diveql:
		ld	a,129		; select operation
dodiveq:
		ld	(__tmp),hl	; so we can use hireg:tmp
		pop	hl		; return address
		ex	(sp),hl		; swap for lval
		push	hl		; save address
		ld	de,__tmp2
		push	af
		call	__copy4		; copy value into tmp2/tmp3
		pop	af
		call	divldo		; result in hireg:tmp
		pop	de
		push	hl
		ld	hl,__tmp2
		call	__copy4		; copy it back
		pop	hl
		ret
__divequl:
		ld	a,1
		jr	dodiveq
__remeql:
		ld	a,128
		jr	dodiveq
__remequl:
		xor	a
		jr	dodiveq
