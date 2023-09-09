		.export __bcora
		.code

__bcora:
		; BC &= HL
		ld a,h
		or b
		ld b,a
		ld a,l
		or c
		ld c,a
		ret
