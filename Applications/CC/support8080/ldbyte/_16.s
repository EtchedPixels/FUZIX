
	.export __ldbyte16

	.setcpu 8080
	.code
__ldbyte16:
	lxi h,16
dad sp
	mov l,m
	ret
