
		.setcpu 8080
		.export __muldeb
		.export __mulde
		.export __mul

		.code

__mul:
		xchg
		pop	h
		xthl
;
;		HL * DE
;
__mulde:	push	b

		mov	b,h		; save old upper byte

		mov	a,l		; work on old lower
		mvi	c,8

		lxi	h,0		; accumulator for the shift/adds

low:		rar
		jnc	noadd1
		dad	d
noadd1:		xchg
		dad	h
		xchg
		dcr	c
		jnz	low

		mov	a,b
		mvi	c,8

hi:		rar
		jnc	noadd2
		dad	d
noadd2:		xchg			; 8085 can ora rdel not really
		dad	h		; worth the hassle ?
		xchg
		dcr	c
		jnz	hi

		; result is in HL

		pop	b
		ret
__muldeb:	mvi	h,0
		jmp	__mulde
