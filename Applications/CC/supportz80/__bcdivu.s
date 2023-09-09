		.export __bcdivu
		.export __bcmodu

		; We compute HL/DE but righh now we have BC / HL
__bcdivu:
		ex de,hl
		ld e,c
		ld d,b
		call __divdeu
		ld c,l
		ld h,b
		ret

__bcmodu:
		ex de,hl
		ld e,c
		ld d,b
		call __divdeu
		ex de,hl
		ld c,l
		ld h,b
		ret
