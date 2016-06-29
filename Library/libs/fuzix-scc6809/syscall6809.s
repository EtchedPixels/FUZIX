	.globl __syscall
	.globl _errno

	.text

__syscall:
	swi
	cmpd #0			; D holds errno, if any
	beq @noerr
	std _errno		; X is -1 in this case
@noerr:
	rts
