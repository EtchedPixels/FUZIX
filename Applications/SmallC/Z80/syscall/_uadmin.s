	.code

	.export _uadmin

_uadmin:
	ld hl, 57
	jp __syscall
