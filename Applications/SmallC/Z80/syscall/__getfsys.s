	.code

	.export __getfsys

__getfsys:
	ld hl, 22
	jp __syscall
