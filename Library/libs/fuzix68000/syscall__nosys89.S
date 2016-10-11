	.text

	.globl _nosys89

_nosys89:
.mri 1
	move.w #89,d0
	trap #14
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts
