	.code

	.export _dup

_dup:
	ld hl, 17
	jp __syscall
