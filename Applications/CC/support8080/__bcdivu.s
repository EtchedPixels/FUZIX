		.export __bcdivu
		.export __bcmodu

		; We compute HL/DE but righh now we have BC / HL
__bcdivu:
		xchg
		mov e,c
		mov d,b
		call __divdeu
		mov c,l
		mov h,b
		ret

__bcmodu:
		xchg
		mov e,c
		mov d,b
		call __divdeu
		xchg
		mov c,l
		mov h,b
		ret
