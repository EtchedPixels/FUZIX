		.export __bcora
		.setcpu 8080
		.code

__bcora:
		; BC &= HL
		mov a,h
		ora b
		mov b,a
		mov a,l
		ora c
		mov c,a
		ret
