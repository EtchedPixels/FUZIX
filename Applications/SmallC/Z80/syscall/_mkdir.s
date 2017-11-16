	.code

	.export _mkdir

_mkdir:
	ld hl, 51
	jp __syscall
