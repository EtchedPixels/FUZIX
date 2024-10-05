		.export __bcana
		.setcpu 8080
		.code

__bcana:
		; BC &= HL
		mov a,h
		ana b
		mov b,a
		mov a,l
		ana c
		mov c,a
		ret
