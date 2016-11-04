	.text

	.globl _accept

_accept:
.mri 1
	move.l 4(sp),d1
	move.w #94,d0
	trap #14
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts
