	.code

	.export __sigdisp

__sigdisp:
	ld hl, 59
	jp __syscall
