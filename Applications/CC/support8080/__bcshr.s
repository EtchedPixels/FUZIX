		.export __bcshr
		.export __bcshru
		.setcpu 8080
		.code

__bcshr:
		; shift BC right by E
		mov	l,c
		mov	h,b
		call	__shrde
		mov	c,l
		mov	b,h
		ret

__bcshru:
		; shift BC right by E
		mov	l,c
		mov	h,b
		call	__shrdeu
		mov	c,l
		mov	b,h
		ret
