		.export __borde
		.code
		.setcpu 8080

__borde:
	mov a,d
	ora h
	mov h,a
	mov a,e
	ora l
	mov l,a
	ret
