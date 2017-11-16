	.code

	.export _getpgrp

_getpgrp:
	ld hl, 61
	jp __syscall
