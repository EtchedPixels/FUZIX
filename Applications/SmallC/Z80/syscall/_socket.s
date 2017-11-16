	.code

	.export _socket

_socket:
	ld hl, 90
	jp __syscall
