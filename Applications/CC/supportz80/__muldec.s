
		.export __muldec
		.code

;
;	L * A into HL
;
;	TODO: switch these around for more Z80 friendly and faster methods
;

__muldec:
		push	bc
		ld	e,a
		ld	d,l		; now D * E
		ld	hl,0		; into HL
		ld	b,8
next:
		ld	a,d
		rra
		ld	d,a
		jr	nc,noadd
		ld	a,h
		add	a,e
		ld	h,a
noadd:		ld	a,h
		rra
		ld	h,a
		ld	a,l
		rra
		ld	l,a
		djnz	next
		pop	bc
		ret
