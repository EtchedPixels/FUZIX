	.code

	.export __stime

__stime:
	ld hl, 28
	jp __syscall
