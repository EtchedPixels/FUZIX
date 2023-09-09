		.export __bandde
		.export __bbandde
		.code
		.setcpu 8080

__bandde:
	mov a,d
	ana h
	mov h,a
	mov a,e
	ana l
	mov l,a
	ret
;
;	Little helper for the if (a & b) ... case
;
__bbandde:
	call __bandde
	jmp __bool
