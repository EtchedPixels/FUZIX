
	.export __stbyte24

	.setcpu 8080
	.code
__stbyte24:
	mov a,l
	lxi h,24
dad sp
	mov m,a
	mov l,a
	ret
