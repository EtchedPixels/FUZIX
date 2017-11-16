	.code

	.export __fstat

__fstat:
	ld hl, 16
	jp __syscall
