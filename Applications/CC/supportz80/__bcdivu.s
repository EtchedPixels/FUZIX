		.export __bcdivu
		.export __bcremu

		; We compute HL/DE but righh now we have BC / HL
__bcdivu:
		ex de,hl
		ld l,c
		ld h,b
		call __divdeu
		ld c,l
		ld h,b
		ret

__bcremu:
		ex de,hl
		ld l,c
		ld h,b
		call __remdeu
		ld c,l
		ld h,b
		ret
