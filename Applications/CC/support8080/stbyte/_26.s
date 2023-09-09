
	.export __stbyte26

	.setcpu 8080
	.code
__stbyte26:
	mov a,l
	lxi h,26
dad sp
	mov m,a
	mov l,a
	ret
