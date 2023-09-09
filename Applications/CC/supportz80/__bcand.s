		.export __bcana
		.code

__bcana:
		; BC &= HL
		ld a,h
		and b
		ld b,a
		ld a,l
		and c
		ld c,a
		ret
