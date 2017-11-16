	.code

	.export __getsockaddrs

__getsockaddrs:
	ld hl, 95
	jp __syscall
