	.module usermem

;
;	6809 + SAM copy to and from userspace
;
;	This is linked by the 6881 using platforms only. MMU based
;	machines simply don't need it.


	include "kernel.def"
        include "../kernel09.def"

	; exported
	.globl __ugetc
	.globl __ugetw
	.globl __uget
	.globl __uputc
	.globl __uputw
	.globl __uput
	.globl __uzero

	.area .common

__ugetc:
	pshs cc		; save IRQ state
	orcc #0x10
	SAM_USER
	ldb ,x
	SAM_KERNEL
	clra
	tfr d,x
	puls cc,pc	; back and return

__ugetw:
	pshs cc
	orcc #0x10
	SAM_USER
	ldx ,x
	SAM_KERNEL
	puls cc,pc

__uget:
	pshs u,y,cc
	ldu 7,s		; user address
	ldy 9,s		; count
	orcc #0x10
ugetl:
	SAM_USER
	lda ,x+
	SAM_KERNEL
	sta ,u+
	leay -1,y
	bne ugetl
	ldx #0
	puls u,y,cc,pc

__uputc:
	pshs cc
	orcc #0x10
	ldd 3,s
	SAM_USER
	exg d,x
	stb ,x
	SAM_KERNEL
	ldx #0
	puls cc,pc

__uputw:
	pshs cc
	orcc #0x10
	ldd 3,s
	SAM_USER
	exg d,x
	std ,x
	SAM_KERNEL
	ldx #0
	puls cc,pc

;	X = source, user, size on stack
__uput:
	pshs u,y,cc
	orcc #0x10
	ldu 7,s		; user address
	ldy 9,s		; count
uputl:
	lda ,x+
	SAM_USER
	sta ,u+
	SAM_KERNEL
	leay -1,y
	bne uputl
	ldx #0
	puls u,y,cc,pc

__uzero:
	pshs y,cc
	lda #0
	ldy 5,s
	orcc #0x10
	SAM_USER
uzloop:
	sta ,x+
	leay -1,y
	bne uzloop
	SAM_KERNEL
	ldx #0
	puls y,cc,pc
