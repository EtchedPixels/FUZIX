	.module usermem

;
;	6809 + SAM copy to and from userspace
;
;	This is linked by the 6881 using platforms only. MMU based
;	machines simply don't need it.


	include "kernel.def"
        include "../kernel09.def"

	.globl __ugetc
	.globl __ugetw
	.globl __uget
	.globl __ugets

	.globl __uputc
	.globl __uputw
	.globl __uput
	.globl __uzero

	.area .common

__ugetc:
	SAM_USER
	ldb ,x
	lda #0
	tfr d,x
	SAM_KERNEL
	rts

__ugetw:
	SAM_USER
	ldx ,x
	SAM_KERNEL
	rts

__uget:
	pshs u,y
	ldu 6,s		; user address
	ldy 8,s		; count
ugetl:
	lda ,x++
	SAM_USER
	sta ,u++
	SAM_KERNEL
	leay -1,y
	cmpy #0
	bne ugetl
	puls u,y,pc

__ugets:
	pshs u,y
	ldu 6,s		; user address
	ldy 8,s		; count
ugetsl:
	SAM_USER
	lda ,x++
	beq ugetse
	SAM_KERNEL
	sta ,u++
	leay -1,y
	cmpy #0
	bne ugetsl
	ldx #0xffff	; unterminated - error
	lda #0
	sta -1,u	; force termination
	puls u,y,pc

ugetse:
	SAM_KERNEL
	sta ,u
	ldx #0
	puls u,y,pc


__uputc:
	SAM_USER
	stb ,x
	SAM_KERNEL
	rts

__uputw:
	SAM_USER
	ldb 2,s
	stb ,x
	SAM_KERNEL
	rts

__uput:
	pshs u,y
	ldu 6,s		; user address
	ldy 8,s		; count
uputl:
	SAM_USER
	lda ,x++
	SAM_KERNEL
	sta ,u++
	leay -1,y
	cmpy #0
	bne uputl
	puls u,y,pc

__uzero:
	pshs y
	lda #0
	ldy 6,s
	SAM_USER
uzloop:
	sta ,x+
	leay -1,y
	cmpy #0
	bne uzloop
	SAM_KERNEL
	puls y,pc
