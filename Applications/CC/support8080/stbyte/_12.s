
	.export __stbyte12

	.setcpu 8080
	.code
__stbyte12:
	mov a,l
	lxi h,12
dad sp
	mov m,a
	mov l,a
	ret
