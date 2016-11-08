	.globl __syscall
	.globl __syscall_mangled
	.globl _errno

	.area .text

__syscall:
	swi
	cmpd #0			; D holds errno, if any
	beq noerr1
	std _errno		; X is -1 in this case
noerr1:
	rts

; for variadic functions:
; compensate for the 1st argument that we removed
__syscall_mangled:
	swi
	cmpd #0
	beq noerr2
	std _errno
noerr2:
	puls d		; get return address
	pshs d,x	; inject a word on stack, e.g. X
	rts
