#include "../kernel-8080.def"

.sect .text
.sect .rom
.sect .data
datastart:
.sect .bss
bssstart:
.sect .common
commonstart:
.sect .discard
discardstart:

.sect .text

.define init

	.data2 0x8085

init:
        di
	! Make sure our interrupt is unmasked but the others are not
	mvi a,0x1D		! R7.5 | MSE | M7.5 | M 5.5
	sim
	lxi sp,kstack_top

        call init_early

	! Common is packed in the BSS space

	lxi h,commonend
	lxi d,commonstart
	call calcsize

	lxi h,bssstart
	lxi d,commonstart

	! Copy it high
nextbyte:
	mov a,m
	stax d
	inx h
	inx d
	dcx b
	mov a,b
	ora c
	jnz nextbyte

	! The discard follows the common

	push h

	lxi h,discardend
	lxi d,discardstart
	call calcsize

	pop h
	dad b
	dcx h
	xchg
	dad b
	dcx h
	xchg

!
!	We copy discard from the top because it will probably overlap
!	on an 8080/8085 type system due to the larger code sizes.
!
nextbyted:
	mov a,m
	stax d
	dcx h
	dcx d
	dcx b
	mov a,b
	ora c
	jnz nextbyted

!	lxi h,bssend		! We should really do this but bssend
				! isnt appearing at the end so plan b
	lxi h,discardstart	! Wipe all the free space
	lxi d,bssstart
	call calcsize

	lxi h,bssstart
wipe:
	mvi m,0
	inx h
	dcx b
	mov a,b
	ora c
	jnz wipe

        call init_hardware

        call _fuzix_main
        di
stop:   hlt
        jmp stop

calcsize:
	mov a,l
	sub e
	mov c,a
	mov a,h
	sbb d
	mov b,a
	ret
