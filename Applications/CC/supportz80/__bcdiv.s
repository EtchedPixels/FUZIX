		.export __bcdiv
		.export __bcrem

		.code

		; We compute HL/DE but right now we have BC / HL
__bcdiv:
		ex de,hl
		ld l,c
		ld h,b
		call __divde
		ld c,l
		ld b,h
		ret

__bcrem:
		ex de,hl
		ld l,c
		ld h,b
		call __remde
		ld c,l
		ld b,h
		ret
