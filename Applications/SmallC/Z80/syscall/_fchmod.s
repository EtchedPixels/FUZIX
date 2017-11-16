	.code

	.export _fchmod

_fchmod:
	ld hl, 49
	jp __syscall
