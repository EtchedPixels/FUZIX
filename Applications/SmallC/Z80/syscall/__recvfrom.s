	.code

	.export __recvfrom

__recvfrom:
	ld hl, 97
	jp __syscall
