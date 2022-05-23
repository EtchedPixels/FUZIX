#include "../kernel-8085.def"

		.code
		.setcpu 8085
.export init

	.word 0x8085

init:
        di
	; Make sure our interrupt is unmasked but the others are not
	mvi a,0x1D		; R7.5 | MSE | M7.5 | M 5.5
	sim
	lxi sp,kstack_top

        call init_early

	; Common is packed in the BSS space

	lxi b,__common_size
	lxi h,__bss
	lxi d,__common

	; Copy it high
nextbyte:
	mov a,m
	stax d
	inx h
	inx d
	dcx b
	mov a,b
	ora c
	jnz nextbyte

	; The discard follows the common

	lxi b, __discard_size
	lxi d, __discard

;
;	We copy discard from the top because it will probably overlap
;	on an 8080/8085 type system due to the larger code sizes.
;
nextbyted:
	mov a,m
	stax d
	dcx h
	dcx d
	dcx b
	mov a,b
	ora c
	jnz nextbyted

	lxi b,__bss_size
	lxi h,__bss
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

