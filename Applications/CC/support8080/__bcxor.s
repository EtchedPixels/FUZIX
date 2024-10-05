		.export __bcxra
		.setcpu 8080
		.code

__bcxra:
		; BC &= HL
		mov a,h
		xra b
		mov b,a
		mov a,l
		xra c
		mov c,a
		ret
