	.code

	.export _geteuid

_geteuid:
	ld hl, 44
	jp __syscall
