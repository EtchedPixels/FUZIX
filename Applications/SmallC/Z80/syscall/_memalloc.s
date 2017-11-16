	.code

	.export _memalloc

_memalloc:
	ld hl, 64
	jp __syscall
