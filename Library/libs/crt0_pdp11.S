		.text
		.even

		.globl ___stdio_init_vars
		.globl _main
		.globl _exit
		.globl _environ
		.globl ___argv
		.globl __start


__start:	br	__start2
		.byte	'F'
		.byte	'Z'
		.byte	'X'
		.byte	'1'
		/* FIXME make the linker do these */
		.byte	0		/* Patched to load addr page */
		.word	0
		.word	_etext		/* Patched to code size */
		.word	_edata		/* Patched to data size */
		.word	_ebss		/* Patched to BSS size */
__start2:
		jsr	pc,___stdio_init_vars
		mov	sp,r0
		mov	r0,___argv
		inc	r0
		inc	r0
		mov	r0,_environ
		jsr	pc,_main
		mov	r0,-(sp)
		jsr	pc,_exit

		.data
_environ:	.word	0
