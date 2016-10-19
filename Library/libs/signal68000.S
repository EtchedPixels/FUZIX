	.section signal,"x"

	.globl _sigunwind

.mri 1
;
;	Must be linked first (base + 0x40)
;
_sigunwind:
	movem.l (sp)+,a0/a1/d0/d1
	move.w (sp)+,ccr
	rts
