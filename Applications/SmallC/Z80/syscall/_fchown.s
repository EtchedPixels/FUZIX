	.code

	.export _fchown

_fchown:
	ld hl, 50
	jp __syscall
