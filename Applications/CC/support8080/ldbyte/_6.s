
	.export __ldbyte6

	.setcpu 8080
	.code
__ldbyte6:
	lxi h,6
dad sp
	mov l,m
	ret
