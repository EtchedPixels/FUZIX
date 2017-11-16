	.code

	.export _brk

_brk:
	ld hl, 30
	jp __syscall
