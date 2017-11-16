	.code

	.export _access

_access:
	ld hl, 12
	jp __syscall
