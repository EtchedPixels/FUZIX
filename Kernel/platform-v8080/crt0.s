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

	lxi h,datastart
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

	lxi h,bssstart
	lxi d,datastart
	call calcsize

	lxi h,datastart
	xra a
wipe:
	mov m,a
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
