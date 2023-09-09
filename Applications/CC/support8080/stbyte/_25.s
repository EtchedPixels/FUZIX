
	.export __stbyte25

	.setcpu 8080
	.code
__stbyte25:
	mov a,l
	lxi h,25
dad sp
	mov m,a
	mov l,a
	ret
