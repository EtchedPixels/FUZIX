	.module usermem

;
;	6809 copy to and from userspace
;
;	TODO: optimise - our layout should be such that all kernel
;	copy to/from user areas are mapped when user is mapped
;

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

	; imported
	.globl map_proc_always
	.globl map_kernel

	.area .common

__ugetc:
	pshs cc		; save IRQ state
	orcc #0x10
	jsr map_proc_always
	ldb ,x
	jsr map_kernel
	clra
	tfr d,x
	puls cc,pc	; back and return

__ugetw:
	pshs cc
	orcc #0x10
	jsr map_proc_always
	ldx ,x
	jsr map_kernel
	puls cc,pc

__uget:
	pshs u,y,cc
	ldu 7,s		; user address
	ldy 9,s		; count
	orcc #0x10
ugetl:
	jsr map_proc_always
	lda ,x+
	jsr map_kernel
	sta ,u+
	leay -1,y
	bne ugetl
	ldx #0
	puls u,y,cc,pc

__uputc:
	pshs cc
	orcc #0x10
	ldd 3,s
	jsr map_proc_always
	exg d,x
	stb ,x
	jsr map_kernel
	ldx #0
	puls cc,pc

__uputw:
	pshs cc
	orcc #0x10
	ldd 3,s
	jsr map_proc_always
	exg d,x
	std ,x
	jsr map_kernel
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
	jsr map_proc_always
	sta ,u+
	jsr map_kernel
	leay -1,y
	bne uputl
	ldx #0
	puls u,y,cc,pc

__uzero:
	pshs y,cc
	ldy 5,s
	orcc #0x10
	jsr map_proc_always
	tfr y,d
	clra
	lsrb		; odd count?
	bcc evenc
	sta ,x+
	leay -1,y
	beq zdone
evenc:
	clrb
uzloop:
	std ,x++
	leay -2,y
	bne uzloop
zdone:
	jsr map_kernel
	ldx #0
	puls y,cc,pc
