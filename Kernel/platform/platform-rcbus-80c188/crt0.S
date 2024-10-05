	.arch i8086,jumps
	.code16
	.att_syntax prefix
	.text

	.globl	__end
	.globl	__bss_start

	.globl	kernel_ds

_start:
/*FIXME	movw	$udata+512,%sp	*//* Set kernel stack */
	movw	%ds,%cs:kernel_ds
	movw	$__bss_start,%si
	movw	$__end,%cx
	subw	%si,%cx
	xorw	%ax,%ax
	rep	movsb

	call	init_early
	call	init_hardware
	call	fuzix_main
stop:
	jmp	stop

kernel_ds:
	.word	0
