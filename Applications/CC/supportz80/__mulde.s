		.export __muldeb
		.export __mulde
		.export __mul

		.code
;
;	TODO: rework for Z80 optimized - use B not C etc
;

__mul:
		ex	de,hl
		pop	hl
		ex	(sp),hl
;
;		HL * DE
;
__mulde:	push	bc

		ld	b,h		; save old upper byte

		ld	a,l		; work on old lower
		ld	c,8

		ld	hl,0		; accumulator for the shift/adds

low:		rra
		jr	nc, noadd1
		add	hl,de
noadd1:		ex	de,hl
		add	hl,hl
		ex	de,hl
		dec	c
		jr	nz, low

		ld	a,b
		ld	c,8

hi:		rra
		jr	nc,noadd2
		add	hl,de
noadd2:		ex	de,hl
		add	hl,hl
		ex	de,hl
		dec	c
		jr	nz,hi

		; result is in HL

		pop	bc
		ret
__muldeb:	ld	h,0
		jr	__mulde
