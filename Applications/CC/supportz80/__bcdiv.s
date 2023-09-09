		.export __bcdiv
		.export __bcrem

		.code

		; We compute HL/DE but righh now we have BC / HL
__bcdiv:
		ex de,hl
		ld e,c
		ld d,b
		call __divde
		ld c,l
		ld h,b
		ret

__bcrem:
		ex de,hl
		ld e,c
		ld d,b
		call __divde
		ex de,hl
		ld c,l
		ld h,b
		ret
