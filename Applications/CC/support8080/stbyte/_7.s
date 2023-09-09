
	.export __stbyte7

	.setcpu 8080
	.code
__stbyte7:
	mov a,l
	lxi h,7
dad sp
	mov m,a
	mov l,a
	ret
