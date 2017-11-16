	.code

	.export _mount

_mount:
	ld hl, 33
	jp __syscall
