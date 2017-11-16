	.code

	.export __select

__select:
	ld hl, 72
	jp __syscall
