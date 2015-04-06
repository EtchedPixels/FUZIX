	.module usermem

;
;	6809 copy to and from userspace
;
;	This is linked by the 6881 using platforms only. MMU based
;	machines simply don't need it.


	include "kernel.def"
        include "../kernel09.def"

	; exported
	.globl __ugetc
	.globl __ugetw
	.globl __uget
	.globl __ugets

	.globl __uputc
	.globl __uputw
	.globl __uput
	.globl __uzero

	; imported
	.globl map_process_always
	.globl map_kernel
	.area .common

__ugetc:
	pshs cc		; save IRQ state
	orcc #0x10
	jsr map_process_always
	ldb ,x
	jsr map_kernel
	clra
	tfr d,x
	puls cc,pc	; back and return

__ugetw:
	pshs cc
	orcc #0x10
	jsr map_process_always
	ldx ,x
	jsr map_kernel
	puls cc,pc

__uget:
	pshs u,y,cc
	ldu 7,s		; user address
	ldy 9,s		; count
	orcc #0x10
ugetl:
	lda ,x+
	jsr map_process_always
	sta ,u+
	jsr map_kernel
	leay -1,y
	bne ugetl
	puls u,y,cc,pc

__ugets:
	pshs u,y,cc
	ldu 7,s		; user address
	ldy 9,s		; count
	orcc #0x10
ugetsl:
	jsr map_process_always
	lda ,x+
	beq ugetse
	jsr map_kernel
	sta ,u+
	leay -1,y
	bne ugetsl
	ldx #0xffff	; unterminated - error
	lda #0
	sta -1,u	; force termination
	puls u,y,cc,pc
ugetse:
	jsr map_kernel
	sta ,u
	ldx #0
	puls u,y,cc,pc


__uputc:
	pshs cc
	orcc #0x10
	ldd 3,s
	jsr map_process_always
	exg d,x
	stb ,x
	jsr map_kernel
	puls cc,pc

__uputw:
	pshs cc
	orcc #0x10
	ldd 3,s
	jsr map_process_always
	exg d,x
	std ,x
	jsr map_kernel
	puls cc,pc

;	X = source, user, size on stack
__uput:
	pshs u,y,cc
	orcc #0x10
	ldu 7,s		; user address
	ldy 9,s		; count
uputl:
	lda ,x+
	jsr map_process_always
	sta ,u+
	jsr map_kernel
	leay -1,y
	bne uputl
	puls u,y,cc,pc

__uzero:
	pshs y,cc
	lda #0
	ldy 5,s
	orcc #0x10
	jsr map_process_always
uzloop:
	sta ,x+
	leay -1,y
	bne uzloop
	jsr map_kernel
	puls y,cc,pc
