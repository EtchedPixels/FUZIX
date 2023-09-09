
	.export __stbyte1

	.setcpu 8080
	.code
__stbyte1:
	mov a,l
	lxi h,1
dad sp
	mov m,a
	mov l,a
	ret
