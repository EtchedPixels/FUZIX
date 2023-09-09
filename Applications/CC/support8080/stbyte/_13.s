
	.export __stbyte13

	.setcpu 8080
	.code
__stbyte13:
	mov a,l
	lxi h,13
dad sp
	mov m,a
	mov l,a
	ret
