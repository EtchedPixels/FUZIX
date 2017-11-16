	.code

	.export _alarm

_alarm:
	ld hl, 38
	jp __syscall
