	.code

	.export __shutdown

__shutdown:
	ld hl, 98
	jp __syscall
