;
;			HL / DE	unsigned
;
		.export __divu
		.export __divdeu
		.export __remu
		.export __remdeu
		.code

__divu:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		call	__remdeu
		ex	de,hl
		ret

__divdeu:
		call	__remdeu
		ex	de,hl
		ret


__remu:
		ex	de,hl
		pop	hl
		ex	(sp),hl
__remdeu:
		push	bc

		;	HL dividend, DE divisor
		ex	de,hl

		;	DE is now the dividend
		;	Negate HL into BC, so we can use dad to 16bit subtract

		ld	a,l
		cpl
		ld	c,a
		ld	a,h
		cpl
		ld	b,a
		inc	bc

		;	16 iterations, clear working register

		ld	hl,0
		ld	a,16

divloop:
		push	af

		;	HLDE <<= 1
		add	hl,hl
		ex	de,hl
		add	hl,hl
		ex	de,hl
		jr	nc,nocopy
		inc	hl		; safe to inc l ?
nocopy:

		push	hl		; save remainder to stack
		add	hl,bc		; subtract
		jr	nc,bigenough

		ex	(sp),hl			; swap remainder with result
		inc	de		; set the low bit (is it safe to inr e ?

bigenough:
		pop	hl		; relde remainder/result from stack
		pop	af		; recover count
		dec	a		; iterate
		jr	nz,divloop

		;	DE = result, HL remainder
		pop	bc
		ret
