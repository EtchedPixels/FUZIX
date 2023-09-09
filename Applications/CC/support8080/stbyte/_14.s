
	.export __stbyte14

	.setcpu 8080
	.code
__stbyte14:
	mov a,l
	lxi h,14
dad sp
	mov m,a
	mov l,a
	ret
