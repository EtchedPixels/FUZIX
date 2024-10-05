		.export __bxorde
		.code
		.setcpu 8080

__bxorde:
	mov a,d
	xra h
	mov h,a
	mov a,e
	xra l
	mov l,a
	ret
