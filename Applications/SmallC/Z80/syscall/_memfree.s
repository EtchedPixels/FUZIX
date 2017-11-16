	.code

	.export _memfree

_memfree:
	ld hl, 65
	jp __syscall
