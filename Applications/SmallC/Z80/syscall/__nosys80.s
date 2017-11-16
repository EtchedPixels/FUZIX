	.code

	.export __nosys80

__nosys80:
	ld hl, 80
	jp __syscall
