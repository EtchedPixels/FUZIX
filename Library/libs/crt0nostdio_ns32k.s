	.globl _start
	.globl ___argv
	.globl _environ

	.data

_environ:
	.long 0

	.text

_start:
	enter [],0
	addr 8(fp),_environ(pc)
	movw 4(fp),___argv(pc)
	jsr _main
	movw r0,tos
	exit []
	jsr _exit
