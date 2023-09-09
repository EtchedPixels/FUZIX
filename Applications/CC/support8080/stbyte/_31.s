
	.export __stbyte31

	.setcpu 8080
	.code
__stbyte31:
	mov a,l
	lxi h,31
dad sp
	mov m,a
	mov l,a
	ret
