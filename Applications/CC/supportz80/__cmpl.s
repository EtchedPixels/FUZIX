;
;	Compare two objects in memory unsigned and set C NC and Z
;	accordingly. Eats the two pointers in H and D
;
		.export __cmpul
		.export __cmpulws
		.export __cmpl
		.export __cmplws
		.code

__cmpulws:			; workspace and stack
		ld	(__tmp),hl	; word before __hireg
		ld	hl,7		; stacked value, high byte
		add	hl,sp
		ld	de,__hireg+1	; high byte first
__cmpul:
		ld	a,(de)
		cp	(hl)
		ret	nz
		dec	de
		dec	hl
		ld	a,(de)
		cp	(hl)
		ret	nz
		dec	de
		dec	hl
		ld	a,(de)
		cp	(hl)
		ret	nz
		dec	de
		dec	hl
		ld	a,(de)
		cp	(hl)
		ret

__cmplws:
		ld	(__tmp),hl
		ld	hl,7
		add	hl,sp
		ld	de,__hireg+1
__cmpl:
		; Same idea but we need to deal with signs first
		ld	a,(de)
		xor	(hl)
		; Same sign - unsigned compare
		jp	p,__cmpul
		xor	(hl)
		jp	m,setnc	; m +ve / d -ve
		inc	a	; clear carry
		; A cannot be 0xFF so this will ensure NZ
		ret
setnc:
		xor	a
		inc	a	; NZ
		scf		; NZ and C
		ret
		
