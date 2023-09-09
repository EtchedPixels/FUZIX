
	.export __stbyte11

	.setcpu 8080
	.code
__stbyte11:
	mov a,l
	lxi h,11
dad sp
	mov m,a
	mov l,a
	ret
