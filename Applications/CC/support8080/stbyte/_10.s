
	.export __stbyte10

	.setcpu 8080
	.code
__stbyte10:
	mov a,l
	lxi h,10
dad sp
	mov m,a
	mov l,a
	ret
