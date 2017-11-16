	.code

	.export _setpgrp

_setpgrp:
	ld hl, 53
	jp __syscall
