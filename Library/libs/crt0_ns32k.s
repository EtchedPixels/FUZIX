	.globl __start
	.globl ___argv
	.globl _environ

	.text

__start:
	jsr ___stdio_init_vars
	sprd sp,r0
	addr 8(r0),_environ(pc)
	movd 4(r0),___argv(pc)
	jsr _main
	movd r0,tos
	jsr _exit

	.data
_environ:
	.long 0
