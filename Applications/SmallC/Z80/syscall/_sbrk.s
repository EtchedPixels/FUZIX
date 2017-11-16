	.code

	.export _sbrk

_sbrk:
	ld hl, 31
	jp __syscall
