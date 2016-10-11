	.text

	.globl _nosys80

_nosys80:
.mri 1
	move.w #80,d0
	trap #14
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts
