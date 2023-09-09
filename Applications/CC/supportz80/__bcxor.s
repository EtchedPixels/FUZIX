		.export __bcxra
		.code

__bcxra:
		; BC &= HL
		ld a,h
		xor b
		ld b,a
		ld a,l
		xor c
		ld c,a
		ret
