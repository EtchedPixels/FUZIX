	.text

	.globl _recvfrom

_recvfrom:
.mri 1
	move.l a2,-(sp)
	move.l 20(sp),a2
	move.l 16(sp),a1
	move.l 12(sp),a0
	move.l 8(sp),d1
	move.w #97,d0
	trap #14
	move.l (sp)+,a2
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts
