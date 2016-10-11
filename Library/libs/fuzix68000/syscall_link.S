	.text

	.globl link

link:
.mri 1
	move.w #5,d0
	trap #14
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts
