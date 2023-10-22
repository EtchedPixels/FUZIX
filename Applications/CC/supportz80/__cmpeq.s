		.export __cmpeq
		.export __cmpeq0d
		.code

;
;	Tighter version with the other value in DE
;
__cmpeq0d:
		ld	d,0
__cmpeq:
		or	a
		sbc	hl,de
		jp	nz,__false
		;	was 0 so now 1 and NZ
		inc	l
		ret
