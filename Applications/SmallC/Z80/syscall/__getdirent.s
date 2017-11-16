	.code

	.export __getdirent

__getdirent:
	ld hl, 24
	jp __syscall
