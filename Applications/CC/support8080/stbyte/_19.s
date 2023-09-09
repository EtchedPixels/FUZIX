
	.export __stbyte19

	.setcpu 8080
	.code
__stbyte19:
	mov a,l
	lxi h,19
dad sp
	mov m,a
	mov l,a
	ret
