		.export __bcdiv
		.export __bcrem

		.setcpu 8080
		.code

		; We compute HL/DE but righh now we have BC / HL
__bcdiv:
		xchg
		mov e,c
		mov d,b
		call __divde
		mov c,l
		mov h,b
		ret

__bcrem:
		xchg
		mov e,c
		mov d,b
		call __divde
		xchg
		mov c,l
		mov h,b
		ret
