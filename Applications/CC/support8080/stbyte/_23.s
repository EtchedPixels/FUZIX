
	.export __stbyte23

	.setcpu 8080
	.code
__stbyte23:
	mov a,l
	lxi h,23
dad sp
	mov m,a
	mov l,a
	ret
