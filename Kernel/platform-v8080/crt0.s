#include "../kernel-8080.def"

.sect .text
.sect .rom
.sect .data
datastart:
.sect .bss
bssstart:
.sect .common
commonstart:

.sect .text

.define init

init:
        di
	lxi sp,kstack_top

        call init_early


	lxi h,commonend
	lxi d,commonstart
	call calcsize

	lxi h,bssstart
	lxi d,commonstart
	
nextbyte:
	mov a,m
	stax d
	inx h
	inx d
	dcx b
	mov a,b
	ora c
	jnz nextbyte

!	lxi h,bssend		! We should really do this but bssend
				! isnt appearing at the end so plan b
	lxi h,commonstart	! Wipe all the free space
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
