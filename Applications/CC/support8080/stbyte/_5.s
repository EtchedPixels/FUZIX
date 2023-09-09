
	.export __stbyte5

	.setcpu 8080
	.code
__stbyte5:
	mov a,l
	lxi h,5
dad sp
	mov m,a
	mov l,a
	ret
